(function(window) {
  var tmcl = {}
  window.tmcl = tmcl;

  tmcl.options = {
    url: 'tmcl'
  }

  var defaultSuccessHandler = (res) => {
  }

  var defaultErrorHandler = (res, error) => {
    console.error('TMCL error', error, res)
    phonon.alert('A TMCL communication error occured.',
      'TMCL error', false, 'Close')
  }

  tmcl.getModuleInfo = (success, error) => {
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

  tmcl.sendInstruction = (instruction, success, error) => {
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
        console.log('< TMCL reply', res);
        success(res)
      },
      error: function(res, errorCode) {
        error(res, errorCode)
      }
    });
  }

  tmcl.rotate = (direction, speed) => {
    var instruction = {
      instruction: direction == 'right' ? 1 : 2,
      value: speed
    }
    tmcl.sendInstruction(instruction);
  }

  tmcl.stop = () => {
    tmcl.sendInstruction({instruction: 3});
  }

  tmcl.setUserVariable = (index, value, success) => {
    tmcl.sendInstruction({instruction: 9, type: index, motor: 2, value: value}, success);
  }

  tmcl.startProgram = () => {
    tmcl.sendInstruction({instruction: 129, type: 1, value: 0});
  }

  tmcl.stopProgram = () => {
    tmcl.sendInstruction({instruction: 128});
  }

  tmcl.startTimelapse = (startPosition = 0, endPosition = 100000,
    shots, interval, stabilize, exposure) => {
    console.log(`Start timelapse ${startPosition}-${endPosition}, shots: ${shots}
      interval: ${interval} stabilize: ${stabilize} exposure: ${exposure}`);

    tmcl.setUserVariable(0, startPosition, () => {
      tmcl.setUserVariable(1, endPosition, () => {
        tmcl.setUserVariable(2, shots, () => {
          tmcl.setUserVariable(3, interval, () => {
            tmcl.setUserVariable(4, stabilize, () => {
              tmcl.setUserVariable(5, exposure, () => {
                tmcl.startProgram();
              });
            });
          });
        });
      });
    });
  }
})(window);
