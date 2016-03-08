(function(window) {
  var tmcl = {}
  window.tmcl = tmcl;

  tmcl.options = {
    url: 'tmcl'
  }

  var defaultSuccessHandler = function(res) {
    console.log('< TMCL reply', res)
  }

  var defaultErrorHandler = function(res, error) {
    console.error('TMCL error', error, res)
    phonon.alert('A TMCL communication error occured.',
      'TMCL error', false, 'Close')
  }

  tmcl.getModuleInfo = function(success, error) {
    success = success || defaultSuccessHandler
    error = error || defaultErrorHandler

    var req = phonon.ajax({
      method: 'GET',
      url: tmcl.options.url,
      dataType: 'json',
      success: function(res) {
        if (success) success(res)
      },
      error: function(res, errorCode) {
        error(res, errorCode)
      }
    });
  };

  tmcl.sendInstruction = function(instruction, success, error) {
    success = success || defaultSuccessHandler
    error = error || defaultErrorHandler
    console.log('> TMCL instruction', instruction)

    var req = phonon.ajax({
      method: 'POST',
      url: tmcl.options.url,
      contentType: 'application/json',
      data: instruction || {},
      dataType: 'json',
      success: function(res) {
        success(res)
      },
      error: function(res, errorCode) {
        error(res, errorCode)
      }
    });
  }

  tmcl.rotate = function(direction, speed) {
    var instruction = {
      instruction: direction == 'right' ? 1 : 2,
      value: speed
    }
    tmcl.sendInstruction(instruction);
  }

  tmcl.stop = function() {
    tmcl.sendInstruction({instruction: 3});
  }
})(window);
