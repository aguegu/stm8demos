Demos on stm8 with SDCC on linux
===

platforms:

* stm8s-discovery
* stm8l-discovery
* stm8s103f3p6 mini (blue)

setup toolchains
===

1. install sdcc and stm8flash (the aur of archlinux is just convenient)
2. setup sdcc patched SPL, ST StandPeriphLib, check: https://github.com/gicking/STM8-SPL_SDCC_patch
3. mkdir spl && cd spl
4. ln -s PATH_FOR_PATCHED_STM8X_StdPeriph_Lib .
5. go to example folders, and try `make`, `make flash`

Troubleshooting
===

1. to solve USB device acquring write access problem

```
libusb: error [_get_usbfs_fd] libusb couldn't open USB device /dev/bus/usb/003/004: Permission denied
libusb: error [_get_usbfs_fd] libusb requires write access to USB device nodes.
Could not open USB device.
```

create `/etc/udev/rules.d/45-stlink.rules` with content:

```
#STLINK V1
ATTRS{idProduct}=="3744", ATTRS{idVendor}=="0483", MODE="666", GROUP="plugdev"

#STLINK V2
ATTRS{idProduct}=="3748", ATTRS{idVendor}=="0483", MODE="666", GROUP="plugdev"
```

and then reboot the system.

reference:
* https://stackoverflow.com/questions/23312087/error-3-opening-st-link-v2-device
* https://www.ondrovo.com/a/20170107-stm8-getting-started/
* https://github.com/gicking/STM8-SPL_SDCC_patch/issues/6
* https://github.com/vdudouyt/stm8flash
