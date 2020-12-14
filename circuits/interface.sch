EESchema Schematic File Version 5
LIBS:open-dash-daughterboard-cache
EELAYER 29 0
EELAYER END
$Descr A 11000 8500
encoding utf-8
Sheet 7 8
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text HLabel 1375 1125 0    50   Input ~ 0
CAN+
Text HLabel 1875 1125 2    50   Input ~ 0
CAN-
Text HLabel 1375 1025 0    50   Input ~ 0
CAMERA+
Text HLabel 1875 925  2    50   Input ~ 0
CAMERA_S
Text HLabel 1875 1025 2    50   Input ~ 0
CAMERA-
Text HLabel 1375 825  0    50   Input ~ 0
BATTERY
Text HLabel 3400 1025 0    50   Input ~ 0
FL_AUDIO_OUT+
Text HLabel 3400 925  0    50   Input ~ 0
FR_AUDIO_OUT+
Text HLabel 3400 825  0    50   Input ~ 0
RL_AUDIO_OUT+
Text HLabel 3400 1125 0    50   Input ~ 0
RR_AUDIO_OUT+
Text HLabel 3900 1125 2    50   Input ~ 0
RR_AUDIO_OUT-
Text HLabel 3900 825  2    50   Input ~ 0
RL_AUDIO_OUT-
Text HLabel 3900 925  2    50   Input ~ 0
FR_AUDIO_OUT-
Text HLabel 3900 1025 2    50   Input ~ 0
FL_AUDIO_OUT-
$Comp
L open-automation:molex-2x4-5.7mm U7
U 1 1 5B1A08B0
P 1625 975
F 0 "U7" H 1625 1225 50  0000 C CNN
F 1 "molex-2x4-5.7mm" H 1650 725 50  0000 C CNN
F 2 "open-automation:molex-2x4-5.7mm" H 1625 575 50  0001 C CNN
F 3 "https://www.digikey.com/products/en?keywords=768290008" H 1625 675 50  0001 C CNN
F 4 "0768290008" H 1625 725 50  0001 C CNN "Part Number"
	1    1625 975 
	1    0    0    -1  
$EndComp
Wire Wire Line
	1375 825  1425 825 
Wire Wire Line
	1875 925  1825 925 
Wire Wire Line
	1825 825  1875 825 
Wire Wire Line
	1875 1025 1825 1025
Wire Wire Line
	1825 1125 1875 1125
Wire Wire Line
	1425 1125 1375 1125
Wire Wire Line
	1375 1025 1425 1025
Wire Wire Line
	3450 925  3400 925 
Wire Wire Line
	3450 1025 3400 1025
Wire Wire Line
	3850 1125 3900 1125
Wire Wire Line
	3900 1025 3850 1025
Wire Wire Line
	3850 925  3900 925 
Wire Wire Line
	3900 825  3850 825 
$Comp
L open-automation:GND #PWR0108
U 1 1 5B1DA0DF
P 1875 825
F 0 "#PWR0108" H 1875 575 50  0001 C CNN
F 1 "GND" V 1850 650 50  0000 C CNN
F 2 "" H 1875 825 50  0000 C CNN
F 3 "" H 1875 825 50  0000 C CNN
	1    1875 825 
	0    -1   -1   0   
$EndComp
Text HLabel 1375 925  0    50   Input ~ 0
SOFT_POWER
Wire Wire Line
	1425 925  1375 925 
$Comp
L open-automation:GND #PWR0112
U 1 1 5B15E1E7
P 3225 2050
F 0 "#PWR0112" H 3225 1800 50  0001 C CNN
F 1 "GND" H 3225 1900 50  0000 C CNN
F 2 "" H 3225 2050 50  0000 C CNN
F 3 "" H 3225 2050 50  0000 C CNN
	1    3225 2050
	-1   0    0    1   
$EndComp
$Comp
L open-automation:GND #PWR0113
U 1 1 5B15E1E8
P 3825 2050
F 0 "#PWR0113" H 3825 1800 50  0001 C CNN
F 1 "GND" H 3825 1900 50  0000 C CNN
F 2 "" H 3825 2050 50  0000 C CNN
F 3 "" H 3825 2050 50  0000 C CNN
	1    3825 2050
	-1   0    0    1   
$EndComp
Wire Wire Line
	3825 2100 3825 2050
Wire Wire Line
	3775 2100 3825 2100
Wire Wire Line
	3225 2100 3275 2100
Wire Wire Line
	3225 2050 3225 2100
Wire Wire Line
	3775 2200 3875 2200
Wire Wire Line
	3875 2400 3775 2400
Wire Wire Line
	3775 2300 3875 2300
Wire Wire Line
	3175 2200 3275 2200
Wire Wire Line
	3175 2400 3275 2400
Wire Wire Line
	3175 2300 3275 2300
Wire Wire Line
	3175 2500 3275 2500
Wire Wire Line
	3775 2500 3875 2500
Text HLabel 3175 2200 0    60   Input ~ 0
3v3
Text HLabel 3175 2300 0    60   Input ~ 0
PROG
Text HLabel 3175 2400 0    60   Input ~ 0
FACTORY
Text HLabel 3875 2500 2    60   Input ~ 0
SCL
Text HLabel 3875 2200 2    60   Input ~ 0
5V
Text HLabel 3875 2300 2    60   Input ~ 0
TXD
Text HLabel 3875 2400 2    60   Input ~ 0
RXD
Text HLabel 3175 2500 0    60   Input ~ 0
SDA
$Comp
L Connector_Generic:Conn_02x06_Odd_Even J4
U 1 1 5B15E1F0
P 3475 2300
F 0 "J4" H 3520 2610 50  0000 C CNN
F 1 "Conn_02x06_Odd_Even" H 3525 1850 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x06_P2.54mm_Vertical" H 3475 2300 50  0001 C CNN
F 3 "~" H 3475 2300 50  0001 C CNN
	1    3475 2300
	1    0    0    -1  
$EndComp
Text HLabel 3150 2600 0    50   Output ~ 0
SOFT_POWER
Wire Wire Line
	3150 2600 3275 2600
$Comp
L open-automation:molex-2x4-5.7mm U14
U 1 1 5B1795E3
P 3650 975
F 0 "U14" H 3650 1350 50  0000 C CNN
F 1 "molex-2x4-5.7mm" H 3650 1259 50  0000 C CNN
F 2 "open-automation:molex-2x4-5.7mm" H 3650 575 50  0001 C CNN
F 3 "https://www.digikey.com/products/en?keywords=768290008" H 3650 675 50  0001 C CNN
F 4 "0768290008" H 3650 725 50  0001 C CNN "Part Number"
	1    3650 975 
	1    0    0    -1  
$EndComp
Wire Wire Line
	3400 1125 3450 1125
Wire Wire Line
	3400 825  3450 825 
$EndSCHEMATC
