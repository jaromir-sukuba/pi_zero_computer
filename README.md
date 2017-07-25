# Raspberry pi portable computer #

## What it is ##
This is small handheld computer, built around Raspberry Pi Zero, referred as RPi in following articles. Though RPi is often thought to be complete computer, it isn't. You need to add some kind of display, input devices (mouse, keyboard or similar), power supply and perhaps network connection to make it usable by today standards.

My decision was to pack it into small portable form. Raspberry pi laptops have been done to death online, but all of them do miss some features I like to have:

1. All-in-one form factor. I don't want one box for brains, another box for BT keyboard, another for USB hub etc... It is simpler from construction point of view, but the device isn't much usable nor portable.
2. Enclosure sturdy enough so I can put the computer into by backpack with no worries of its disintegration. 
3. At least two USB ports, audio output, battery life at least 5 hours.
4. No major modifications to Raspberry pi. 
5. Easy to replicate by others. I mean - off the shelf components, no complicated wiring (once the PCB is done).

I must admit this computer goes in line with my long term "portable computer fixation", exhibited [before](https://hackaday.io/project/18445-brainfcktor), [and here](https://hackaday.io/project/3511-pavapro-portable-avr-programmer), [also here](https://hackaday.io/project/1757-pp04-camel-computer) or [here too](https://hackaday.io/project/643-minibsd-laptop-computer).

![img](https://user-images.githubusercontent.com/6984904/28559091-5760ec50-7115-11e7-964d-dd828cae46ee.jpg)

## What it isn't ##
This device isn't superior to thousands or millions manhours of cummulative work put even into the cheapest smartphone you can buy at usual sources. Despite all the work invested into the devices, those are fit to majority of users and (from obvious reasons) often miss details vital for folks like me - full sized USB ports or GPIO pins attached to main processor or 3,5mm audio jack.

## FAQ ##
Q: How is this better than my phone/tablet/smart toaster?

A: If you need to ask this question, then it isn't any better.


## Hardware ##
All hardware is monted on two PCBs, sandwitched together.
![](https://user-images.githubusercontent.com/6984904/28559104-65e447d6-7115-11e7-9785-cfcf32f72e6f.jpg)
The smaller one contains TFT display and a few minor components, vast majority of components is on bottom PCB.
![](https://user-images.githubusercontent.com/6984904/28560415-db543558-711a-11e7-89ea-09debd7a2657.jpg)
![](https://user-images.githubusercontent.com/6984904/28560511-3c21baea-711b-11e7-9aa6-b7a6d7a1d1fd.jpg)

The circuit seems to be complex, but in fact it's only a bit convoluted power supply and user interface. 
![](https://user-images.githubusercontent.com/6984904/28559120-721b9266-7115-11e7-970a-dac326c9414f.jpg)
RPi acts as heart of this, with USB hub based on FE1.1s, attached to WiFi module, STM32F072 enumerating as USB HID device with two interfaces - one for keyboard (operated by 4x4 matrix keyboard) and one for mouse (operated by analog joystick and three discrete buttons). Audio comes as no surprise to Raspberry pi users, it done by low-pass RC filter and jelly-bean TDA1308 in most common circuit. STM32L011 is connected to RPi by means of serial interface and can turn on/off three power supply voltages (for RPi, for USB, for WiFi) on request of RPi or user. Power is taken from two LiPo cells in series, those are charged by MCP73844 from external 10V source.

Enclosure is designed in FreeCad and printed on hobbyist grade DIY 3D printer
![](https://user-images.githubusercontent.com/6984904/28561092-531d4ffa-711d-11e7-849d-5dffa5ec66ea.png)
Though self tapered screws would do the job too, I used threaded inserts for machined screws, to allow multiple open-close cycles.
![](https://user-images.githubusercontent.com/6984904/28559113-6b6970be-7115-11e7-8d54-e261784134b3.jpg)

Software
I tried to use as little custom software as possible and rely on existing software drivers. That is behind decision of placing the keyboard behind USB HID wall or using RTL8188EUS module for WiFi. Despite that, device doesn't work as-is with mainstream Raspbian distribution, some customized setup is needed.

* download raspbian, create bootable uSD card and put file ssh.txt into /boot of the uSD card - to allow ssh access to RPi
* connect USB-Ethernet convertor into one USB port of device. 3USD one from Asian sources will do the job
* start device, wait a bit, discover it's IP, login, run raspi-config, setup new password, expand filesystem
* run sudo apt-get update
* run sudo apt-get upgrade
* Add to file /etc/modules-load.d/fbtft.conf
> 	spi-bcm2835
> 	fbtft_device

* Add to file /etc/modprobe.d/fbtft.conf
> 	options fbtft_device custom name=fb_ili9341 gpios=reset:25,dc:24,led:18 speed=16000000 rotate=90 bgr=1


* sudo nano /boot/config.txt
> 	dtoverlay=rpi-display

* sudo reboot
*	sudo nano /boot/cmdline.txt
	
> 	fbcon=map:10 fbcon=font:VGA7x14 logo.nologo

*	sudo nano /usr/share/X11/xorg.conf.d/99-fbdev.conf
> 		Section "Device"
> 		  Identifier "bla"
> 		  Driver "fbdev"
> 		  Option "fbdev" "/dev/fb1"
> 		EndSection
 		
 now you should be able to see display working

* in /boot/config.txt add the following line to enable audio interface:
>     dtoverlay=pwm-2chan,pin=18,func=2,pin2=13,func2=4
>     audio_pwm_mode=2
	
* sudo reboot
* sudo apt-get purge rfkill - to prevent WiFi module from sleeping

## Arduino, just for blog cred ##
Yes, you can upload your arduino sketches in the field
![](https://user-images.githubusercontent.com/6984904/28559114-6ddcd480-7115-11e7-9c49-3d7f19a29f4a.jpg)

## Miscellany ##
More info is to be found on my [hackady.io page](https://hackaday.io/project/19437-raspberry-pi-zero-handheld-computer)

This project is licensed by MIT license, see LICENSE.md file
