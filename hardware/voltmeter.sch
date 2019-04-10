EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:open-automation
LIBS:liger-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 9 9
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text GLabel 6700 3550 2    60   Input ~ 0
IO4
$Comp
L R R?
U 1 1 590D1B56
P 6400 3300
F 0 "R?" H 6450 3150 50  0000 C CNN
F 1 "100K" V 6400 3300 50  0000 C CNN
F 2 "Resistors_SMD:R_0603_HandSoldering" V 6330 3300 50  0001 C CNN
F 3 "http://www.digikey.com/en/resources/datasheets/yageo/rc-series-l-suffix-datasheet" H 6400 3300 50  0001 C CNN
F 4 "RC0603JR-0710KL" V 6400 3300 60  0001 C CNN "Part Number"
	1    6400 3300
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 590D1B57
P 6400 4000
F 0 "#PWR?" H 6400 3750 50  0001 C CNN
F 1 "GND" H 6400 3850 50  0000 C CNN
F 2 "" H 6400 4000 50  0000 C CNN
F 3 "" H 6400 4000 50  0000 C CNN
	1    6400 4000
	1    0    0    -1  
$EndComp
$Comp
L R R?
U 1 1 590D1B5A
P 6400 3750
F 0 "R?" H 6350 3900 50  0000 C CNN
F 1 "25K" V 6400 3750 50  0000 C CNN
F 2 "Resistors_SMD:R_0603_HandSoldering" V 6330 3750 50  0001 C CNN
F 3 "http://www.digikey.com/en/resources/datasheets/yageo/rc-series-l-suffix-datasheet" H 6400 3750 50  0001 C CNN
F 4 "RC0603JR-0710KL" V 6400 3750 60  0001 C CNN "Part Number"
	1    6400 3750
	-1   0    0    1   
$EndComp
Wire Wire Line
	6400 3900 6400 4000
Wire Wire Line
	6400 3450 6400 3600
Wire Wire Line
	6700 3550 6400 3550
Connection ~ 6400 3550
Text GLabel 6400 2950 1    60   Input ~ 0
VIN
Wire Wire Line
	6400 2950 6400 3150
$EndSCHEMATC
