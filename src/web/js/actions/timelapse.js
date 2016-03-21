// Action Types

export const SET_SHOTS_AND_INTERVAL = 'SET_SHOTS_AND_INTERVAL'
export const SET_TIME_AND_INTERVAL = 'SET_TIME_AND_INTERVAL'
export const SET_EXPOSURE = 'SET_EXPOSURE'
export const SET_STABILIZE = 'SET_STABILIZE'
export const SET_START_POSITION = 'SET_START_POSITION'
export const SET_END_POSITION = 'SET_END_POSITION'

// Action Creators

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

export function setStartPosition(startPosition) {
  return {
    type: SET_START_POSITION,
    startPosition
  }
}

export function setEndPosition(endPosition) {
  return {
    type: SET_END_POSITION,
    endPosition
  }
}
