import * as actions from '../actions/timelapse'
import moment from 'moment'

function timelapse(state = {
                    startPosition: 0,
                    endPosition: 100000,
                    shots: 300,
                    interval: moment.duration(1, 'seconds'),
                    time: moment.duration(300, 'seconds'),
                    exposure: moment.duration(100, 'milliseconds'),
                    stabilize: moment.duration(0, 'milliseconds')
                  }, action) {

  switch (action.type) {
    case actions.SET_START_POSITION:
      return Object.assign({}, state, { startPosition: action.startPosition })

    case actions.SET_END_POSITION:
      return Object.assign({}, state, { endPosition: action.endPosition })

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
      return Object.assign({}, state, { stabilize: actions.stabilize })

    default:
      return state
  }
}

export default timelapse
