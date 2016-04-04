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

// TMCL Control Functions
export const TMCL_START_APPLICATION = 129 // eslint-disable-line no-unused-vars
export const TMCL_RESET_APPLICATION = 131 // eslint-disable-line no-unused-vars
export const TMCL_GET_APPLICATION_STATUS = 135 // eslint-disable-line no-unused-vars
export const TMCL_GET_FIRMWARE_VERSION = 136 // eslint-disable-line no-unused-vars

// Axis Parameters
export const AP_TARGET_POSITION = 0 // eslint-disable-line no-unused-vars
export const AP_ACTUAL_POSITION = 1 // eslint-disable-line no-unused-vars

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

export function getModuleInfo(address) {
  sendInstruction({
    address: address,
    instruction: TMCL_GET_FIRMWARE_VERSION,
    type: 0
  })(store.dispatch)

  sendInstruction({
    address: address,
    instruction: TMCL_GET_FIRMWARE_VERSION,
    type: 1
  })(store.dispatch)
}

export function rotate(address, motor, direction, speed) {
  sendInstruction({
    address: address,
    motor: motor,
    instruction: direction == 'right' ? TMCL_ROTATE_RIGHT : TMCL_ROTATE_LEFT,
    value: speed
  })(store.dispatch)
}

export function stop(address, motor) {
  sendInstruction({
    address: address,
    motor: motor,
    instruction: TMCL_MOTOR_STOP
  })(store.dispatch)
}

export function setUserVariable(address, index, value) {
  sendInstruction({
    address: address,
    instruction: TMCL_SET_GLOBAL_PARAMETER,
    type: index,
    motor: 2,
    value: value
  })(store.dispatch)
}

export function startApplication(address, startAddress = 0) {
  sendInstruction({
    address: address,
    instruction: TMCL_START_APPLICATION,
    type: 1,
    value: startAddress
  })(store.dispatch)
}

export function stopApplication(address) {
  sendInstruction({
    address: address,
    instruction: TMCL_RESET_APPLICATION
  })(store.dispatch)
}

export function getAxisParameter(address, motor, param) {
  sendInstruction({
    address: address,
    instruction: TMCL_GET_AXIS_PARAMETER,
    motor: motor,
    type: param
  })(store.dispatch)
}

export function setAxisParameter(address, motor, param, value) {
  sendInstruction({
    address: address,
    instruction: TMCL_SET_AXIS_PARAMETER,
    motor: motor,
    type: param,
    value: value
  })(store.dispatch)
}

export function setHome(address, motor) {
  setAxisParameter(address, motor, AP_ACTUAL_POSITION, 0)
}

export function getPosition(address, motor) {
  getAxisParameter(address, motor, AP_ACTUAL_POSITION)
}
