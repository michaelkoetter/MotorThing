
import phonon from 'phonon';

import quantity from '../tags/timelapse-quantity.html';
import time from '../tags/timelapse-time.html';

// import the top-level Riot tags (pages)
import home from'../tags/home.html';
import velocity from'../tags/velocity.html';
import timelapse from'../tags/timelapse.html';

phonon.options({
    navigator: {
        defaultPage: 'home',
        animatePages: true,
        enableBrowserBackButton: true,
        templateRootDirectory: './template'
    },
    i18n: null
});

var app = phonon.navigator();
app.start();
