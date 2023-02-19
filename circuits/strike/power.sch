EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 4 5
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text Label 1650 1550 0    50   ~ 0
POWER
Wire Wire Line
	1600 1550 1650 1550
Text HLabel 1600 1550 0    60   Input ~ 0
POWER
Text Label 6200 3700 0    50   ~ 0
3V3
$Comp
L Open_Automation:C_10uF C?
U 1 1 5FFACB78
P 4900 3800
AR Path="/5A7BADB8/5FFACB78" Ref="C?"  Part="1" 
AR Path="/6045445B/5FFACB78" Ref="C?"  Part="1" 
AR Path="/61E50C45/5FFACB78" Ref="C?"  Part="1" 
AR Path="/61E5C0D3/5FFACB78" Ref="C?"  Part="1" 
AR Path="/61E5C138/5FFACB78" Ref="C8"  Part="1" 
F 0 "C8" H 4700 3800 50  0000 L CNN
F 1 "C_10uF" H 4600 3700 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 5000 4100 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL21A106KAYNNNE_C15850.pdf" H 4900 3550 50  0001 C CNN
F 4 "CL21A106KAYNNNE" H 5025 4000 60  0001 C CNN "Part Number"
F 5 "C15850" H 600 700 50  0001 C CNN "LCSC"
	1    4900 3800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5FFACB6C
P 6350 4450
AR Path="/5A7BADB8/5FFACB6C" Ref="#PWR?"  Part="1" 
AR Path="/6045445B/5FFACB6C" Ref="#PWR?"  Part="1" 
AR Path="/61E50C45/5FFACB6C" Ref="#PWR?"  Part="1" 
AR Path="/61E5C0D3/5FFACB6C" Ref="#PWR?"  Part="1" 
AR Path="/61E5C138/5FFACB6C" Ref="#PWR015"  Part="1" 
F 0 "#PWR015" H 6350 4200 50  0001 C CNN
F 1 "GND" H 6400 4300 50  0000 R CNN
F 2 "" H 6350 4450 50  0001 C CNN
F 3 "" H 6350 4450 50  0001 C CNN
	1    6350 4450
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:C_10uF C?
U 1 1 5FFACB57
P 6100 3850
AR Path="/5A7BADB8/5FFACB57" Ref="C?"  Part="1" 
AR Path="/6045445B/5FFACB57" Ref="C?"  Part="1" 
AR Path="/61E50C45/5FFACB57" Ref="C?"  Part="1" 
AR Path="/61E5C0D3/5FFACB57" Ref="C?"  Part="1" 
AR Path="/61E5C138/5FFACB57" Ref="C9"  Part="1" 
F 0 "C9" H 5900 3850 50  0000 L CNN
F 1 "C_10uF" H 5800 3750 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 6200 4150 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL21A106KAYNNNE_C15850.pdf" H 6100 3600 50  0001 C CNN
F 4 "CL21A106KAYNNNE" H 6225 4050 60  0001 C CNN "Part Number"
F 5 "C15850" H 700 600 50  0001 C CNN "LCSC"
	1    6100 3850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5FE0B92B
P 4900 4000
AR Path="/5A7BADB8/5FE0B92B" Ref="#PWR?"  Part="1" 
AR Path="/6045445B/5FE0B92B" Ref="#PWR?"  Part="1" 
AR Path="/61E50C45/5FE0B92B" Ref="#PWR?"  Part="1" 
AR Path="/61E5C0D3/5FE0B92B" Ref="#PWR?"  Part="1" 
AR Path="/61E5C138/5FE0B92B" Ref="#PWR013"  Part="1" 
F 0 "#PWR013" H 4900 3750 50  0001 C CNN
F 1 "GND" H 4950 3850 50  0000 R CNN
F 2 "" H 4900 4000 50  0001 C CNN
F 3 "" H 4900 4000 50  0001 C CNN
	1    4900 4000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5FE15096
P 5150 3850
AR Path="/5A7BADB8/5FE15096" Ref="#PWR?"  Part="1" 
AR Path="/6045445B/5FE15096" Ref="#PWR?"  Part="1" 
AR Path="/61E50C45/5FE15096" Ref="#PWR?"  Part="1" 
AR Path="/61E5C0D3/5FE15096" Ref="#PWR?"  Part="1" 
AR Path="/61E5C138/5FE15096" Ref="#PWR014"  Part="1" 
F 0 "#PWR014" H 5150 3600 50  0001 C CNN
F 1 "GND" H 5200 3700 50  0000 R CNN
F 2 "" H 5150 3850 50  0001 C CNN
F 3 "" H 5150 3850 50  0001 C CNN
	1    5150 3850
	1    0    0    -1  
$EndComp
Wire Wire Line
	5150 3850 5150 3800
Wire Wire Line
	5150 3800 5200 3800
Wire Wire Line
	4900 3650 4900 3600
Wire Wire Line
	4900 3600 5200 3600
Wire Wire Line
	4900 4000 4900 3950
$Comp
L Open_Automation:REG_AMS1117-3.3 U?
U 1 1 61E2B9EA
P 5600 3700
AR Path="/61E50C45/61E2B9EA" Ref="U?"  Part="1" 
AR Path="/61E5C0D3/61E2B9EA" Ref="U?"  Part="1" 
AR Path="/61E5C138/61E2B9EA" Ref="U3"  Part="1" 
F 0 "U3" H 5575 4025 50  0000 C CNN
F 1 "REG_AMS1117-3.3" H 5575 3934 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-223-3_TabPin2" H 5550 3425 50  0001 C CIN
F 3 "https://datasheet.lcsc.com/szlcsc/1811201117_Advanced-Monolithic-Systems-AMS-AMS1117-3-3_C6186.pdf" H 5575 3350 50  0001 C CNN
F 4 "AMS1117-3.3" H 5625 4025 60  0001 C CNN "Part Number"
F 5 "C347222" H 5550 4100 50  0001 C CNN "LCSC"
	1    5600 3700
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:LED_G D?
U 1 1 61E385A9
P 6350 3850
AR Path="/5A7BADB8/61E385A9" Ref="D?"  Part="1" 
AR Path="/6045445B/61E385A9" Ref="D?"  Part="1" 
AR Path="/61E50C45/61E385A9" Ref="D?"  Part="1" 
AR Path="/61E5C0D3/61E385A9" Ref="D?"  Part="1" 
AR Path="/61E5C138/61E385A9" Ref="D4"  Part="1" 
F 0 "D4" V 6350 3800 50  0000 R CNN
F 1 "LED_G" V 6350 3800 50  0001 R CNN
F 2 "LED_SMD:LED_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 6250 3850 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Everlight-Elec-19-217-BHC-ZL1M2RY-3T_C72041.pdf" H 6350 3950 50  0001 C CNN
F 4 "19-217/BHC-ZL1M2RY/3T" H 6450 4050 50  0001 C CNN "Part Number"
F 5 "C72041" H -3750 1850 50  0001 C CNN "LCSC"
	1    6350 3850
	0    -1   -1   0   
$EndComp
Wire Wire Line
	6350 4050 6350 4000
Wire Wire Line
	6350 4350 6350 4400
Wire Wire Line
	6350 4400 6100 4400
Wire Wire Line
	6100 4400 6100 4000
Connection ~ 6350 4400
Wire Wire Line
	6350 4400 6350 4450
Wire Wire Line
	5950 3700 6100 3700
Connection ~ 6100 3700
Wire Wire Line
	6100 3700 6350 3700
Text Label 1650 1650 0    50   ~ 0
3V3
Text HLabel 1600 1650 0    60   Input ~ 0
3V3
Wire Wire Line
	1600 1650 1650 1650
Text Label 5100 3600 2    50   ~ 0
POWER
$Comp
L Open_Automation:R_1K_0402 R3
U 1 1 61F4CAE7
P 6350 4200
F 0 "R3" V 6350 4150 50  0000 L CNN
F 1 "R_1K_0402" V 6450 3900 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 6280 4200 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/panasonic-electronic-components/ERJ-3GEYJ102V/P1.0KGDKR-ND/577615" V 6430 4200 50  0001 C CNN
F 4 "0402WGF1001TCE" V 6530 4300 60  0001 C CNN "Part Number"
F 5 "C11702" H 6350 4200 50  0001 C CNN "LCSC"
	1    6350 4200
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:L_6.8uH L?
U 1 1 620EC1AC
P 6500 2400
F 0 "L?" H 6553 2446 50  0000 L CNN
F 1 "L_6.8uH" H 6553 2355 50  0000 L CNN
F 2 "Inductor_SMD:L_Taiyo-Yuden_NR-30xx_HandSoldering" V 6350 2300 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/tdk-corporation/GLFR1608T6R8M-LR/445-6156-1-ND/2465482" V 6450 2400 50  0001 C CNN
F 4 "GLFR1608T6R8M-LR" V 6550 2500 50  0001 C CNN "Part Number"
	1    6500 2400
	1    0    0    -1  
$EndComp
$Comp
L Converter_DCDC:TPSM53602RDA U?
U 1 1 620EC96D
P 8050 3000
F 0 "U?" H 8050 3567 50  0000 C CNN
F 1 "TPSM53602RDA" H 8050 3476 50  0000 C CNN
F 2 "Package_DFN_QFN:Texas_B3QFN-14-1EP_5x5.5mm_P0.65mm_ThermalVia" H 8050 3700 50  0001 L CNN
F 3 "https://www.ti.com/lit/ds/symlink/tpsm53602.pdf" H 8150 3600 50  0001 C CNN
	1    8050 3000
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:TPS54331DR U?
U 1 1 620F78D7
P 5300 2600
F 0 "U?" H 5300 3125 50  0000 C CNN
F 1 "TPS54331DR" H 5300 3034 50  0000 C CNN
F 2 "Package_SO:SOIC-8-1EP_3.9x4.9mm_P1.27mm_EP2.514x3.2mm_ThermalVias" H 5300 3300 50  0001 L CNN
F 3 "https://datasheet.lcsc.com/lcsc/1808272040_Texas-Instruments-TPS54331DR_C9865.pdf" H 5400 3200 50  0001 C CNN
	1    5300 2600
	1    0    0    -1  
$EndComp
$EndSCHEMATC
