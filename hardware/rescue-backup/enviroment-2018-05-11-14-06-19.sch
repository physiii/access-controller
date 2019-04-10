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
Sheet 7 9
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text Label 5125 1950 0    197  ~ 0
Environment
Text GLabel 3400 3425 0    60   Input ~ 0
3V3
$Comp
L GND #PWR024
U 1 1 59EBC48D
P 3850 3575
F 0 "#PWR024" H 3850 3325 50  0001 C CNN
F 1 "GND" H 3850 3425 50  0000 C CNN
F 2 "" H 3850 3575 50  0000 C CNN
F 3 "" H 3850 3575 50  0000 C CNN
	1    3850 3575
	1    0    0    -1  
$EndComp
Text GLabel 4850 3425 2    60   Input ~ 0
SCL
Text GLabel 4850 3525 2    60   Input ~ 0
SDA
$Comp
L SI7020-A20 U6
U 1 1 59EBC48E
P 4350 3525
F 0 "U6" H 4550 3425 60  0000 C CNN
F 1 "SI7020-A20" H 4350 3725 60  0000 C CNN
F 2 "open-automation:SI7020-A20" H 4350 3225 60  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/silicon-labs/SI7020-A20-GM1R/336-3596-1-ND/5823452" H 4350 3325 60  0001 C CNN
F 4 "SI7020-A20-GM1R" H 4450 3325 60  0001 C CNN "Part Number"
	1    4350 3525
	1    0    0    -1  
$EndComp
$Comp
L C_0.1uF C7
U 1 1 59EBC48F
P 3550 3625
F 0 "C7" H 3575 3725 50  0000 L CNN
F 1 "C_0.1uF" H 3250 3525 50  0000 L CNN
F 2 "Capacitors_SMD:C_0603_HandSoldering" H 3650 3925 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/kemet/C0603C104K5RACTU/399-5089-1-ND/1465623" H 3550 3375 50  0001 C CNN
F 4 "C0603C104K5RACTU" H 3675 3825 60  0001 C CNN "Part Number"
	1    3550 3625
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR025
U 1 1 59EBC490
P 3550 3825
F 0 "#PWR025" H 3550 3575 50  0001 C CNN
F 1 "GND" H 3550 3675 50  0000 C CNN
F 2 "" H 3550 3825 50  0000 C CNN
F 3 "" H 3550 3825 50  0000 C CNN
	1    3550 3825
	1    0    0    -1  
$EndComp
Text Label 7675 3250 0    60   ~ 0
Light-Level
Text GLabel 8425 3625 2    60   Input ~ 0
SDA
Text GLabel 8425 3500 2    60   Input ~ 0
SCL
$Comp
L GND #PWR026
U 1 1 59EBC491
P 7425 3675
F 0 "#PWR026" H 7425 3425 50  0001 C CNN
F 1 "GND" H 7425 3525 50  0000 C CNN
F 2 "" H 7425 3675 50  0000 C CNN
F 3 "" H 7425 3675 50  0000 C CNN
	1    7425 3675
	1    0    0    -1  
$EndComp
Text GLabel 7100 3525 0    60   Input ~ 0
3V3
$Comp
L C_0.1uF C9
U 1 1 59EBC492
P 7200 3700
F 0 "C9" H 7225 3800 50  0000 L CNN
F 1 "C_0.1uF" H 6900 3600 50  0000 L CNN
F 2 "Capacitors_SMD:C_0603_HandSoldering" H 7300 4000 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/kemet/C0603C104K5RACTU/399-5089-1-ND/1465623" H 7200 3450 50  0001 C CNN
F 4 "C0603C104K5RACTU" H 7325 3900 60  0001 C CNN "Part Number"
	1    7200 3700
	1    0    0    -1  
$EndComp
Text Label 4050 3200 0    60   ~ 0
Temp-Humidty
Wire Wire Line
	3400 3425 3900 3425
Wire Wire Line
	3900 3525 3850 3525
Wire Wire Line
	4850 3525 4750 3525
Wire Wire Line
	4750 3425 4850 3425
Wire Wire Line
	3550 3475 3550 3425
Connection ~ 3550 3425
Wire Wire Line
	3550 3825 3550 3775
Wire Wire Line
	3850 3525 3850 3575
Wire Wire Line
	7425 3675 7425 3625
Wire Wire Line
	7425 3625 7475 3625
Wire Wire Line
	7100 3525 7475 3525
Wire Wire Line
	7200 3525 7200 3550
$Comp
L GND #PWR027
U 1 1 59EBC493
P 7200 3900
F 0 "#PWR027" H 7200 3650 50  0001 C CNN
F 1 "GND" H 7200 3750 50  0000 C CNN
F 2 "" H 7200 3900 50  0000 C CNN
F 3 "" H 7200 3900 50  0000 C CNN
	1    7200 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	7200 3900 7200 3850
Connection ~ 7200 3525
Wire Wire Line
	8425 3625 8325 3625
Wire Wire Line
	8325 3525 8375 3525
Wire Wire Line
	8375 3525 8375 3500
Wire Wire Line
	8375 3500 8425 3500
$Comp
L TSL45315CL U8
U 1 1 59EBC494
P 7925 3625
F 0 "U8" H 8125 3525 60  0000 C CNN
F 1 "TSL45315CL" H 7925 3825 60  0000 C CNN
F 2 "open-automation:LIGHT-SENSOR-TSL45315CL" H 7925 3325 60  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/ams/TSL45315CL/TSL45315-CLCT-ND/3095326" H 7925 3425 60  0001 C CNN
F 4 "TSL45315CL" H 7925 3225 60  0001 C CNN "Part Number"
	1    7925 3625
	1    0    0    -1  
$EndComp
$EndSCHEMATC
