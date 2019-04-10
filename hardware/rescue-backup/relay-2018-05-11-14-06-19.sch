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
Sheet 8 9
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text GLabel 5000 2275 0    60   Input ~ 0
RELAY
Text Label 4525 1325 0    394  ~ 0
Relay
$Comp
L R_10k R26
U 1 1 59B9995F
P 5275 2275
F 0 "R26" V 5275 2275 50  0000 C CNN
F 1 "R_10k" V 5200 2275 50  0000 C CNN
F 2 "Resistors_SMD:R_0603_HandSoldering" V 5205 2275 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/yageo/RC0603JR-0710KL/311-10KGRCT-ND/729647" V 5355 2275 50  0001 C CNN
F 4 "RC0603JR-0710KL" V 5455 2375 60  0001 C CNN "Part Number"
	1    5275 2275
	0    -1   -1   0   
$EndComp
$Comp
L GND #PWR028
U 1 1 59B99A98
P 5475 2275
F 0 "#PWR028" H 5475 2025 50  0001 C CNN
F 1 "GND" H 5475 2125 50  0000 C CNN
F 2 "" H 5475 2275 50  0000 C CNN
F 3 "" H 5475 2275 50  0000 C CNN
	1    5475 2275
	0    -1   -1   0   
$EndComp
Wire Wire Line
	5475 2275 5425 2275
Wire Wire Line
	5000 2275 5125 2275
$EndSCHEMATC
