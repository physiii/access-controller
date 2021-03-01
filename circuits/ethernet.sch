EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 12 12
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
L ESP32-PoE_Rev_E:PWR_FLAG #FLG?
U 1 1 58211FD0
P 5400 1500
AR Path="/60035F16/58211FD0" Ref="#FLG?"  Part="1" 
AR Path="/60042654/58211FD0" Ref="#FLG?"  Part="1" 
AR Path="/600457E1/58211FD0" Ref="#FLG?"  Part="1" 
AR Path="/618D2964/58211FD0" Ref="#FLG04"  Part="1" 
F 0 "#FLG04" H 5400 1595 50  0001 C CNN
F 1 "PWR_FLAG" V 5300 1600 31  0000 C CNN
F 2 "" H 5400 1500 60  0000 C CNN
F 3 "" H 5400 1500 60  0000 C CNN
	1    5400 1500
	1    0    0    -1  
$EndComp
$Comp
L ESP32-PoE_Rev_E:PWR_FLAG #FLG?
U 1 1 582123AC
P 6800 1950
AR Path="/60035F16/582123AC" Ref="#FLG?"  Part="1" 
AR Path="/60042654/582123AC" Ref="#FLG?"  Part="1" 
AR Path="/600457E1/582123AC" Ref="#FLG?"  Part="1" 
AR Path="/618D2964/582123AC" Ref="#FLG05"  Part="1" 
F 0 "#FLG05" H 6800 2045 50  0001 C CNN
F 1 "PWR_FLAG" H 6950 2100 31  0000 C CNN
F 2 "" H 6800 1950 60  0000 C CNN
F 3 "" H 6800 1950 60  0000 C CNN
	1    6800 1950
	1    0    0    -1  
$EndComp
Wire Wire Line
	5300 1550 5400 1550
Connection ~ 5400 1550
Wire Wire Line
	6600 1950 6800 1950
Wire Wire Line
	6600 1950 6600 2050
Wire Wire Line
	6800 2050 6800 1950
Wire Wire Line
	6000 2450 6200 2450
Wire Wire Line
	6600 2450 6600 2250
Wire Wire Line
	6400 1750 6700 1750
Wire Wire Line
	6700 1550 6700 1750
Wire Wire Line
	6000 1850 6000 1550
Connection ~ 6000 1550
Wire Wire Line
	6000 2050 6000 2450
Connection ~ 6000 2450
Connection ~ 6600 2450
Wire Wire Line
	6400 2050 6400 2450
Connection ~ 6400 2450
Wire Wire Line
	6200 2050 6200 2450
Connection ~ 6200 2450
Wire Wire Line
	6000 2450 6000 2650
Wire Wire Line
	6400 2450 6600 2450
Wire Wire Line
	6200 2450 6400 2450
Wire Wire Line
	5900 2250 5900 3250
Wire Wire Line
	3800 950  3800 1050
Wire Wire Line
	3800 1500 3800 1650
Wire Wire Line
	3800 2850 3800 2750
Connection ~ 3800 1650
Wire Wire Line
	3800 1650 3800 1850
$Comp
L ESP32-PoE_Rev_E:PWR_FLAG #FLG?
U 1 1 601255B2
P 4400 950
AR Path="/60035F16/601255B2" Ref="#FLG?"  Part="1" 
AR Path="/60042654/601255B2" Ref="#FLG?"  Part="1" 
AR Path="/600457E1/601255B2" Ref="#FLG?"  Part="1" 
AR Path="/618D2964/601255B2" Ref="#FLG03"  Part="1" 
F 0 "#FLG03" H 4400 1045 50  0001 C CNN
F 1 "PWR_FLAG" V 4300 1250 50  0000 C CNN
F 2 "" H 4400 950 60  0000 C CNN
F 3 "" H 4400 950 60  0000 C CNN
	1    4400 950 
	0    1    1    0   
$EndComp
Wire Wire Line
	3100 1200 3100 950 
Wire Wire Line
	3100 1400 3100 1650
Wire Wire Line
	3800 2150 3800 2350
Wire Wire Line
	4000 1050 4000 950 
Wire Wire Line
	4000 950  4400 950 
Wire Wire Line
	4400 950  4400 1550
Wire Wire Line
	4400 1550 4600 1550
Text HLabel 1600 900  0    60   Input ~ 0
3V3
Wire Wire Line
	1650 900  1600 900 
Text Label 1650 900  0    50   ~ 0
3V3
Text HLabel 1600 1000 0    60   Input ~ 0
PHY_PWR
Wire Wire Line
	1650 1000 1600 1000
Text Label 1650 1000 0    50   ~ 0
PHY_PWR
Text HLabel 1600 1100 0    60   Input ~ 0
EMAC_CLK_OUT
Wire Wire Line
	1650 1100 1600 1100
Text Label 1650 1100 0    50   ~ 0
EMAC_CLK_OUT
$Comp
L Open_Automation:R_10K_0402 R?
U 1 1 5FEB9491
P 3400 1300
AR Path="/60035F16/5FEB9491" Ref="R?"  Part="1" 
AR Path="/60042654/5FEB9491" Ref="R?"  Part="1" 
AR Path="/600457E1/5FEB9491" Ref="R?"  Part="1" 
AR Path="/618D2964/5FEB9491" Ref="R67"  Part="1" 
F 0 "R67" V 3400 1250 50  0000 L CNN
F 1 "R_10K_0402" V 3500 1050 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 3330 1300 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0402WGF1002TCE_C25744.pdf" V 3480 1300 50  0001 C CNN
F 4 "0402WGF1002TCE" V 3580 1400 60  0001 C CNN "Part Number"
F 5 "C25744" V 3650 1300 50  0001 C CNN "LCSC"
	1    3400 1300
	1    0    0    -1  
$EndComp
Wire Wire Line
	3100 1650 3400 1650
Wire Wire Line
	3400 1500 3400 1650
Connection ~ 3400 1650
Wire Wire Line
	3400 1650 3800 1650
Wire Wire Line
	3400 1100 3400 950 
Wire Wire Line
	3100 950  3400 950 
Connection ~ 3400 950 
Text Label 4150 2550 0    50   ~ 0
PHY_PWR
Text Label 3150 950  0    50   ~ 0
3V3
$Comp
L Open_Automation:PFET FET?
U 1 1 5FF0A7D5
P 3900 1250
AR Path="/60035F16/5FF0A7D5" Ref="FET?"  Part="1" 
AR Path="/60042654/5FF0A7D5" Ref="FET?"  Part="1" 
AR Path="/600457E1/5FF0A7D5" Ref="FET?"  Part="1" 
AR Path="/618D2964/5FF0A7D5" Ref="FET1"  Part="1" 
F 0 "FET1" H 3684 1278 60  0000 R CNN
F 1 "PFET" H 3684 1172 60  0000 R CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 3900 1250 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Will-Semicon-WPM2015-3-TR_C213159.pdf" H 3900 1250 60  0001 C CNN
F 4 "WPM2015-3/TR" H 3900 1250 50  0001 C CNN "Part Number"
F 5 "C213159" H 3900 1250 50  0001 C CNN "LCSC"
	1    3900 1250
	-1   0    0    -1  
$EndComp
$Comp
L Open_Automation:R_1K_0402 R?
U 1 1 5FF49295
P 3800 2000
AR Path="/60035F16/5FF49295" Ref="R?"  Part="1" 
AR Path="/60042654/5FF49295" Ref="R?"  Part="1" 
AR Path="/600457E1/5FF49295" Ref="R?"  Part="1" 
AR Path="/618D2964/5FF49295" Ref="R68"  Part="1" 
F 0 "R68" V 3800 1950 50  0000 L CNN
F 1 "R_1K_0402" V 3900 1800 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 3730 2000 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/panasonic-electronic-components/ERJ-3GEYJ102V/P1.0KGDKR-ND/577615" V 3880 2000 50  0001 C CNN
F 4 "0402WGF1001TCE" V 3980 2100 60  0001 C CNN "Part Number"
F 5 "C11702" H 3800 2000 50  0001 C CNN "LCSC"
	1    3800 2000
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:Q_LMUN2211LT1G Q?
U 1 1 5FF5CBE7
P 3900 2550
AR Path="/60035F16/5FF5CBE7" Ref="Q?"  Part="1" 
AR Path="/60042654/5FF5CBE7" Ref="Q?"  Part="1" 
AR Path="/600457E1/5FF5CBE7" Ref="Q?"  Part="1" 
AR Path="/618D2964/5FF5CBE7" Ref="Q9"  Part="1" 
F 0 "Q9" H 4104 2596 50  0000 L CNN
F 1 "Q_LMUN2211LT1G" H 4104 2505 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 3930 2700 20  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Leshan-Radio-LMUN2211LT1G_C12775.pdf" H 3900 2550 60  0001 C CNN
F 4 "LMUN2211LT1G" H 3900 2550 50  0001 C CNN "Part Number"
F 5 "C12775" H 3900 2550 50  0001 C CNN "LCSC"
	1    3900 2550
	-1   0    0    -1  
$EndComp
$Comp
L Open_Automation:C_22uF C?
U 1 1 5FF8D914
P 3100 1300
AR Path="/60035F16/5FF8D914" Ref="C?"  Part="1" 
AR Path="/60042654/5FF8D914" Ref="C?"  Part="1" 
AR Path="/600457E1/5FF8D914" Ref="C?"  Part="1" 
AR Path="/618D2964/5FF8D914" Ref="C29"  Part="1" 
F 0 "C29" H 3000 1400 50  0000 L CNN
F 1 "C_22uF" H 2800 1200 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric_Pad1.08x0.95mm_HandSolder" H 3100 1300 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL10A226MQ8NRNC_C59461.pdf" H 3100 1300 60  0001 C CNN
F 4 "CL10A226MQ8NRNC" H 3100 1300 50  0001 C CNN "Part Number"
F 5 "C59461" H 3100 1300 50  0001 C CNN "LCSC"
	1    3100 1300
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:C_22uF C?
U 1 1 5FF92947
P 4400 1700
AR Path="/60035F16/5FF92947" Ref="C?"  Part="1" 
AR Path="/60042654/5FF92947" Ref="C?"  Part="1" 
AR Path="/600457E1/5FF92947" Ref="C?"  Part="1" 
AR Path="/618D2964/5FF92947" Ref="C30"  Part="1" 
F 0 "C30" H 4200 1700 50  0000 L CNN
F 1 "C_22uF" H 4100 1600 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric_Pad1.08x0.95mm_HandSolder" H 4400 1700 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL10A226MQ8NRNC_C59461.pdf" H 4400 1700 60  0001 C CNN
F 4 "CL10A226MQ8NRNC" H 4400 1700 50  0001 C CNN "Part Number"
F 5 "C59461" H 4400 1700 50  0001 C CNN "LCSC"
	1    4400 1700
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:FB L?
U 1 1 5FFA0046
P 5100 1550
AR Path="/60035F16/5FFA0046" Ref="L?"  Part="1" 
AR Path="/60042654/5FFA0046" Ref="L?"  Part="1" 
AR Path="/600457E1/5FFA0046" Ref="L?"  Part="1" 
AR Path="/618D2964/5FFA0046" Ref="L2"  Part="1" 
F 0 "L2" H 4900 1600 50  0000 C CNN
F 1 "FB" H 5100 1674 50  0000 C CNN
F 2 "Inductor_SMD:L_1206_3216Metric" H 5050 1550 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/1810231132_Sunlord-PZ3216U300-6R0TF_C316417.pdf" H 5050 1550 60  0001 C CNN
F 4 "PZ3216U300-6R0TF" H 5100 1550 50  0001 C CNN "Part Number"
F 5 "C316417" H 5100 1550 50  0001 C CNN "LCSC"
	1    5100 1550
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:C_100nF_0402 C?
U 1 1 5FFBCFEF
P 4600 1750
AR Path="/60035F16/5FFBCFEF" Ref="C?"  Part="1" 
AR Path="/60042654/5FFBCFEF" Ref="C?"  Part="1" 
AR Path="/600457E1/5FFBCFEF" Ref="C?"  Part="1" 
AR Path="/618D2964/5FFBCFEF" Ref="C31"  Part="1" 
F 0 "C31" H 4500 1650 50  0000 C CNN
F 1 "C_100nF_0402" H 4300 1850 50  0000 C CNN
F 2 "Capacitor_SMD:C_0402_1005Metric_Pad0.74x0.62mm_HandSolder" H 4700 2050 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL05B104KO5NNNC_C1525.pdf" H 4600 1500 50  0001 C CNN
F 4 "C1525" H 4600 2150 50  0001 C CNN "LCSC"
F 5 "CL05B104KO5NNNC" H 4725 1950 60  0001 C CNN "Part Number"
	1    4600 1750
	-1   0    0    1   
$EndComp
$Comp
L Open_Automation:C_100nF_0402 C?
U 1 1 5FFF6A63
P 5400 4100
AR Path="/60035F16/5FFF6A63" Ref="C?"  Part="1" 
AR Path="/60042654/5FFF6A63" Ref="C?"  Part="1" 
AR Path="/600457E1/5FFF6A63" Ref="C?"  Part="1" 
AR Path="/618D2964/5FFF6A63" Ref="C32"  Part="1" 
F 0 "C32" H 5450 4000 50  0000 C CNN
F 1 "C_100nF_0402" V 5250 4150 50  0000 C CNN
F 2 "Capacitor_SMD:C_0402_1005Metric_Pad0.74x0.62mm_HandSolder" H 5500 4400 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL05B104KO5NNNC_C1525.pdf" H 5400 3850 50  0001 C CNN
F 4 "C1525" H 5400 4500 50  0001 C CNN "LCSC"
F 5 "CL05B104KO5NNNC" H 5525 4300 60  0001 C CNN "Part Number"
	1    5400 4100
	-1   0    0    1   
$EndComp
Wire Wire Line
	5400 4250 5400 4550
$Comp
L Open_Automation:R_2K2_0402 R?
U 1 1 60017672
P 5900 4150
AR Path="/60035F16/60017672" Ref="R?"  Part="1" 
AR Path="/60042654/60017672" Ref="R?"  Part="1" 
AR Path="/600457E1/60017672" Ref="R?"  Part="1" 
AR Path="/618D2964/60017672" Ref="R75"  Part="1" 
F 0 "R75" V 5900 4100 50  0000 L CNN
F 1 "R_2K2_0402" V 6000 3950 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 5830 4150 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL05B104KO5NNNC_C1525.pdf" V 5980 4150 50  0001 C CNN
F 4 "CL05B104KO5NNNC" V 6080 4250 60  0001 C CNN "Part Number"
F 5 "C1525" V 5750 4150 50  0001 C CNN "LCSC"
	1    5900 4150
	1    0    0    -1  
$EndComp
Wire Wire Line
	5900 3350 5900 4000
Wire Wire Line
	5900 4300 5900 4550
$Comp
L Open_Automation:R_49.9_0402 R?
U 1 1 60046093
P 5500 2100
AR Path="/60035F16/60046093" Ref="R?"  Part="1" 
AR Path="/60042654/60046093" Ref="R?"  Part="1" 
AR Path="/600457E1/60046093" Ref="R?"  Part="1" 
AR Path="/618D2964/60046093" Ref="R69"  Part="1" 
F 0 "R69" V 5500 2050 50  0000 L CNN
F 1 "R_49.9_0402" V 5450 1500 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 5430 2100 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0402WGF499JTCE_C25120.pdf" V 5580 2100 50  0001 C CNN
F 4 "0402WGF499JTCE" V 5680 2200 60  0001 C CNN "Part Number"
F 5 "C25120" H 5500 2100 50  0001 C CNN "LCSC"
	1    5500 2100
	1    0    0    -1  
$EndComp
Wire Wire Line
	5400 1550 5500 1550
$Comp
L Open_Automation:R_49.9_0402 R?
U 1 1 60052BB5
P 5600 2100
AR Path="/60035F16/60052BB5" Ref="R?"  Part="1" 
AR Path="/60042654/60052BB5" Ref="R?"  Part="1" 
AR Path="/600457E1/60052BB5" Ref="R?"  Part="1" 
AR Path="/618D2964/60052BB5" Ref="R71"  Part="1" 
F 0 "R71" V 5600 2050 50  0000 L CNN
F 1 "R_49.9_0402" V 5550 1500 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 5530 2100 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0402WGF499JTCE_C25120.pdf" V 5680 2100 50  0001 C CNN
F 4 "0402WGF499JTCE" V 5780 2200 60  0001 C CNN "Part Number"
F 5 "C25120" H 5600 2100 50  0001 C CNN "LCSC"
	1    5600 2100
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:R_49.9_0402 R?
U 1 1 600534D8
P 5700 2100
AR Path="/60035F16/600534D8" Ref="R?"  Part="1" 
AR Path="/60042654/600534D8" Ref="R?"  Part="1" 
AR Path="/600457E1/600534D8" Ref="R?"  Part="1" 
AR Path="/618D2964/600534D8" Ref="R72"  Part="1" 
F 0 "R72" V 5700 2050 50  0000 L CNN
F 1 "R_49.9_0402" V 5650 1500 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 5630 2100 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0402WGF499JTCE_C25120.pdf" V 5780 2100 50  0001 C CNN
F 4 "0402WGF499JTCE" V 5880 2200 60  0001 C CNN "Part Number"
F 5 "C25120" H 5700 2100 50  0001 C CNN "LCSC"
	1    5700 2100
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:R_49.9_0402 R?
U 1 1 60053AF8
P 5800 2100
AR Path="/60035F16/60053AF8" Ref="R?"  Part="1" 
AR Path="/60042654/60053AF8" Ref="R?"  Part="1" 
AR Path="/600457E1/60053AF8" Ref="R?"  Part="1" 
AR Path="/618D2964/60053AF8" Ref="R73"  Part="1" 
F 0 "R73" V 5800 2050 50  0000 L CNN
F 1 "R_49.9_0402" V 5750 1500 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 5730 2100 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0402WGF499JTCE_C25120.pdf" V 5880 2100 50  0001 C CNN
F 4 "0402WGF499JTCE" V 5980 2200 60  0001 C CNN "Part Number"
F 5 "C25120" H 5800 2100 50  0001 C CNN "LCSC"
	1    5800 2100
	1    0    0    -1  
$EndComp
Wire Wire Line
	5500 1950 5500 1550
Connection ~ 5500 1550
Wire Wire Line
	5500 1550 5600 1550
Wire Wire Line
	5600 1950 5600 1550
Connection ~ 5600 1550
Wire Wire Line
	5600 1550 5700 1550
Wire Wire Line
	5700 1950 5700 1550
Connection ~ 5700 1550
Wire Wire Line
	5700 1550 5800 1550
Wire Wire Line
	5800 1950 5800 1550
Connection ~ 5800 1550
$Comp
L Open_Automation:R_2K2_0402 R?
U 1 1 6007908A
P 5900 2100
AR Path="/60035F16/6007908A" Ref="R?"  Part="1" 
AR Path="/60042654/6007908A" Ref="R?"  Part="1" 
AR Path="/600457E1/6007908A" Ref="R?"  Part="1" 
AR Path="/618D2964/6007908A" Ref="R74"  Part="1" 
F 0 "R74" V 5900 2050 50  0000 L CNN
F 1 "R_2K2_0402" V 5850 1500 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 5830 2100 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL05B104KO5NNNC_C1525.pdf" V 5980 2100 50  0001 C CNN
F 4 "CL05B104KO5NNNC" V 6080 2200 60  0001 C CNN "Part Number"
F 5 "C1525" V 5750 2100 50  0001 C CNN "LCSC"
	1    5900 2100
	1    0    0    -1  
$EndComp
Wire Wire Line
	5800 1550 5900 1550
Wire Wire Line
	5900 1950 5900 1550
Connection ~ 5900 1550
Wire Wire Line
	5900 1550 6000 1550
$Comp
L Open_Automation:C_22uF C?
U 1 1 600924F0
P 6000 1950
AR Path="/60035F16/600924F0" Ref="C?"  Part="1" 
AR Path="/60042654/600924F0" Ref="C?"  Part="1" 
AR Path="/600457E1/600924F0" Ref="C?"  Part="1" 
AR Path="/618D2964/600924F0" Ref="C33"  Part="1" 
F 0 "C33" V 6050 2000 50  0000 L CNN
F 1 "C_22uF" V 6050 1650 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric_Pad1.08x0.95mm_HandSolder" H 6000 1950 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL10A226MQ8NRNC_C59461.pdf" H 6000 1950 60  0001 C CNN
F 4 "CL10A226MQ8NRNC" H 6000 1950 50  0001 C CNN "Part Number"
F 5 "C59461" H 6000 1950 50  0001 C CNN "LCSC"
	1    6000 1950
	1    0    0    -1  
$EndComp
Wire Wire Line
	6200 1750 6200 1550
Wire Wire Line
	6000 1550 6200 1550
Wire Wire Line
	6200 1550 6700 1550
Connection ~ 6200 1550
$Comp
L Open_Automation:C_100nF_0402 C?
U 1 1 600988ED
P 6200 1900
AR Path="/60035F16/600988ED" Ref="C?"  Part="1" 
AR Path="/60042654/600988ED" Ref="C?"  Part="1" 
AR Path="/600457E1/600988ED" Ref="C?"  Part="1" 
AR Path="/618D2964/600988ED" Ref="C35"  Part="1" 
F 0 "C35" H 6250 1800 50  0000 C CNN
F 1 "C_100nF_0402" V 6150 2250 50  0000 C CNN
F 2 "Capacitor_SMD:C_0402_1005Metric_Pad0.74x0.62mm_HandSolder" H 6300 2200 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL05B104KO5NNNC_C1525.pdf" H 6200 1650 50  0001 C CNN
F 4 "C1525" H 6200 2300 50  0001 C CNN "LCSC"
F 5 "CL05B104KO5NNNC" H 6325 2100 60  0001 C CNN "Part Number"
	1    6200 1900
	-1   0    0    1   
$EndComp
$Comp
L Open_Automation:C_100nF_0402 C?
U 1 1 600A78A9
P 6400 1900
AR Path="/60035F16/600A78A9" Ref="C?"  Part="1" 
AR Path="/60042654/600A78A9" Ref="C?"  Part="1" 
AR Path="/600457E1/600A78A9" Ref="C?"  Part="1" 
AR Path="/618D2964/600A78A9" Ref="C36"  Part="1" 
F 0 "C36" H 6300 1800 50  0000 C CNN
F 1 "C_100nF_0402" V 6350 2250 50  0000 C CNN
F 2 "Capacitor_SMD:C_0402_1005Metric_Pad0.74x0.62mm_HandSolder" H 6500 2200 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL05B104KO5NNNC_C1525.pdf" H 6400 1650 50  0001 C CNN
F 4 "C1525" H 6400 2300 50  0001 C CNN "LCSC"
F 5 "CL05B104KO5NNNC" H 6525 2100 60  0001 C CNN "Part Number"
	1    6400 1900
	-1   0    0    1   
$EndComp
Wire Wire Line
	6600 2450 6800 2450
$Comp
L Open_Automation:C_100nF_0402 C?
U 1 1 600AA948
P 6800 2200
AR Path="/60035F16/600AA948" Ref="C?"  Part="1" 
AR Path="/60042654/600AA948" Ref="C?"  Part="1" 
AR Path="/600457E1/600AA948" Ref="C?"  Part="1" 
AR Path="/618D2964/600AA948" Ref="C38"  Part="1" 
F 0 "C38" H 6700 2100 50  0000 C CNN
F 1 "C_100nF_0402" V 6750 2550 50  0000 C CNN
F 2 "Capacitor_SMD:C_0402_1005Metric_Pad0.74x0.62mm_HandSolder" H 6900 2500 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL05B104KO5NNNC_C1525.pdf" H 6800 1950 50  0001 C CNN
F 4 "C1525" H 6800 2600 50  0001 C CNN "LCSC"
F 5 "CL05B104KO5NNNC" H 6925 2400 60  0001 C CNN "Part Number"
	1    6800 2200
	-1   0    0    1   
$EndComp
Wire Wire Line
	6800 2350 6800 2450
$Comp
L Open_Automation:C_2.2uF_0402 C?
U 1 1 600C20AA
P 6600 2150
AR Path="/60035F16/600C20AA" Ref="C?"  Part="1" 
AR Path="/60042654/600C20AA" Ref="C?"  Part="1" 
AR Path="/600457E1/600C20AA" Ref="C?"  Part="1" 
AR Path="/618D2964/600C20AA" Ref="C37"  Part="1" 
F 0 "C37" H 6600 2250 50  0000 L CNN
F 1 "C_2.2uF_0402" V 6650 1600 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric_Pad0.74x0.62mm_HandSolder" H 6600 2150 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL05A225MQ5NSNC_C12530.pdf" H 6600 2150 60  0001 C CNN
F 4 "CL05A225MQ5NSNC" H 6600 2150 50  0001 C CNN "Part Number"
F 5 "C12530" H 6600 2150 50  0001 C CNN "LCSC"
	1    6600 2150
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:R_10K_0402 R?
U 1 1 5FE5B873
P 6050 5600
AR Path="/60035F16/5FE5B873" Ref="R?"  Part="1" 
AR Path="/60042654/5FE5B873" Ref="R?"  Part="1" 
AR Path="/600457E1/5FE5B873" Ref="R?"  Part="1" 
AR Path="/618D2964/5FE5B873" Ref="R76"  Part="1" 
F 0 "R76" V 6050 5600 50  0000 C CNN
F 1 "R_10K_0402" V 5950 5600 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 5980 5600 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0402WGF1002TCE_C25744.pdf" V 6130 5600 50  0001 C CNN
F 4 "0402WGF1002TCE" V 6230 5700 60  0001 C CNN "Part Number"
F 5 "C25744" V 6300 5600 50  0001 C CNN "LCSC"
	1    6050 5600
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:C_10uF C?
U 1 1 5FE83977
P 6050 6100
AR Path="/60035F16/5FE83977" Ref="C?"  Part="1" 
AR Path="/60042654/5FE83977" Ref="C?"  Part="1" 
AR Path="/600457E1/5FE83977" Ref="C?"  Part="1" 
AR Path="/618D2964/5FE83977" Ref="C34"  Part="1" 
F 0 "C34" H 5950 6000 50  0000 C CNN
F 1 "C_10uF" H 6000 6200 50  0000 C CNN
F 2 "Capacitor_SMD:C_0603_1608Metric_Pad1.08x0.95mm_HandSolder" H 6300 6550 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL10A106MA8NRNC_C96446.pdf" H 6050 5850 50  0001 C CNN
F 4 "CL10A106MA8NRNC" H 6100 6650 60  0001 C CNN "Part Number"
F 5 "C96446" H 6050 6750 50  0001 C CNN "LCSC"
	1    6050 6100
	-1   0    0    1   
$EndComp
$Comp
L Open_Automation:R_12.1K_0402 R?
U 1 1 5FEA0D11
P 5500 5750
AR Path="/60035F16/5FEA0D11" Ref="R?"  Part="1" 
AR Path="/60042654/5FEA0D11" Ref="R?"  Part="1" 
AR Path="/600457E1/5FEA0D11" Ref="R?"  Part="1" 
AR Path="/618D2964/5FEA0D11" Ref="R70"  Part="1" 
F 0 "R70" V 5500 5750 50  0000 C CNN
F 1 "R_12.1K_0402" V 5600 5800 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 5430 5750 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0402WGF1212TCE_C25852.pdf" V 5580 5750 50  0001 C CNN
F 4 "0402WGF1212TCE" V 5680 5850 60  0001 C CNN "Part Number"
F 5 "C25852" V 5750 5750 50  0001 C CNN "LCSC"
	1    5500 5750
	-1   0    0    1   
$EndComp
Text HLabel 1600 1200 0    60   Input ~ 0
EMAC_TXD0
Wire Wire Line
	1650 1200 1600 1200
Text Label 1650 1200 0    50   ~ 0
EMAC_TXD0
Text HLabel 1600 1400 0    60   Input ~ 0
EMAC_TX_EN
Wire Wire Line
	1650 1400 1600 1400
Text Label 1650 1400 0    50   ~ 0
EMAC_TX_EN
Text Label 6900 6150 2    50   ~ 0
EMAC_RXD1
Text Label 1650 1500 0    50   ~ 0
EMAC_RXD0
Text Label 1650 1600 0    50   ~ 0
EMAC_RXD1
Text HLabel 1600 1500 0    60   Input ~ 0
EMAC_RXD0
Text HLabel 1600 1600 0    60   Input ~ 0
EMAC_RXD1
Wire Wire Line
	1600 1500 1650 1500
Wire Wire Line
	1600 1600 1650 1600
Text HLabel 1600 1300 0    60   Input ~ 0
EMAC_TXD1
Wire Wire Line
	1650 1300 1600 1300
Text Label 1650 1300 0    50   ~ 0
EMAC_TXD1
Text Label 4100 5950 2    50   ~ 0
MDIO
Text Label 1650 1700 0    50   ~ 0
MDC
Text Label 1650 1800 0    50   ~ 0
MDIO
Text Label 4100 6050 2    50   ~ 0
EMAC_RX_CRS_DV
Text Label 1650 1900 0    50   ~ 0
EMAC_RX_CRS_DV
Text HLabel 1600 1700 0    60   Input ~ 0
MDC
Text HLabel 1600 1800 0    60   Input ~ 0
MDIO
Text HLabel 1600 1900 0    60   Input ~ 0
EMAC_RX_CRS_DV
Wire Wire Line
	1600 1900 1650 1900
Wire Wire Line
	1650 1800 1600 1800
Wire Wire Line
	1600 1700 1650 1700
$Comp
L power:GND #PWR?
U 1 1 603F5D1A
P 3800 2850
AR Path="/5ABD187E/603F5D1A" Ref="#PWR?"  Part="1" 
AR Path="/603F5D1A" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/603F5D1A" Ref="#PWR?"  Part="1" 
AR Path="/600457E1/603F5D1A" Ref="#PWR?"  Part="1" 
AR Path="/618D2964/603F5D1A" Ref="#PWR0108"  Part="1" 
F 0 "#PWR0108" H 3800 2600 50  0001 C CNN
F 1 "GND" H 3800 2700 50  0000 C CNN
F 2 "" H 3800 2850 50  0001 C CNN
F 3 "" H 3800 2850 50  0001 C CNN
	1    3800 2850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 60406505
P 4600 1950
AR Path="/5ABD187E/60406505" Ref="#PWR?"  Part="1" 
AR Path="/60406505" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/60406505" Ref="#PWR?"  Part="1" 
AR Path="/600457E1/60406505" Ref="#PWR?"  Part="1" 
AR Path="/618D2964/60406505" Ref="#PWR0109"  Part="1" 
F 0 "#PWR0109" H 4600 1700 50  0001 C CNN
F 1 "GND" H 4600 1800 50  0000 C CNN
F 2 "" H 4600 1950 50  0001 C CNN
F 3 "" H 4600 1950 50  0001 C CNN
	1    4600 1950
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 60406C0D
P 6000 2650
AR Path="/5ABD187E/60406C0D" Ref="#PWR?"  Part="1" 
AR Path="/60406C0D" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/60406C0D" Ref="#PWR?"  Part="1" 
AR Path="/600457E1/60406C0D" Ref="#PWR?"  Part="1" 
AR Path="/618D2964/60406C0D" Ref="#PWR0114"  Part="1" 
F 0 "#PWR0114" H 6000 2400 50  0001 C CNN
F 1 "GND" H 6000 2500 50  0000 C CNN
F 2 "" H 6000 2650 50  0001 C CNN
F 3 "" H 6000 2650 50  0001 C CNN
	1    6000 2650
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 60408359
P 5400 4550
AR Path="/5ABD187E/60408359" Ref="#PWR?"  Part="1" 
AR Path="/60408359" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/60408359" Ref="#PWR?"  Part="1" 
AR Path="/600457E1/60408359" Ref="#PWR?"  Part="1" 
AR Path="/618D2964/60408359" Ref="#PWR0111"  Part="1" 
F 0 "#PWR0111" H 5400 4300 50  0001 C CNN
F 1 "GND" H 5400 4400 50  0000 C CNN
F 2 "" H 5400 4550 50  0001 C CNN
F 3 "" H 5400 4550 50  0001 C CNN
	1    5400 4550
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 60408A74
P 5900 4550
AR Path="/5ABD187E/60408A74" Ref="#PWR?"  Part="1" 
AR Path="/60408A74" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/60408A74" Ref="#PWR?"  Part="1" 
AR Path="/600457E1/60408A74" Ref="#PWR?"  Part="1" 
AR Path="/618D2964/60408A74" Ref="#PWR0113"  Part="1" 
F 0 "#PWR0113" H 5900 4300 50  0001 C CNN
F 1 "GND" H 5900 4400 50  0000 C CNN
F 2 "" H 5900 4550 50  0001 C CNN
F 3 "" H 5900 4550 50  0001 C CNN
	1    5900 4550
	1    0    0    -1  
$EndComp
Text Label 5900 3350 0    50   ~ 0
LL-
Text Label 5900 3250 0    50   ~ 0
RL+
Wire Wire Line
	4150 2550 4100 2550
Wire Wire Line
	3400 950  3800 950 
Text Label 4400 950  2    50   ~ 0
3V3_LAN
Text Label 6950 5500 2    50   ~ 0
3V3_LAN
Text Label 4600 5950 0    50   ~ 0
3V3_LAN
Text Label 4600 6050 0    50   ~ 0
3V3_LAN
Text Label 6050 5350 2    50   ~ 0
3V3_LAN
Text Label 7000 6150 0    50   ~ 0
EMAC_RXD0
Text Label 5500 3850 0    50   ~ 0
RD-
Text Label 5600 3750 0    50   ~ 0
RD+
Text Label 5400 2900 1    50   ~ 0
CT
Text Label 5800 3550 0    50   ~ 0
TD+
Wire Wire Line
	5800 2250 5800 3550
Wire Wire Line
	5700 2250 5700 3650
Wire Wire Line
	5600 2250 5600 3750
Wire Wire Line
	5500 2250 5500 3850
Text Label 5700 3650 0    50   ~ 0
TD-
$Comp
L Open_Automation:RA_2.2K_(4x0603)_4B8_Smashed RM_2.2K?
U 4 1 5FE9CD81
P 7050 5900
AR Path="/600457E1/5FE9CD81" Ref="RM_2.2K?"  Part="1" 
AR Path="/618D2964/5FE9CD81" Ref="RM_2.2K1"  Part="4" 
F 0 "RM_2.2K1" H 7150 6150 60  0000 C CNN
F 1 "RA_2.2K_(4x0603)_4B8_Smashed" H 7058 5940 60  0001 C CNN
F 2 "Open_Automation:R_0603_MATRIX_4" H 7100 5800 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-4D03WGJ0222T5E_C12001.pdf" H 7100 5800 60  0001 C CNN
F 4 "4D03WGJ0222T5E" H 7050 5900 50  0001 C CNN "Part Number"
F 5 "C12001" H 7050 5900 50  0001 C CNN "LCSC"
	4    7050 5900
	0    -1   -1   0   
$EndComp
$Comp
L Open_Automation:RA_2.2K_(4x0603)_4B8_Smashed RM_2.2K?
U 1 1 5FE9DB49
P 7150 5900
AR Path="/600457E1/5FE9DB49" Ref="RM_2.2K?"  Part="2" 
AR Path="/618D2964/5FE9DB49" Ref="RM_2.2K2"  Part="1" 
F 0 "RM_2.2K2" H 7250 5950 60  0000 C CNN
F 1 "RA_2.2K_(4x0603)_4B8_Smashed" H 7158 5940 60  0001 C CNN
F 2 "Open_Automation:R_0603_MATRIX_4" H 7200 5800 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-4D03WGJ0222T5E_C12001.pdf" H 7200 5800 60  0001 C CNN
F 4 "4D03WGJ0222T5E" H 7150 5900 50  0001 C CNN "Part Number"
F 5 "C12001" H 7150 5900 50  0001 C CNN "LCSC"
	1    7150 5900
	0    -1   -1   0   
$EndComp
$Comp
L Open_Automation:RA_2.2K_(4x0603)_4B8_Smashed RM_2.2K?
U 1 1 5FEA4579
P 4350 5700
AR Path="/600457E1/5FEA4579" Ref="RM_2.2K?"  Part="2" 
AR Path="/618D2964/5FEA4579" Ref="RM_2.2K1"  Part="1" 
F 0 "RM_2.2K1" H 4350 5750 60  0000 C CNN
F 1 "RA_2.2K_(4x0603)_4B8_Smashed" H 4358 5740 60  0001 C CNN
F 2 "Open_Automation:R_0603_MATRIX_4" H 4400 5600 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-4D03WGJ0222T5E_C12001.pdf" H 4400 5600 60  0001 C CNN
F 4 "4D03WGJ0222T5E" H 4350 5700 50  0001 C CNN "Part Number"
F 5 "C12001" H 4350 5700 50  0001 C CNN "LCSC"
	1    4350 5700
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:RA_2.2K_(4x0603)_4B8_Smashed RM_2.2K?
U 2 1 5FEA4D2C
P 4350 6100
AR Path="/600457E1/5FEA4D2C" Ref="RM_2.2K?"  Part="3" 
AR Path="/618D2964/5FEA4D2C" Ref="RM_2.2K1"  Part="2" 
F 0 "RM_2.2K1" H 4350 6350 60  0000 C CNN
F 1 "RA_2.2K_(4x0603)_4B8_Smashed" H 4358 6140 60  0001 C CNN
F 2 "Open_Automation:R_0603_MATRIX_4" H 4400 6000 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-4D03WGJ0222T5E_C12001.pdf" H 4400 6000 60  0001 C CNN
F 4 "4D03WGJ0222T5E" H 4350 6100 50  0001 C CNN "Part Number"
F 5 "C12001" H 4350 6100 50  0001 C CNN "LCSC"
	2    4350 6100
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:RA_2.2K_(4x0603)_4B8_Smashed RM_2.2K?
U 3 1 5FEA598A
P 4350 6200
AR Path="/600457E1/5FEA598A" Ref="RM_2.2K?"  Part="4" 
AR Path="/618D2964/5FEA598A" Ref="RM_2.2K1"  Part="3" 
F 0 "RM_2.2K1" H 4350 6250 60  0000 C CNN
F 1 "RA_2.2K_(4x0603)_4B8_Smashed" H 4358 6240 60  0001 C CNN
F 2 "Open_Automation:R_0603_MATRIX_4" H 4400 6100 60  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-4D03WGJ0222T5E_C12001.pdf" H 4400 6100 60  0001 C CNN
F 4 "4D03WGJ0222T5E" H 4350 6200 50  0001 C CNN "Part Number"
F 5 "C12001" H 4350 6200 50  0001 C CNN "LCSC"
	3    4350 6200
	1    0    0    -1  
$EndComp
$Comp
L Interface_Ethernet:LAN8720A U?
U 1 1 5FE9DCA3
P 9450 3800
AR Path="/600457E1/5FE9DCA3" Ref="U?"  Part="1" 
AR Path="/618D2964/5FE9DCA3" Ref="U15"  Part="1" 
F 0 "U15" H 10000 2750 50  0000 C CNN
F 1 "LAN8720A" H 8950 2750 50  0000 C CNN
F 2 "Package_DFN_QFN:QFN-24-1EP_4x4mm_P0.5mm_EP2.6x2.6mm" H 9500 2750 50  0001 L CNN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/8720a.pdf" H 9250 2850 50  0001 C CNN
F 4 "C45223" H 0   0   50  0001 C CNN "LCSC"
F 5 "LAN8720A-CP-TR" H 0   0   50  0001 C CNN "Part Number"
	1    9450 3800
	1    0    0    -1  
$EndComp
Text Label 9250 2550 2    50   ~ 0
3V3_LAN
Wire Wire Line
	9250 2550 9250 2600
Text Label 6450 1550 2    50   ~ 0
VDD1A
Text Label 9450 2450 0    50   ~ 0
VDD1A
Wire Wire Line
	9450 2450 9450 2600
Text Label 9550 2550 0    50   ~ 0
VDD2A
Wire Wire Line
	9550 2550 9550 2600
Text Label 6450 1750 2    50   ~ 0
VDD2A
Text Label 6750 1950 2    50   ~ 0
VDDCR
Text Label 9350 2450 2    50   ~ 0
VDDCR
Wire Wire Line
	9350 2450 9350 2600
$Comp
L power:GND #PWR?
U 1 1 5FEEE8E0
P 9450 4950
AR Path="/5ABD187E/5FEEE8E0" Ref="#PWR?"  Part="1" 
AR Path="/5FEEE8E0" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/5FEEE8E0" Ref="#PWR?"  Part="1" 
AR Path="/600457E1/5FEEE8E0" Ref="#PWR?"  Part="1" 
AR Path="/618D2964/5FEEE8E0" Ref="#PWR0116"  Part="1" 
F 0 "#PWR0116" H 9450 4700 50  0001 C CNN
F 1 "GND" H 9450 4800 50  0000 C CNN
F 2 "" H 9450 4950 50  0001 C CNN
F 3 "" H 9450 4950 50  0001 C CNN
	1    9450 4950
	1    0    0    -1  
$EndComp
Wire Wire Line
	9450 4950 9450 4900
Text Label 10200 4200 0    50   ~ 0
RL-
Text Label 10200 4100 0    50   ~ 0
LL+
Wire Wire Line
	10200 4100 10150 4100
Wire Wire Line
	10200 4200 10150 4200
Text Label 8600 4500 2    50   ~ 0
EMAC_CLK_OUT
Wire Wire Line
	8600 4500 8650 4500
NoConn ~ 8650 4600
Text Label 10200 3300 0    50   ~ 0
TD+
Wire Wire Line
	10200 3300 10150 3300
Text Label 10200 3400 0    50   ~ 0
TD-
Wire Wire Line
	10200 3400 10150 3400
Text Label 10200 3500 0    50   ~ 0
RD+
Wire Wire Line
	10200 3500 10150 3500
Text Label 10200 3600 0    50   ~ 0
RD-
Wire Wire Line
	10200 3600 10150 3600
Text Label 5500 5500 2    50   ~ 0
RBIAS
Text Label 10200 3900 0    50   ~ 0
RBIAS
Wire Wire Line
	10200 3900 10150 3900
Text Label 8600 3200 2    50   ~ 0
EMAC_TXD0
Wire Wire Line
	8600 3200 8650 3200
Text Label 8600 3300 2    50   ~ 0
EMAC_TXD1
Wire Wire Line
	8600 3300 8650 3300
Text Label 8600 3400 2    50   ~ 0
EMAC_TX_EN
Wire Wire Line
	8600 3400 8650 3400
Text Label 6050 5950 2    50   ~ 0
RST
Text Label 8600 4200 2    50   ~ 0
RST
Wire Wire Line
	8600 4200 8650 4200
Text Label 8600 3800 2    50   ~ 0
EMAC_RX_CRS_DV
Wire Wire Line
	8600 3800 8650 3800
Text Label 8600 3900 2    50   ~ 0
MDIO
Text Label 8600 4000 2    50   ~ 0
MDC
Wire Wire Line
	8600 4000 8650 4000
Wire Wire Line
	8650 3900 8600 3900
Text Label 4100 5550 2    50   ~ 0
RXER
Text Label 8600 3700 2    50   ~ 0
RXER
Wire Wire Line
	8600 3700 8650 3700
Text Label 8600 3600 2    50   ~ 0
EMAC_RXD1
Wire Wire Line
	8600 3600 8650 3600
Text Label 8600 3500 2    50   ~ 0
EMAC_RXD0
Wire Wire Line
	8600 3500 8650 3500
NoConn ~ 8650 4300
Wire Wire Line
	4400 1600 4400 1550
Connection ~ 4400 1550
Wire Wire Line
	4600 1600 4600 1550
Connection ~ 4600 1550
Wire Wire Line
	4400 1800 4400 1900
Wire Wire Line
	4400 1900 4600 1900
Wire Wire Line
	4600 1900 4600 1950
Connection ~ 4600 1900
Connection ~ 6800 1950
Wire Wire Line
	5500 5500 5500 5550
$Comp
L power:GND #PWR?
U 1 1 6022F6EC
P 6050 6300
AR Path="/5ABD187E/6022F6EC" Ref="#PWR?"  Part="1" 
AR Path="/6022F6EC" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/6022F6EC" Ref="#PWR?"  Part="1" 
AR Path="/600457E1/6022F6EC" Ref="#PWR?"  Part="1" 
AR Path="/618D2964/6022F6EC" Ref="#PWR0115"  Part="1" 
F 0 "#PWR0115" H 6050 6050 50  0001 C CNN
F 1 "GND" H 6050 6150 50  0000 C CNN
F 2 "" H 6050 6300 50  0001 C CNN
F 3 "" H 6050 6300 50  0001 C CNN
	1    6050 6300
	1    0    0    -1  
$EndComp
Wire Wire Line
	6050 6300 6050 6250
Wire Wire Line
	6050 5800 6050 5950
Wire Wire Line
	6050 5350 6050 5400
Wire Wire Line
	4100 5950 4150 5950
Wire Wire Line
	4100 6050 4150 6050
$Comp
L ESP32-PoE_Rev_E:GND #PWR?
U 1 1 602D8C15
P 4600 5550
AR Path="/60035F16/602D8C15" Ref="#PWR?"  Part="1" 
AR Path="/60042654/602D8C15" Ref="#PWR?"  Part="1" 
AR Path="/600457E1/602D8C15" Ref="#PWR?"  Part="1" 
AR Path="/618D2964/602D8C15" Ref="#PWR0110"  Part="1" 
F 0 "#PWR0110" H 4600 5300 50  0001 C CNN
F 1 "GND" V 4600 5350 50  0000 C CNN
F 2 "" H 4600 5550 60  0000 C CNN
F 3 "" H 4600 5550 60  0000 C CNN
	1    4600 5550
	0    -1   1    0   
$EndComp
Wire Wire Line
	4600 5550 4550 5550
Wire Wire Line
	4150 5550 4100 5550
Wire Wire Line
	4600 5950 4550 5950
Wire Wire Line
	4550 6050 4600 6050
Wire Wire Line
	6900 6150 6900 6100
Wire Wire Line
	7000 6150 7000 6100
Connection ~ 4400 950 
Wire Wire Line
	4600 1550 4900 1550
Wire Wire Line
	5400 1500 5400 1550
Wire Wire Line
	5400 1550 5400 3950
$Comp
L power:GND #PWR?
U 1 1 5FF8A7A6
P 5500 6000
AR Path="/5ABD187E/5FF8A7A6" Ref="#PWR?"  Part="1" 
AR Path="/5FF8A7A6" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/5FF8A7A6" Ref="#PWR?"  Part="1" 
AR Path="/600457E1/5FF8A7A6" Ref="#PWR?"  Part="1" 
AR Path="/618D2964/5FF8A7A6" Ref="#PWR0112"  Part="1" 
F 0 "#PWR0112" H 5500 5750 50  0001 C CNN
F 1 "GND" H 5500 5850 50  0000 C CNN
F 2 "" H 5500 6000 50  0001 C CNN
F 3 "" H 5500 6000 50  0001 C CNN
	1    5500 6000
	1    0    0    -1  
$EndComp
Wire Wire Line
	5500 5950 5500 6000
Wire Wire Line
	6950 5500 6950 5600
Wire Wire Line
	6950 5600 6900 5600
Wire Wire Line
	6900 5600 6900 5700
Wire Wire Line
	6950 5600 7000 5600
Wire Wire Line
	7000 5600 7000 5700
Connection ~ 6950 5600
Text Label 1750 3050 0    50   ~ 0
TD+
Text Label 1750 3150 0    50   ~ 0
CT
Text Label 1750 3250 0    50   ~ 0
TD-
Text Label 1750 3400 0    50   ~ 0
RD+
Text Label 1750 3500 0    50   ~ 0
CT
Text Label 1750 3600 0    50   ~ 0
RD-
Text Label 1750 2900 0    50   ~ 0
LL+
Text Label 1750 2800 0    50   ~ 0
LL-
Text Label 1750 2700 0    50   ~ 0
RL+
Text Label 1750 2600 0    50   ~ 0
RL-
Text HLabel 1700 2600 0    60   Input ~ 0
RL-
Wire Wire Line
	1750 2600 1700 2600
Text HLabel 1700 2700 0    60   Input ~ 0
RL+
Wire Wire Line
	1750 2700 1700 2700
Text HLabel 1700 2800 0    60   Input ~ 0
LL-
Wire Wire Line
	1750 2800 1700 2800
Text HLabel 1700 2900 0    60   Input ~ 0
LL+
Wire Wire Line
	1700 2900 1750 2900
Text HLabel 1700 3050 0    60   Input ~ 0
TD+
Wire Wire Line
	1750 3050 1700 3050
Text HLabel 1700 3150 0    60   Input ~ 0
MDCT1
Wire Wire Line
	1750 3150 1700 3150
Text HLabel 1700 3250 0    60   Input ~ 0
TD-
Wire Wire Line
	1750 3250 1700 3250
Text HLabel 1700 3400 0    60   Input ~ 0
RD+
Wire Wire Line
	1750 3400 1700 3400
Text HLabel 1700 3500 0    60   Input ~ 0
MDCT2
Wire Wire Line
	1750 3500 1700 3500
Text HLabel 1700 3600 0    60   Input ~ 0
RD-
Wire Wire Line
	1750 3600 1700 3600
$EndSCHEMATC
