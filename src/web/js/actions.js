
// action type constants

export const TMCL_VERSION_REQUEST = 'TMCL_VERSION_REQUEST'
export const TMCL_VERSION_REPLY = 'TMCL_VERSION_REPLY'
export const TMCL_VERSION_ERROR = 'TMCL_VERSION_ERROR'

export const TMCL_INSTRUCTION_REQUEST = 'TMCL_INSTRUCTION_REQUEST'
export const TMCL_INSTRUCTION_REPLY = 'TMCL_INSTRUCTION_REPLY'
export const TMCL_INSTRUCTION_ERROR = 'TMCL_INSTRUCTION_ERROR'

// Action Creators

export function requestTMCLVersion() {
  return {
    type: TMCL_VERSION_REQUEST
  }
}

export function receiveTMCLVersionReply(reply) {
  return {
    type: TMCL_VERSION_REPLY,
    reply
  }
}

export function receiveTMCLVersionError(error) {
  return {
    type: TMCL_VERSION_ERROR,
    error
  }
}

export function sendTMCLInstruction(instruction) {
  return {
    type: TMCL_INSTRUCTION_REQUEST,
    instruction
  }
}

export function receiveTMCLReply(instruction, reply, remaining) {
  return {
    type: TMCL_INSTRUCTION_REPLY,
    instruction,
    reply,
    remaining
  }
}

export function receiveTMCLError(instruction, error) {
  return {
    type: TMCL_INSTRUCTION_ERROR,
    instruction,
    error
  }
}
