EESchema Schematic File Version 4
LIBS:liger-cache
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 9 8
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L liger-rescue:SW_PUSH REBOOT1
U 1 1 59092D1B
P 5525 3900
F 0 "REBOOT1" V 5750 4075 50  0000 C CNN
F 1 "SW_PUSH" H 5525 3820 50  0000 C CNN
F 2 "Button_Switch_SMD:SW_SPST_KMR2" H 5525 3900 50  0001 C CNN
F 3 "http://www.mouser.com/ds/2/60/KMR2_9aug12-25127.pdf" H 5525 3900 50  0001 C CNN
F 4 "KMR211NG LFS" H 5525 3900 60  0001 C CNN "Part Number"
	1    5525 3900
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR019
U 1 1 59092D1C
P 5525 4275
F 0 "#PWR019" H 5525 4025 50  0001 C CNN
F 1 "GND" H 5525 4125 50  0000 C CNN
F 2 "" H 5525 4275 50  0000 C CNN
F 3 "" H 5525 4275 50  0000 C CNN
	1    5525 4275
	1    0    0    -1  
$EndComp
$Comp
L liger-rescue:soft-button-4x U12
U 1 1 597D230D
P 7825 3325
F 0 "U12" H 8175 3575 60  0000 C CNN
F 1 "soft-button-4x" H 8275 2875 60  0000 C CNN
F 2 "open-automation:soft-button-4" H 7825 3325 60  0001 C CNN
F 3 "" H 7825 3325 60  0001 C CNN
	1    7825 3325
	1    0    0    -1  
$EndComp
Wire Wire Line
	5525 2500 5525 2650
Connection ~ 5525 3500
Wire Wire Line
	5525 3400 5525 3500
Wire Wire Line
	5525 2950 5525 3100
Wire Wire Line
	7925 2875 7925 2925
Wire Wire Line
	7375 3425 7425 3425
Wire Wire Line
	7925 3925 7925 3975
Wire Wire Line
	8475 3425 8425 3425
Wire Wire Line
	5775 3500 5525 3500
$Comp
L liger-rescue:SW_PUSH PROG1
U 1 1 59CA58CF
P 4625 3900
F 0 "PROG1" V 4850 4100 50  0000 C CNN
F 1 "SW_PUSH" H 4625 3820 50  0000 C CNN
F 2 "Button_Switch_SMD:SW_SPST_KMR2" H 4625 3900 50  0001 C CNN
F 3 "http://www.mouser.com/ds/2/60/KMR2_9aug12-25127.pdf" H 4625 3900 50  0001 C CNN
F 4 "KMR211NG LFS" H 4625 3900 60  0001 C CNN "Part Number"
	1    4625 3900
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR020
U 1 1 59CA58D6
P 4625 4275
F 0 "#PWR020" H 4625 4025 50  0001 C CNN
F 1 "GND" H 4625 4125 50  0000 C CNN
F 2 "" H 4625 4275 50  0000 C CNN
F 3 "" H 4625 4275 50  0000 C CNN
	1    4625 4275
	1    0    0    -1  
$EndComp
Wire Wire Line
	4625 2500 4625 2650
Connection ~ 4625 3500
Wire Wire Line
	4625 3400 4625 3500
Wire Wire Line
	4625 2950 4625 3100
Wire Wire Line
	4875 3500 4625 3500
Wire Wire Line
	4625 4200 4625 4275
Wire Wire Line
	5525 4200 5525 4275
$Comp
L liger-rescue:SW_PUSH FACTORY1
U 1 1 59CA6153
P 3575 3900
F 0 "FACTORY1" V 3800 4100 50  0000 C CNN
F 1 "SW_PUSH" H 3575 3820 50  0000 C CNN
F 2 "Button_Switch_SMD:SW_SPST_KMR2" H 3575 3900 50  0001 C CNN
F 3 "http://www.mouser.com/ds/2/60/KMR2_9aug12-25127.pdf" H 3575 3900 50  0001 C CNN
F 4 "KMR211NG LFS" H 3575 3900 60  0001 C CNN "Part Number"
	1    3575 3900
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR021
U 1 1 59CA6159
P 3575 4275
F 0 "#PWR021" H 3575 4025 50  0001 C CNN
F 1 "GND" H 3575 4125 50  0000 C CNN
F 2 "" H 3575 4275 50  0000 C CNN
F 3 "" H 3575 4275 50  0000 C CNN
	1    3575 4275
	1    0    0    -1  
$EndComp
Wire Wire Line
	3575 2500 3575 2650
Connection ~ 3575 3500
Wire Wire Line
	3575 3400 3575 3500
Wire Wire Line
	3575 2950 3575 3100
Wire Wire Line
	3825 3500 3575 3500
Wire Wire Line
	3575 4200 3575 4275
$Comp
L power:GND #PWR022
U 1 1 5A16D01B
P 4050 5200
F 0 "#PWR022" H 4050 4950 50  0001 C CNN
F 1 "GND" H 4050 5050 50  0000 C CNN
F 2 "" H 4050 5200 50  0000 C CNN
F 3 "" H 4050 5200 50  0000 C CNN
	1    4050 5200
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR023
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
$Comp
L liger-rescue:Conn_02x08_Odd_Even J1
U 1 1 5A16D01F
P 4300 5550
F 0 "J1" H 4350 5950 50  0000 C CNN
F 1 "Conn_02x08_Odd_Even" H 4350 5050 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x08_P2.54mm_Vertical" H 4300 5550 50  0001 C CNN
F 3 "" H 4300 5550 50  0001 C CNN
	1    4300 5550
	1    0    0    -1  
$EndComp
Wire Wire Line
	4050 5200 4050 5250
Wire Wire Line
	4050 5250 4100 5250
Wire Wire Line
	4600 5250 4650 5250
Wire Wire Line
	4650 5250 4650 5200
Wire Wire Line
	5525 3500 5525 3600
Wire Wire Line
	4625 3500 4625 3600
Wire Wire Line
	3575 3500 3575 3600
Text HLabel 4050 5450 0    60   Input ~ 0
V_BAT
Text HLabel 4050 5550 0    60   Input ~ 0
3V3
Text HLabel 4050 5650 0    60   Input ~ 0
ZERO_DETECT
Text HLabel 4650 5850 2    60   Output ~ 0
RELAY_2
Text HLabel 4050 5950 0    60   Input ~ 0
PROG
Text HLabel 4650 5350 2    60   Input ~ 0
V_USB
Text HLabel 4650 5450 2    60   Output ~ 0
TXD
Text HLabel 4650 5550 2    60   Input ~ 0
RXD
Text HLabel 4650 5750 2    60   Output ~ 0
RELAY_3
Text HLabel 4650 5950 2    60   Output ~ 0
RELAY_1
Text HLabel 6775 5475 0    60   Input ~ 0
BUTTON_LED
Text HLabel 3575 2500 1    60   Input ~ 0
3V3
Text HLabel 4625 2500 1    60   Input ~ 0
3V3
Text HLabel 5525 2500 1    60   Input ~ 0
3V3
Text HLabel 7375 3425 0    60   Input ~ 0
T_LEFT
Text HLabel 7925 2875 1    60   Input ~ 0
T_UP
Text HLabel 8475 3425 2    60   Input ~ 0
T_RIGHT
Text HLabel 7925 3975 3    60   Input ~ 0
T_DOWN
Text HLabel 4875 3500 2    60   Input ~ 0
PROG
Text HLabel 3825 3500 2    60   Input ~ 0
FACTORY_RST
Text HLabel 5775 3500 2    60   Input ~ 0
EN
Wire Wire Line
	6775 5475 6975 5475
Wire Wire Line
	7275 5775 7275 5875
$Comp
L open-automation:GND #PWR0101
U 1 1 5B2D0F5E
P 7275 5875
F 0 "#PWR0101" H 7275 5625 50  0001 C CNN
F 1 "GND" H 7280 5702 50  0000 C CNN
F 2 "" H 7275 5875 50  0000 C CNN
F 3 "" H 7275 5875 50  0000 C CNN
	1    7275 5875
	1    0    0    -1  
$EndComp
Text HLabel 7275 4975 1    60   Input ~ 0
V_IN
$Comp
L open-automation:GND #PWR0102
U 1 1 5B2DB9CA
P 7700 5100
F 0 "#PWR0102" H 7700 4850 50  0001 C CNN
F 1 "GND" V 7650 4950 50  0000 C CNN
F 2 "" H 7700 5100 50  0000 C CNN
F 3 "" H 7700 5100 50  0000 C CNN
	1    7700 5100
	0    -1   -1   0   
$EndComp
$Comp
L open-automation:C_1uF C12
U 1 1 5B2DBE58
P 7450 5100
F 0 "C12" V 7500 5225 50  0000 C CNN
F 1 "C_1uF" V 7400 5250 50  0000 C CNN
F 2 "Capacitor_SMD:C_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 7550 5400 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/tdk-corporation/CGA3E1X7R1C105M080AC/445-12540-1-ND/3954206" H 7450 4850 50  0001 C CNN
F 4 "CL10B105KP8NNNC" H 7575 5300 60  0001 C CNN "Part Number"
	1    7450 5100
	0    1    1    0   
$EndComp
$Comp
L open-automation:R_1k R15
U 1 1 5B317B7D
P 4625 3250
F 0 "R15" H 4695 3296 50  0000 L CNN
F 1 "R_1k" H 4695 3205 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 4555 3250 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/panasonic-electronic-components/ERJ-3GEYJ102V/P1.0KGDKR-ND/577615" V 4705 3250 50  0001 C CNN
F 4 "ESR03EZPJ102" V 4805 3350 60  0001 C CNN "Part Number"
	1    4625 3250
	1    0    0    -1  
$EndComp
$Comp
L open-automation:R_1k R16
U 1 1 5B317C71
P 5525 3250
F 0 "R16" H 5595 3296 50  0000 L CNN
F 1 "R_1k" H 5595 3205 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 5455 3250 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/panasonic-electronic-components/ERJ-3GEYJ102V/P1.0KGDKR-ND/577615" V 5605 3250 50  0001 C CNN
F 4 "ESR03EZPJ102" V 5705 3350 60  0001 C CNN "Part Number"
	1    5525 3250
	1    0    0    -1  
$EndComp
$Comp
L open-automation:R_1k R14
U 1 1 5B317D10
P 3575 3250
F 0 "R14" H 3645 3296 50  0000 L CNN
F 1 "R_1k" H 3645 3205 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 3505 3250 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/panasonic-electronic-components/ERJ-3GEYJ102V/P1.0KGDKR-ND/577615" V 3655 3250 50  0001 C CNN
F 4 "ESR03EZPJ102" V 3755 3350 60  0001 C CNN "Part Number"
	1    3575 3250
	1    0    0    -1  
$EndComp
$Comp
L open-automation:LED-0603-G D2
U 1 1 5B31D656
P 3575 2800
F 0 "D2" V 3613 2683 50  0000 R CNN
F 1 "LED-0603-G" V 3522 2683 50  0000 R CNN
F 2 "LED_SMD:LED_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 3475 2800 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/rohm-semiconductor/SML-D12M8WT86/511-1578-1-ND/1641810" H 3575 2900 50  0001 C CNN
F 4 "SML-D12M8WT86" H 3675 3000 50  0001 C CNN "Part Number"
	1    3575 2800
	0    -1   -1   0   
$EndComp
$Comp
L open-automation:LED-0603-G D4
U 1 1 5B31D7FF
P 4625 2800
F 0 "D4" V 4663 2683 50  0000 R CNN
F 1 "LED-0603-G" V 4572 2683 50  0000 R CNN
F 2 "LED_SMD:LED_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 4525 2800 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/rohm-semiconductor/SML-D12M8WT86/511-1578-1-ND/1641810" H 4625 2900 50  0001 C CNN
F 4 "SML-D12M8WT86" H 4725 3000 50  0001 C CNN "Part Number"
	1    4625 2800
	0    -1   -1   0   
$EndComp
$Comp
L open-automation:LED-0603-G D5
U 1 1 5B31D8E3
P 5525 2800
F 0 "D5" V 5563 2683 50  0000 R CNN
F 1 "LED-0603-G" V 5472 2683 50  0000 R CNN
F 2 "LED_SMD:LED_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 5425 2800 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/rohm-semiconductor/SML-D12M8WT86/511-1578-1-ND/1641810" H 5525 2900 50  0001 C CNN
F 4 "SML-D12M8WT86" H 5625 3000 50  0001 C CNN "Part Number"
	1    5525 2800
	0    -1   -1   0   
$EndComp
Wire Wire Line
	7275 5100 7275 5175
Connection ~ 7275 5100
Wire Wire Line
	7300 5100 7275 5100
Wire Wire Line
	7600 5100 7700 5100
$Comp
L LED:WS2812B D7
U 1 1 5B7E61EB
P 7275 5475
F 0 "D7" H 7616 5521 50  0000 L CNN
F 1 "WS2812B" H 7616 5430 50  0000 L CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 7325 5175 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 7375 5100 50  0001 L TNN
	1    7275 5475
	1    0    0    -1  
$EndComp
Wire Wire Line
	7275 4975 7275 5100
$Comp
L open-automation:housing-mounts U7
U 1 1 5BBF1815
P 9275 2450
F 0 "U7" H 9553 2503 60  0000 L CNN
F 1 "housing-mounts" H 9553 2397 60  0000 L CNN
F 2 "open-automation:generic-housing-mounts-md" H 9475 2250 60  0001 C CNN
F 3 "" H 9275 2500 60  0001 C CNN
	1    9275 2450
	1    0    0    -1  
$EndComp
$Comp
L open-automation:GND #PWR0104
U 1 1 5BBF1873
P 8900 2475
F 0 "#PWR0104" H 8900 2225 50  0001 C CNN
F 1 "GND" H 8905 2302 50  0000 C CNN
F 2 "" H 8900 2475 50  0000 C CNN
F 3 "" H 8900 2475 50  0000 C CNN
	1    8900 2475
	1    0    0    -1  
$EndComp
Wire Wire Line
	8900 2475 8900 2450
Wire Wire Line
	8900 2450 8925 2450
Text HLabel 4650 5650 2    60   Output ~ 0
RELAY_4
Wire Wire Line
	4650 5650 4600 5650
Wire Wire Line
	4650 5550 4600 5550
Wire Wire Line
	4600 5450 4650 5450
Wire Wire Line
	4650 5350 4600 5350
Wire Wire Line
	4600 5750 4650 5750
Wire Wire Line
	4650 5850 4600 5850
Wire Wire Line
	4600 5950 4650 5950
Wire Wire Line
	4100 5350 4050 5350
Wire Wire Line
	4050 5350 4050 5250
Connection ~ 4050 5250
Wire Wire Line
	4050 5450 4100 5450
Wire Wire Line
	4100 5550 4050 5550
Wire Wire Line
	4050 5650 4100 5650
Wire Wire Line
	4100 5750 4050 5750
Wire Wire Line
	4050 5850 4100 5850
Wire Wire Line
	4100 5950 4050 5950
Text HLabel 4050 5850 0    60   Input ~ 0
I2S_BCLK
Text HLabel 4050 5750 0    60   Input ~ 0
I2S_WS
$EndSCHEMATC
