
import phonon from 'phonon';

// eslint-disable-next-line no-unused-vars
import tags from '../tags';

// eslint-disable-next-line no-unused-vars
import momentDurationFormat from 'moment-duration-format';

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
