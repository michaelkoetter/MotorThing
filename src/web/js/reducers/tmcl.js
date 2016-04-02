const TMCL_GET_AXIS_PARAMETER = 6 // eslint-disable-line no-unused-vars

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

function axisParameters(state = {}, action) {
  switch (action.reply.instruction) {
    case TMCL_GET_AXIS_PARAMETER: {
        let axisParameter = {}
        axisParameter[action.instruction.type] = action.reply.value
        return Object.assign({}, state, axisParameter)
      }
    default:
      return state;
  }
}

function tmcl(state = {
                pending: [],
                errors: [],
                replies: [],
                modules: [],
                axisParameters: {}
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
        modules: modules(state.modules, action),
        axisParameters: axisParameters(state.axisParameters, action)
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
