# ESP_HTTPServer

[![Build Status](https://travis-ci.org/michaelkoetter/ESP_HTTPServer.svg?branch=master)](https://travis-ci.org/michaelkoetter/ESP_HTTPServer)

**_Work in Progress!_**

## Yet another ESP8266 HTTP server

This is a fast and robust HTTP server for the ESP8266 MCU, based on
[nodejs/http-parser](//github.com/nodejs/http-parser).

Currently it's not much more than a proof of concept.

## Features

- Uses [nodejs/http-parser](//github.com/nodejs/http-parser) for fast and
  robust HTTP parsing
- Uses [esp8266/Arduino](//github.com/esp8266/Arduino) ESP8266WiFi library for
  connection handling
- HTTP Keep-Alive support
- Multiple concurrent connections
- "_Low memory thresholds_" to prevent MCU crashes under load

_Planned:_
- URL parsing
- Request handler API ('_CGI_')
- CORS support
- Make HTTP Keep-Alive optional
- Native ESP SDK network code. This should be more flexible than the ESP8266WiFi
  library.
- `multipart/form-data` parsing

## Performance

The library has a few features that should improve its performance.

### Minimal-Copy Request Handling

While not zero-copy, the library attempts to copy very little data during request
parsing.
A single fixed size buffer is used, and only header values (as many as you like)
and the request URL are copied during request parsing.

### HTTP Keep-Alive

TCP connection setup is expensive, and especially so on an embedded platform.
HTTP Keep-Alive mitigates this by using a persistent connection for
multiple requests.

The library will maintain up to four concurrent persistent connections.
If a client wants more than four connections, the library will try to close
an 'idle' connection (= a connection that's currently not handling a
request) to accomodate for the new connection.

> The library currently supports up to four concurrent persistent connections
> because of a limitation in the ESP8266WiFi library.
>
> Hopefully this can be increased in a future update, using the native SDK.

### Show me the Numbers!

Okay.

```
Name                                                          # reqs      # fails     Avg     Min     Max  |  Median   req/s
--------------------------------------------------------------------------------------------------------------------------------------------
GET /                                                           2000     0(0.00%)      12       1     275  |       9   79.60
--------------------------------------------------------------------------------------------------------------------------------------------
Total                                                           2000     0(0.00%)                                      79.60

Percentage of the requests completed within given times
Name                                                           # reqs    50%    66%    75%    80%    90%    95%    98%    99%   100%
--------------------------------------------------------------------------------------------------------------------------------------------
GET /                                                            2000      9     11     13     15     23     30     41     52    275
--------------------------------------------------------------------------------------------------------------------------------------------
```

These [Locust](//locust.io) stats show 2000 simple requests (A `HTTP/1.1 GET /` request, returning a simple
`HTTP/1.1 204 No Content` response).
The client used a single connection (HTTP Keep-Alive) for all requests.

The average request rate is almost 80 requests per second, with an average
response time of 12 milliseconds - not bad for a $5 chip!

> Real-world performance will depend on many other factors (network
> congestion, other tasks running on the MCU... you name it).

## Memory Management

The library has a few settings to tune its memory consumption and guard against
memory-related crashes. These are all defined in `config.h`, but can be
set during compile time as well (using `-D...` flag).

I tried to find sensible defaults, but that doesn't mean it will fit your
projects - so have a look at `config.h`.

### Low Memory Thresholds

There are two thresholds, `HTTPSERVER_ACCEPT_MEMORY_THRESHOLD` and
`HTTPSERVER_HANDLE_MEMORY_THRESHOLD`.
The define when the library will stop accepting new connections, and when
it will stop handling requests altogether.

If you'd rather have your MCU crash, just set them to `0`.

### TCP Connection Memory Consumption

Each TCP connection consumes around 200 bytes of memory, even if it's closed
already - it will stay in the `CLOSE_WAIT` state for quite some time, still
consuming memory.
That means it is possible to use up all available memory just by opening lots of
connections and immediately closing them.

The current version of this library can't protect against this, because the
ESP8266WiFi code already accepts TCP connections before we have a chance to
intervene.

All we can do at the moment is trying to stop aggressive clients by stalling
connections when memory is low - that's what
`HTTPSERVER_ACCEPT_MEMORY_THRESHOLD` does.

If the client backs off before the MCU crashes, it will recover eventually
(when connections leave the `CLOSE_WAIT` state, freeing up memory) and start
processing requests again.
