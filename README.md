# BRUSCHETTA-board
 The Multi-Protocol Swiss-Army-Knife for Hardware Hackers

## Operating Modes<br>
BRUSCHETTA-board has 4 Operating Modes, however the ones that you will need most are Mode 2 (UART1+I2C+SPI-VCP) and Mode 4 (UART1+JTAG).<br>
To switch between them you have to set use the DPI-switch S1 & S2:<br>
- Mode 2 (UART1+I2C+SPI-VCP): S1=ON and S2=OFF 
- Mode 4 (UART1+JTAG): S1=ON and S2=ON
  
## Level Shifters<br>
A nice feature of BRUSCHETTA-board that makes it perfect for Hardware Hackers is the fact it mounts Level Shifters.<br>
No matter if you have to use it for UART, JTAG, SPI or I2C... if you target is working at 1.8V, 2.5V, 3.3V or 5V... BRUSCHETTA-board is here to help you!<br>
You just need to move the Level Shifter Jumper to the target's voltage BEFORE connecting it to the device and the PC.<br>

## Windows<br>
**Drivers Installation**<br>
Before plugging the BRUSCHETTA-board on a Windows OS do install the driver CH341PAR.EXE<br>

**UART**<br>
Putty https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html<br>

**JTAG - OpenOCD**<br>
A precompiled version of OpenOCD compatible with BRUSCHETTA-board is already availabvle in this repo<br>

**SPI/I2C - MultiProgrammer v.1.40** <br>
(from http://www.yaojiedianzi.com/index.php?m=Product&a=show&id=19)<br>

**SPI - Flashrom**<br>
Flashrom 1.4.0dev build for windows x64
https://github.com/therealdreg/flashrom_build_windows_x64<br>


## Linux

**Prerequisites**<br>
The following steps were succesfully tested on Ubuntu 22.04:<br>
```sudo apt-get update```<br>
```sudo apt-get install --yes software-properties-common  apt-transport-https ca-certificates```<br>
```sudo update-ca-certificates```<br>
```sudo apt-get install libtool pkg-config texinfo libusb-dev libusb-1.0.0-dev libftdi-dev autoconf automake make git libftdi* libhidapi-hidraw0 libudev-dev python3-sphinx```<br>

**UART**<br>
```sudo screen /dev/ttyACM0 115200```<br>

**SPI - Flashrom**<br>
```git clone https://github.com/flashrom/flashrom```<br>
```cd flashrom```<br>
```make```<br>
(optional) ```sudo make install```<br>

```sudo ./flashrom -p ch347_spi  -r firmware.bin```<br>

**SPI - SPI-NAND PROG**<br>
https://github.com/981213/spi-nand-prog<br>

**JTAG & SWD - OpenOCD**<br>

This is the official repo from the IC manufacturer, however I do recommend to use the openocd repo within mine Github repository since it was succesfully compiled and tested with BRUSCHETTA-board <br>
https://github.com/WCHSoftGroup/ch347<br>

```cd ~/Desktop```<br>
```git clone https://github.com/whid-injector/BRUSCHETTA-Board```<br>
```cd BRUSCHETTA-Board/Linux/openocd-bruschetta```<br>
```./bootstrap```<br>
```./configure --prefix=/home/<YOUR-USER>/Desktop/BRUSCHETTA-Board/openocd-ch347 --disable-doxygen-html --disable-doxygen-pdf --disable-gccwarnings --disable-wextra --enable-ch347```<br>
```make -j 4```<br>
```sudo make install```<br>

Now copy bruschetta-openocd.cfg in ```/home/<YOUR-USER>/Desktop/BRUSCHETTA-Board/openocd-ch347/bin```<br>

```sudo ./openocd -f bruschetta-openocd.cfg -f TARGET.cfg```<br>


**I2C - Ch347eeprom**<br>
```git clone https://github.com/minkione/ch347eeprom```<br>
```cd ch347eeprom```<br>
```make```<br>


```sudo ./ch347eeprom -v -s 24c08 -r firmware.bin```<br>
