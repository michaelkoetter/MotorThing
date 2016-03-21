import phonon from 'phonon';

import { requestTMCLVersion, receiveTMCLVersionReply, receiveTMCLVersionError,
  sendTMCLInstruction, receiveTMCLReply, receiveTMCLError } from './actions'
import fetch from 'isomorphic-fetch'
import store from './store'

// TMCL Control Functions
const TMCL_STOP_APPLICATION = 128
const TMCL_START_APPLICATION = 129
const TMCL_RESET_APPLICATION = 131
const TMCL_GET_APPLICATION_STATUS = 135
const TMCL_GET_FIRMWARE_VERSION = 136

let options = {
  url: 'tmcl'
};

let indicator = undefined
let indicatorVisible = false

store.on('change', () => {
  if (store.getState().tmcl.completed) {
    if (indicatorVisible) {
      indicatorVisible = false
      indicator.close()
      indicator = undefined
    }
  } else {
    if (!indicatorVisible) {
      indicatorVisible = true
      indicator = phonon.indicator('Wait...', false)
    }
  }
})

function checkStatus(response) {
  if (response.status >= 200 && response.status < 300) {
    return response
  } else {
    let error = new Error(response.statusText)
    error.response = response
    throw error
  }
}

export function getModuleInfo() {
  return dispatch => {
    dispatch(requestTMCLVersion())
    return fetch(options.url)
      .then(checkStatus)
      .then(response => response.json())
      .then(json => dispatch(receiveTMCLVersionReply(json.version)))
      .catch(error => dispatch(receiveTMCLVersionError(error)))
  }
}

export function sendInstructions(instructions) {
  let instruction = instructions.shift()
  console.log('> TMCL instruction', instruction)
  return dispatch => {
    dispatch(sendTMCLInstruction(instruction))
    return fetch(options.url, { method: 'POST', body: JSON.stringify(instruction) })
      .then(checkStatus)
      .then(response => response.json())
      .then(json => {
        console.log('< TMCL Reply', json)
        dispatch(receiveTMCLReply(instruction, json, instructions.length))
        if (instructions.length > 0)
          dispatch(sendInstructions(instructions))
      })
      .catch(error => dispatch(receiveTMCLError(instruction, error)))
  }
}

export function rotate(direction, speed) {
  return {
    instruction: direction == 'right' ? 1 : 2,
    value: speed
  }
}

export function stop() {
  return {instruction: 3}
}

export function setUserVariable(index, value) {
  return {instruction: 9, type: index, motor: 2, value: value}
}

export function startApplication(address = 0) {
  return {instruction: TMCL_START_APPLICATION, type: 1, value: address};
}

export function stopApplication() {
  return {instruction: TMCL_STOP_APPLICATION};
}

export function startTimelapse(startPosition = 0, endPosition = 100000,
  shots, interval, stabilize, exposure) {
  console.log(`Start timelapse ${startPosition}-${endPosition}, shots: ${shots}
    interval: ${interval} stabilize: ${stabilize} exposure: ${exposure}`);

  return [
    setUserVariable(0, startPosition),
    setUserVariable(1, endPosition),
    setUserVariable(2, shots),
    setUserVariable(3, interval),
    setUserVariable(4, stabilize),
    setUserVariable(5, exposure),
    startApplication()
  ]
}
