EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 3 5
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
NoConn ~ 5350 3400
NoConn ~ 5350 3600
NoConn ~ 5350 3700
NoConn ~ 5350 3800
NoConn ~ 5350 4000
Wire Wire Line
	3250 3000 3150 3000
Wire Wire Line
	3150 3800 2850 3800
Wire Wire Line
	2850 3800 2850 3900
Wire Wire Line
	3150 4000 2850 4000
Connection ~ 2850 4000
Wire Wire Line
	5350 3100 5850 3100
Wire Wire Line
	5350 3500 6050 3500
Wire Wire Line
	5350 3300 5850 3300
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
	5850 3300 5850 3800
Wire Wire Line
	2850 4000 2850 4200
Wire Wire Line
	5850 3800 5850 4200
Wire Wire Line
	6050 3500 6150 3500
Wire Wire Line
	3950 3900 2850 3900
Connection ~ 2850 3900
Wire Wire Line
	2850 3900 2850 4000
NoConn ~ 5350 3900
Wire Wire Line
	3450 3000 4050 3000
Wire Wire Line
	4050 2600 4050 3000
Connection ~ 4050 3000
Wire Wire Line
	4050 3000 4350 3000
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
Wire Wire Line
	5850 2600 5850 3100
Connection ~ 5850 3100
Wire Wire Line
	8050 2700 8750 2700
Text Label 4250 2600 0    50   ~ 0
5V
Text HLabel 950  1450 0    50   Input ~ 0
3V3
Wire Wire Line
	950  1450 1000 1450
Text Label 1000 1450 0    50   ~ 0
3V3
Text Label 8750 2700 0    50   ~ 0
3V3
Text Label 9250 3200 0    50   ~ 0
ESP_EN
Text Label 9250 3100 0    50   ~ 0
USB_TXD
Text Label 9250 3000 0    50   ~ 0
USB_RXD
Text HLabel 950  1350 0    60   Input ~ 0
IO2
Text Label 1000 1350 0    50   ~ 0
IO2
Wire Wire Line
	1000 1350 950  1350
Text Label 2900 1200 0    50   ~ 0
5V
Wire Wire Line
	2900 1200 2850 1200
Text Label 2900 1500 0    50   ~ 0
D-
Wire Wire Line
	2900 1500 2850 1500
Text Label 2900 1400 0    50   ~ 0
D+
Wire Wire Line
	2900 1400 2850 1400
$Comp
L power:GND #PWR?
U 1 1 604145B1
P 2850 4200
AR Path="/5ABD187E/604145B1" Ref="#PWR?"  Part="1" 
AR Path="/604145B1" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/604145B1" Ref="#PWR?"  Part="1" 
AR Path="/60DDD162/604145B1" Ref="#PWR?"  Part="1" 
AR Path="/61E5A1A8/604145B1" Ref="#PWR?"  Part="1" 
AR Path="/61E5B8A5/604145B1" Ref="#PWR09"  Part="1" 
F 0 "#PWR09" H 2850 3950 50  0001 C CNN
F 1 "GND" H 2850 4050 50  0000 C CNN
F 2 "" H 2850 4200 50  0001 C CNN
F 3 "" H 2850 4200 50  0001 C CNN
	1    2850 4200
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 6041598A
P 2550 1850
AR Path="/5ABD187E/6041598A" Ref="#PWR?"  Part="1" 
AR Path="/6041598A" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/6041598A" Ref="#PWR?"  Part="1" 
AR Path="/60DDD162/6041598A" Ref="#PWR?"  Part="1" 
AR Path="/61E5A1A8/6041598A" Ref="#PWR?"  Part="1" 
AR Path="/61E5B8A5/6041598A" Ref="#PWR08"  Part="1" 
F 0 "#PWR08" H 2550 1600 50  0001 C CNN
F 1 "GND" H 2550 1700 50  0000 C CNN
F 2 "" H 2550 1850 50  0001 C CNN
F 3 "" H 2550 1850 50  0001 C CNN
	1    2550 1850
	1    0    0    -1  
$EndComp
Wire Wire Line
	2550 1850 2550 1800
Text Label 4300 3500 2    50   ~ 0
D-
Wire Wire Line
	4300 3500 4350 3500
Text Label 4300 3600 2    50   ~ 0
D+
Wire Wire Line
	4300 3600 4350 3600
$Comp
L power:GND #PWR?
U 1 1 6042DDD6
P 3150 3000
AR Path="/5ABD187E/6042DDD6" Ref="#PWR?"  Part="1" 
AR Path="/6042DDD6" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/6042DDD6" Ref="#PWR?"  Part="1" 
AR Path="/60DDD162/6042DDD6" Ref="#PWR?"  Part="1" 
AR Path="/61E5A1A8/6042DDD6" Ref="#PWR?"  Part="1" 
AR Path="/61E5B8A5/6042DDD6" Ref="#PWR010"  Part="1" 
F 0 "#PWR010" H 3150 2750 50  0001 C CNN
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
AR Path="/60DDD162/6042E609" Ref="#PWR?"  Part="1" 
AR Path="/61E5A1A8/6042E609" Ref="#PWR?"  Part="1" 
AR Path="/61E5B8A5/6042E609" Ref="#PWR011"  Part="1" 
F 0 "#PWR011" H 3150 2850 50  0001 C CNN
F 1 "GND" V 3100 2950 50  0000 C CNN
F 2 "" H 3150 3100 50  0001 C CNN
F 3 "" H 3150 3100 50  0001 C CNN
	1    3150 3100
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR?
U 1 1 6042E874
P 4300 3300
AR Path="/5ABD187E/6042E874" Ref="#PWR?"  Part="1" 
AR Path="/6042E874" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/6042E874" Ref="#PWR?"  Part="1" 
AR Path="/60DDD162/6042E874" Ref="#PWR?"  Part="1" 
AR Path="/61E5A1A8/6042E874" Ref="#PWR?"  Part="1" 
AR Path="/61E5B8A5/6042E874" Ref="#PWR012"  Part="1" 
F 0 "#PWR012" H 4300 3050 50  0001 C CNN
F 1 "GND" V 4250 3150 50  0000 C CNN
F 2 "" H 4300 3300 50  0001 C CNN
F 3 "" H 4300 3300 50  0001 C CNN
	1    4300 3300
	0    1    1    0   
$EndComp
Wire Wire Line
	3150 3100 3300 3100
Wire Wire Line
	3600 3100 4150 3100
$Comp
L Open_Automation:C_100nF_0402 C?
U 1 1 60432E92
P 3450 3100
AR Path="/61E5A1A8/60432E92" Ref="C?"  Part="1" 
AR Path="/61E5B8A5/60432E92" Ref="C7"  Part="1" 
F 0 "C7" V 3500 3000 50  0000 C CNN
F 1 "C_100nF_0402" V 3400 3450 50  0000 C CNN
F 2 "Capacitor_SMD:C_0402_1005Metric_Pad0.74x0.62mm_HandSolder" H 3550 3400 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL05B104KO5NNNC_C1525.pdf" H 3450 2850 50  0001 C CNN
F 4 "C1525" H 3450 3500 50  0001 C CNN "LCSC"
F 5 "CL05B104KO5NNNC" H 3575 3300 60  0001 C CNN "Part Number"
	1    3450 3100
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:C_22uF C?
U 1 1 60430DBE
P 3350 3000
AR Path="/61E5A1A8/60430DBE" Ref="C?"  Part="1" 
AR Path="/61E5B8A5/60430DBE" Ref="C6"  Part="1" 
F 0 "C6" V 3300 3100 50  0000 C CNN
F 1 "C_22uF" V 3212 3000 50  0000 C CNN
F 2 "Capacitor_SMD:C_0603_1608Metric_Pad1.08x0.95mm_HandSolder" H 3350 3000 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL10A226MQ8NRNC_C59461.pdf" H 3350 3000 60  0001 C CNN
F 4 "CL10A226MQ8NRNC" H 3350 3000 50  0001 C CNN "Part Number"
F 5 "C59461" H 3350 3000 50  0001 C CNN "LCSC"
	1    3350 3000
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:R_10K_0402 R?
U 1 1 6043D0F4
P 4950 2600
AR Path="/61E5A1A8/6043D0F4" Ref="R?"  Part="1" 
AR Path="/61E5B8A5/6043D0F4" Ref="R4"  Part="1" 
F 0 "R4" V 4950 2600 50  0000 C CNN
F 1 "R_10K_0402" V 4900 2950 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 4880 2600 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0402WGF1002TCE_C25744.pdf" V 5030 2600 50  0001 C CNN
F 4 "0402WGF1002TCE" V 5130 2700 60  0001 C CNN "Part Number"
F 5 "C25744" V 5200 2600 50  0001 C CNN "LCSC"
	1    4950 2600
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:R_10K_0402 R?
U 1 1 60446CBF
P 4950 2700
AR Path="/61E5A1A8/60446CBF" Ref="R?"  Part="1" 
AR Path="/61E5B8A5/60446CBF" Ref="R5"  Part="1" 
F 0 "R5" V 4950 2700 50  0000 C CNN
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
L Open_Automation:C_27pF_0402 C?
U 1 1 60450786
P 3250 3800
AR Path="/61E5A1A8/60450786" Ref="C?"  Part="1" 
AR Path="/61E5B8A5/60450786" Ref="C4"  Part="1" 
F 0 "C4" V 3200 3700 50  0000 C CNN
F 1 "C_27pF_0402" V 3200 4100 50  0000 C CNN
F 2 "Capacitor_SMD:C_0402_1005Metric_Pad0.74x0.62mm_HandSolder" H 3250 3800 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Guangdong-Fenghua-Advanced-Tech-0402CG270J500NTN_C1557.pdf" H 3250 3800 60  0001 C CNN
F 4 "0402CG270J500NTN" H 3250 3800 50  0001 C CNN "Part Number"
F 5 "C1557" H 3250 3800 50  0001 C CNN "LCSC"
	1    3250 3800
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:C_27pF_0402 C?
U 1 1 604515C0
P 3250 4000
AR Path="/61E5A1A8/604515C0" Ref="C?"  Part="1" 
AR Path="/61E5B8A5/604515C0" Ref="C5"  Part="1" 
F 0 "C5" V 3200 3900 50  0000 C CNN
F 1 "C_27pF_0402" V 3200 4300 50  0000 C CNN
F 2 "Capacitor_SMD:C_0402_1005Metric_Pad0.74x0.62mm_HandSolder" H 3250 4000 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Guangdong-Fenghua-Advanced-Tech-0402CG270J500NTN_C1557.pdf" H 3250 4000 60  0001 C CNN
F 4 "0402CG270J500NTN" H 3250 4000 50  0001 C CNN "Part Number"
F 5 "C1557" H 3250 4000 50  0001 C CNN "LCSC"
	1    3250 4000
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:R_10K_0402 R?
U 1 1 6045B2FE
P 7850 2700
AR Path="/61E5A1A8/6045B2FE" Ref="R?"  Part="1" 
AR Path="/61E5B8A5/6045B2FE" Ref="R8"  Part="1" 
F 0 "R8" V 7850 2700 50  0000 C CNN
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
L Open_Automation:R_1K_0402 R?
U 1 1 6045ED26
P 6300 4200
AR Path="/61E5A1A8/6045ED26" Ref="R?"  Part="1" 
AR Path="/61E5B8A5/6045ED26" Ref="R7"  Part="1" 
F 0 "R7" V 6300 4200 50  0000 C CNN
F 1 "R_1K_0402" V 6400 4200 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 6230 4200 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/panasonic-electronic-components/ERJ-3GEYJ102V/P1.0KGDKR-ND/577615" V 6380 4200 50  0001 C CNN
F 4 "0402WGF1001TCE" V 6480 4300 60  0001 C CNN "Part Number"
F 5 "C11702" H 6300 4200 50  0001 C CNN "LCSC"
	1    6300 4200
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:R_1K_0402 R?
U 1 1 60460EF6
P 6300 3500
AR Path="/61E5A1A8/60460EF6" Ref="R?"  Part="1" 
AR Path="/61E5B8A5/60460EF6" Ref="R6"  Part="1" 
F 0 "R6" V 6300 3500 50  0000 C CNN
F 1 "R_1K_0402" V 6400 3500 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 6230 3500 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/panasonic-electronic-components/ERJ-3GEYJ102V/P1.0KGDKR-ND/577615" V 6380 3500 50  0001 C CNN
F 4 "0402WGF1001TCE" V 6480 3600 60  0001 C CNN "Part Number"
F 5 "C11702" H 6300 3500 50  0001 C CNN "LCSC"
	1    6300 3500
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:Q_SS8050 Q?
U 1 1 60477658
P 6750 3500
AR Path="/61E5A1A8/60477658" Ref="Q?"  Part="1" 
AR Path="/61E5B8A5/60477658" Ref="Q2"  Part="1" 
F 0 "Q2" H 6940 3546 50  0000 L CNN
F 1 "Q_SS8050" H 6940 3455 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 6950 3600 29  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Changjiang-Electronics-Tech-CJ-SS8050_C2150.pdf" H 6750 3500 60  0001 C CNN
F 4 "SS8050" H 6750 3500 50  0001 C CNN "Part Number"
F 5 "C2150" H 6750 3500 50  0001 C CNN "LCSC"
	1    6750 3500
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:Q_SS8550 Q?
U 1 1 6047FF44
P 6750 4200
AR Path="/61E5A1A8/6047FF44" Ref="Q?"  Part="1" 
AR Path="/61E5B8A5/6047FF44" Ref="Q3"  Part="1" 
F 0 "Q3" H 6940 4246 50  0000 L CNN
F 1 "Q_SS8550" H 6940 4155 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 6950 4125 50  0001 L CIN
F 3 "https://datasheet.lcsc.com/szlcsc/Changjiang-Electronics-Tech-CJ-SS8550_C8542.pdf" H 6750 4200 50  0001 L CNN
F 4 "SS8550" H 6750 4200 50  0001 C CNN "Part Number"
F 5 "C8542" H 6750 4200 50  0001 C CNN "LCSC"
	1    6750 4200
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:CH340T(SSOP20W) U?
U 1 1 605022CE
P 4850 3500
AR Path="/61E5A1A8/605022CE" Ref="U?"  Part="1" 
AR Path="/61E5B8A5/605022CE" Ref="U2"  Part="1" 
F 0 "U2" H 5100 4150 60  0000 C CNN
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
L Open_Automation:Crystal_12Mhz Q?
U 1 1 6045824B
P 4050 3900
AR Path="/61E5A1A8/6045824B" Ref="Q?"  Part="1" 
AR Path="/61E5B8A5/6045824B" Ref="Q1"  Part="1" 
F 0 "Q1" V 4050 3950 50  0000 L CNN
F 1 "Crystal_12Mhz" V 4200 3600 50  0000 L CNN
F 2 "Crystal:Crystal_SMD_3225-4Pin_3.2x2.5mm_HandSoldering" H 4050 3900 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Yangxing-Tech-X322512MSB4SI_C9002.pdf" H 4050 3900 60  0001 C CNN
F 4 "X322512MSB4SI" H 4050 3900 50  0001 C CNN "Part Number"
F 5 "C9002" H 4050 3900 50  0001 C CNN "LCSC"
	1    4050 3900
	0    1    1    0   
$EndComp
Wire Wire Line
	5350 3000 7250 3000
Wire Wire Line
	5850 3100 9250 3100
Wire Wire Line
	6850 4450 6850 4400
Text Label 6850 4450 0    50   ~ 0
PROG
$Comp
L strike-controller-rescue:USB_B_Micro-Open_Automation J2
U 1 1 61EAB842
P 2550 1400
AR Path="/61EAB842" Ref="J2"  Part="1" 
AR Path="/61E5B8A5/61EAB842" Ref="J2"  Part="1" 
F 0 "J2" H 2607 1867 50  0000 C CNN
F 1 "USB_B_Micro" H 2607 1776 50  0000 C CNN
F 2 "Connector_USB:USB_Micro-B_Amphenol_10118194_Horizontal" H 2700 1350 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/lcsc/1811131822_Amphenol-ICC-10118194-0001LF_C132563.pdf" H 2700 1350 50  0001 C CNN
F 4 "10118194-0001LF" H 2550 1400 60  0001 C CNN "Part Number"
F 5 "C132563" H 2550 1400 60  0001 C CNN "LCSC"
	1    2550 1400
	1    0    0    -1  
$EndComp
Wire Wire Line
	2450 1800 2450 1850
Wire Wire Line
	2450 1850 2550 1850
Connection ~ 2550 1850
Wire Wire Line
	4300 3300 4350 3300
$EndSCHEMATC
