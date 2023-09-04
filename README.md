# BRUSCHETTA-board🥷🏴‍☠️<br>
⚠️THE Multi-Protocol Swiss-Army-Knife you wanna have around your Hardware Hacking Lab⚠️<br>

 <img src="https://github.com/whid-injector/BRUSCHETTA-board/assets/26245612/f1694cbb-7c0c-4142-8442-79680e0e944c" width=25% height=25%/>

 Bruschetta-Board has been designed by [Luca Bongiorni](https://www.linkedin.com/in/lucabongiorni) and made available by the awesome guys from https://www.aprbrother.com/ for any hardware hacker out there that is looking for a fairly-priced all-in-one debugger & programmer that supports: UART, JTAG, I2C & SPI protocols and allows to interact with different targets' voltages (i.e., 1.8, 2.5, 3.3 and 5 Volts!).<br>

⚠️ **IMPORTANT REMINDER** ⚠️<br>
If you plan to use the BRUSCHETTA-Board as UART and/or JTAG debugger REMEMBER to first unplug any I2C and SPI device that may keep busy the data lines!!!
Otherwise it will create communication issues!!!<br>

## Operating Modes<br>
BRUSCHETTA-board has 4 Operating Modes, however the ones that you will need most are Mode 2 (UART1+I2C+SPI-VCP) and Mode 4 (UART1+JTAG).
**To switch between them you have to set use the DPI-switch S1 & S2:<br>**

**- Mode 2 (UART1+I2C+SPI-VCP): S1=ON and S2=OFF<br>**
    Before starting using your BRUSCHETTA-Board as SPI or I2C Programmer... you will have to change the mode:
    
          • Switch it to Mode-2 by setting the DIP switch S1 to ON and the S2 to OFF (see image below);
          
          • Set the target voltage level (which in the case below was 3.3V);
          
          • Set the Jumper of the Chip Select to CS0.
<img src="https://github.com/whid-injector/BRUSCHETTA-board/assets/26245612/0be6ca79-f1db-4f6c-9b9e-a3f860b249fd" width=50% height=50%>     <br>     

**- Mode 4 (UART1+JTAG): S1=ON and S2=ON<br>**
    To use it as UART and JTAG debugger (both working at the same time) you will have to set both S1 & S2 to ON.
<img src="https://github.com/whid-injector/BRUSCHETTA-board/assets/26245612/7b73cc58-cfcb-4b18-8761-c7fafe5ad335" width=50% height=50%>     <br>  
  
## Level Shifters<br>
A nice feature of BRUSCHETTA-board that makes it perfect for Hardware Hackers is the fact it mounts Level Shifters.<br>
No matter if you have to use it for UART, JTAG, SPI or I2C... if you target is working at 1.8V, 2.5V, 3.3V or 5V... BRUSCHETTA-board is here to help you! You just need to move the Level Shifter Jumper to the target's voltage BEFORE connecting it to the device and the PC.<br>

## Windows<br>
**Drivers Installation**<br>
Before plugging the BRUSCHETTA-board on a Windows OS do install the driver CH341PAR.EXE<br>

**UART**<br>
Putty https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html<br>

**JTAG - OpenOCD**<br>
A precompiled version of OpenOCD compatible with BRUSCHETTA-board is already availabvle in this repo<br>

**SPI/I2C - MultiProgrammer v.1.40** <br>
(from http://www.yaojiedianzi.com/index.php?m=Product&a=show&id=19)<br><br>
<img src="https://github.com/whid-injector/BRUSCHETTA-board/assets/26245612/db57f811-07ab-4031-b96f-db75cecf6954" width=50% height=50%>    <br>


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

<img src="https://github.com/whid-injector/BRUSCHETTA-board/assets/26245612/3ba68eb7-e318-4760-b154-95917c2f38bd" width=50% height=50%>    
<img src="https://github.com/whid-injector/BRUSCHETTA-board/assets/26245612/2af54feb-efc5-4644-9cb2-f7e20ebb9d87" width=50% height=50%>    
<img src="https://github.com/whid-injector/BRUSCHETTA-board/assets/26245612/ef859ba9-cab9-4095-8459-991dc8e9b767" width=50% height=50%>  
<img src="https://github.com/whid-injector/BRUSCHETTA-board/assets/26245612/9d257f83-c7e6-4f09-893e-5c9420d4c2dc" width=50% height=50%> <br>   <br>

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

```sudo ./openocd -f bruschetta-openocd.cfg -f TARGET.cfg```<br><br>
<img src="https://github.com/whid-injector/BRUSCHETTA-board/assets/26245612/44c17039-921e-4596-84ba-48286b8e5eac" width=50% height=50%>    <br>


**I2C - Ch347eeprom**<br>
```git clone https://github.com/minkione/ch347eeprom```<br>
```cd ch347eeprom```<br>
```make```<br>


```sudo ./ch347eeprom -v -s 24c08 -r firmware.bin```<br><br>
<img src="https://github.com/whid-injector/BRUSCHETTA-board/assets/26245612/38bc9723-4d0a-4910-af89-9ad3948a940e" width=50% height=50%>    <br>


## Tips & Tricks
In case you plan to use BRUSCHETTA-Board on a VM I do recommend to enable the USB3.0 controller.<br><br>
<img src="https://github.com/whid-injector/BRUSCHETTA-board/assets/26245612/c9aeed2c-0e41-426e-8d66-d4f831928d09" width=50% height=50%>    

## 3D Printed Case
In this repo you can find the STL CAD file to print your own case for BRUSCHETTA-board. <br>
Ifyou do not own a 3D printer... there are plenty of cheap services out there. <br>
If you live in EU, I do love the guys from [Craftrum](https://www.craftrum.eu), they already printed ton of cases for me 🥇<br>

