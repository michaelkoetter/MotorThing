<velocity class="app-page">
    <header class="header-bar">
        <a data-navigation="$previous-page" class="btn pull-left icon icon-arrow-back"></a>
        <div class="center">
            <h1 class="title">Velocity Mode</h1>
        </div>
    </header>

    <div class="content">
        <div class="padded-full">

          <div class="input-wrapper">
              <input class="with-label" type="number" name="speed" id="speed"
                oninput="{ input }" value="{ model.speed }">
              <label class="floating-label" for="speed">Speed</label>
          </div>

            <ul class="list">
                <li class="padded-for-list">
                    <label class="radio">
                        <input type="radio" name="direction" value="left"
                          onchange="{ this.input }" checked="{ model.direction == 'left' }">
                        Rotate Left
                        <span></span>
                    </label>
                </li>
                <li class="padded-for-list">
                    <label class="radio">
                        <input type="radio" name="direction" value="right"
                          onchange="{ input }" checked="{ model.direction == 'right' }">
                        Rotate Right
                        <span></span>
                    </label>
                </li>
            </ul>
            <button class="btn fit-parent primary padded-top" onclick="{ start }">Start</button>
            <button class="btn fit-parent negative padded-top" onclick="{ stop }">Stop</button>
        </div>

    </div>

    <script>
        let self = this;
        this.model = {
          speed: 100,
          direction: 'left'
        }

        this.input = (e) => {
          this.model[e.target.name] = e.target.value;
        }

        this.start = () => {
          tmcl.rotate(this.model.direction, parseInt(this.model.speed));
        }

        this.stop = () => {
          tmcl.stop();
        }
    </script>
</velocity>
