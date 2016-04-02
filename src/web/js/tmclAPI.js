import * as actions from './actions/tmcl'
import fetch from 'isomorphic-fetch'
import store from './store'

// Common TMCL Instructions
const TMCL_ROTATE_RIGHT = 1 // eslint-disable-line no-unused-vars
const TMCL_ROTATE_LEFT = 2 // eslint-disable-line no-unused-vars
const TMCL_MOTOR_STOP = 3 // eslint-disable-line no-unused-vars
const TMCL_MOVE_TO_POSITION = 4 // eslint-disable-line no-unused-vars
const TMCL_SET_AXIS_PARAMETER = 5 // eslint-disable-line no-unused-vars
const TMCL_GET_AXIS_PARAMETER = 6 // eslint-disable-line no-unused-vars
const TMCL_STORE_AXIS_PARAMETER = 7 // eslint-disable-line no-unused-vars
const TMCL_RESTORE_AXIS_PARAMETER = 8 // eslint-disable-line no-unused-vars
const TMCL_SET_GLOBAL_PARAMETER = 9 // eslint-disable-line no-unused-vars
const TMCL_GET_GLOBAL_PARAMETER = 10 // eslint-disable-line no-unused-vars
const TMCL_STORE_GLOBAL_PARAMETER = 11 // eslint-disable-line no-unused-vars
const TMCL_RESTORE_GLOBAL_PARAMETER = 12 // eslint-disable-line no-unused-vars

// TMCL Control Functions
const TMCL_START_APPLICATION = 129 // eslint-disable-line no-unused-vars
const TMCL_RESET_APPLICATION = 131 // eslint-disable-line no-unused-vars
const TMCL_GET_APPLICATION_STATUS = 135 // eslint-disable-line no-unused-vars
const TMCL_GET_FIRMWARE_VERSION = 136 // eslint-disable-line no-unused-vars

// Axis Parameters
const AP_TARGET_POSITION = 0 // eslint-disable-line no-unused-vars
const AP_ACTUAL_POSITION = 1 // eslint-disable-line no-unused-vars

let options = {
  url: 'tmcl'
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
      let requestOptions = {
        method: isStringVersionRequest ? 'GET' : 'POST',
        body: isStringVersionRequest ? undefined : JSON.stringify(instruction)
      }

      console.debug('> TMCL Instruction', instruction)
      return fetch(options.url, requestOptions)
        .then(checkStatus)
        .then(response => response.json())
        .then(json => {
          console.debug('< TMCL Reply', json)
          dispatch(actions.receiveReply(instruction, json))

          // continue dispatching
          dispatch(dispatchNextPendingInstruction())
        })
        .catch(error => {
          console.debug('< TMCL Error', error)
          dispatch(actions.receiveError(instruction, error))
          // continue dispatching
          // TODO maybe we should abort here?
          dispatch(dispatchNextPendingInstruction())
        })
    } else {
      // there is nothing to dispatch
      console.debug('- TMCL No more pending instructions')
    }
  }

}

function sendInstruction(instruction, replyFn) {
  return dispatch => {
    if (instruction) {
      // This will add the instruction to the 'pending' list
      dispatch(actions.sendInstruction(instruction, replyFn))

      // ... then check if we need to start dispatching
      if (store.getState().tmcl.pending.length === 1) {
        // This is the first pending instruction, start dispatching
        dispatch(dispatchNextPendingInstruction())
      }
    }
  }
}

export function getModuleInfo() {
  sendInstruction({
    instruction: TMCL_GET_FIRMWARE_VERSION,
    type: 0
  })(store.dispatch)

  sendInstruction({
    instruction: TMCL_GET_FIRMWARE_VERSION,
    type: 1
  })(store.dispatch)
}

export function rotate(direction, speed) {
  sendInstruction({
    instruction: direction == 'right' ? TMCL_ROTATE_RIGHT : TMCL_ROTATE_LEFT,
    value: speed
  })(store.dispatch)
}

export function stop() {
  sendInstruction({
    instruction: TMCL_MOTOR_STOP
  })(store.dispatch)
}

export function setUserVariable(index, value) {
  sendInstruction({
    instruction: TMCL_SET_GLOBAL_PARAMETER,
    type: index,
    motor: 2,
    value: value
  })(store.dispatch)
}

export function startApplication(address = 0) {
  sendInstruction({
    instruction: TMCL_START_APPLICATION,
    type: 1,
    value: address
  })(store.dispatch)
}

export function stopApplication() {
  sendInstruction({
    instruction: TMCL_RESET_APPLICATION
  })(store.dispatch)
}

export function getAxisParameter(param) {
  sendInstruction({
    instruction: TMCL_GET_AXIS_PARAMETER,
    type: param
  })(store.dispatch)
}

export function setAxisParameter(param, value) {
  sendInstruction({
    instruction: TMCL_SET_AXIS_PARAMETER,
    type: param,
    value: value
  })(store.dispatch)
}

export function setHome() {
  setAxisParameter(AP_ACTUAL_POSITION, 0)
}

export function getPosition() {
  getAxisParameter(AP_ACTUAL_POSITION)
}
