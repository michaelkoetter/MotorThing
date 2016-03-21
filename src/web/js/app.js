
import phonon from 'phonon'
import riot from 'riot'
import momentDurationFormat from 'moment-duration-format' // eslint-disable-line no-unused-vars
import tags from '../tags' // eslint-disable-line no-unused-vars

import store from './store'

phonon.options({
    navigator: {
        defaultPage: 'home',
        animatePages: true,
        enableBrowserBackButton: true,
        templateRootDirectory: './template'
    },
    i18n: null
});

let globalMixin = {
  // make the Redux store available to all tags as a mixin
  store,

  // make the current state available to all tags as a mixin
  // this enables shorter expressions like { state().property }
  // instead of { store.getState().property }
  get state() {
    return () => store.getState()
  }
}
riot.mixin(globalMixin)

// manually mount top-level tags that are not pages
riot.mount('tmcl-notification')

let app = phonon.navigator();
app.start();
