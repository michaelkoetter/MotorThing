
import phonon from 'phonon'
import riot from 'riot'
// eslint-disable-next-line no-unused-vars
import momentDurationFormat from 'moment-duration-format'
// eslint-disable-next-line no-unused-vars
import tags from '../tags'

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

// make the Redux store available to all tags as a mixin
let storeMixin = { store }
riot.mixin(storeMixin)

let app = phonon.navigator();
app.start();
