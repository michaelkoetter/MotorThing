import * as actions from './actions/tmcl'
import fetch from 'isomorphic-fetch'
import store from './store'

// Common TMCL Instructions
export const TMCL_ROTATE_RIGHT = 1 // eslint-disable-line no-unused-vars
export const TMCL_ROTATE_LEFT = 2 // eslint-disable-line no-unused-vars
export const TMCL_MOTOR_STOP = 3 // eslint-disable-line no-unused-vars
export const TMCL_MOVE_TO_POSITION = 4 // eslint-disable-line no-unused-vars
export const TMCL_SET_AXIS_PARAMETER = 5 // eslint-disable-line no-unused-vars
export const TMCL_GET_AXIS_PARAMETER = 6 // eslint-disable-line no-unused-vars
export const TMCL_STORE_AXIS_PARAMETER = 7 // eslint-disable-line no-unused-vars
export const TMCL_RESTORE_AXIS_PARAMETER = 8 // eslint-disable-line no-unused-vars
export const TMCL_SET_GLOBAL_PARAMETER = 9 // eslint-disable-line no-unused-vars
export const TMCL_GET_GLOBAL_PARAMETER = 10 // eslint-disable-line no-unused-vars
export const TMCL_STORE_GLOBAL_PARAMETER = 11 // eslint-disable-line no-unused-vars
export const TMCL_RESTORE_GLOBAL_PARAMETER = 12 // eslint-disable-line no-unused-vars
export const TMCL_SET_COORDINATE = 30 // eslint-disable-line no-unused-vars
export const TMCL_GET_COORDINATE = 31 // eslint-disable-line no-unused-vars
export const TMCL_CAPTURE_COORDINATE = 32 // eslint-disable-line no-unused-vars

// TMCL Control Functions
export const TMCL_START_APPLICATION = 129 // eslint-disable-line no-unused-vars
export const TMCL_RESET_APPLICATION = 131 // eslint-disable-line no-unused-vars
export const TMCL_GET_APPLICATION_STATUS = 135 // eslint-disable-line no-unused-vars
export const TMCL_GET_FIRMWARE_VERSION = 136 // eslint-disable-line no-unused-vars

// Axis Parameters
export const AP_TARGET_POSITION = 0 // eslint-disable-line no-unused-vars
export const AP_ACTUAL_POSITION = 1 // eslint-disable-line no-unused-vars

// Movement
export const MOVE_ABSOLUTE = 0 // eslint-disable-line no-unused-vars
export const MOVE_RELATIVE = 1 // eslint-disable-line no-unused-vars
export const MOVE_TO_COORDINATE = 2 // eslint-disable-line no-unused-vars

let options = {
  url: 'http://192.168.4.1/tmcl'
};

// Check the TMCL API response
function checkStatus(response) {
  if (response.status >= 200 && response.status < 300) {
    return response
    // TODO check TMCL error code
  } else {
    let error = new Error(response.statusText)
    error.response = response
    throw error
  }
}

function dispatchNextPendingInstruction() {
  return dispatch => {
    let pending = store.getState().tmcl.pending;
    if (pending.length > 0) {
      // There is at least one pending instruction, dispatch
      let instruction = pending[0];

      // Firmware version request is special
      let isStringVersionRequest =
        instruction.instruction === TMCL_GET_FIRMWARE_VERSION && instruction.type === 0;
      let url = isStringVersionRequest ?
        `${options.url}?address=${instruction.address || 1}` : options.url
      let requestOptions = {
        method: isStringVersionRequest ? 'GET' : 'POST',
        body: isStringVersionRequest ? undefined : JSON.stringify(instruction)
      }

      console.debug('> TMCL Instruction', instruction)
      return fetch(url, requestOptions)
        .then(checkStatus)
        .then(response => response.json())
        .then(json => {
          console.debug('< TMCL Reply', json)
          dispatch(actions.receiveReply(instruction, json))
          // continue dispatching
          dispatch(dispatchNextPendingInstruction())
        })
        .catch(error => {
          console.debug('< TMCL Error', error, error.response)

          // see if we can get a JSON error message
          error.response.json()
            .then(json => {
              dispatch(actions.receiveError(instruction, `TMCL Error: ${json.error}`))
              // continue dispatching
              dispatch(dispatchNextPendingInstruction())
            })
            .catch(() => {
              dispatch(actions.receiveError(instruction, error))
              // continue dispatching
              dispatch(dispatchNextPendingInstruction())
            })

        })
    } else {
      // there is nothing to dispatch
      console.debug('- TMCL No more pending instructions')
    }
  }

}

function sendInstruction(instruction) {
  return dispatch => {
    if (instruction) {
      // This will add the instruction to the 'pending' list
      dispatch(actions.sendInstruction(instruction))

      // ... then check if we need to start dispatching
      if (store.getState().tmcl.pending.length === 1) {
        // This is the first pending instruction, start dispatching
        dispatch(dispatchNextPendingInstruction())
      }
    }
  }
}


function saveModulesState() {
  // we store only the address
  let modules = store.getState().tmcl.modules.map(m => {
    return { address: m.address }
  });

  localStorage.tmclModules = JSON.stringify(modules)
}

export function loadModules() {
  let _modules = localStorage.tmclModules
  if (_modules) {
    let modules = JSON.parse(_modules)
    console.debug('load modules', modules)
    modules.forEach(m => {
      store.dispatch(actions.addModule(m.address))
      getModuleInfo(m.address)
    })
  }
}

export function addModule(address) {
  return dispatch => {
    dispatch(actions.addModule(address))
    getModuleInfo(address)
    saveModulesState()
  }
}

export function removeModule(address) {
  return dispatch => {
    dispatch(actions.removeModule(address))
    saveModulesState()
  }
}

export function getModuleInfo(address) {
  sendInstruction({
    address,
    instruction: TMCL_GET_FIRMWARE_VERSION,
    type: 0
  })(store.dispatch)
}

export function rotate(address, motor, direction, speed) {
  sendInstruction({
    address,
    motor,
    instruction: direction == 'right' ? TMCL_ROTATE_RIGHT : TMCL_ROTATE_LEFT,
    value: speed
  })(store.dispatch)
}

export function stop(address, motor) {
  sendInstruction({
    address,
    motor,
    instruction: TMCL_MOTOR_STOP
  })(store.dispatch)
}

export function setUserVariable(address, index, value) {
  sendInstruction({
    address,
    instruction: TMCL_SET_GLOBAL_PARAMETER,
    type: index,
    motor: 2,
    value
  })(store.dispatch)
}

export function startApplication(address, startAddress = 0) {
  sendInstruction({
    address,
    instruction: TMCL_START_APPLICATION,
    type: 1,
    value: startAddress
  })(store.dispatch)
}

export function stopApplication(address) {
  sendInstruction({
    address,
    instruction: TMCL_RESET_APPLICATION
  })(store.dispatch)
}

export function getAxisParameter(address, motor, param) {
  sendInstruction({
    address,
    instruction: TMCL_GET_AXIS_PARAMETER,
    motor,
    type: param
  })(store.dispatch)
}

export function setAxisParameter(address, motor, param, value) {
  sendInstruction({
    address,
    instruction: TMCL_SET_AXIS_PARAMETER,
    motor,
    type: param,
    value: value
  })(store.dispatch)
}

export function setHome(address, motor) {
  setAxisParameter(address, motor, AP_ACTUAL_POSITION, 0)
}

export function moveToHome(address, motor) {
  sendInstruction({
    address,
    motor,
    instruction: TMCL_MOVE_TO_POSITION,
    type: MOVE_ABSOLUTE,
    value: 0
  })(store.dispatch)
}

export function getPosition(address, motor) {
  getAxisParameter(address, motor, AP_ACTUAL_POSITION)
}

export function captureCoordinate(address, motor, coordinate) {
  sendInstruction({
    address,
    motor,
    instruction: TMCL_CAPTURE_COORDINATE,
    type: coordinate
  })(store.dispatch)
}

export function moveToCoordinate(address, motor, coordinate) {
  sendInstruction({
    address,
    motor,
    instruction: TMCL_MOVE_TO_POSITION,
    type: MOVE_TO_COORDINATE,
    value: coordinate
  })(store.dispatch)
}
