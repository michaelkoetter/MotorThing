import { createStore, compose, applyMiddleware, combineReducers } from 'redux'
import thunkMiddleware from 'redux-thunk'
import riot from 'riot'

import * as reducers from './reducers'

const app = combineReducers(reducers)
let store = createStore(app, {}, compose(
  applyMiddleware(thunkMiddleware),
  window.devToolsExtension ? window.devToolsExtension() : f => f
))

let observableStore = riot.observable(store);
store.subscribe(() => observableStore.trigger('change'))

export default observableStore
