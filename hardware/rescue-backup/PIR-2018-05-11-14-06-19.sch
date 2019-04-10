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
Text Label 2750 1225 0    394  ~ 0
PIR_MOTION_SENSOR
Text GLabel 6425 2275 2    60   Input ~ 0
PIR
$Comp
L GND #PWR029
U 1 1 59B7E3DB
P 5125 2375
F 0 "#PWR029" H 5125 2125 50  0001 C CNN
F 1 "GND" H 5125 2225 50  0000 C CNN
F 2 "" H 5125 2375 50  0000 C CNN
F 3 "" H 5125 2375 50  0000 C CNN
	1    5125 2375
	1    0    0    -1  
$EndComp
Text GLabel 5050 2200 0    60   Input ~ 0
3V3
$Comp
L R_100k R18
U 1 1 5A160B80
P 6050 2275
F 0 "R18" V 6050 2275 50  0000 C CNN
F 1 "R_100k" V 5950 2275 50  0000 C CNN
F 2 "Resistors_SMD:R_0603_HandSoldering" V 5980 2275 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/yageo/RC0603JR-07100KL/311-100KGRCT-ND/729645" V 6130 2275 50  0001 C CNN
F 4 "RC0603JR-07100KL" V 6230 2375 60  0001 C CNN "Part Number"
	1    6050 2275
	0    1    1    0   
$EndComp
$Comp
L R_100k R20
U 1 1 5A1610DC
P 6275 2475
F 0 "R20" V 6275 2475 50  0000 C CNN
F 1 "R_100k" V 6175 2475 50  0000 C CNN
F 2 "Resistors_SMD:R_0603_HandSoldering" V 6205 2475 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/yageo/RC0603JR-07100KL/311-100KGRCT-ND/729645" V 6355 2475 50  0001 C CNN
F 4 "RC0603JR-07100KL" V 6455 2575 60  0001 C CNN "Part Number"
	1    6275 2475
	-1   0    0    1   
$EndComp
Wire Wire Line
	5125 2375 5125 2350
Wire Wire Line
	5125 2350 5175 2350
Wire Wire Line
	5050 2200 5175 2200
Wire Wire Line
	5900 2275 5775 2275
Wire Wire Line
	6200 2275 6425 2275
Wire Wire Line
	6275 2275 6275 2325
$Comp
L GND #PWR030
U 1 1 5A1624EE
P 6275 2675
F 0 "#PWR030" H 6275 2425 50  0001 C CNN
F 1 "GND" H 6275 2525 50  0000 C CNN
F 2 "" H 6275 2675 50  0000 C CNN
F 3 "" H 6275 2675 50  0000 C CNN
	1    6275 2675
	1    0    0    -1  
$EndComp
Wire Wire Line
	6275 2675 6275 2625
Connection ~ 6275 2275
$Comp
L EKMC1601112 U10
U 1 1 5A1702E3
P 5425 2275
F 0 "U10" H 5625 2100 60  0000 C CNN
F 1 "EKMC1601112" H 5425 2475 60  0000 C CNN
F 2 "open-automation:EKMC1601112" H 5575 1950 60  0001 C CNN
F 3 "https://www.digikey.com/products/en?keywords=EKMC1601112" H 5575 2025 60  0001 C CNN
F 4 "EKMC1601112" H 5425 2575 60  0001 C CNN "Part Number"
	1    5425 2275
	1    0    0    -1  
$EndComp
$EndSCHEMATC
