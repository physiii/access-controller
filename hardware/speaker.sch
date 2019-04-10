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
Sheet 7 10
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text Label 3600 1575 0    394  ~ 0
Speaker-Driver
$Comp
L R_100k R20
U 1 1 59BA214E
P 5525 3100
F 0 "R20" V 5525 3100 50  0000 C CNN
F 1 "R_100k" V 5450 3100 50  0000 C CNN
F 2 "Resistors_SMD:R_0603_HandSoldering" V 5455 3100 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/yageo/RC0603JR-07100KL/311-100KGRCT-ND/729645" V 5605 3100 50  0001 C CNN
F 4 "RC0603JR-07100KL" V 5705 3200 60  0001 C CNN "Part Number"
	1    5525 3100
	0    1    1    0   
$EndComp
$Comp
L GND #PWR021
U 1 1 59EB6805
P 5750 3325
F 0 "#PWR021" H 5750 3075 50  0001 C CNN
F 1 "GND" H 5750 3175 50  0000 C CNN
F 2 "" H 5750 3325 50  0000 C CNN
F 3 "" H 5750 3325 50  0000 C CNN
	1    5750 3325
	1    0    0    -1  
$EndComp
Text GLabel 5700 2925 0    60   Input ~ 0
I2S_DATA_OUT
Text GLabel 5425 3225 0    60   Input ~ 0
V_IN
Wire Wire Line
	5750 3325 5750 3300
Wire Wire Line
	5750 3300 5775 3300
Wire Wire Line
	5425 3225 5775 3225
Wire Wire Line
	5700 2925 5775 2925
Wire Wire Line
	6875 2925 7025 2925
Wire Wire Line
	6875 3025 7100 3025
Text GLabel 6975 3125 2    60   Input ~ 0
I2S_WS
Text GLabel 6975 3225 2    60   Input ~ 0
I2S_BCLK
Wire Wire Line
	6975 3225 6875 3225
Wire Wire Line
	6975 3125 6875 3125
$Comp
L MAX98357AETE+T U5
U 1 1 59EB8FEC
P 6125 3050
F 0 "U5" H 6575 2700 60  0000 C CNN
F 1 "MAX98357AETE+T" H 6325 3275 60  0000 C CNN
F 2 "open-automation:MAX98357A" H 6475 2600 60  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/maxim-integrated/MAX98357AETE-T/MAX98357AETE-TCT-ND/7603777" H 6300 2625 60  0001 C CNN
F 4 "MAX98357AETE+T" H 6325 3350 60  0000 C CNN "Part Number"
	1    6125 3050
	1    0    0    -1  
$EndComp
Wire Wire Line
	5775 3125 5725 3125
Wire Wire Line
	5725 3125 5725 3225
Connection ~ 5725 3225
$Comp
L GND #PWR022
U 1 1 59EB98C9
P 5275 3100
F 0 "#PWR022" H 5275 2850 50  0001 C CNN
F 1 "GND" V 5225 2925 50  0000 C CNN
F 2 "" H 5275 3100 50  0000 C CNN
F 3 "" H 5275 3100 50  0000 C CNN
	1    5275 3100
	0    1    1    0   
$EndComp
Wire Wire Line
	5275 3100 5375 3100
Wire Wire Line
	5675 3100 5700 3100
Wire Wire Line
	5700 3100 5700 3025
Wire Wire Line
	5700 3025 5775 3025
$Comp
L C_1uF C6
U 1 1 59EB9DF5
P 5525 3400
F 0 "C6" H 5550 3500 50  0000 L CNN
F 1 "C_1uF" H 5300 3300 50  0000 L CNN
F 2 "Capacitors_SMD:C_0603_HandSoldering" H 5625 3700 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/yageo/CC0603ZRY5V7BB105/311-1372-1-ND/2103156" H 5525 3150 50  0001 C CNN
F 4 "CC0603ZRY5V7BB105" H 5650 3600 60  0001 C CNN "Part Number"
	1    5525 3400
	1    0    0    -1  
$EndComp
Wire Wire Line
	5525 3250 5525 3225
Connection ~ 5525 3225
$Comp
L GND #PWR023
U 1 1 59EB9E7A
P 5525 3600
F 0 "#PWR023" H 5525 3350 50  0001 C CNN
F 1 "GND" H 5525 3450 50  0000 C CNN
F 2 "" H 5525 3600 50  0000 C CNN
F 3 "" H 5525 3600 50  0000 C CNN
	1    5525 3600
	1    0    0    -1  
$EndComp
Wire Wire Line
	5525 3600 5525 3550
$Comp
L speaker-CVS-1508 LS1
U 1 1 59EB9CC6
P 7350 2725
F 0 "LS1" H 7350 2850 50  0000 R CNN
F 1 "speaker-CVS-1508" H 7400 2875 50  0001 R CNN
F 2 "open-automation:speaker-1508" H 7350 2525 50  0001 C CNN
F 3 "http://www.cui.com/product/resource/cvs-1508.pdf" H 7340 2675 50  0001 C CNN
F 4 "CVS-1508" H 7500 3050 60  0001 C CNN "Part Number"
	1    7350 2725
	1    0    0    -1  
$EndComp
Wire Wire Line
	7025 2925 7025 2725
Wire Wire Line
	7025 2725 7150 2725
Wire Wire Line
	7100 3025 7100 2825
Wire Wire Line
	7100 2825 7150 2825
$EndSCHEMATC
