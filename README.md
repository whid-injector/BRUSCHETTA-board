# BRUSCHETTA-board
⚠️THE Multi-Protocol Swiss-Army-Knife you wanna have around your Hardware Hacking Lab⚠️<br>

**Available already assembled and ready for hacking at:**

* [Aliexpress](https://www.aliexpress.com/item/1005006010220678.html) <br>
* [Tindie](https://www.tindie.com/products/aprbrother/bruschetta-board/) <br>
* [Ebay](https://www.ebay.com/itm/256207986212) <br>
* [Alibaba](https://www.alibaba.com/product-detail/BURSCHETTA-Board-a-Multi-Protocol-Swiss_1600938879092.html) <br>
* [AprBrother store](https://store.aprbrother.com/product/bruschetta-board) <br>
* [KSEC store](https://labs.ksec.co.uk/product/burschetta-board-a-multi-protocol-swiss-army-knife-for-hardware-hacking-that-supports-uart-jtag-spi-i2c/
) EUROPE <br>

<img src="https://github.com/whid-injector/BRUSCHETTA-board/blob/main/images/THE%20GIF.gif" width=25% height=25%/>

Bruschetta-Board has been designed by [Luca Bongiorni](https://www.linkedin.com/in/lucabongiorni) and made available by the awesome guys from https://www.aprbrother.com/ for any hardware hacker out there that is looking for a fairly-priced all-in-one debugger & programmer that supports: UART, JTAG, I2C & SPI protocols and allows to interact with different targets' voltages (i.e., 1.8, 2.5, 3.3 and 5 Volts!).<br>


## WHID's Trainings
The 𝙊𝙛𝙛𝙚𝙣𝙨𝙞𝙫𝙚 𝙃𝙖𝙧𝙙𝙬𝙖𝙧𝙚 𝙃𝙖𝙘𝙠𝙞𝙣𝙜 𝙏𝙧𝙖𝙞𝙣𝙞𝙣𝙜 is a Self-Paced training including Videos, a printed Workbook and a cool Hardware Hackit Kit. And... you get everything shipped home Worldwide! 🌍🔥😎<br>
For more info... ➡ https://www.whid.ninja/store <br><br>

[![WHID's Trainings](https://files.gandi.ws/64/2e/642e05f6-84e1-48fe-8a59-d678c7d635e3.PNG)](https://www.youtube.com/watch?v=zbUuBZJIHkE)




⚠️ **IMPORTANT REMINDER** ⚠️
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
<img src="https://github.com/whid-injector/BRUSCHETTA-board/blob/main/images/Mode%202%20-%20SPI%20and%20I2C.jpg" width=50% height=50%>     <br>     

**- Mode 4 (UART1+JTAG): S1=ON and S2=ON<br>**
    To use it as UART and JTAG debugger (both working at the same time) you will have to set both S1 & S2 to ON.
<img src="https://github.com/whid-injector/BRUSCHETTA-board/blob/main/images/Mode%204%20-%20UART%20and%20JTAG.jpg" width=50% height=50%>     <br>  
  
## Level Shifters
A nice feature of BRUSCHETTA-board that makes it perfect for Hardware Hackers is the fact it mounts Level Shifters.<br>
No matter if you have to use it for UART, JTAG, SPI or I2C... if you target is working at 1.8V, 2.5V, 3.3V or 5V... BRUSCHETTA-board is here to help you! You just need to move the Level Shifter Jumper to the target's voltage BEFORE connecting it to the device and the PC.<br>

## Windows
**Drivers Installation**<br>
Before plugging the BRUSCHETTA-board on a Windows OS do install the driver CH341PAR.EXE<br>

**UART**<br>
Putty https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html<br>

**JTAG - OpenOCD**<br>
A precompiled version of OpenOCD compatible with BRUSCHETTA-board is already availabvle in this repo<br>

**SPI/I2C - MultiProgrammer v.1.40** <br>
(from http://www.yaojiedianzi.com/index.php?m=Product&a=show&id=19)<br><br>
<img src="https://github.com/whid-injector/BRUSCHETTA-board/blob/main/images/windows%20multi%20programmer.png" width=50% height=50%>    <br>


**SPI - Flashrom**<br>
Flashrom 1.4.0dev build for windows x64
https://github.com/whid-injector/flashrom_build_windows_x64<br>


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

<img src="https://github.com/whid-injector/BRUSCHETTA-board/blob/main/images/Flashrom%20SPI%20Dump.png" width=50% height=50%><br>

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
<img src="https://github.com/whid-injector/BRUSCHETTA-board/assets/26245612/e98213f1-904d-411a-8e93-a463af320995" width=50% height=50%>    <br>


**I2C - Ch347eeprom**<br>
```git clone https://github.com/whid-injector/ch347eeprom```<br>
```cd ch347eeprom```<br>
```make```<br>


```sudo ./ch347eeprom -v -s 24c08 -r firmware.bin```<br><br>
<img src="https://github.com/whid-injector/BRUSCHETTA-board/blob/main/images/ch347eeprom.png" width=50% height=50%>    <br>


## Tips & Tricks
In case you plan to use BRUSCHETTA-Board on a VM I do recommend to enable the USB3.0 controller.<br><br>
<img src="https://github.com/whid-injector/BRUSCHETTA-board/blob/main/images/vmware%20setting.png" width=50% height=50%>    

## 3D Printed Case
In this repo you can find the STL CAD file to print your own case for BRUSCHETTA-board. <br>
Ifyou do not own a 3D printer... there are plenty of cheap services out there. <br>
If you live in EU, I do love the guys from [Craftrum](https://www.craftrum.eu), they already printed ton of cases for me 🥇<br>

