import phonon from 'phonon'

export function enterMotor(address, motor) {
  enterNumber('Select Motor', { value: motor, min: 0, max: 255, step: 1 }, (input) => {
    // FIXME there seems to be no clean way in Phonon to just change the params?
    window.location.hash = `!${phonon.navigator().currentPage}/${address}/${input}`
  })
}

export function enterNumber(title, opts, confirmFn = () => {}, cancelFn = () => {}) {
  let dialogEl = document.querySelector('#number-dialog')
  let inputEl = dialogEl.querySelector('input')
  let titleEl = dialogEl.querySelector('h3')
  inputEl.value = opts.value
  inputEl.min = opts.min
  inputEl.max = opts.max
  inputEl.step = opts.step

  titleEl.innerHTML = title

  let dialog = phonon.dialog(dialogEl)
  dialog.on('confirm', confirmFn)
  dialog.on('cancel', cancelFn)
  dialog.open()
}
