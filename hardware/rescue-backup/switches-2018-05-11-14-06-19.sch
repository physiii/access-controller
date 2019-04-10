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
Sheet 6 9
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text GLabel 7275 4925 0    60   Input ~ 0
LED_R
$Comp
L SW_PUSH REBOOT1
U 1 1 59092D1B
P 5350 3900
F 0 "REBOOT1" V 5575 4075 50  0000 C CNN
F 1 "SW_PUSH" H 5350 3820 50  0000 C CNN
F 2 "Buttons_Switches_SMD:SW_SPST_KMR2" H 5350 3900 50  0001 C CNN
F 3 "http://www.mouser.com/ds/2/60/KMR2_9aug12-25127.pdf" H 5350 3900 50  0001 C CNN
F 4 "KMR211NG LFS" H 5350 3900 60  0001 C CNN "Part Number"
	1    5350 3900
	0    1    1    0   
$EndComp
Text GLabel 4700 3500 2    60   Input ~ 0
PROG
Text GLabel 5600 3500 2    60   Input ~ 0
EN
$Comp
L GND #PWR019
U 1 1 59092D1C
P 5350 4275
F 0 "#PWR019" H 5350 4025 50  0001 C CNN
F 1 "GND" H 5350 4125 50  0000 C CNN
F 2 "" H 5350 4275 50  0000 C CNN
F 3 "" H 5350 4275 50  0000 C CNN
	1    5350 4275
	1    0    0    -1  
$EndComp
$Comp
L LED D2
U 1 1 59092D24
P 5350 2800
F 0 "D2" H 5350 2900 50  0000 C CNN
F 1 "LED" V 5475 2725 50  0000 C CNN
F 2 "LEDs:LED_0603_HandSoldering" H 5350 2800 50  0001 C CNN
F 3 "http://www.osram-os.com/Graphics/XPic9/00128792_0.pdf" H 5350 2800 50  0001 C CNN
F 4 "475-2512-2-ND" H 5350 2800 60  0001 C CNN "Part Number"
	1    5350 2800
	0    -1   -1   0   
$EndComp
Text GLabel 5350 2500 1    60   Input ~ 0
3V3
$Comp
L soft-button-4x U12
U 1 1 597D230D
P 7825 3325
F 0 "U12" H 8175 3575 60  0000 C CNN
F 1 "soft-button-4x" H 8275 2875 60  0000 C CNN
F 2 "open-automation:soft-button-4" H 7825 3325 60  0001 C CNN
F 3 "" H 7825 3325 60  0001 C CNN
	1    7825 3325
	1    0    0    -1  
$EndComp
Text GLabel 7375 3425 0    60   Input ~ 0
T_LEFT
Text GLabel 8475 3425 2    60   Input ~ 0
T_RIGHT
Text GLabel 7925 2875 1    60   Input ~ 0
T_UP
Text GLabel 7925 3975 3    60   Input ~ 0
T_DOWN
$Comp
L RGB-LED U7
U 1 1 5980E7D1
P 7875 4925
F 0 "U7" H 8125 4625 60  0000 C CNN
F 1 "RGB-LED" H 7875 5225 60  0000 C CNN
F 2 "open-automation:RGB-LED-J-Lead-SMD-6" H 8025 4925 60  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/osram-opto-semiconductors-inc/LTRBGFSF-ABCB-QKYO-Z/475-2899-6-ND/2415092" H 8025 4925 60  0001 C CNN
F 4 "LTRBGFSF-ABCB-QKYO-Z" H 7875 4925 60  0001 C CNN "Part Number"
	1    7875 4925
	1    0    0    -1  
$EndComp
Wire Wire Line
	5350 2500 5350 2650
Connection ~ 5350 3500
Wire Wire Line
	5350 3400 5350 3600
Wire Wire Line
	5350 2950 5350 3100
Wire Wire Line
	7925 2875 7925 2925
Wire Wire Line
	7375 3425 7425 3425
Wire Wire Line
	7925 3925 7925 3975
Wire Wire Line
	8475 3425 8425 3425
Text GLabel 7275 5075 0    60   Input ~ 0
LED_G
Text GLabel 7275 4775 0    60   Input ~ 0
LED_B
Wire Wire Line
	7275 4775 7375 4775
Wire Wire Line
	7275 4925 7375 4925
Wire Wire Line
	7275 5075 7375 5075
$Comp
L R R8
U 1 1 5980F245
P 8625 4775
F 0 "R8" V 8705 4775 50  0000 C CNN
F 1 "36" V 8625 4775 50  0000 C CNN
F 2 "Resistors_SMD:R_0603_HandSoldering" V 8555 4775 50  0001 C CNN
F 3 "http://www.yageo.com.tw/exep/pages/download/literatures/PYu-R_INT-thick_7.pdf" H 8625 4775 50  0001 C CNN
F 4 "RC0603JR-0736RL" V 8625 4775 60  0001 C CNN "Part Number"
	1    8625 4775
	0    1    1    0   
$EndComp
Text GLabel 8875 4675 1    60   Input ~ 0
3V3
$Comp
L R R12
U 1 1 5980F487
P 8625 4925
F 0 "R12" V 8705 4925 50  0000 C CNN
F 1 "36" V 8625 4925 50  0000 C CNN
F 2 "Resistors_SMD:R_0603_HandSoldering" V 8555 4925 50  0001 C CNN
F 3 "http://www.yageo.com.tw/exep/pages/download/literatures/PYu-R_INT-thick_7.pdf" H 8625 4925 50  0001 C CNN
F 4 "RC0603JR-0736RL" V 8625 4925 60  0001 C CNN "Part Number"
	1    8625 4925
	0    1    1    0   
$EndComp
$Comp
L R R13
U 1 1 5980F4D4
P 8625 5075
F 0 "R13" V 8705 5075 50  0000 C CNN
F 1 "36" V 8625 5075 50  0000 C CNN
F 2 "Resistors_SMD:R_0603_HandSoldering" V 8555 5075 50  0001 C CNN
F 3 "http://www.yageo.com.tw/exep/pages/download/literatures/PYu-R_INT-thick_7.pdf" H 8625 5075 50  0001 C CNN
F 4 "RC0603JR-0736RL" V 8625 5075 60  0001 C CNN "Part Number"
	1    8625 5075
	0    1    1    0   
$EndComp
Wire Wire Line
	8375 4775 8475 4775
Wire Wire Line
	8375 4925 8475 4925
Wire Wire Line
	8375 5075 8475 5075
Wire Wire Line
	8875 4675 8875 5075
Wire Wire Line
	8875 4775 8775 4775
Wire Wire Line
	8875 4925 8775 4925
Connection ~ 8875 4775
Wire Wire Line
	8875 5075 8775 5075
Connection ~ 8875 4925
$Comp
L R_1k R15
U 1 1 59BA2DC7
P 5350 3250
F 0 "R15" V 5350 3250 50  0000 C CNN
F 1 "R_1k" V 5275 3250 50  0000 C CNN
F 2 "Resistors_SMD:R_0603_HandSoldering" V 5280 3250 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/yageo/RC0603JR-071KL/311-1.0KGRCT-ND/729624" V 5430 3250 50  0001 C CNN
F 4 "RC0603JR-071KL" V 5530 3350 60  0001 C CNN "Part Number"
	1    5350 3250
	-1   0    0    1   
$EndComp
Wire Wire Line
	5600 3500 5350 3500
$Comp
L SW_PUSH PROG1
U 1 1 59CA58CF
P 4450 3900
F 0 "PROG1" V 4675 4100 50  0000 C CNN
F 1 "SW_PUSH" H 4450 3820 50  0000 C CNN
F 2 "Buttons_Switches_SMD:SW_SPST_KMR2" H 4450 3900 50  0001 C CNN
F 3 "http://www.mouser.com/ds/2/60/KMR2_9aug12-25127.pdf" H 4450 3900 50  0001 C CNN
F 4 "KMR211NG LFS" H 4450 3900 60  0001 C CNN "Part Number"
	1    4450 3900
	0    1    1    0   
$EndComp
$Comp
L GND #PWR020
U 1 1 59CA58D6
P 4450 4275
F 0 "#PWR020" H 4450 4025 50  0001 C CNN
F 1 "GND" H 4450 4125 50  0000 C CNN
F 2 "" H 4450 4275 50  0000 C CNN
F 3 "" H 4450 4275 50  0000 C CNN
	1    4450 4275
	1    0    0    -1  
$EndComp
$Comp
L LED D5
U 1 1 59CA58DD
P 4450 2800
F 0 "D5" H 4450 2900 50  0000 C CNN
F 1 "LED" V 4575 2725 50  0000 C CNN
F 2 "LEDs:LED_0603_HandSoldering" H 4450 2800 50  0001 C CNN
F 3 "http://www.osram-os.com/Graphics/XPic9/00128792_0.pdf" H 4450 2800 50  0001 C CNN
F 4 "475-2512-2-ND" H 4450 2800 60  0001 C CNN "Part Number"
	1    4450 2800
	0    -1   -1   0   
$EndComp
Text GLabel 4450 2500 1    60   Input ~ 0
3V3
Wire Wire Line
	4450 2500 4450 2650
Connection ~ 4450 3500
Wire Wire Line
	4450 3400 4450 3600
Wire Wire Line
	4450 2950 4450 3100
$Comp
L R_1k R16
U 1 1 59CA58E9
P 4450 3250
F 0 "R16" V 4450 3250 50  0000 C CNN
F 1 "R_1k" V 4375 3250 50  0000 C CNN
F 2 "Resistors_SMD:R_0603_HandSoldering" V 4380 3250 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/yageo/RC0603JR-071KL/311-1.0KGRCT-ND/729624" V 4530 3250 50  0001 C CNN
F 4 "RC0603JR-071KL" V 4630 3350 60  0001 C CNN "Part Number"
	1    4450 3250
	-1   0    0    1   
$EndComp
Wire Wire Line
	4700 3500 4450 3500
Wire Wire Line
	4450 4200 4450 4275
Wire Wire Line
	5350 4200 5350 4275
Text GLabel 3825 3500 2    60   Input ~ 0
FACTORY
$Comp
L SW_PUSH FACTORY1
U 1 1 59CA6153
P 3575 3900
F 0 "FACTORY1" V 3800 4100 50  0000 C CNN
F 1 "SW_PUSH" H 3575 3820 50  0000 C CNN
F 2 "Buttons_Switches_SMD:SW_SPST_KMR2" H 3575 3900 50  0001 C CNN
F 3 "http://www.mouser.com/ds/2/60/KMR2_9aug12-25127.pdf" H 3575 3900 50  0001 C CNN
F 4 "KMR211NG LFS" H 3575 3900 60  0001 C CNN "Part Number"
	1    3575 3900
	0    1    1    0   
$EndComp
$Comp
L GND #PWR021
U 1 1 59CA6159
P 3575 4275
F 0 "#PWR021" H 3575 4025 50  0001 C CNN
F 1 "GND" H 3575 4125 50  0000 C CNN
F 2 "" H 3575 4275 50  0000 C CNN
F 3 "" H 3575 4275 50  0000 C CNN
	1    3575 4275
	1    0    0    -1  
$EndComp
$Comp
L LED D4
U 1 1 59CA6160
P 3575 2800
F 0 "D4" H 3575 2900 50  0000 C CNN
F 1 "LED" V 3700 2725 50  0000 C CNN
F 2 "LEDs:LED_0603_HandSoldering" H 3575 2800 50  0001 C CNN
F 3 "http://www.osram-os.com/Graphics/XPic9/00128792_0.pdf" H 3575 2800 50  0001 C CNN
F 4 "475-2512-2-ND" H 3575 2800 60  0001 C CNN "Part Number"
	1    3575 2800
	0    -1   -1   0   
$EndComp
Text GLabel 3575 2500 1    60   Input ~ 0
3V3
Wire Wire Line
	3575 2500 3575 2650
Connection ~ 3575 3500
Wire Wire Line
	3575 3400 3575 3600
Wire Wire Line
	3575 2950 3575 3100
$Comp
L R_1k R14
U 1 1 59CA616C
P 3575 3250
F 0 "R14" V 3575 3250 50  0000 C CNN
F 1 "R_1k" V 3500 3250 50  0000 C CNN
F 2 "Resistors_SMD:R_0603_HandSoldering" V 3505 3250 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/yageo/RC0603JR-071KL/311-1.0KGRCT-ND/729624" V 3655 3250 50  0001 C CNN
F 4 "RC0603JR-071KL" V 3755 3350 60  0001 C CNN "Part Number"
	1    3575 3250
	-1   0    0    1   
$EndComp
Wire Wire Line
	3825 3500 3575 3500
Wire Wire Line
	3575 4200 3575 4275
$Comp
L GND #PWR022
U 1 1 5A16D01B
P 4050 5200
F 0 "#PWR022" H 4050 4950 50  0001 C CNN
F 1 "GND" H 4050 5050 50  0000 C CNN
F 2 "" H 4050 5200 50  0000 C CNN
F 3 "" H 4050 5200 50  0000 C CNN
	1    4050 5200
	-1   0    0    1   
$EndComp
Text GLabel 4000 5450 0    60   Input ~ 0
3V3
Text GLabel 4700 5550 2    60   Input ~ 0
RXD
Text GLabel 4700 5450 2    60   Input ~ 0
TXD
Text GLabel 4700 5350 2    60   Input ~ 0
V_USB
$Comp
L GND #PWR023
U 1 1 5A16D01C
P 4650 5200
F 0 "#PWR023" H 4650 4950 50  0001 C CNN
F 1 "GND" H 4650 5050 50  0000 C CNN
F 2 "" H 4650 5200 50  0000 C CNN
F 3 "" H 4650 5200 50  0000 C CNN
	1    4650 5200
	-1   0    0    1   
$EndComp
Text Label 4800 1075 0    197  ~ 0
INTERFACE
Text GLabel 4000 5950 0    60   Input ~ 0
PROG
Text GLabel 4000 5550 0    60   Input ~ 0
FACTORY
Text GLabel 4000 5650 0    60   Input ~ 0
SCL
Text GLabel 4700 5650 2    60   Input ~ 0
SDA
Text GLabel 4000 5850 0    60   Input ~ 0
I2S_BCLK
Text GLabel 4700 5750 2    60   Input ~ 0
I2S_DATA_IN
Text GLabel 4000 5750 0    60   Input ~ 0
I2S_WS
Text GLabel 4700 5850 2    60   Input ~ 0
I2S_DATA_OUT
$Comp
L Conn_02x08_Odd_Even J1
U 1 1 5A16D01F
P 4300 5550
F 0 "J1" H 4350 5950 50  0000 C CNN
F 1 "Conn_02x08_Odd_Even" H 4350 5050 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_2x08_Pitch2.54mm" H 4300 5550 50  0001 C CNN
F 3 "" H 4300 5550 50  0001 C CNN
	1    4300 5550
	1    0    0    -1  
$EndComp
Text GLabel 4700 5950 2    60   Input ~ 0
RELAY
Text GLabel 4000 5350 0    60   Input ~ 0
V_BAT
Wire Wire Line
	4600 5950 4700 5950
Wire Wire Line
	4000 5950 4100 5950
Wire Wire Line
	4600 5850 4700 5850
Wire Wire Line
	4000 5750 4100 5750
Wire Wire Line
	4600 5750 4700 5750
Wire Wire Line
	4000 5850 4100 5850
Wire Wire Line
	4600 5650 4700 5650
Wire Wire Line
	4000 5650 4100 5650
Wire Wire Line
	4000 5450 4100 5450
Wire Wire Line
	4000 5550 4100 5550
Wire Wire Line
	4000 5350 4100 5350
Wire Wire Line
	4600 5450 4700 5450
Wire Wire Line
	4700 5550 4600 5550
Wire Wire Line
	4600 5350 4700 5350
Wire Wire Line
	4050 5200 4050 5250
Wire Wire Line
	4050 5250 4100 5250
Wire Wire Line
	4600 5250 4650 5250
Wire Wire Line
	4650 5250 4650 5200
$EndSCHEMATC
