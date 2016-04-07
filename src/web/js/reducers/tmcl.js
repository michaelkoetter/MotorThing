import {
  TMCL_ROTATE_LEFT,
  TMCL_ROTATE_RIGHT,
  TMCL_MOTOR_STOP,
  TMCL_MOVE_TO_POSITION,
  TMCL_GET_AXIS_PARAMETER,
  TMCL_SET_AXIS_PARAMETER,

  TMCL_GET_GLOBAL_PARAMETER, // eslint-disable-line no-unused-vars
  TMCL_SET_GLOBAL_PARAMETER // eslint-disable-line no-unused-vars
} from '../tmclAPI.js'

import * as actions from '../actions/tmcl'

/**
 * These are the reducers for the TMCL state tree. They store some parameters
 * of the TMCL module when they are seen in replies, such as global parameters,
 * axis parameters and module info.
 *
 * The subtree looks like this:
 *
 * {
 *   pending: [],
 *   errors: [],
 *   replies: [],
 *   modules: [
 *     {
 *        address: 1,
 *        version: "version",
 *        motors: [
 *          {
 *            motor: 0,
 *            parameters: [
 *              {
 *                parameter: 1,
 *                value: 123
 *              },
 *              ...
 *            ]
 *          },
 *          ...
 *        ],
 *        globalParameters: [
 *          {
 *            parameter: 1,
 *            value: 123
 *          },
 *          ...
 *        ]
 *     }
 *   ]
 * }
 *
 */


function globalParameters(state = [], action) { // eslint-disable-line no-unused-vars
  return state
}

function axisParameter(state = {}, action) {
  if (state.parameter == action.instruction.type) {
    return Object.assign({}, state, {
      value: action.reply.value
    })
  }

  return state
}

function axisParameters(state = [], action) {
  if (action.reply.instruction == TMCL_GET_AXIS_PARAMETER ||
      action.reply.instruction == TMCL_SET_AXIS_PARAMETER) {
      if (!state.find(p => p.parameter == action.instruction.type)) {
        return [
          ...state,
          axisParameter({ parameter: action.instruction.type }, action)
        ]
      }
      return state.map(p => axisParameter(p, action))
    }

    return state
}

function motor(state = {}, action) {
  let _motor = action.instruction.motor || 0
  if (state.motor == _motor) {
    return Object.assign({}, state, {
      parameters: axisParameters(state.parameters, action)
    })
  }
  return state
}

function motors(state = [], action) {
  switch (action.reply.instruction) {
    case TMCL_ROTATE_LEFT:
    case TMCL_ROTATE_RIGHT:
    case TMCL_MOTOR_STOP:
    case TMCL_MOVE_TO_POSITION:
    case TMCL_GET_AXIS_PARAMETER:
    case TMCL_SET_AXIS_PARAMETER: {
      let _motor = action.instruction.motor || 0
      if (!state.find(a => a.motor == _motor)) {
        // motor discovery: got a reply for new motor, so add it to the model
        console.debug('Discovered new motor: ', _motor)
        return [
          ...state,
          motor({ motor: _motor }, action)
        ]
      }

      return state.map(m => motor(m, action))
    }
    default:
      return state;
  }
}

function module(state = {}, action) {
  if (action.reply && state.address == action.reply.address) {
    if (action.reply.version) {
      return Object.assign({}, state, {
        version: action.reply.version,
        globalParameters: globalParameters(state.globalParameters, action),
        motors: motors(state.motors, action)
      })
    } else {
      return Object.assign({}, state, {
        globalParameters: globalParameters(state.globalParameters, action),
        motors: motors(state.motors, action)
      })
    }
  }
  return state
}

function modules(state = [], action) {
  switch (action.type) {
    case actions.ADD_MODULE: {
      if (!state.find(m => m.address == action.address))
        return [
          ...state,
          { address: action.address }
        ]

      return state // make sure we only add each address once
    }

    case actions.REMOVE_MODULE:
      return state.filter(m => m.address != action.address)

    default:
      return state.map(m => module(m, action))
  }
}

function tmcl(state = {
                pending: [],
                errors: [],
                replies: [],
                modules: []
              }, action) {

  switch (action.type) {
    case actions.ADD_MODULE:
    case actions.REMOVE_MODULE:
      return Object.assign({}, state, {
        modules: modules(state.modules, action)
      })

    case actions.INSTRUCTION_REQUEST:
      return Object.assign({}, state, {
        pending: [
          ...state.pending,
          action.instruction
        ],
        modules: modules(state.modules, action)
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
          { instruction: action.instruction, error: action.error }
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
