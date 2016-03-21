
import * as actions from '../actions/tmcl'

function modules(state = [], action) {
  if (action.reply.version) {
    // discovered new module
    // TODO actually handle multiple modules
    return [
      {
        address: 1,
        version: action.reply.version
      }
    ]
  }

  return state
}

function tmcl(state = {
                pending: [],
                errors: [],
                replies: [],
                modules: []
              }, action) {

  switch (action.type) {
    case actions.INSTRUCTION_REQUEST:
      return Object.assign({}, state, {
        pending: [
          ...state.pending,
          action.instruction
        ]
      })

    case actions.INSTRUCTION_REPLY:
      return Object.assign({}, state, {
        pending: state.pending.filter( instruction => instruction !== action.instruction ),
        replies: [
          ...state.replies,
          { instruction: action.instruction, reply: action.reply }
        ],
        modules: modules(state.modules, action)
      })

    case actions.INSTRUCTION_ERROR:
      return Object.assign({}, state, {
        pending: state.pending.filter( instruction => instruction !== action.instruction ),
        errors: [
          ...state.errors,
          { instruction: action.instructíon, error: action.error }
        ]
      })

    case actions.DISMISS_ALL_ERRORS:
      return Object.assign({}, state, {
        errors: []
      })

    default:
      return state;
  }
}

export default tmcl
