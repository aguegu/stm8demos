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
4. ln -s PATH_FOR_PATCHED_STM8S_StdPeriph_Lib .
5. go to example folders, and try `make`, `make flash`
