import { setUserVariable, startApplication, stopApplication } from './tmclAPI'

export function setupTimelapse(startPosition = 0, endPosition = 100000,
  shots, interval, stabilize, exposure) {
    setUserVariable(0, startPosition)
    setUserVariable(1, endPosition)
    setUserVariable(2, shots)
    setUserVariable(3, interval)
    setUserVariable(4, stabilize)
    setUserVariable(5, exposure)
}

export function startTimelapse() {
  startApplication()
}

export function stopTimelapse() {
  stopApplication()
}
