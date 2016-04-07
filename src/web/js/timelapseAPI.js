import store from './store'
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

export function startTimelapse(address = 1) {
  // setup
  let timelapse = store.getState().timelapse
  console.debug('Start timelapse with params: ', JSON.stringify(timelapse))

  setUserVariable(PARAM_SHOTS, timelapse.shots)
  setUserVariable(PARAM_INTERVAL, timelapse.interval.asMilliseconds())
  setUserVariable(PARAM_STABILIZE, timelapse.stabilize.asMilliseconds())
  setUserVariable(PARAM_EXPOSURE, timelapse.exposure.asMilliseconds())
  setUserVariable(PARAM_REVERSE, timelapse.reverse ? 1 : 0)

  // start the application
  startApplication(address, APPLICATION_ADDRESS)
}

export function stopTimelapse(address = 1) {
  stopApplication(address)
}
