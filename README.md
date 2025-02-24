# PIC16F-ZeroToHero
HAL for PIC16F family (not PIC16F1X). Actually in development using the old but lovely PIC16F886

## Recomendation about project structure
Example of project using ARM but it could be used for PIC16F as well.

```
Project_name
-firmware  - this is directory for firmware
 - debug - debug build of firmware
 - release - release build of firmware
 - source - source directory often people use src, but abbreviations can be evil
   - drivers - this is where peripheral drivers are stored
   - libraries - these are code libraries not specific to the project
   - application - this code and tasks specific to application
   - startup - this is processor CMSIS directory as well as reset handler, linker scripts
   main.c  - this is the main() function file that basically calls application
   board.h - this is macros and #define specific to the PCB board
-hardware  - directory for PCB designs
  -Rev0.01 - each board release has its own directory
-datasheets - data sheets for parts used in design
-documents - documents for project

Note on the drivers and library I create a directory for each for example

-drivers
  -uart -uart driver
   -uart.c  - c file for driver
   -uart.h  - header file
```
