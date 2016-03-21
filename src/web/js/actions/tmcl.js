
// Action Types

export const INSTRUCTION_REQUEST = 'INSTRUCTION_REQUEST'
export const INSTRUCTION_REPLY = 'INSTRUCTION_REPLY'
export const INSTRUCTION_ERROR = 'INSTRUCTION_ERROR'
export const DISMISS_ALL_ERRORS = 'DISMISS_ALL_ERRORS'

// Action Creators

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
