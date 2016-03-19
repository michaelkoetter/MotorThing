<timelapse-time>
  <div class="padded-full">
      <div class="input-wrapper">
        <input class="with-label" type="number" step="1" min="1" name="time"
            oninput="{ update }">
        <label class="floating-label" for="speed">Time (m)</label>
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
            <th>Shots</th>
            <th each="{ fps in durations }">{ fps }fps</th>
          </thead>
        </tr>
        <tbody>
          <tr>
              <td>
                { shots }
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
      this.duration = moment.duration(parseInt(this.time.value), 'minutes');
      this.shots = Math.round(this.duration.asSeconds() / this.interval.value);

      for (let fps in this.durations) {
        this.durations[fps] = moment.duration(1 / fps * this.shots, 'seconds');
      }
    }

    this.start = (e) => {
      let interval = moment.duration(parseFloat(this.interval.value), 'seconds');
      let exposure = moment.duration(parseFloat(this.exposure.value), 'seconds');
      tmcl.startTimelapse(undefined, undefined,
          this.shots, interval.asMilliseconds(), 0, exposure.asMilliseconds());
    }

    this.stop = (e) => {
      tmcl.stopProgram();
    }

    this.on('update', () => {
      this.updateDurations();
    });

    this.on('mount', () => {
      this.time.value = 10;
      this.interval.value = 1;
      this.exposure.value = 0.1;

      this.update();
    });
  </script>
</timelapse-time>
