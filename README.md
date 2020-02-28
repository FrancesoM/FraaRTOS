# FraaRTOS
This is my first tentative to create a small OS for ARM Cortex M4

The repo is divided in the OS files (Src and Inc) and the testbench. 
The testbench contains a very small project for an STM32 DISCOVERY board, which I use to test the OS.

In order to compile the testbench, aside from the hardware (discovery board and micro usb cable), you will need:

ARM toolchain: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads
+ add to PATH the bin dir: gcc-arm-none-eabi-9-2019-q4-major/bin

st-link: used to flash the micro and to remote debug: https://github.com/texane/stlink
+ libraries required to compile it:

    Debian based distros (debian, ubuntu)
        build-essential
    cmake
    libusb-1.0 (plus development headers for building, on debian based distros libusb-1.0-0-dev package)
    (optional) for stlink-gui we need libgtk-3-dev

(Optional) to create a new testbench the STMCubeMX application is very useful and creates a makefile for creating the .elf executable right out of the box
