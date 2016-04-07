
// Action Types

export const ADD_MODULE = 'ADD_MODULE'
export const REMOVE_MODULE = 'REMOVE_MODULE'

export const INSTRUCTION_REQUEST = 'INSTRUCTION_REQUEST'
export const INSTRUCTION_REPLY = 'INSTRUCTION_REPLY'
export const INSTRUCTION_ERROR = 'INSTRUCTION_ERROR'
export const DISMISS_ALL_ERRORS = 'DISMISS_ALL_ERRORS'

// Action Creators

export function addModule(address) {
  return {
    type: ADD_MODULE,
    address
  }
}

export function removeModule(address) {
  return {
    type: REMOVE_MODULE,
    address
  }
}

export function sendInstruction(instruction) {
  return {
    type: INSTRUCTION_REQUEST,
    instruction
  }
}

export function receiveReply(instruction, reply) {
  return {
    type: INSTRUCTION_REPLY,
    instruction,
    reply
  }
}

export function receiveError(instruction, error) {
  return {
    type: INSTRUCTION_ERROR,
    instruction,
    error
  }
}

export function dismissAllErrors() {
  return {
    type: DISMISS_ALL_ERRORS
  }
}
