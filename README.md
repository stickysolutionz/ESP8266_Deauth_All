# ESP8266 Deauth All

This is a fork from https://github.com/justcallmekoko I removed the OLED screen reqs and tightened up the code. just program and power up :) If the AP is vulnerable youll see discons within a minute of boot.

## Table of Condiments
  - [Background](#background)
  - [How it works](#how-it-works)
  - [Setup](#setup)
    - [Hardware](#hardware)
    - [Uploading Code Using Arduino IDE](#uploading-code-using-arduino-ide)
  - [Usage](#usage)
  - [Issues](#issues)
  
## Background
A WiFi [deauthentication attack](https://en.wikipedia.org/wiki/Wi-Fi_deauthentication_attack) is a form of denial of service where an attacker crafts 802.11 frames designed to sever the connection between a WiFi client and a wireless access point.  

These frames are unencrypted since most devices do not support the 802.11w standard and because of this vulnerability, there is no way to guard against this attack as long as these frames appear to come from a legitimate source.  

An attacker can scan for surrounding access points, clone each BSSID it finds, and broadcast deauthentication frames spoofing the source MAC address as the access points it found. As other devices receive these deauthentication frames from their access point, they will be forced off of the network. The attacker doesn't even need to be on the target network to perform this attack.  

My main objective for this project was to create a device that would be small enough to carry anywhere and be able to scan and maintain a list containing only of access points it has found recently while removing access points no longer in the area. Each new scan will then result in a deauthentication attack spanning one minute.  

<b>Do not be a dumbass and get yourself in trouble. This is a form of denial of service and in most countries, it is illegal without the explicit consent from the owner of the target network.</b>

## How it works
  1. Scan all channels twice
    <p align="center"><img alt="Scan" src="https://github.com/the-red-team/ESP8266_Deauth_All/blob/master/images/de3.jpg" width="700"></p>  
  2. Clean out any APs from the last scan that did not show up in the current scan
  3. Deauthenticate all devices for one minute by sending broadcast deauth frames from all APs
    <p align="center"><img alt="Deauth" src="https://github.com/the-red-team/ESP8266_Deauth_All/blob/master/images/de.jpg" width="600"></p>  
  4. Repeat

### Hardware
  * [WEMOS D1 Mini](https://www.amazon.com/Makerfocus-NodeMcu-Development-ESP8266-Compatible/dp/B01N3P763C/ref=sr_1_3?ie=UTF8&qid=1531324588&sr=8-3&keywords=wemos+d1+mini)
  
### Uploading Code Using Arduino IDE
1. Install and open the [Arduino IDE](https://www.arduino.cc/en/Main/Software)
2. Add URLs to board manager URLs (File, Preferences, Additional Boards Manager URLs):  
   * `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
   * `http://phpsecu.re/esp8266/package_deauther_index.json`
3. Install ESP8266 boards (Tools, Boards Manager):  
   * `arduino-esp8266-deauther`
   * `esp8266`
4. Copy and paste one of the 2 options below. Option a. will deauth all APs. Option b. has an editable ignore list on line 45.
   * [a.](https://raw.githubusercontent.com/stickysolutionz/ESP8266_Deauth_All/master/deauth_all.ino)
   * [b.](https://raw.githubusercontent.com/stickysolutionz/ESP8266_Deauth_All/master/deauth_all.ignore.list.ino)
5. Select the WeMos D1 Mini Board (Tools, Board, <b>ESP8266 Deauther Modules</b>)
6. Upload
  
## Usage
Just plug the device into any USB power source whether it's a computer, wall socket, or portable power supply, and let the device do the rest of the work. 

## Issues
Some devices are able to ignore deauth frames sent to broadcast so this may not work against all devices in your area
