# MotorThing - WiFi-enabled Motion Control

[![Build Status](https://travis-ci.org/michaelkoetter/MotorThing.svg?branch=master)](https://travis-ci.org/michaelkoetter/MotorThing)

**_Work in Progress!_**

The MotorThing is a WiFi bridge for [Trinamic TMCL][TMCL] (_Trinamic Motion Control
Language_) products, such as the TMCM-1110 'StepRocker' board or PANdrive
motors.

TMCL instructions can be sent using a simple HTTP JSON interface. A serial
interface (RS-485) is used to communicate with the TMCL module.

## Hardware

MotorThing requires an ESP8266 MCU and a TMCL enabled stepper motor controller.

To connect to the TMCL module, an RS-485 transceiver (MAX3485 or similar) is
used. This transceiver needs three GPIO pins, so make sure your ESP8266 has
them.

![Transceiver Connection](doc/transceiver.png?raw=true)

## Building the Firmware

This project uses the [PlatformIO] build system.
You can use the command line tools or their IDE (recommended) to build
and upload the firmware.

Usually it boils down to a single command:

```bash
# Compile and upload to the ESP8266 / nodeMCU
platformio run -t upload
```

Please refer to the available [PlatformIO Documentation](http://docs.platformio.org/en/latest/)
for details on how to use it.

> The project includes an environment definition for the nodeMCU v2 board,
> you might need to adjust this if you use a different ESP8266 board
> (see `platformio.ini`).

## Building the Webapp

Building the webapp requires multiple steps.

- First Source files from `src/web` and their dependencies are concatenated
(JavaScript and CSS) and minified.
- Then, unique short filenames are derived using a content hash.
This is necessary to work around the 32-character limit for filenames in the
embedded SPIFFS filesystem, and also has the advantage that these files can
be cached forever.
- The resulting files are compressed using gzip to save space
- Finally, the SPIFFS filesystem is built and uploaded to the ESP8266.

The build process uses [npm][nodejs] and [Gulp], so you will need to install
them once.

```bash
# Install node.js distribution (this includes npm)
# Install Gulp CLI - This needs to be done only once
npm install -g gulp-cli

# Build everything & Upload it to the ESP8266
# The initial run can take a while as npm will download dependencies
platformio run -t uploadfs

# You can also test the webapp in a local browser
gulp webserver
```

> We are currently builing a 512k SPIFFS filesystem, which means you need
> at least 1M flash on your ESP8266 module.

## Automatically Download TMCL Program

You can automatically download a binary TMCL program to the module each time
the MCU boots.
Binary TMCL programs can be created using the [Trinamic TMCL-IDE][TMCL].

To use this feature, place the compiled program in `data/tmcl.bin` and run the command
`platformio run -t uploadfs`.

> **Warning:** This is potentially dangerous and has not been tested much.
> Use at your own risk.
>
> That being said, it seems almost impossible to brick a TMCL module.
> If the module is "stuck" after a failed download, cycle power
> and try again.

## Web API

[![Run in Postman](https://run.pstmn.io/button.png)](https://www.getpostman.com/run-collection/9dce439679000a723515#?env%5Besp8266-ap%5D=W3sia2V5IjoiaG9zdCIsInZhbHVlIjoiMTkyLjE2OC40LjEiLCJ0eXBlIjoidGV4dCIsImVuYWJsZWQiOnRydWV9XQ==")

The Web API endpoint is `http://<ip>/tmcl`. It supports two methods, `GET`
and `POST`.

### Get Module Version

`GET /tmcl` will get the module version and return it as a JSON object.

```
{
    "version": "<version>"
}
```

### Execute TMCL Instruction

`POST /tmcl` will execute a TMCL instruction, specified as JSON object:

```
POST /tmcl
{
  "instruction": <instruction>,
  "type": <type>,
  "motor": <motor>,
  "value": <value>
}
```

The `"instruction"` attribute is mandatory, all other attributes are optional
and default to `0`.

The response contains the TMCL reply as a JSON object:

```
{
  "status": <status>,
  "instruction": <instruction>,
  "value": <value>
}
```

In some cases, an HTTP error status will be sent:
* If there is a timeout in the TMCL communication, HTTP status `504` (Gateway
  Timeout) will be sent.
* If there is a checksum error in the TMCL telegram, HTTP status `502` (Bad
  Gateway) will be sent.
* If the request is malformed, HTTP status `400` is sent.

In addition, the TMCL reply `"status"` attribute might contain an error code
(`100` means success).

Please refer to the [TMCL Documentation][TMCL] for information on available
instructions and their parameters.

### Download TMCL Program

`PUT /tmcl` will download a binary TMCL program to the module's EEPROM.
Binary TMCL programs can be created using the [Trinamic TMCL-IDE][TMCL].

Use a `multipart/form-data` file upload to send the binary file.

If there is an error, HTTP status `500` will be sent.

> **Warning:** This is potentially dangerous and has not been tested much.
> Use at your own risk.
>
> That being said, it seems almost impossible to brick a TMCL module.
> If the module is "stuck" after a failed download, cycle power
> and try again.

## Examples

### Get Module Version

Request:
```
GET http://<ip>/tmcl
```

Response:
```json
{
    "version": "110V3.40"
}
```

### Rotate Right

Request:
```json
POST http://<ip>/tmcl

{
  "instruction": 1,
  "value": 100
}
```

Response:
```json
{
  "status": 100,
  "instruction": 1,
  "value": 100
}
```

### Get Axis Parameter 'actual position'

Request:
```json
POST http://<ip>/tmcl

{
  "instruction": 6,
  "type": 1
}
```

Response:
```json
{
  "status": 100,
  "instruction": 6,
  "value": 2099833
}
```

[TMCL]: http://www.trinamic.com/software-tools/tmcl-ide
[Bower]: http://bower.io/
[Gulp]: http://gulpjs.com/
[nodejs]: http://nodejs.org/
[PlatformIO]: http://platformio.org/
[FNV1a]: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
