# spin
Signal Processing Instrument for NMR. Based on the RedPitaya and Zynq7 FPGA 

This project is based on [@pavel-demin](https://www.github.com/pavel-demin)'s RedPitaya knowledgebase. He put an incredible amount of love into it, and the RP community would not be where it is today without him.

## About
The Signal Processing Instrument for NMR (SPIN) is a FPGA-based DSP platform tailored for use with the [Compact-NMR](https://github.com/ARTS-Laboratory/Compact-NMR) project from the Adaptive Real-Time Systems Laboratory at the University of South Carolina. It's primary focus is to produce high-fidelity Larmor RF and precise CPMG pulse-trains, as well as to collect time-domain spin echo response signals for post processing. The project **is currently a work in progress,** as I am taking time refining ideas from my previous revision of the same system based on the EclypseZ7 from Digilent. 

I put together a custom operating system for the RedPitaya board based on the v5.15 Linux kernel, it can be found in the /img directory. I plan to release a yocto template for general use based on the project's OS, which can hopefully make someone's initial exploration into kernel development with the RP a little easier. 

## Usage
You'll need, at minimum, Vivado 22.2 installed on your system. You can reproduce the hardware design by opening a tcl console and navigating to the `spin` directory. Invoking `source cnmr-hw.tcl` inside the directory will automatically generate a Vivado block design for you. From there you can run bitstream generation then import the hardware platform into a Vitis project to write your own standalone application system to your liking. 

A demo NMR application is pre-enabled in the OS image and stored as a binary in the root filesystem. You can follow the standard partitioning rhythm to prepare an SD card for booting. I found u-boot is most consistent if one leaves at least 4MB of unallocated space before the boot partition, then giving the boot partition at least 500MB of fat32. rootfs can be formatted as an ext4 parition occupying the remainder of the SD card. Copy BOOT.BIN, boot.scr, and image.ub to the boot partition, then extract the rootfs.tar.gz compression into the rootfs partition. Insert the SD card into the cage on the RedPitaya, then attach a micro-usb cable to the COM port of the board. Serial usually shows up as ttyUSB0 on most Linux systems, but this can vary depending on your machine. Open the serial interface using a serial monitor like putty or minicom, then power on the board. After a few moments of u-boot verbose, log into the root user with root:root. The demo application can be executed by invoking `/usr/bin/cnmr-driver`.  



