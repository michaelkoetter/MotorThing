<timelapse-quantity>
  <div class="padded-full">
      <div class="input-wrapper">
        <input class="with-label" type="number" step="1" min="1" name="shots"
            oninput="{ update }">
        <label class="floating-label" for="speed">Number of shots</label>
      </div>
      <div class="input-wrapper">
        <input class="with-label" type="number" step="0.1" min="0.1" name="interval"
            oninput="{ update }">
        <label class="floating-label" for="speed">Interval (s)</label>
      </div>
      <div class="input-wrapper">
        <input class="with-label" type="number" step="0.1" min="0" name="exposure"
            oninput="{ update }">
        <label class="floating-label" for="speed">Exposure (s)</label>
      </div>

      <table class="table padded-top">
        <tr>
          <thead>
            <th>Time</th>
            <th each="{ fps in durations }">{ fps }fps</th>
          </thead>
        </tr>
        <tbody>
          <tr>
              <td>
                { duration.format('h[h] m[m] s[s]') }
              </td>
              <td each="{ fps, duration in durations }">
                { duration.format('h[h] m[m] s[s]') }
              </td>
          </tr>
        </tbody>
      </table>

      <button class="btn fit-parent primary padded-top" onclick="{ start }">Start</button>
      <button class="btn fit-parent negative padded-top" onclick="{ stop }">Stop</button>
  </div>

  <script>
    let self = this;

    this.durations = {
      24: 0,
      25: 0,
      30: 0
    }

    this.updateDurations = () => {
      this.duration = moment.duration(this.shots.value * this.interval.value, 'seconds');
      for (let fps in this.durations) {
        this.durations[fps] = moment.duration(1 / fps * this.shots.value, 'seconds');
      }
    }

    this.start = (e) => {
      let interval = moment.duration(parseFloat(this.interval.value), 'seconds');
      let exposure = moment.duration(parseFloat(this.exposure.value), 'seconds');
      tmcl.startTimelapse(undefined, undefined,
        parseInt(this.shots.value), interval.asMilliseconds(), 0, exposure.asMilliseconds());
    }

    this.stop = (e) => {
      tmcl.stopProgram();
    }

    this.on('update', () => {
      this.updateDurations();
    });

    this.on('mount', () => {
      this.shots.value = 100;
      this.interval.value = 1;
      this.exposure.value = 0.1;

      this.update();
    });
  </script>

</timelapse-quantity>
