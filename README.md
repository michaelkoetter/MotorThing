# MotorThing - WiFi-enabled Motion Control

[![Build Status](https://travis-ci.org/michaelkoetter/MotorThing.svg?branch=master)](https://travis-ci.org/michaelkoetter/MotorThing)

**_Work in Progress!_**

The MotorThing is a WiFi bridge for [Trinamic TMCL][1] (_Trinamic Motion Control
Language_) products, such as the TMCM-1110 'StepRocker' board or PANdrive
motors.

TMCL instructions can be sent using a simple HTTP JSON interface. A serial
interface (RS-485) is used to communicate with the TMCL module.

## Hardware

MotorThing requires an ESP8266 MCU and a TMCL enabled stepper motor controller.

To connect to the TMCL module, an RS-485 transceiver (MAX3485 or similar) is
used. This transceiver needs three GPIO pins, so make sure your ESP8266 has
them.

For development, the _nodeMCU v2_ board is a good choice.

## API


The HTTP API endpoint is `http://<ip>/tmcl`. It supports two methods, `GET`
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
  "status": <statu>,
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

Please refer to the [TMCL Documentation][1] for information on available
 instructions and their parameters.

### Download TMCL Program

`PUT /tmcl` will download a binary TMCL program to the module's EEPROM.
Binary TMCL programs can be created using the [Trinamic TMCL-IDE][1].

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

[1]: http://www.trinamic.com/software-tools/tmcl-ide
