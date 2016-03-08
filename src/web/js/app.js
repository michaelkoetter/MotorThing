(function() {
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

  app.on({page: 'home', preventClose: true, content: null});
  app.on({page: 'velocity', preventClose: false, content: null});

  app.start();
})();
