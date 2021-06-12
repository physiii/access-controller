EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 9 12
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text HLabel 950  950  0    60   Input ~ 0
RXD
Text HLabel 950  850  0    60   Input ~ 0
TXD
Text HLabel 950  1150 0    60   Input ~ 0
ESP_EN
Text HLabel 950  1050 0    50   Input ~ 0
5V
Wire Wire Line
	950  950  1000 950 
Wire Wire Line
	1000 850  950  850 
Text Label 1000 850  0    50   ~ 0
USB_TXD
Text Label 1000 950  0    50   ~ 0
USB_RXD
Wire Wire Line
	950  1050 1000 1050
Text Label 1000 1050 0    50   ~ 0
5V
Text Label 1000 1150 0    50   ~ 0
ESP_EN
Wire Wire Line
	1000 1150 950  1150
Text HLabel 950  1250 0    60   Input ~ 0
PROG
Text Label 1000 1250 0    50   ~ 0
PROG
Wire Wire Line
	1000 1250 950  1250
Text Label 8450 3100 0    50   ~ 10
GPIO1\U0TXD
Text Label 8450 3000 0    50   ~ 10
GPIO3\U0RXD
Text Label 8450 3200 0    50   ~ 10
ESP_EN
Text Label 8450 4400 0    50   ~ 10
GPIO0
Text Notes 8425 3475 0    60   ~ 12
Auto program\n
Text Notes 8250 3600 0    60   ~ 12
DTR  RTS->EN  IO0
Text Notes 8325 3700 0    60   ~ 12
1
Text Notes 8575 3700 0    60   ~ 12
1
Text Notes 8825 3700 0    60   ~ 12
1
Text Notes 9050 3700 0    60   ~ 12
1
Text Notes 8325 3800 0    60   ~ 12
0
Text Notes 8575 3800 0    60   ~ 12
0
Text Notes 8825 3800 0    60   ~ 12
1
Text Notes 9050 3800 0    60   ~ 12
1
Text Notes 8325 3900 0    60   ~ 12
1
Text Notes 8575 3900 0    60   ~ 12
0
Text Notes 8825 3900 0    60   ~ 12
0
Text Notes 9050 3900 0    60   ~ 12
1
Text Notes 8325 4000 0    60   ~ 12
0
Text Notes 8575 4000 0    60   ~ 12
1
Text Notes 8825 4000 0    60   ~ 12
1
Text Notes 9050 4000 0    60   ~ 12
0
Text Label 8450 5000 0    50   ~ 10
GPIO2\HS2_DATA0
NoConn ~ 5350 3400
NoConn ~ 5350 3600
NoConn ~ 5350 3700
NoConn ~ 5350 3800
NoConn ~ 5350 4000
Text Label 7650 4700 0    60   ~ 12
D_Com
Wire Wire Line
	3250 3000 3150 3000
Wire Wire Line
	3150 3300 4350 3300
Wire Wire Line
	3150 3800 2850 3800
Wire Wire Line
	2850 3800 2850 3900
Wire Wire Line
	3150 4000 2850 4000
Connection ~ 2850 4000
Wire Wire Line
	5350 3000 6150 3000
Wire Wire Line
	5350 3100 5850 3100
Wire Wire Line
	5350 3500 6050 3500
Wire Wire Line
	5350 3300 5850 3300
Wire Wire Line
	6350 3000 7250 3000
Wire Wire Line
	6350 3100 9250 3100
Wire Wire Line
	6450 3500 6550 3500
Wire Wire Line
	6450 4200 6550 4200
Wire Wire Line
	5850 4200 6150 4200
Wire Wire Line
	6850 3700 6850 3800
Wire Wire Line
	6850 3800 5850 3800
Connection ~ 5850 3800
Wire Wire Line
	6850 4000 6850 3900
Wire Wire Line
	6850 3900 6050 3900
Wire Wire Line
	6050 3900 6050 3500
Connection ~ 6050 3500
Wire Wire Line
	6850 3300 6850 3200
Wire Wire Line
	6850 3200 9250 3200
Wire Notes Line
	8250 3400 8250 4025
Wire Notes Line
	8250 3600 9200 3600
Wire Notes Line
	9200 3400 9200 4025
Wire Notes Line
	9200 3400 8250 3400
Wire Notes Line
	9200 4025 8250 4025
Wire Notes Line
	8250 3500 9200 3500
Wire Notes Line
	8475 3600 8475 4025
Wire Notes Line
	8725 3600 8725 4025
Wire Notes Line
	8975 3600 8975 4025
Wire Wire Line
	8350 4400 9250 4400
Wire Wire Line
	5850 3300 5850 3800
Wire Wire Line
	6850 4400 6850 4700
Wire Wire Line
	8350 4400 8350 4500
Wire Wire Line
	8350 4900 8350 5000
Wire Wire Line
	8350 5000 9250 5000
Wire Wire Line
	2850 4000 2850 4200
Wire Wire Line
	5850 3800 5850 4200
Wire Wire Line
	6050 3500 6150 3500
Text Notes 550  6650 0    100  Italic 20
!!!WARNING!!!\nWhen the board is powered via PoE, don't include the USB cable for to programming it!!!\nIn case this is required, USB-ISO must be used for to guarantee galvanic isolation!\nSee:   https://www.olimex.com/Products/USB-Modules/USB-ISO/
Wire Wire Line
	7150 4700 6850 4700
Wire Wire Line
	7450 4700 8150 4700
Wire Wire Line
	3950 3900 2850 3900
Connection ~ 2850 3900
Wire Wire Line
	2850 3900 2850 4000
NoConn ~ 5350 3900
Wire Wire Line
	3450 3000 4050 3000
Wire Wire Line
	2850 2600 4050 2600
Wire Wire Line
	4050 2600 4050 3000
Connection ~ 4050 3000
Wire Wire Line
	4050 3000 4350 3000
$Comp
L ESP32-PoE_Rev_E:PWR_FLAG #FLG01
U 1 1 601255AC
P 2850 2400
F 0 "#FLG01" H 2850 2495 50  0001 C CNN
F 1 "PWR_FLAG" H 2850 2600 50  0000 C CNN
F 2 "" H 2850 2400 60  0000 C CNN
F 3 "" H 2850 2400 60  0000 C CNN
	1    2850 2400
	1    0    0    -1  
$EndComp
Wire Wire Line
	2850 2400 2850 2600
Wire Wire Line
	4750 2700 4150 2700
Wire Wire Line
	4150 2700 4150 3100
Connection ~ 4150 3100
Wire Wire Line
	4150 3100 4350 3100
Wire Wire Line
	7250 2700 7250 3000
Connection ~ 7250 3000
Wire Wire Line
	7250 3000 9250 3000
Wire Wire Line
	4750 2600 4050 2600
Connection ~ 4050 2600
Wire Wire Line
	5850 2600 5850 3100
Connection ~ 5850 3100
Wire Wire Line
	5850 3100 6150 3100
Wire Wire Line
	3950 3200 4150 3200
Wire Wire Line
	4150 3200 4150 3100
Wire Wire Line
	8750 2500 8750 2700
Wire Wire Line
	8050 2700 8750 2700
Text Label 3400 2600 0    50   ~ 0
5V
Text HLabel 950  1450 0    50   Input ~ 0
3V3
Wire Wire Line
	950  1450 1000 1450
Text Label 1000 1450 0    50   ~ 0
3V3
$Comp
L ESP32-PoE_Rev_E:+3.3V #PWR080
U 1 1 60036A30
P 8750 2500
F 0 "#PWR080" H 8750 2350 50  0001 C CNN
F 1 "+3.3V" H 8765 2673 50  0000 C CNN
F 2 "" H 8750 2500 60  0000 C CNN
F 3 "" H 8750 2500 60  0000 C CNN
	1    8750 2500
	1    0    0    -1  
$EndComp
Text Label 8750 2700 0    50   ~ 0
3V3
Text Label 9250 3200 0    50   ~ 0
ESP_EN
Text Label 9250 3100 0    50   ~ 0
USB_TXD
Text Label 9250 3000 0    50   ~ 0
USB_RXD
Text Label 9250 4400 0    50   ~ 0
PROG
Text HLabel 950  1350 0    60   Input ~ 0
IO2
Text Label 1000 1350 0    50   ~ 0
IO2
Wire Wire Line
	1000 1350 950  1350
Text Label 9250 5000 0    50   ~ 0
IO2
$Comp
L Open_Automation:USB_MINI_B J18
U 1 1 603DE312
P 2350 1150
F 0 "J18" H 2350 1617 50  0000 C CNN
F 1 "USB_MINI_B" H 2350 1526 50  0000 C CNN
F 2 "Open_Automation:USB-mini-B-0548190519" H 2250 1900 50  0001 C CNN
F 3 "https://www.molex.com/pdm_docs/sd/548190519_sd.pdf" H 2250 1800 50  0001 C CNN
F 4 "https://www.digikey.com/product-detail/en/molex/0548190519/WM17115-ND/773802" H 2350 1800 50  0001 C CNN "Website"
F 5 "0548190519" H 2250 1700 50  0001 C CNN "Part Number"
	1    2350 1150
	1    0    0    -1  
$EndComp
Text Label 2700 950  0    50   ~ 0
5V
Wire Wire Line
	2700 950  2650 950 
Text Label 2700 1150 0    50   ~ 0
D-
Wire Wire Line
	2700 1150 2650 1150
Text Label 2700 1250 0    50   ~ 0
D+
Wire Wire Line
	2700 1250 2650 1250
$Comp
L power:GND #PWR?
U 1 1 604145B1
P 2850 4200
AR Path="/5ABD187E/604145B1" Ref="#PWR?"  Part="1" 
AR Path="/604145B1" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/604145B1" Ref="#PWR?"  Part="1" 
AR Path="/60DDD162/604145B1" Ref="#PWR076"  Part="1" 
F 0 "#PWR076" H 2850 3950 50  0001 C CNN
F 1 "GND" H 2850 4050 50  0000 C CNN
F 2 "" H 2850 4200 50  0001 C CNN
F 3 "" H 2850 4200 50  0001 C CNN
	1    2850 4200
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 6041598A
P 2000 1400
AR Path="/5ABD187E/6041598A" Ref="#PWR?"  Part="1" 
AR Path="/6041598A" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/6041598A" Ref="#PWR?"  Part="1" 
AR Path="/60DDD162/6041598A" Ref="#PWR075"  Part="1" 
F 0 "#PWR075" H 2000 1150 50  0001 C CNN
F 1 "GND" H 2000 1250 50  0000 C CNN
F 2 "" H 2000 1400 50  0001 C CNN
F 3 "" H 2000 1400 50  0001 C CNN
	1    2000 1400
	1    0    0    -1  
$EndComp
Wire Wire Line
	2000 1400 2000 1350
Wire Wire Line
	2000 1350 2050 1350
Text Label 4300 3500 2    50   ~ 0
D-
Wire Wire Line
	4300 3500 4350 3500
Text Label 4300 3600 2    50   ~ 0
D+
Wire Wire Line
	4300 3600 4350 3600
Text Label 3900 2600 0    60   ~ 12
+5V_USB
$Comp
L power:GND #PWR?
U 1 1 6042DDD6
P 3150 3000
AR Path="/5ABD187E/6042DDD6" Ref="#PWR?"  Part="1" 
AR Path="/6042DDD6" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/6042DDD6" Ref="#PWR?"  Part="1" 
AR Path="/60DDD162/6042DDD6" Ref="#PWR077"  Part="1" 
F 0 "#PWR077" H 3150 2750 50  0001 C CNN
F 1 "GND" V 3100 2850 50  0000 C CNN
F 2 "" H 3150 3000 50  0001 C CNN
F 3 "" H 3150 3000 50  0001 C CNN
	1    3150 3000
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR?
U 1 1 6042E609
P 3150 3100
AR Path="/5ABD187E/6042E609" Ref="#PWR?"  Part="1" 
AR Path="/6042E609" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/6042E609" Ref="#PWR?"  Part="1" 
AR Path="/60DDD162/6042E609" Ref="#PWR078"  Part="1" 
F 0 "#PWR078" H 3150 2850 50  0001 C CNN
F 1 "GND" V 3100 2950 50  0000 C CNN
F 2 "" H 3150 3100 50  0001 C CNN
F 3 "" H 3150 3100 50  0001 C CNN
	1    3150 3100
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR?
U 1 1 6042E874
P 3150 3300
AR Path="/5ABD187E/6042E874" Ref="#PWR?"  Part="1" 
AR Path="/6042E874" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/6042E874" Ref="#PWR?"  Part="1" 
AR Path="/60DDD162/6042E874" Ref="#PWR079"  Part="1" 
F 0 "#PWR079" H 3150 3050 50  0001 C CNN
F 1 "GND" V 3100 3150 50  0000 C CNN
F 2 "" H 3150 3300 50  0001 C CNN
F 3 "" H 3150 3300 50  0001 C CNN
	1    3150 3300
	0    1    1    0   
$EndComp
Wire Wire Line
	3150 3100 3300 3100
Wire Wire Line
	3600 3100 4150 3100
$Comp
L Open_Automation:C_100nF_0402 C10
U 1 1 60432E92
P 3450 3100
F 0 "C10" V 3500 3000 50  0000 C CNN
F 1 "C_100nF_0402" V 3400 3450 50  0000 C CNN
F 2 "Capacitor_SMD:C_0402_1005Metric_Pad0.74x0.62mm_HandSolder" H 3550 3400 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL05B104KO5NNNC_C1525.pdf" H 3450 2850 50  0001 C CNN
F 4 "C1525" H 3450 3500 50  0001 C CNN "LCSC"
F 5 "CL05B104KO5NNNC" H 3575 3300 60  0001 C CNN "Part Number"
	1    3450 3100
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:C_22uF C9
U 1 1 60430DBE
P 3350 3000
F 0 "C9" V 3300 3100 50  0000 C CNN
F 1 "C_22uF" V 3212 3000 50  0000 C CNN
F 2 "Capacitor_SMD:C_0603_1608Metric_Pad1.08x0.95mm_HandSolder" H 3350 3000 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL10A226MQ8NRNC_C59461.pdf" H 3350 3000 60  0001 C CNN
F 4 "CL10A226MQ8NRNC" H 3350 3000 50  0001 C CNN "Part Number"
F 5 "C59461" H 3350 3000 50  0001 C CNN "LCSC"
	1    3350 3000
	0    1    1    0   
$EndComp
$Comp
L ESP32-PoE_Rev_E:PWR_FLAG #FLG02
U 1 1 5D0EC642
P 3950 3200
F 0 "#FLG02" H 3950 3295 50  0001 C CNN
F 1 "PWR_FLAG" V 3900 3550 50  0000 C CNN
F 2 "" H 3950 3200 60  0000 C CNN
F 3 "" H 3950 3200 60  0000 C CNN
	1    3950 3200
	0    -1   -1   0   
$EndComp
$Comp
L Open_Automation:R_10K_0402 R41
U 1 1 6043D0F4
P 4950 2600
F 0 "R41" V 4950 2600 50  0000 C CNN
F 1 "R_10K_0402" V 4900 2950 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 4880 2600 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0402WGF1002TCE_C25744.pdf" V 5030 2600 50  0001 C CNN
F 4 "0402WGF1002TCE" V 5130 2700 60  0001 C CNN "Part Number"
F 5 "C25744" V 5200 2600 50  0001 C CNN "LCSC"
	1    4950 2600
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:R_10K_0402 R42
U 1 1 60446CBF
P 4950 2700
F 0 "R42" V 4950 2700 50  0000 C CNN
F 1 "R_10K_0402" V 4900 3050 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 4880 2700 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0402WGF1002TCE_C25744.pdf" V 5030 2700 50  0001 C CNN
F 4 "0402WGF1002TCE" V 5130 2800 60  0001 C CNN "Part Number"
F 5 "C25744" V 5200 2700 50  0001 C CNN "LCSC"
	1    4950 2700
	0    1    1    0   
$EndComp
Wire Wire Line
	5150 2700 7250 2700
Wire Wire Line
	5150 2600 5850 2600
$Comp
L Open_Automation:C_27pF_0402 C7
U 1 1 60450786
P 3250 3800
F 0 "C7" V 3200 3700 50  0000 C CNN
F 1 "C_27pF_0402" V 3200 4100 50  0000 C CNN
F 2 "Capacitor_SMD:C_0402_1005Metric_Pad0.74x0.62mm_HandSolder" H 3250 3800 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Guangdong-Fenghua-Advanced-Tech-0402CG270J500NTN_C1557.pdf" H 3250 3800 60  0001 C CNN
F 4 "0402CG270J500NTN" H 3250 3800 50  0001 C CNN "Part Number"
F 5 "C1557" H 3250 3800 50  0001 C CNN "LCSC"
	1    3250 3800
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:C_27pF_0402 C8
U 1 1 604515C0
P 3250 4000
F 0 "C8" V 3200 3900 50  0000 C CNN
F 1 "C_27pF_0402" V 3200 4300 50  0000 C CNN
F 2 "Capacitor_SMD:C_0402_1005Metric_Pad0.74x0.62mm_HandSolder" H 3250 4000 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Guangdong-Fenghua-Advanced-Tech-0402CG270J500NTN_C1557.pdf" H 3250 4000 60  0001 C CNN
F 4 "0402CG270J500NTN" H 3250 4000 50  0001 C CNN "Part Number"
F 5 "C1557" H 3250 4000 50  0001 C CNN "LCSC"
	1    3250 4000
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:R_10K_0402 R46
U 1 1 6045B2FE
P 7850 2700
F 0 "R46" V 7850 2700 50  0000 C CNN
F 1 "R_10K_0402" V 7800 3050 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 7780 2700 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0402WGF1002TCE_C25744.pdf" V 7930 2700 50  0001 C CNN
F 4 "0402WGF1002TCE" V 8030 2800 60  0001 C CNN "Part Number"
F 5 "C25744" V 8100 2700 50  0001 C CNN "LCSC"
	1    7850 2700
	0    1    1    0   
$EndComp
Wire Wire Line
	7650 2700 7250 2700
Connection ~ 7250 2700
$Comp
L Open_Automation:R_1K_0402 R44
U 1 1 6045ED26
P 6300 4200
F 0 "R44" V 6300 4200 50  0000 C CNN
F 1 "R_1K_0402" V 6400 4200 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 6230 4200 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/panasonic-electronic-components/ERJ-3GEYJ102V/P1.0KGDKR-ND/577615" V 6380 4200 50  0001 C CNN
F 4 "0402WGF1001TCE" V 6480 4300 60  0001 C CNN "Part Number"
F 5 "C11702" H 6300 4200 50  0001 C CNN "LCSC"
	1    6300 4200
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:R_1K_0402 R43
U 1 1 60460EF6
P 6300 3500
F 0 "R43" V 6300 3500 50  0000 C CNN
F 1 "R_1K_0402" V 6400 3500 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 6230 3500 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/panasonic-electronic-components/ERJ-3GEYJ102V/P1.0KGDKR-ND/577615" V 6380 3500 50  0001 C CNN
F 4 "0402WGF1001TCE" V 6480 3600 60  0001 C CNN "Part Number"
F 5 "C11702" H 6300 3500 50  0001 C CNN "LCSC"
	1    6300 3500
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:D_1N5819 D5
U 1 1 6046A572
P 6250 3000
F 0 "D5" H 6250 3200 50  0000 C CNN
F 1 "D_1N5819" H 5950 2950 50  0000 C CNN
F 2 "Diode_SMD:D_SOD-323_HandSoldering" V 6250 3000 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/1903051130_MDD-Jiangsu-Yutai-Elec-B5819WS_C64886.pdf" V 6250 3000 60  0001 C CNN
F 4 "B5819WS" H 6250 3000 50  0001 C CNN "Part Number"
F 5 "C64886" H 6250 3000 50  0001 C CNN "LCSC"
	1    6250 3000
	-1   0    0    1   
$EndComp
$Comp
L Open_Automation:D_1N5819 D6
U 1 1 6046B2F8
P 6250 3100
F 0 "D6" H 6250 3300 50  0000 C CNN
F 1 "D_1N5819" H 6550 3050 50  0000 C CNN
F 2 "Diode_SMD:D_SOD-323_HandSoldering" V 6250 3100 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/1903051130_MDD-Jiangsu-Yutai-Elec-B5819WS_C64886.pdf" V 6250 3100 60  0001 C CNN
F 4 "B5819WS" H 6250 3100 50  0001 C CNN "Part Number"
F 5 "C64886" H 6250 3100 50  0001 C CNN "LCSC"
	1    6250 3100
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:Q_SS8050 Q6
U 1 1 60477658
P 6750 3500
F 0 "Q6" H 6940 3546 50  0000 L CNN
F 1 "Q_SS8050" H 6940 3455 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 6950 3600 29  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Changjiang-Electronics-Tech-CJ-SS8050_C2150.pdf" H 6750 3500 60  0001 C CNN
F 4 "SS8050" H 6750 3500 50  0001 C CNN "Part Number"
F 5 "C2150" H 6750 3500 50  0001 C CNN "LCSC"
	1    6750 3500
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:Q_SS8550 Q7
U 1 1 6047FF44
P 6750 4200
F 0 "Q7" H 6940 4246 50  0000 L CNN
F 1 "Q_SS8550" H 6940 4155 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 6950 4125 50  0001 L CIN
F 3 "https://datasheet.lcsc.com/szlcsc/Changjiang-Electronics-Tech-CJ-SS8550_C8542.pdf" H 6750 4200 50  0001 L CNN
F 4 "SS8550" H 6750 4200 50  0001 C CNN "Part Number"
F 5 "C8542" H 6750 4200 50  0001 C CNN "LCSC"
	1    6750 4200
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:R_220_0402 R45
U 1 1 604ACCCE
P 7300 4700
F 0 "R45" V 7300 4700 50  0000 C CNN
F 1 "R_220_0402" V 7400 4700 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 7230 4700 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0402WGF2200TCE_C25091.pdf" V 7380 4700 50  0001 C CNN
F 4 "0402WGF2200TCE" V 7480 4800 60  0001 C CNN "Part Number"
F 5 "C25091" H 7300 4700 50  0001 C CNN "LCSC"
	1    7300 4700
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:BAT54C D7
U 1 1 604B459D
P 8350 4700
F 0 "D7" V 8297 4828 60  0000 L CNN
F 1 "BAT54C" V 8403 4828 60  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 8350 4700 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Leshan-Radio-LBAT54CLT1G_C49991.pdf" H 8350 4700 60  0001 C CNN
F 4 "LBAT54CLT1G" H 8350 4700 50  0001 C CNN "Part Number"
F 5 "C49991" H 8350 4700 50  0001 C CNN "LCSC"
	1    8350 4700
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:CH340T(SSOP20W) U10
U 1 1 605022CE
P 4850 3500
F 0 "U10" H 5100 4150 60  0000 C CNN
F 1 "CH340T(SSOP20W)" H 4850 2850 60  0000 C CNN
F 2 "Open_Automation:SSOP-20W" H 4850 3500 60  0001 C CNN
F 3 "" H 4850 3500 60  0001 C CNN
F 4 "CH340T" H 4850 3500 50  0001 C CNN "Part Number"
F 5 "C8689" H 4850 3500 50  0001 C CNN "LCSC"
	1    4850 3500
	1    0    0    -1  
$EndComp
Wire Wire Line
	4050 4000 4350 4000
Wire Wire Line
	3350 4000 4050 4000
Connection ~ 4050 4000
Wire Wire Line
	4050 3800 4350 3800
Wire Wire Line
	3350 3800 4050 3800
Connection ~ 4050 3800
$Comp
L Open_Automation:Crystal_12Mhz Q5
U 1 1 6045824B
P 4050 3900
F 0 "Q5" V 4050 3950 50  0000 L CNN
F 1 "Crystal_12Mhz" V 4200 3600 50  0000 L CNN
F 2 "Crystal:Crystal_SMD_3225-4Pin_3.2x2.5mm_HandSoldering" H 4050 3900 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Yangxing-Tech-X322512MSB4SI_C9002.pdf" H 4050 3900 60  0001 C CNN
F 4 "X322512MSB4SI" H 4050 3900 50  0001 C CNN "Part Number"
F 5 "C9002" H 4050 3900 50  0001 C CNN "LCSC"
	1    4050 3900
	0    1    1    0   
$EndComp
$EndSCHEMATC
