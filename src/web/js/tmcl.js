import phonon from 'phonon';

let options = {
  url: 'tmcl'
};

let defaultSuccessHandler = (res) => {
};

let defaultErrorHandler = (res, error) => {
  console.error('TMCL error', error, res)
  phonon.alert('A TMCL communication error occured.',
    'TMCL error', false, 'Close')
};

export function getModuleInfo(success, error) {
  success = success || defaultSuccessHandler
  error = error || defaultErrorHandler

  var req = phonon.ajax({
    method: 'GET',
    url: options.url,
    dataType: 'json',
    success: function(res) {
      if (success) success(res)
    },
    error: function(res, errorCode) {
      error(res, errorCode)
    }
  });
}

export function sendInstruction(instruction, success, error) {
  success = success || defaultSuccessHandler
  error = error || defaultErrorHandler
  console.log('> TMCL instruction', instruction)

  var req = phonon.ajax({
    method: 'POST',
    url: options.url,
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

export function rotate(direction, speed) {
  var instruction = {
    instruction: direction == 'right' ? 1 : 2,
    value: speed
  }
  sendInstruction(instruction);
}

export function stop() {
  sendInstruction({instruction: 3});
}

export function setUserVariable(index, value, success) {
  sendInstruction({instruction: 9, type: index, motor: 2, value: value}, success);
}

export function startProgram() {
  sendInstruction({instruction: 129, type: 1, value: 0});
}

export function stopProgram() {
  sendInstruction({instruction: 128});
}

export function startTimelapse(startPosition = 0, endPosition = 100000,
  shots, interval, stabilize, exposure) {
  console.log(`Start timelapse ${startPosition}-${endPosition}, shots: ${shots}
    interval: ${interval} stabilize: ${stabilize} exposure: ${exposure}`);

  setUserVariable(0, startPosition, () => {
    setUserVariable(1, endPosition, () => {
      setUserVariable(2, shots, () => {
        setUserVariable(3, interval, () => {
          setUserVariable(4, stabilize, () => {
            setUserVariable(5, exposure, () => {
              startProgram();
            });
          });
        });
      });
    });
  });
}
