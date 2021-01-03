EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 10 12
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Wire Wire Line
	6200 4700 6700 4700
Wire Wire Line
	5500 4700 6000 4700
Text Notes 4500 1200 0    200  Italic 40
PoE Power Supply
Wire Wire Line
	9100 2700 9100 2800
Text Notes 3300 2700 0    50   ~ 10
The Input filter capacitor must be >5 μF for to MPS!
Text Notes 6700 2700 0    50   ~ 10
250mW(50mA@5V) minimal power consumption \non the PD required for to Maintain Power Signature(MPS)!
NoConn ~ 4300 4000
NoConn ~ 4300 4200
Wire Wire Line
	6800 4600 7000 4600
Wire Wire Line
	6800 4800 6800 4600
Wire Wire Line
	6700 4500 7000 4500
Wire Wire Line
	6700 4700 6700 4500
Connection ~ 6800 4600
Wire Wire Line
	5500 4600 6800 4600
Connection ~ 6700 4500
Wire Wire Line
	5500 4500 6700 4500
Text Notes 2900 4350 0    60   ~ 12
Class=0 (0.44 to 12.95W)
Text Notes 3900 4450 2    30   ~ 6
Rclass
Text Notes 2475 4075 1    30   ~ 6
CDET
Text Notes 5800 2950 1    30   ~ 6
RDET
Wire Wire Line
	2500 4700 4300 4700
Text GLabel 7000 4600 2    50   Input ~ 10
Spare2
Text GLabel 7000 4500 2    50   Input ~ 10
Spare1
Text Notes 8075 2925 0    50   ~ 10
Rb
Text Notes 8075 3425 0    50   ~ 10
Ra
Text Notes 7800 4000 0    50   ~ 10
VOUT=1.35*(1+Ra/Rb)
Text Notes 2900 3900 0    70   ~ 14
Vss
Text Notes 2900 2800 0    70   ~ 14
Vpos
Text Notes 2900 4800 0    70   ~ 14
Vneg
Text Notes 3700 3200 0    70   ~ 14
Vss
Connection ~ 2500 4700
Wire Wire Line
	2500 4900 2500 4800
Wire Wire Line
	2500 4800 2500 4700
Connection ~ 2500 4800
Wire Wire Line
	4300 4800 2500 4800
Wire Wire Line
	2500 4500 2500 4700
Wire Wire Line
	3700 4500 2500 4500
Wire Wire Line
	4000 4500 4300 4500
Wire Wire Line
	3000 3600 2800 3600
Connection ~ 5800 2800
Wire Wire Line
	5800 3000 5800 2800
Wire Wire Line
	5800 3500 5800 3300
Wire Wire Line
	5500 3500 5800 3500
Wire Wire Line
	4300 3600 4000 3600
Connection ~ 2800 3600
Wire Wire Line
	2800 3800 2800 3600
Wire Wire Line
	4300 3800 2800 3800
Wire Wire Line
	2800 3200 3050 3200
Connection ~ 2800 3800
Wire Wire Line
	2800 3900 2800 3800
Wire Wire Line
	4300 3900 2800 3900
Wire Wire Line
	3900 3100 3900 3200
Connection ~ 3900 2800
Wire Wire Line
	3900 2900 3900 2800
Wire Wire Line
	9100 3800 9100 3900
Connection ~ 9100 2800
Wire Wire Line
	9100 2800 9100 3200
Connection ~ 9100 3800
Wire Wire Line
	9100 3400 9100 3800
Wire Wire Line
	8200 3300 8200 3400
Wire Wire Line
	7600 3600 5500 3600
Wire Wire Line
	7600 3300 7600 3600
Wire Wire Line
	8200 3300 7600 3300
Connection ~ 8200 3300
Wire Wire Line
	8200 3200 8200 3300
Connection ~ 8200 2800
Wire Wire Line
	8200 2900 8200 2800
Wire Wire Line
	8200 3800 8200 3700
Connection ~ 8200 3800
Wire Wire Line
	6700 3300 6700 3800
Wire Wire Line
	6700 2800 8200 2800
Connection ~ 6700 2800
Wire Wire Line
	6700 3100 6700 2800
Wire Wire Line
	5800 2800 6100 2800
Wire Wire Line
	6100 2800 6700 2800
Connection ~ 6100 2800
Wire Wire Line
	6100 3100 6100 2800
Wire Wire Line
	3900 2800 5600 2800
Wire Wire Line
	5600 2800 5800 2800
Connection ~ 5600 2800
Wire Wire Line
	5600 3400 5600 2800
Wire Wire Line
	5500 3400 5600 3400
Connection ~ 6700 3800
Wire Wire Line
	6600 3800 6700 3800
Wire Wire Line
	6100 3800 6200 3800
Wire Wire Line
	6100 3300 6100 3800
Connection ~ 6100 3800
Wire Wire Line
	5500 3800 6100 3800
$Comp
L ESP32-PoE_Rev_E:Si3402-B-GM(QFN-20_5x5mm) U11
U 1 1 5AE0CC1A
P 4900 4100
F 0 "U11" H 4900 4950 50  0000 C CNN
F 1 "Si3402-B-GM(QFN-20_5x5mm)" H 4900 3200 50  0000 C CNN
F 2 "Open_Automation:QFN-20_5x5mm(Si3402-B-GM)" H 4900 4100 50  0001 C CNN
F 3 "https://www.mouser.com/datasheet/2/368/Si3402-B-1666231.pdf" H 4900 4100 50  0001 C CNN
F 4 "SI3402-B-GM" H 0   0   50  0001 C CNN "Part Number"
	1    4900 4100
	1    0    0    -1  
$EndComp
Text HLabel 1000 750  0    60   Input ~ 0
12V
Wire Wire Line
	1050 750  1000 750 
Text Label 1050 750  0    50   ~ 0
12V
Text HLabel 1000 850  0    60   Input ~ 0
SP1
Wire Wire Line
	1050 850  1000 850 
Text Label 1050 850  0    50   ~ 0
SP1
Text HLabel 1000 950  0    60   Input ~ 0
SP2
Wire Wire Line
	1050 950  1000 950 
Text Label 1050 950  0    50   ~ 0
SP2
$Comp
L Open_Automation:R_8K2_0402 R50
U 1 1 6021DFD9
P 8200 3550
F 0 "R50" V 8200 3500 50  0000 L CNN
F 1 "R_8K2_0402" V 8300 3300 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 8130 3550 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0402WGF8201TCE_C25924.pdf" V 8280 3550 50  0001 C CNN
F 4 "0402WGF8201TCE" V 8380 3650 60  0001 C CNN "Part Number"
F 5 "C25924" H 8200 3550 50  0001 C CNN "LCSC"
	1    8200 3550
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:R_1K_0402 R49
U 1 1 602232B7
P 8200 3050
F 0 "R49" V 8200 3000 50  0000 L CNN
F 1 "R_1K_0402" V 8300 2850 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 8130 3050 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/panasonic-electronic-components/ERJ-3GEYJ102V/P1.0KGDKR-ND/577615" V 8280 3050 50  0001 C CNN
F 4 "0402WGF1001TCE" V 8380 3150 60  0001 C CNN "Part Number"
F 5 "C11702" H 8200 3050 50  0001 C CNN "LCSC"
	1    8200 3050
	1    0    0    -1  
$EndComp
Text Label 9100 2700 0    50   ~ 0
12V
$Comp
L Open_Automation:C_100nF_1206 C11
U 1 1 6024190C
P 2500 3700
F 0 "C11" H 2400 3800 50  0000 L CNN
F 1 "C_100nF_1206" H 1900 3600 50  0000 L CNN
F 2 "Capacitor_SMD:C_1206_3216Metric_Pad1.33x1.80mm_HandSolder" H 2600 4000 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL31B104KCFNNNE_C1945.pdf" H 2500 3450 50  0001 C CNN
F 4 "C1945" H 2500 4100 50  0001 C CNN "LCSC"
F 5 "CL31B104KCFNNNE" H 2625 3900 60  0001 C CNN "Part Number"
	1    2500 3700
	1    0    0    -1  
$EndComp
Wire Wire Line
	2500 3550 2500 2800
Wire Wire Line
	2500 3850 2500 4500
Connection ~ 2500 4500
$Comp
L Open_Automation:C_1uF_1206 C12
U 1 1 5FE36864
P 2800 3000
F 0 "C12" H 2700 3100 50  0000 L CNN
F 1 "C_1uF_1206" H 2350 2900 50  0000 L CNN
F 2 "Capacitor_SMD:C_1206_3216Metric_Pad1.33x1.80mm_HandSolder" H 2900 3300 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL31B105KCHNNNE_C13832.pdf" H 2800 2750 50  0001 C CNN
F 4 "C13832" H 2800 3400 50  0001 C CNN "LCSC"
F 5 "CL31B105KCHNNNE" H 2925 3200 60  0001 C CNN "Part Number"
	1    2800 3000
	1    0    0    -1  
$EndComp
Wire Wire Line
	2500 2800 2800 2800
$Comp
L Open_Automation:C_1uF_1206 C13
U 1 1 5FE3CA31
P 3050 3000
F 0 "C13" H 3150 3000 50  0000 L CNN
F 1 "C_1uF_1206" H 3050 2900 50  0000 L CNN
F 2 "Capacitor_SMD:C_1206_3216Metric_Pad1.33x1.80mm_HandSolder" H 3150 3300 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL31B105KCHNNNE_C13832.pdf" H 3050 2750 50  0001 C CNN
F 4 "C13832" H 3050 3400 50  0001 C CNN "LCSC"
F 5 "CL31B105KCHNNNE" H 3175 3200 60  0001 C CNN "Part Number"
	1    3050 3000
	1    0    0    -1  
$EndComp
Wire Wire Line
	2800 2850 2800 2800
Connection ~ 2800 2800
Wire Wire Line
	2800 2800 3050 2800
Wire Wire Line
	2800 3150 2800 3200
Connection ~ 2800 3200
Wire Wire Line
	3050 3150 3050 3200
Connection ~ 3050 3200
Wire Wire Line
	3050 3200 3900 3200
Wire Wire Line
	3050 2850 3050 2800
Connection ~ 3050 2800
Wire Wire Line
	3050 2800 3900 2800
Wire Wire Line
	2800 3200 2800 3400
Wire Wire Line
	3150 3400 2800 3400
Connection ~ 2800 3400
Wire Wire Line
	2800 3400 2800 3600
Wire Wire Line
	3450 3400 4300 3400
$Comp
L Open_Automation:C_100nF_0402 C15
U 1 1 5FE52BFF
P 3300 3400
F 0 "C15" V 3350 3500 50  0000 C CNN
F 1 "C_100nF_0402" V 3350 3050 50  0000 C CNN
F 2 "Capacitor_SMD:C_0402_1005Metric_Pad0.74x0.62mm_HandSolder" H 3400 3700 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL05B104KO5NNNC_C1525.pdf" H 3300 3150 50  0001 C CNN
F 4 "C1525" H 3300 3800 50  0001 C CNN "LCSC"
F 5 "CL05B104KO5NNNC" H 3425 3600 60  0001 C CNN "Part Number"
	1    3300 3400
	0    -1   -1   0   
$EndComp
$Comp
L Open_Automation:C_1nF_0402 C14
U 1 1 5FE6081E
P 3150 3600
F 0 "C14" V 3200 3700 50  0000 C CNN
F 1 "C_1nF_0402" V 3200 3300 50  0000 C CNN
F 2 "Capacitor_SMD:C_0402_1005Metric_Pad0.74x0.62mm_HandSolder" H 3250 3900 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Guangdong-Fenghua-Advanced-Tech-0402B102K500NT_C1523.pdf" H 3150 3350 50  0001 C CNN
F 4 "C1523" H 3150 4000 50  0001 C CNN "LCSC"
F 5 "0402B102K500NT" H 3275 3800 60  0001 C CNN "Part Number"
	1    3150 3600
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3300 3600 3700 3600
$Comp
L Open_Automation:R_47K_0402 R46
U 1 1 5FE6A32D
P 3850 3600
F 0 "R46" V 3850 3600 50  0000 C CNN
F 1 "R_47K_0402" V 3950 3600 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 3780 3600 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0402WGF4702TCE_C25792.pdf" V 3930 3600 50  0001 C CNN
F 4 "0402WGF4702TCE" V 4030 3700 60  0001 C CNN "Part Number"
F 5 "C25792" H 3850 3600 50  0001 C CNN "LCSC"
	1    3850 3600
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:R_49.9_0402 R47
U 1 1 5FE71F28
P 3850 4500
F 0 "R47" V 3850 4500 50  0000 C CNN
F 1 "R_49.9_0402" V 3950 4500 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 3780 4500 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0402WGF499JTCE_C25120.pdf" V 3930 4500 50  0001 C CNN
F 4 "0402WGF499JTCE" V 4030 4600 60  0001 C CNN "Part Number"
F 5 "C25120" H 3850 4500 50  0001 C CNN "LCSC"
	1    3850 4500
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:R_25K5 R48
U 1 1 5FEA5C8E
P 5800 3150
F 0 "R48" V 5800 3100 50  0000 L CNN
F 1 "R_25K5" V 5700 3000 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad0.98x0.95mm_HandSolder" V 5730 3150 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0603WAF2552T5E_C22920.pdf" V 5880 3150 50  0001 C CNN
F 4 "0603WAF2552T5E" V 5980 3250 60  0001 C CNN "Part Number"
F 5 "C22920" V 5650 3150 50  0001 C CNN "LCSC"
	1    5800 3150
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:SS510 D7
U 1 1 5FEBAABB
P 6100 3200
F 0 "D7" H 5900 3150 50  0000 L CNN
F 1 "SS510" H 6000 3300 50  0000 L CNN
F 2 "Diode_SMD:D_SMC_Handsoldering" V 6100 3200 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/1903042030_MDD-Jiangsu-Yutai-Elec-SS510_C19229.pdf" V 6100 3200 60  0001 C CNN
F 4 "SS510" H 6100 3200 50  0001 C CNN "Part Number"
F 5 "C19229" H 6100 3200 50  0001 C CNN "LCSC"
	1    6100 3200
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:L_33uH L1
U 1 1 5FE480A4
P 6400 3800
F 0 "L1" H 6400 3900 50  0000 C CNN
F 1 "L_33uH" H 6400 3750 50  0000 C CNN
F 2 "Inductor_SMD:L_12x12mm_H6mm" H 6350 3800 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Sunlord-SWRB1205S-330MT_C169386.pdf" H 6350 3800 60  0001 C CNN
F 4 "SWRB1205S-330MT" H 6400 3800 50  0001 C CNN "Part Number"
F 5 "C169386" H 6400 3800 50  0001 C CNN "LCSC"
	1    6400 3800
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:FB L2
U 1 1 5FE51804
P 7600 3800
F 0 "L2" H 7600 3900 50  0000 C CNN
F 1 "FB" H 7600 3750 50  0000 C CNN
F 2 "Inductor_SMD:L_1206_3216Metric" H 7550 3800 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/1810231132_Sunlord-PZ3216U300-6R0TF_C316417.pdf" H 7550 3800 60  0001 C CNN
F 4 "PZ3216U300-6R0TF" H 7600 3800 50  0001 C CNN "Part Number"
F 5 "C316417" H 7600 3800 50  0001 C CNN "LCSC"
	1    7600 3800
	1    0    0    -1  
$EndComp
Wire Wire Line
	6700 3800 7400 3800
Wire Wire Line
	7800 3800 8200 3800
$Comp
L Open_Automation:C_22uF_0805 C18
U 1 1 5FE5ACD8
P 9100 3300
F 0 "C18" H 9100 3400 50  0000 L CNN
F 1 "C_22uF_0805" H 9100 3200 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 9100 3300 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL21A226MAQNNNE_C45783.pdf" H 9100 3300 60  0001 C CNN
F 4 "CL21A226MAQNNNE" H 9100 3300 50  0001 C CNN "Part Number"
F 5 "C45783" H 9100 3300 50  0001 C CNN "LCSC"
	1    9100 3300
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:SJ SJ2
U 1 1 5FE6AAA4
P 6100 4800
F 0 "SJ2" H 5950 4850 50  0000 C CNN
F 1 "SJ" H 6050 4700 50  0001 L BNN
F 2 "Open_Automation:SJUMP" H 6108 4862 20  0001 C CNN
F 3 "" H 6100 4800 60  0001 C CNN
	1    6100 4800
	1    0    0    -1  
$EndComp
Wire Wire Line
	5500 4800 6000 4800
Wire Wire Line
	6200 4800 6800 4800
$Comp
L power:GND #PWR?
U 1 1 603F1FF4
P 9100 3900
AR Path="/5ABD187E/603F1FF4" Ref="#PWR?"  Part="1" 
AR Path="/603F1FF4" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/603F1FF4" Ref="#PWR?"  Part="1" 
AR Path="/60124B38/603F1FF4" Ref="#PWR085"  Part="1" 
F 0 "#PWR085" H 9100 3650 50  0001 C CNN
F 1 "GND" H 9100 3750 50  0000 C CNN
F 2 "" H 9100 3900 50  0001 C CNN
F 3 "" H 9100 3900 50  0001 C CNN
	1    9100 3900
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:C_15uF C16
U 1 1 604E6A04
P 3900 3000
F 0 "C16" H 3988 3046 50  0000 L CNN
F 1 "C_15uF" H 3988 2955 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D6.3mm_P2.50mm" H 3900 3000 60  0001 C CNN
F 3 "http://nichicon-us.com/english/products/pdfs/e-uhe.pdf" H 3900 3000 60  0001 C CNN
F 4 "UHE2A150MED1TD" H 3900 3000 50  0001 C CNN "Part Number"
	1    3900 3000
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:C_680uF C17
U 1 1 604F49E8
P 6700 3200
F 0 "C17" H 6788 3246 50  0000 L CNN
F 1 "C_680uF" H 6788 3155 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D6.3mm_P2.50mm" H 6700 3200 60  0001 C CNN
F 3 "https://www.digikey.com/en/products/detail/panasonic-electronic-components/EEU-FC1A681L/266232" H 6700 3200 60  0001 C CNN
F 4 "EEU-FC1A681L" H 6700 3200 50  0001 C CNN "Part Number"
	1    6700 3200
	1    0    0    -1  
$EndComp
Text Label 5550 4500 0    50   ~ 0
SP1
Text Label 5550 4600 0    50   ~ 0
SP2
Wire Wire Line
	8200 2800 9100 2800
Wire Wire Line
	8200 3800 9100 3800
$Comp
L power:Earth #PWR0104
U 1 1 6020A6C8
P 2500 4900
F 0 "#PWR0104" H 2500 4650 50  0001 C CNN
F 1 "Earth" H 2500 4750 50  0001 C CNN
F 2 "" H 2500 4900 50  0001 C CNN
F 3 "~" H 2500 4900 50  0001 C CNN
	1    2500 4900
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:SJ SJ1
U 1 1 5FE690AB
P 6100 4700
F 0 "SJ1" H 5950 4750 50  0000 C CNN
F 1 "SJ" H 6050 4600 50  0001 L BNN
F 2 "Open_Automation:SJUMP" H 6108 4762 20  0001 C CNN
F 3 "" H 6100 4700 60  0001 C CNN
	1    6100 4700
	1    0    0    -1  
$EndComp
Wire Wire Line
	6000 4700 6200 4700
Connection ~ 6000 4700
Connection ~ 6200 4700
Wire Wire Line
	6200 4800 6000 4800
Connection ~ 6200 4800
Connection ~ 6000 4800
$EndSCHEMATC
