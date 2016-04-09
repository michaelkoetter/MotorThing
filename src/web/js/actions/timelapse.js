// Action Types

export const LOAD_TIMELAPSE_STATE = 'LOAD_TIMELAPSE_STATE'

export const SET_SHOTS_AND_INTERVAL = 'SET_SHOTS_AND_INTERVAL'
export const SET_TIME_AND_INTERVAL = 'SET_TIME_AND_INTERVAL'
export const SET_EXPOSURE = 'SET_EXPOSURE'
export const SET_STABILIZE = 'SET_STABILIZE'
export const SET_REVERSE = 'SET_REVERSE'

// Action Creators

export function loadTimelapseState(savedState) {
    return {
      type: LOAD_TIMELAPSE_STATE,
      savedState
    }
}

export function setShotsAndInterval(shots, interval) {
  return {
    type: SET_SHOTS_AND_INTERVAL,
    shots,
    interval
  }
}

export function setTimeAndInterval(time, interval) {
  return {
    type: SET_TIME_AND_INTERVAL,
    time,
    interval
  }
}

export function setExposure(exposure) {
  return {
    type: SET_EXPOSURE,
    exposure
  }
}

export function setStabilize(stabilize) {
  return {
    type: SET_STABILIZE,
    stabilize
  }
}

export function setReverse(reverse) {
  return {
    type: SET_REVERSE,
    reverse
  }
}
