import store from './store'
import * as actions from './actions/timelapse.js'
import { setUserVariable, startApplication, stopApplication } from './tmclAPI'

// The start address of our TMCL Application
const APPLICATION_ADDRESS = 0

// These are the User Variables used to pass
// the timelapse parameters to the TMCL Application
const PARAM_SHOTS = 0
const PARAM_INTERVAL = 1
const PARAM_STABILIZE = 2
const PARAM_EXPOSURE = 3
const PARAM_REVERSE = 4

export function startTimelapse(address) {
  // setup
  let timelapse = store.getState().timelapse
  console.debug('Start timelapse with params: ', JSON.stringify(timelapse))

  setUserVariable(address, PARAM_SHOTS, timelapse.shots)
  setUserVariable(address, PARAM_INTERVAL, timelapse.interval.asMilliseconds())
  setUserVariable(address, PARAM_STABILIZE, timelapse.stabilize.asMilliseconds())
  setUserVariable(address, PARAM_EXPOSURE, timelapse.exposure.asMilliseconds())
  setUserVariable(address, PARAM_REVERSE, timelapse.reverse ? 1 : 0)

  // start the application
  startApplication(address, APPLICATION_ADDRESS)
}

export function stopTimelapse(address) {
  stopApplication(address)
}

function saveTimelapseState() {
  localStorage.timelapseState = JSON.stringify(store.getState().timelapse)
}

export function loadTimelapseState() {
  let _timelapseState = localStorage.timelapseState
  if (_timelapseState) {
    let timelapseState = JSON.parse(_timelapseState)
    store.dispatch(actions.loadTimelapseState(timelapseState))
  }
}
export function setParameter(action) {
  return dispatch => {
    dispatch(action)
    saveTimelapseState()
  }
}
