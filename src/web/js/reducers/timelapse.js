import * as actions from '../actions/timelapse'
import moment from 'moment'

function timelapse(state = {
                    shots: 300,
                    interval: moment.duration(1, 'seconds'),
                    time: moment.duration(300, 'seconds'),
                    exposure: moment.duration(100, 'milliseconds'),
                    stabilize: moment.duration(100, 'milliseconds'),
                    reverse: false
                  }, action) {

  switch (action.type) {
    case actions.LOAD_TIMELAPSE_STATE:
      return {
        shots: action.savedState.shots,
        interval: moment.duration(action.savedState.interval),
        time: moment.duration(action.savedState.time),
        exposure: moment.duration(action.savedState.exposure),
        stabilize: moment.duration(action.savedState.stabilize),
        reverse: action.savedState.reverse
      }

    case actions.SET_SHOTS_AND_INTERVAL:
      // calculate time
      return Object.assign({}, state, {
        shots: action.shots,
        interval: action.interval,
        time: moment.duration(action.shots * action.interval.asMilliseconds())
      })

    case actions.SET_TIME_AND_INTERVAL:
      // calculate shots
      return Object.assign({}, state, {
        time: action.time,
        interval: action.interval,
        shots: action.time.asMilliseconds() / action.interval.asMilliseconds()
      })

    case actions.SET_EXPOSURE:
      return Object.assign({}, state, { exposure: action.exposure })

    case actions.SET_STABILIZE:
      return Object.assign({}, state, { stabilize: action.stabilize })

   case actions.SET_REVERSE:
      return Object.assign({}, state, { reverse: action.reverse })

    default:
      return state
  }
}

export default timelapse
