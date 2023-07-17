## SD Card support for Pano Logic Thin Clients

<img src="https://github.com/skiphansen/panog2_sdcard/blob/master/assets/mounted.png" width=50%>

https://github.com/skiphansen/panog2_sdcard

This project provides an example of how an SD card can be connected to a 
Pano Logic G2 to provide mass storage support.

USB of course would be ideal for mass storage, but we have been unable to 
get USB working.  **HELP** is **welcome** and **NEEDED** getting 
[panog2_usb](https://github.com/skiphansen/panog2_usb) working!

A simple test CLI is provided via a serial port for testing.

## Requirements

* A Pano Logic G2
* A suitable 5 volt power supply
* An SD card breakout board and SD card.
* A serial port cable to access test CLI.
* A JTAG programmer to load the bitstream into the FPGA.

## Connecting SD Card

The SD Card is connected in parallel with the existing SPI flash using the SPI 
flash's write protect line as a chip select for the SD card.  Normal operation 
of the existing SPI flash is not affected.


| Pano      | SD CARD | SPI flash<br> pin | FPGA pin |  
|-----------|--------|---------------|----------|
| +3.3V     | +3.3V  |      8        |          |
| GND       | GND    |      4        |          |
| SPI_CCLK  | CLK    |      6        |    Y21   |
| FPGA_DIN  | SO/D0  |      2        |    AA20  |
| SPI_MOSI  | SI/CMD |      5        |    AB20  |
| SPI_WR_N  | CS/D3  |      3        |    AA18  |

Note it may be easier to solder to vias or other component leads rather than
the pins of the SPI flash chip.

| SD CARD | SPI flash<br> pin | alternate connection points |
|--------|----------|----------|
| +3.3V  | 8        | rear end of C79 on bottom of PCB<br><br><img src="https://github.com/skiphansen/panog2_sdcard/blob/master/assets/vcc.png" width=50%> |
| GND    | 4        | JTAG connector P2 pin 6 on bottom of PCB<br><br><img src="https://github.com/skiphansen/panog2_sdcard/blob/master/assets/gnd.png" width=50%>
| SI/CMD | 5        | via next to pin 5 on bottom of PCB<br><br><img src="https://github.com/skiphansen/panog2_sdcard/blob/master/assets/top_big.png" width=50%> |
| CLK    | 6        | via next to pin 6 on bottom of PCB |
| SO/D0  | 2        | rear end of R145 on top of PCB<br><br><img src="https://github.com/skiphansen/panog2_sdcard/blob/master/assets/bottom_big.png" width=50%>|
| CS/D3  | 3        | Front end of R154 on top of PCB |

## Mounting SD card

I used an this Micro SD Card breakout board from [Adafruit](https://www.adafruit.com/product/4682) and
mounted it using double stick tape to the side of the Pano.<br><br>
<img src="https://github.com/skiphansen/panog2_sdcard/blob/master/assets/double_stick_tape.png" width=50%>|

To provide a channel for wires routing the rubber gasket was cut.<br><br>
<img src="https://github.com/skiphansen/panog2_sdcard/blob/master/assets/wire_exit_1.png" width=50%><img src="https://github.com/skiphansen/panog2_sdcard/blob/master/assets/reassembled.png" width=50%>|

## CLI

To use the CLI provided by this example to test your connections you will need 
a serial port cable plugged into the micro HDMI connector.  

Please see [fpga_test_soc](https://github.com/skiphansen/fpga_test_soc/tree/master/fpga/panologic_g2#serial-port)
for connection information.

This example provides commands to test the ability to read, write, erase and 
list files on and SD card with a FAT filesystem.

For example:

````
Pano SD Card test program compiled Jul 16 2023 18:07:15
sdcard> help
Commands:
  csd - dump CSD register
  cid - dump CID register
  del - del <path> - delete a file
  dir - directory <path>
  dump - dump <path>
  disk_init - Initialize SDCARD interface
  test_write - <path> create a file
  type - <path> display file on console
sdcard> dir
----A 2023/01/24 09:00     72872  de10_nano_hdmi_config.bin
----A 2023/01/24 09:00   3368700  de10-nano.rbf
----A 2023/01/24 09:00     70180  dump_adv7513_edid.bin
----A 2023/01/24 09:00     66672  dump_adv7513_regs.bin
D---- 2023/01/24 09:00         0  extlinux
----A 2023/01/24 09:00     29940  socfpga_cyclone5_de10_nano.dtb
----A 2023/01/24 09:00    134035  splash.png
----A 2023/01/24 09:00   8067928  zImage
----A 2023/01/24 09:00  92491518  release.7z
D---- 2023/01/24 09:00         0  Scripts
----A 2023/01/24 09:00    150989  gamecontrollerdb.txt
D---- 2023/01/24 09:00         0  config
   9 File(s), 104452834 bytes total
   3 Dir(s),   15849472 bytes free
sdcard>

````

## Building, etc

**TODO**

## Pano Links

Links to other Pano logic information can be found on the 
[Pano Logic Hackers wiki](https://github.com/tomverbeure/panologic-g2/wiki)

# Acknowledgement and Thanks
This project uses code from several other projects including:
- [ultraembedded's fpga_test_soc](https://github.com/ultraembedded/fpga_test_soc.git)
- [ChaN's FatFs](http://elm-chan.org/fsw/ff/00index_e.html)

# LEGAL 

My original work (the sd_card example program and gateware) is 
released under the GNU General Public License, version 2.

