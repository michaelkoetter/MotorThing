import store from './store'
import { setUserVariable, startApplication, stopApplication } from './tmclAPI'

// The start address of our TMCL Application
const APPLICATION_ADDRESS = 0

// These are the User Variables used to pass
// the timelapse parameters to the TMCL Application
const PARAM_START_POSITION = 0
const PARAM_END_POSITION = 1
const PARAM_SHOTS = 2
const PARAM_INTERVAL = 3
const PARAM_STABILIZE = 4
const PARAM_EXPOSURE = 5

export function startTimelapse(address = 1) {
  // setup
  let timelapse = store.getState().timelapse
  console.debug('Start timelapse with params: ', JSON.stringify(timelapse))

  setUserVariable(address, PARAM_START_POSITION, timelapse.startPosition)
  setUserVariable(address, PARAM_END_POSITION, timelapse.endPosition)
  setUserVariable(address, PARAM_SHOTS, timelapse.shots)
  setUserVariable(address, PARAM_INTERVAL, timelapse.interval.asMilliseconds())
  setUserVariable(address, PARAM_STABILIZE, timelapse.stabilize.asMilliseconds())
  setUserVariable(address, PARAM_EXPOSURE, timelapse.exposure.asMilliseconds())

  // start the application
  startApplication(address, APPLICATION_ADDRESS)
}

export function stopTimelapse(address = 1) {
  stopApplication(address)
}
