## SD Card support for Pano Logic Thin Clients

https://github.com/skiphansen/panog2_sdcard

This is a work in progress to provide mass storage support for G2 Panos since I have 
been unable to get USB working.  

USB of course would be ideal if we can get [panog2_usb](https://github.com/skiphansen/panog2_usb) working!
**HELP** is **welcome** and **NEEDED**!

## Project Status

Seems to be working fine, can list a directory, and read/write/delete files.
A simple test CLI is provided via a serial port for testing.

## HW Requirements

* A Pano Logic G2
* A suitable 5 volt power supply
* A JTAG programmer to load the bitstream into the FPGA.

## Connecting SD Card

The ultimate plan is to connect the SD Card in parallel with the existing SPI flash using the SPI flash's write protect line 
as a chip select for the SD card.  This should be allow an SD card to be added without compromising any other functionality.

Currently for wiring convenience I have connected the SD Card to some of the DVI port signals. 

| Pano      | SDCARD | board to board<br>connector pin| CN3<br>pin  | ribbon cable<br>color | FPGA<br>pin |  
|-----------|--------|----------------|------|--------|------|
| +3.3V     | +3.3V  | outside 2      | 22   | brown  |      |
| GND       | GND    | inside 22      | 2    | brown  |      |
| GND       | GND    | outside 22     | 4    | orange |      |
| DVI_D[0]  | CLK    | Outside 3      | 25   | blue   | D17  |
| DVI_D[1]  | SO/D0  | Inside 3       | 26   | green  | A14  |
| DVI_D[2]  | SI/CMD | Outside 4      | 23   | yellow | A16  |
| DVI_D[3]  | CS/D3  | inside 4       | 24   | orange | A14  |
| DVI_D[4]  | D1     | outside 5      | 21   | red    | A17  |
| DVI_D[5]  | D2     | inside 5       | 19   | black  | A18  |
| DVI_D[6]  | DET    | outside 7      | 20   | white  | D14  |
| DVI_D[7]  |   -    | inside 7       | 17   | gray   | B14  |
| DVI_D[8]  |   -    | outside 8      | 18   | violet | B16  |
| DVI_D[9]  |   -    | inside 8       | 15   | blue   | B18  |
| DVI_D[10] |   -    | outside 9      | 16   | green  | E16  |
| DVI_D[11] |   -    | inside 9       | 13   | yellow | D15  |
| DVI_H     |   -    | inside 20      | 14   | orange | F12  |

## Serial port 

Please see the [fpga_test_soc](https://github.com/skiphansen/fpga_test_soc/tree/master/fpga/panologic_g2#serial-port) for connection information.

## Building, etc

**TODO**

## Pano Links

Links to other Pano logic information can be found on the 
[Pano Logic Hackers wiki](https://github.com/tomverbeure/panologic-g2/wiki)

