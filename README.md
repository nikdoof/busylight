ESP8266 Busylight
=================

A simple ESP8266 Busylight.

Configuration
-------------

Edit `config.h`, and adjust the following values as needed:

* `LED_TYPE` - 0/1 for Anode/Cathode
* LED Pin numbers
* SSID, and Key

Usage
-----

The ESP8266 runs a simple webserver, and statuses can be changed with simple `GET` calls to `/<status name>`. E.g. `/away`.

The index shows a list of links to the available statuses.

Acknowledgements
----------------

This project is heavily inspired by [David Sword's Busylight](https://davidsword.ca/esp8266-busy-server/) using a NodeMCU ESP8266.