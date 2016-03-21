
import * as actions from './actions'

export function tmcl(state = {}, action) {
  switch (action.type) {
    case actions.TMCL_VERSION_REQUEST:
    case actions.TMCL_INSTRUCTION_REQUEST:
      return Object.assign({}, state, {
        completed: false
      })
    case actions.TMCL_VERSION_REPLY:
      return Object.assign({}, state, {
        completed: true,
        modules: [
          {
            // TODO actually handle multiple modules (needs support in MCU)
            address: 1,
            version: action.reply
          }
        ]
      })
    case actions.TMCL_INSTRUCTION_REPLY:
      return Object.assign({}, state, {
        completed: action.remaining === 0,
        replies: [
          ...state.replies,
          { instruction: action.instruction, reply: action.reply }
        ]
      })
    case actions.TMCL_VERSION_ERROR:
    case actions.TMCL_INSTRUCTION_ERROR:
      return Object.assign({}, state, {
        completed: true,
        errors: [
          ...state.errors,
          { instruction: action.instructíon, error: action.error }
        ]
      })

    default:
      // initial state
      return {
        completed: true,
        errors: [],
        replies: [],
        modules: []
      };
  }
}
