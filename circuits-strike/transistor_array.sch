EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 5 5
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
L Open_Automation:Q_MMBT5551 Q5
U 1 1 61E799E1
P 3400 2350
F 0 "Q5" H 3591 2396 50  0000 L CNN
F 1 "Q_MMBT5551" H 3591 2305 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 3600 2450 29  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Changjiang-Electronics-Tech-CJ-SS8050_C2150.pdf" H 3400 2350 60  0001 C CNN
F 4 "MMBT5551" H 3400 2350 50  0001 C CNN "Part Number"
F 5 "C2145" H 3400 2350 50  0001 C CNN "LCSC"
	1    3400 2350
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:R_1K_0402 R11
U 1 1 61E79DBA
P 3000 2350
F 0 "R11" V 3000 2300 50  0000 L CNN
F 1 "R_1K_0402" V 3100 2200 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 2930 2350 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/panasonic-electronic-components/ERJ-3GEYJ102V/P1.0KGDKR-ND/577615" V 3080 2350 50  0001 C CNN
F 4 "0402WGF1001TCE" V 3180 2450 60  0001 C CNN "Part Number"
F 5 "C11702" H 3000 2350 50  0001 C CNN "LCSC"
	1    3000 2350
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:R_1K_0402 R12
U 1 1 61E7B073
P 3500 1850
F 0 "R12" V 3500 1800 50  0000 L CNN
F 1 "R_1K_0402" H 3050 2000 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 3430 1850 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/panasonic-electronic-components/ERJ-3GEYJ102V/P1.0KGDKR-ND/577615" V 3580 1850 50  0001 C CNN
F 4 "0402WGF1001TCE" V 3680 1950 60  0001 C CNN "Part Number"
F 5 "C11702" H 3500 1850 50  0001 C CNN "LCSC"
	1    3500 1850
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61E7D119
P 3500 2600
AR Path="/5ABD187E/61E7D119" Ref="#PWR?"  Part="1" 
AR Path="/61E7D119" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/61E7D119" Ref="#PWR?"  Part="1" 
AR Path="/6045445B/61E7D119" Ref="#PWR?"  Part="1" 
AR Path="/61E50C45/61E7D119" Ref="#PWR?"  Part="1" 
AR Path="/61E5C0D3/61E7D119" Ref="#PWR?"  Part="1" 
AR Path="/61E5C138/61E7D119" Ref="#PWR?"  Part="1" 
AR Path="/61E73398/61E7D119" Ref="#PWR017"  Part="1" 
F 0 "#PWR017" H 3500 2350 50  0001 C CNN
F 1 "GND" H 3500 2450 50  0000 C CNN
F 2 "" H 3500 2600 50  0000 C CNN
F 3 "" H 3500 2600 50  0000 C CNN
	1    3500 2600
	1    0    0    -1  
$EndComp
Wire Wire Line
	3500 2600 3500 2550
Wire Wire Line
	3150 2350 3200 2350
Text HLabel 900  1100 0    60   Input ~ 0
IN1
Text Label 2800 2350 2    60   ~ 0
IN1
Wire Wire Line
	2850 2350 2800 2350
Text Label 950  1100 0    60   ~ 0
IN1
Wire Wire Line
	950  1100 900  1100
Text HLabel 1000 800  0    60   Input ~ 0
POWER
Text Label 1050 800  0    60   ~ 0
PWR
Wire Wire Line
	1050 800  1000 800 
Wire Wire Line
	3500 1650 3500 1700
Text HLabel 900  1200 0    60   Input ~ 0
IN2
Text Label 950  1200 0    60   ~ 0
IN2
Wire Wire Line
	950  1200 900  1200
Text HLabel 900  1300 0    60   Input ~ 0
IN3
Text Label 950  1300 0    60   ~ 0
IN3
Wire Wire Line
	950  1300 900  1300
Text HLabel 900  1400 0    60   Input ~ 0
IN4
Text Label 950  1400 0    60   ~ 0
IN4
Wire Wire Line
	950  1400 900  1400
Text HLabel 900  1500 0    60   Input ~ 0
IN5
Text Label 950  1500 0    60   ~ 0
IN5
Wire Wire Line
	950  1500 900  1500
$Comp
L Open_Automation:Q_MMBT5551 Q7
U 1 1 61E82A1E
P 5000 2400
F 0 "Q7" H 5191 2446 50  0000 L CNN
F 1 "Q_MMBT5551" H 5191 2355 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 5200 2500 29  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Changjiang-Electronics-Tech-CJ-SS8050_C2150.pdf" H 5000 2400 60  0001 C CNN
F 4 "MMBT5551" H 5000 2400 50  0001 C CNN "Part Number"
F 5 "C2145" H 5000 2400 50  0001 C CNN "LCSC"
	1    5000 2400
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:R_1K_0402 R14
U 1 1 61E82A2A
P 4600 2400
F 0 "R14" V 4600 2350 50  0000 L CNN
F 1 "R_1K_0402" V 4700 2250 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 4530 2400 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/panasonic-electronic-components/ERJ-3GEYJ102V/P1.0KGDKR-ND/577615" V 4680 2400 50  0001 C CNN
F 4 "0402WGF1001TCE" V 4780 2500 60  0001 C CNN "Part Number"
F 5 "C11702" H 4600 2400 50  0001 C CNN "LCSC"
	1    4600 2400
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:R_1K_0402 R15
U 1 1 61E82A36
P 5100 1900
F 0 "R15" V 5100 1850 50  0000 L CNN
F 1 "R_1K_0402" H 4650 2050 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 5030 1900 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/panasonic-electronic-components/ERJ-3GEYJ102V/P1.0KGDKR-ND/577615" V 5180 1900 50  0001 C CNN
F 4 "0402WGF1001TCE" V 5280 2000 60  0001 C CNN "Part Number"
F 5 "C11702" H 5100 1900 50  0001 C CNN "LCSC"
	1    5100 1900
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61E82A40
P 5100 2650
AR Path="/5ABD187E/61E82A40" Ref="#PWR?"  Part="1" 
AR Path="/61E82A40" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/61E82A40" Ref="#PWR?"  Part="1" 
AR Path="/6045445B/61E82A40" Ref="#PWR?"  Part="1" 
AR Path="/61E50C45/61E82A40" Ref="#PWR?"  Part="1" 
AR Path="/61E5C0D3/61E82A40" Ref="#PWR?"  Part="1" 
AR Path="/61E5C138/61E82A40" Ref="#PWR?"  Part="1" 
AR Path="/61E73398/61E82A40" Ref="#PWR019"  Part="1" 
F 0 "#PWR019" H 5100 2400 50  0001 C CNN
F 1 "GND" H 5100 2500 50  0000 C CNN
F 2 "" H 5100 2650 50  0000 C CNN
F 3 "" H 5100 2650 50  0000 C CNN
	1    5100 2650
	1    0    0    -1  
$EndComp
Wire Wire Line
	5100 2650 5100 2600
Wire Wire Line
	4750 2400 4800 2400
Text Label 4400 2400 2    60   ~ 0
IN2
Wire Wire Line
	4450 2400 4400 2400
Wire Wire Line
	5100 1700 5100 1750
$Comp
L Open_Automation:Q_MMBT5551 Q8
U 1 1 61E83670
P 6550 2450
F 0 "Q8" H 6741 2496 50  0000 L CNN
F 1 "Q_MMBT5551" H 6741 2405 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 6750 2550 29  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Changjiang-Electronics-Tech-CJ-SS8050_C2150.pdf" H 6550 2450 60  0001 C CNN
F 4 "MMBT5551" H 6550 2450 50  0001 C CNN "Part Number"
F 5 "C2145" H 6550 2450 50  0001 C CNN "LCSC"
	1    6550 2450
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:R_1K_0402 R16
U 1 1 61E8367C
P 6150 2450
F 0 "R16" V 6150 2400 50  0000 L CNN
F 1 "R_1K_0402" V 6250 2300 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 6080 2450 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/panasonic-electronic-components/ERJ-3GEYJ102V/P1.0KGDKR-ND/577615" V 6230 2450 50  0001 C CNN
F 4 "0402WGF1001TCE" V 6330 2550 60  0001 C CNN "Part Number"
F 5 "C11702" H 6150 2450 50  0001 C CNN "LCSC"
	1    6150 2450
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:R_1K_0402 R18
U 1 1 61E83688
P 6650 1950
F 0 "R18" V 6650 1900 50  0000 L CNN
F 1 "R_1K_0402" H 6200 2100 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 6580 1950 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/panasonic-electronic-components/ERJ-3GEYJ102V/P1.0KGDKR-ND/577615" V 6730 1950 50  0001 C CNN
F 4 "0402WGF1001TCE" V 6830 2050 60  0001 C CNN "Part Number"
F 5 "C11702" H 6650 1950 50  0001 C CNN "LCSC"
	1    6650 1950
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61E83692
P 6650 2700
AR Path="/5ABD187E/61E83692" Ref="#PWR?"  Part="1" 
AR Path="/61E83692" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/61E83692" Ref="#PWR?"  Part="1" 
AR Path="/6045445B/61E83692" Ref="#PWR?"  Part="1" 
AR Path="/61E50C45/61E83692" Ref="#PWR?"  Part="1" 
AR Path="/61E5C0D3/61E83692" Ref="#PWR?"  Part="1" 
AR Path="/61E5C138/61E83692" Ref="#PWR?"  Part="1" 
AR Path="/61E73398/61E83692" Ref="#PWR020"  Part="1" 
F 0 "#PWR020" H 6650 2450 50  0001 C CNN
F 1 "GND" H 6650 2550 50  0000 C CNN
F 2 "" H 6650 2700 50  0000 C CNN
F 3 "" H 6650 2700 50  0000 C CNN
	1    6650 2700
	1    0    0    -1  
$EndComp
Wire Wire Line
	6650 2700 6650 2650
Wire Wire Line
	6300 2450 6350 2450
Text Label 5950 2450 2    60   ~ 0
IN3
Wire Wire Line
	6000 2450 5950 2450
Wire Wire Line
	6650 1750 6650 1800
$Comp
L Open_Automation:Q_MMBT5551 Q10
U 1 1 61E836A5
P 8150 2500
F 0 "Q10" H 8341 2546 50  0000 L CNN
F 1 "Q_MMBT5551" H 8341 2455 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 8350 2600 29  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Changjiang-Electronics-Tech-CJ-SS8050_C2150.pdf" H 8150 2500 60  0001 C CNN
F 4 "MMBT5551" H 8150 2500 50  0001 C CNN "Part Number"
F 5 "C2145" H 8150 2500 50  0001 C CNN "LCSC"
	1    8150 2500
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:R_1K_0402 R19
U 1 1 61E836B1
P 7750 2500
F 0 "R19" V 7750 2450 50  0000 L CNN
F 1 "R_1K_0402" V 7850 2350 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 7680 2500 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/panasonic-electronic-components/ERJ-3GEYJ102V/P1.0KGDKR-ND/577615" V 7830 2500 50  0001 C CNN
F 4 "0402WGF1001TCE" V 7930 2600 60  0001 C CNN "Part Number"
F 5 "C11702" H 7750 2500 50  0001 C CNN "LCSC"
	1    7750 2500
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:R_1K_0402 R21
U 1 1 61E836BD
P 8250 2000
F 0 "R21" V 8250 1950 50  0000 L CNN
F 1 "R_1K_0402" H 7800 2150 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 8180 2000 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/panasonic-electronic-components/ERJ-3GEYJ102V/P1.0KGDKR-ND/577615" V 8330 2000 50  0001 C CNN
F 4 "0402WGF1001TCE" V 8430 2100 60  0001 C CNN "Part Number"
F 5 "C11702" H 8250 2000 50  0001 C CNN "LCSC"
	1    8250 2000
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61E836C7
P 8250 2750
AR Path="/5ABD187E/61E836C7" Ref="#PWR?"  Part="1" 
AR Path="/61E836C7" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/61E836C7" Ref="#PWR?"  Part="1" 
AR Path="/6045445B/61E836C7" Ref="#PWR?"  Part="1" 
AR Path="/61E50C45/61E836C7" Ref="#PWR?"  Part="1" 
AR Path="/61E5C0D3/61E836C7" Ref="#PWR?"  Part="1" 
AR Path="/61E5C138/61E836C7" Ref="#PWR?"  Part="1" 
AR Path="/61E73398/61E836C7" Ref="#PWR022"  Part="1" 
F 0 "#PWR022" H 8250 2500 50  0001 C CNN
F 1 "GND" H 8250 2600 50  0000 C CNN
F 2 "" H 8250 2750 50  0000 C CNN
F 3 "" H 8250 2750 50  0000 C CNN
	1    8250 2750
	1    0    0    -1  
$EndComp
Wire Wire Line
	8250 2750 8250 2700
Wire Wire Line
	7900 2500 7950 2500
Text Label 7550 2500 2    60   ~ 0
IN4
Wire Wire Line
	7600 2500 7550 2500
Wire Wire Line
	8250 1800 8250 1850
$Comp
L power:GND #PWR?
U 1 1 61E865A3
P 3350 4600
AR Path="/5ABD187E/61E865A3" Ref="#PWR?"  Part="1" 
AR Path="/61E865A3" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/61E865A3" Ref="#PWR?"  Part="1" 
AR Path="/6045445B/61E865A3" Ref="#PWR?"  Part="1" 
AR Path="/61E50C45/61E865A3" Ref="#PWR?"  Part="1" 
AR Path="/61E5C0D3/61E865A3" Ref="#PWR?"  Part="1" 
AR Path="/61E5C138/61E865A3" Ref="#PWR?"  Part="1" 
AR Path="/61E73398/61E865A3" Ref="#PWR016"  Part="1" 
F 0 "#PWR016" H 3350 4350 50  0001 C CNN
F 1 "GND" H 3350 4450 50  0000 C CNN
F 2 "" H 3350 4600 50  0000 C CNN
F 3 "" H 3350 4600 50  0000 C CNN
	1    3350 4600
	1    0    0    -1  
$EndComp
Wire Wire Line
	3350 4600 3350 4550
Wire Wire Line
	3000 4350 3050 4350
Text Label 2650 4350 2    60   ~ 0
IO_OUT1
Wire Wire Line
	2700 4350 2650 4350
$Comp
L Open_Automation:Q_MMBT5551 Q6
U 1 1 61E865B6
P 4950 4350
F 0 "Q6" H 5141 4396 50  0000 L CNN
F 1 "Q_MMBT5551" H 5141 4305 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 5150 4450 29  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Changjiang-Electronics-Tech-CJ-SS8050_C2150.pdf" H 4950 4350 60  0001 C CNN
F 4 "MMBT5551" H 4950 4350 50  0001 C CNN "Part Number"
F 5 "C2145" H 4950 4350 50  0001 C CNN "LCSC"
	1    4950 4350
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61E865D8
P 5050 4600
AR Path="/5ABD187E/61E865D8" Ref="#PWR?"  Part="1" 
AR Path="/61E865D8" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/61E865D8" Ref="#PWR?"  Part="1" 
AR Path="/6045445B/61E865D8" Ref="#PWR?"  Part="1" 
AR Path="/61E50C45/61E865D8" Ref="#PWR?"  Part="1" 
AR Path="/61E5C0D3/61E865D8" Ref="#PWR?"  Part="1" 
AR Path="/61E5C138/61E865D8" Ref="#PWR?"  Part="1" 
AR Path="/61E73398/61E865D8" Ref="#PWR018"  Part="1" 
F 0 "#PWR018" H 5050 4350 50  0001 C CNN
F 1 "GND" H 5050 4450 50  0000 C CNN
F 2 "" H 5050 4600 50  0000 C CNN
F 3 "" H 5050 4600 50  0000 C CNN
	1    5050 4600
	1    0    0    -1  
$EndComp
Wire Wire Line
	5050 4600 5050 4550
Wire Wire Line
	4700 4350 4750 4350
Text Label 4350 4350 2    60   ~ 0
IO_OUT2
Wire Wire Line
	4400 4350 4350 4350
$Comp
L Open_Automation:Q_MMBT5551 Q9
U 1 1 61E865EB
P 6700 4400
F 0 "Q9" H 6891 4446 50  0000 L CNN
F 1 "Q_MMBT5551" H 6891 4355 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 6900 4500 29  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Changjiang-Electronics-Tech-CJ-SS8050_C2150.pdf" H 6700 4400 60  0001 C CNN
F 4 "MMBT5551" H 6700 4400 50  0001 C CNN "Part Number"
F 5 "C2145" H 6700 4400 50  0001 C CNN "LCSC"
	1    6700 4400
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61E8660D
P 6800 4650
AR Path="/5ABD187E/61E8660D" Ref="#PWR?"  Part="1" 
AR Path="/61E8660D" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/61E8660D" Ref="#PWR?"  Part="1" 
AR Path="/6045445B/61E8660D" Ref="#PWR?"  Part="1" 
AR Path="/61E50C45/61E8660D" Ref="#PWR?"  Part="1" 
AR Path="/61E5C0D3/61E8660D" Ref="#PWR?"  Part="1" 
AR Path="/61E5C138/61E8660D" Ref="#PWR?"  Part="1" 
AR Path="/61E73398/61E8660D" Ref="#PWR021"  Part="1" 
F 0 "#PWR021" H 6800 4400 50  0001 C CNN
F 1 "GND" H 6800 4500 50  0000 C CNN
F 2 "" H 6800 4650 50  0000 C CNN
F 3 "" H 6800 4650 50  0000 C CNN
	1    6800 4650
	1    0    0    -1  
$EndComp
Wire Wire Line
	6800 4650 6800 4600
Wire Wire Line
	6450 4400 6500 4400
Wire Wire Line
	6800 4150 6800 4200
Text Label 6100 4400 2    60   ~ 0
IO_OUT3
Text Label 7850 4400 2    60   ~ 0
IO_OUT4
Text HLabel 950  1700 0    60   Input ~ 0
OUT1
Text Label 1000 1700 0    60   ~ 0
OUT1
Wire Wire Line
	1000 1700 950  1700
Text HLabel 950  1800 0    60   Input ~ 0
OUT2
Text Label 1000 1800 0    60   ~ 0
OUT2
Wire Wire Line
	1000 1800 950  1800
Text HLabel 950  1900 0    60   Input ~ 0
OUT3
Text Label 1000 1900 0    60   ~ 0
OUT3
Wire Wire Line
	1000 1900 950  1900
Text HLabel 950  2000 0    60   Input ~ 0
OUT4
Text Label 1000 2000 0    60   ~ 0
OUT4
Wire Wire Line
	1000 2000 950  2000
Text HLabel 950  2100 0    60   Input ~ 0
OUT5
Text Label 1000 2100 0    60   ~ 0
OUT5
Wire Wire Line
	1000 2100 950  2100
Text HLabel 1000 900  0    60   Input ~ 0
3V3
Text Label 1050 900  0    60   ~ 0
3V3
Wire Wire Line
	1050 900  1000 900 
Text Label 3500 1650 0    60   ~ 0
3V3
Text Label 5100 1700 0    60   ~ 0
3V3
Text Label 6650 1750 0    60   ~ 0
3V3
Text Label 8250 1800 0    60   ~ 0
3V3
Text HLabel 1700 1050 0    60   Input ~ 0
IO_IN1
Text Label 1750 1050 0    60   ~ 0
IO_IN1
Wire Wire Line
	1750 1050 1700 1050
Text HLabel 1700 1150 0    60   Input ~ 0
IO_IN2
Text Label 1750 1150 0    60   ~ 0
IO_IN2
Wire Wire Line
	1750 1150 1700 1150
Text HLabel 1700 1250 0    60   Input ~ 0
IO_IN3
Text Label 1750 1250 0    60   ~ 0
IO_IN3
Wire Wire Line
	1750 1250 1700 1250
Text HLabel 1700 1350 0    60   Input ~ 0
IO_IN4
Text Label 1750 1350 0    60   ~ 0
IO_IN4
Wire Wire Line
	1750 1350 1700 1350
Text HLabel 1700 1450 0    60   Input ~ 0
IO_IN5
Text Label 1750 1450 0    60   ~ 0
IO_IN5
Wire Wire Line
	1750 1450 1700 1450
Text HLabel 1800 1700 0    60   Input ~ 0
IO_OUT1
Text Label 1850 1700 0    60   ~ 0
IO_OUT1
Wire Wire Line
	1850 1700 1800 1700
Text HLabel 1800 1800 0    60   Input ~ 0
IO_OUT2
Text Label 1850 1800 0    60   ~ 0
IO_OUT2
Wire Wire Line
	1850 1800 1800 1800
Text Label 1850 1900 0    60   ~ 0
IO_OUT3
Wire Wire Line
	1850 1900 1800 1900
Text HLabel 1800 2000 0    60   Input ~ 0
IO_OUT4
Text Label 1850 2000 0    60   ~ 0
IO_OUT4
Wire Wire Line
	1850 2000 1800 2000
Text HLabel 1800 1900 0    60   Input ~ 0
IO_OUT3
Text HLabel 1800 2100 0    60   Input ~ 0
IO_OUT5
Text Label 1850 2100 0    60   ~ 0
IO_OUT5
Wire Wire Line
	1850 2100 1800 2100
Wire Wire Line
	3500 2000 3500 2100
Wire Wire Line
	5100 2050 5100 2150
Wire Wire Line
	6650 2100 6650 2200
Wire Wire Line
	8250 2150 8250 2250
Text Label 3450 2100 2    60   ~ 0
IO_IN1
Wire Wire Line
	3450 2100 3500 2100
Connection ~ 3500 2100
Wire Wire Line
	3500 2100 3500 2150
Text Label 5050 2150 2    60   ~ 0
IO_IN2
Wire Wire Line
	5050 2150 5100 2150
Connection ~ 5100 2150
Wire Wire Line
	5100 2150 5100 2200
Text Label 6600 2200 2    60   ~ 0
IO_IN3
Wire Wire Line
	6600 2200 6650 2200
Connection ~ 6650 2200
Wire Wire Line
	6650 2200 6650 2250
Text Label 8200 2250 2    60   ~ 0
IO_IN4
Wire Wire Line
	8200 2250 8250 2250
Connection ~ 8250 2250
Wire Wire Line
	8250 2250 8250 2300
$Comp
L Open_Automation:Q_MMBT5551 Q12
U 1 1 61ED0BDA
P 9700 2500
F 0 "Q12" H 9891 2546 50  0000 L CNN
F 1 "Q_MMBT5551" H 9891 2455 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 9900 2600 29  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Changjiang-Electronics-Tech-CJ-SS8050_C2150.pdf" H 9700 2500 60  0001 C CNN
F 4 "MMBT5551" H 9700 2500 50  0001 C CNN "Part Number"
F 5 "C2145" H 9700 2500 50  0001 C CNN "LCSC"
	1    9700 2500
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:R_1K_0402 R22
U 1 1 61ED0DAC
P 9300 2500
F 0 "R22" V 9300 2450 50  0000 L CNN
F 1 "R_1K_0402" V 9400 2350 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 9230 2500 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/panasonic-electronic-components/ERJ-3GEYJ102V/P1.0KGDKR-ND/577615" V 9380 2500 50  0001 C CNN
F 4 "0402WGF1001TCE" V 9480 2600 60  0001 C CNN "Part Number"
F 5 "C11702" H 9300 2500 50  0001 C CNN "LCSC"
	1    9300 2500
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:R_1K_0402 R24
U 1 1 61ED0DB8
P 9800 2000
F 0 "R24" V 9800 1950 50  0000 L CNN
F 1 "R_1K_0402" H 9350 2150 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 9730 2000 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/panasonic-electronic-components/ERJ-3GEYJ102V/P1.0KGDKR-ND/577615" V 9880 2000 50  0001 C CNN
F 4 "0402WGF1001TCE" V 9980 2100 60  0001 C CNN "Part Number"
F 5 "C11702" H 9800 2000 50  0001 C CNN "LCSC"
	1    9800 2000
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61ED0DC2
P 9800 2750
AR Path="/5ABD187E/61ED0DC2" Ref="#PWR?"  Part="1" 
AR Path="/61ED0DC2" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/61ED0DC2" Ref="#PWR?"  Part="1" 
AR Path="/6045445B/61ED0DC2" Ref="#PWR?"  Part="1" 
AR Path="/61E50C45/61ED0DC2" Ref="#PWR?"  Part="1" 
AR Path="/61E5C0D3/61ED0DC2" Ref="#PWR?"  Part="1" 
AR Path="/61E5C138/61ED0DC2" Ref="#PWR?"  Part="1" 
AR Path="/61E73398/61ED0DC2" Ref="#PWR024"  Part="1" 
F 0 "#PWR024" H 9800 2500 50  0001 C CNN
F 1 "GND" H 9800 2600 50  0000 C CNN
F 2 "" H 9800 2750 50  0000 C CNN
F 3 "" H 9800 2750 50  0000 C CNN
	1    9800 2750
	1    0    0    -1  
$EndComp
Wire Wire Line
	9800 2750 9800 2700
Wire Wire Line
	9450 2500 9500 2500
Text Label 9100 2500 2    60   ~ 0
IN5
Wire Wire Line
	9150 2500 9100 2500
Wire Wire Line
	9800 1800 9800 1850
$Comp
L Open_Automation:Q_MMBT5551 Q13
U 1 1 61ED0DD3
P 10050 4400
F 0 "Q13" H 10241 4446 50  0000 L CNN
F 1 "Q_MMBT5551" H 10241 4355 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 10250 4500 29  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Changjiang-Electronics-Tech-CJ-SS8050_C2150.pdf" H 10050 4400 60  0001 C CNN
F 4 "MMBT5551" H 10050 4400 50  0001 C CNN "Part Number"
F 5 "C2145" H 10050 4400 50  0001 C CNN "LCSC"
	1    10050 4400
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61ED0DF5
P 10150 4650
AR Path="/5ABD187E/61ED0DF5" Ref="#PWR?"  Part="1" 
AR Path="/61ED0DF5" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/61ED0DF5" Ref="#PWR?"  Part="1" 
AR Path="/6045445B/61ED0DF5" Ref="#PWR?"  Part="1" 
AR Path="/61E50C45/61ED0DF5" Ref="#PWR?"  Part="1" 
AR Path="/61E5C0D3/61ED0DF5" Ref="#PWR?"  Part="1" 
AR Path="/61E5C138/61ED0DF5" Ref="#PWR?"  Part="1" 
AR Path="/61E73398/61ED0DF5" Ref="#PWR025"  Part="1" 
F 0 "#PWR025" H 10150 4400 50  0001 C CNN
F 1 "GND" H 10150 4500 50  0000 C CNN
F 2 "" H 10150 4650 50  0000 C CNN
F 3 "" H 10150 4650 50  0000 C CNN
	1    10150 4650
	1    0    0    -1  
$EndComp
Wire Wire Line
	10150 4650 10150 4600
Wire Wire Line
	9800 4400 9850 4400
Wire Wire Line
	10150 4150 10150 4200
Text Label 9450 4400 2    60   ~ 0
IO_OUT5
Wire Wire Line
	9500 4400 9450 4400
Text Label 9800 1800 0    60   ~ 0
3V3
Wire Wire Line
	9800 2150 9800 2250
Text Label 9750 2250 2    60   ~ 0
IO_IN5
Wire Wire Line
	9750 2250 9800 2250
Connection ~ 9800 2250
Wire Wire Line
	9800 2250 9800 2300
Wire Wire Line
	5050 4100 5050 4150
$Comp
L Open_Automation:Q_MMBT5551 Q4
U 1 1 61E86493
P 3250 4350
F 0 "Q4" H 3441 4396 50  0000 L CNN
F 1 "Q_MMBT5551" H 3441 4305 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 3450 4450 29  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Changjiang-Electronics-Tech-CJ-SS8050_C2150.pdf" H 3250 4350 60  0001 C CNN
F 4 "MMBT5551" H 3250 4350 50  0001 C CNN "Part Number"
F 5 "C2145" H 3250 4350 50  0001 C CNN "LCSC"
	1    3250 4350
	1    0    0    -1  
$EndComp
Text Label 3350 4100 2    60   ~ 0
OUT1
Wire Wire Line
	3350 4100 3350 4150
Text Label 5050 4100 2    60   ~ 0
OUT2
Text Label 6800 4150 2    60   ~ 0
OUT3
Text Label 10150 4150 2    60   ~ 0
OUT5
Text Label 8550 4150 2    60   ~ 0
OUT4
Wire Wire Line
	7900 4400 7850 4400
Wire Wire Line
	8550 4150 8550 4200
Wire Wire Line
	8200 4400 8250 4400
Wire Wire Line
	8550 4650 8550 4600
$Comp
L power:GND #PWR?
U 1 1 61E86642
P 8550 4650
AR Path="/5ABD187E/61E86642" Ref="#PWR?"  Part="1" 
AR Path="/61E86642" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/61E86642" Ref="#PWR?"  Part="1" 
AR Path="/6045445B/61E86642" Ref="#PWR?"  Part="1" 
AR Path="/61E50C45/61E86642" Ref="#PWR?"  Part="1" 
AR Path="/61E5C0D3/61E86642" Ref="#PWR?"  Part="1" 
AR Path="/61E5C138/61E86642" Ref="#PWR?"  Part="1" 
AR Path="/61E73398/61E86642" Ref="#PWR023"  Part="1" 
F 0 "#PWR023" H 8550 4400 50  0001 C CNN
F 1 "GND" H 8550 4500 50  0000 C CNN
F 2 "" H 8550 4650 50  0000 C CNN
F 3 "" H 8550 4650 50  0000 C CNN
	1    8550 4650
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:Q_MMBT5551 Q11
U 1 1 61E86620
P 8450 4400
F 0 "Q11" H 8641 4446 50  0000 L CNN
F 1 "Q_MMBT5551" H 8641 4355 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 8650 4500 29  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Changjiang-Electronics-Tech-CJ-SS8050_C2150.pdf" H 8450 4400 60  0001 C CNN
F 4 "MMBT5551" H 8450 4400 50  0001 C CNN "Part Number"
F 5 "C2145" H 8450 4400 50  0001 C CNN "LCSC"
	1    8450 4400
	1    0    0    -1  
$EndComp
Wire Wire Line
	6100 4400 6150 4400
$Comp
L Open_Automation:D_SM4007PL D5
U 1 1 61E674AA
P 3500 4100
F 0 "D5" H 3400 4150 50  0000 C CNN
F 1 "D_SM4007PL" H 3350 4000 50  0000 C CNN
F 2 "Diode_SMD:D_SOD-123F" H 3500 3925 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/lcsc/1811131822_Amphenol-ICC-10118194-0001LF_C132563.pdf" H 3500 4100 50  0001 C CNN
F 4 "SM4007PL" H 3500 4100 50  0001 C CNN "Part Number"
F 5 "C64898" H 3500 4100 50  0001 C CNN "LCSC"
	1    3500 4100
	-1   0    0    1   
$EndComp
Text Label 3700 4100 0    60   ~ 0
PWR
Wire Wire Line
	3700 4100 3650 4100
$Comp
L Open_Automation:D_SM4007PL D6
U 1 1 61E6F86C
P 5200 4100
F 0 "D6" H 5100 4150 50  0000 C CNN
F 1 "D_SM4007PL" H 5050 4000 50  0000 C CNN
F 2 "Diode_SMD:D_SOD-123F" H 5200 3925 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/lcsc/1811131822_Amphenol-ICC-10118194-0001LF_C132563.pdf" H 5200 4100 50  0001 C CNN
F 4 "SM4007PL" H 5200 4100 50  0001 C CNN "Part Number"
F 5 "C64898" H 5200 4100 50  0001 C CNN "LCSC"
	1    5200 4100
	-1   0    0    1   
$EndComp
Text Label 5400 4100 0    60   ~ 0
PWR
Wire Wire Line
	5400 4100 5350 4100
$Comp
L Open_Automation:D_SM4007PL D7
U 1 1 61E7492A
P 6950 4150
F 0 "D7" H 6850 4200 50  0000 C CNN
F 1 "D_SM4007PL" H 6800 4050 50  0000 C CNN
F 2 "Diode_SMD:D_SOD-123F" H 6950 3975 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/lcsc/1811131822_Amphenol-ICC-10118194-0001LF_C132563.pdf" H 6950 4150 50  0001 C CNN
F 4 "SM4007PL" H 6950 4150 50  0001 C CNN "Part Number"
F 5 "C64898" H 6950 4150 50  0001 C CNN "LCSC"
	1    6950 4150
	-1   0    0    1   
$EndComp
Text Label 7150 4150 0    60   ~ 0
PWR
Wire Wire Line
	7150 4150 7100 4150
$Comp
L Open_Automation:D_SM4007PL D8
U 1 1 61E7A92E
P 8700 4150
F 0 "D8" H 8600 4200 50  0000 C CNN
F 1 "D_SM4007PL" H 8550 4050 50  0000 C CNN
F 2 "Diode_SMD:D_SOD-123F" H 8700 3975 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/lcsc/1811131822_Amphenol-ICC-10118194-0001LF_C132563.pdf" H 8700 4150 50  0001 C CNN
F 4 "SM4007PL" H 8700 4150 50  0001 C CNN "Part Number"
F 5 "C64898" H 8700 4150 50  0001 C CNN "LCSC"
	1    8700 4150
	-1   0    0    1   
$EndComp
Text Label 8900 4150 0    60   ~ 0
PWR
Wire Wire Line
	8900 4150 8850 4150
$Comp
L Open_Automation:D_SM4007PL D9
U 1 1 61E7EBEB
P 10300 4150
F 0 "D9" H 10200 4200 50  0000 C CNN
F 1 "D_SM4007PL" H 10150 4050 50  0000 C CNN
F 2 "Diode_SMD:D_SOD-123F" H 10300 3975 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/lcsc/1811131822_Amphenol-ICC-10118194-0001LF_C132563.pdf" H 10300 4150 50  0001 C CNN
F 4 "SM4007PL" H 10300 4150 50  0001 C CNN "Part Number"
F 5 "C64898" H 10300 4150 50  0001 C CNN "LCSC"
	1    10300 4150
	-1   0    0    1   
$EndComp
Text Label 10500 4150 0    60   ~ 0
PWR
Wire Wire Line
	10500 4150 10450 4150
$Comp
L Open_Automation:R_100 R10
U 1 1 62071C2D
P 2850 4350
F 0 "R10" V 2850 4350 50  0000 C CNN
F 1 "R_100" V 2950 4350 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 2780 4350 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0402WGF1000TCE_C25076.pdf" V 2930 4350 50  0001 C CNN
F 4 "0402WGF1000TCE" V 3030 4450 60  0001 C CNN "Part Number"
F 5 "C25076" V 3150 4350 50  0001 C CNN "LCSC"
	1    2850 4350
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:R_100 R13
U 1 1 620899AB
P 4550 4350
F 0 "R13" V 4550 4350 50  0000 C CNN
F 1 "R_100" V 4650 4350 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 4480 4350 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0402WGF1000TCE_C25076.pdf" V 4630 4350 50  0001 C CNN
F 4 "0402WGF1000TCE" V 4730 4450 60  0001 C CNN "Part Number"
F 5 "C25076" V 4850 4350 50  0001 C CNN "LCSC"
	1    4550 4350
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:R_100 R17
U 1 1 6208DF85
P 6350 4400
F 0 "R17" V 6350 4400 50  0000 C CNN
F 1 "R_100" V 6450 4400 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 6280 4400 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0402WGF1000TCE_C25076.pdf" V 6430 4400 50  0001 C CNN
F 4 "0402WGF1000TCE" V 6530 4500 60  0001 C CNN "Part Number"
F 5 "C25076" V 6650 4400 50  0001 C CNN "LCSC"
	1    6350 4400
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:R_100 R20
U 1 1 6208E189
P 8050 4400
F 0 "R20" V 8050 4400 50  0000 C CNN
F 1 "R_100" V 8150 4400 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 7980 4400 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0402WGF1000TCE_C25076.pdf" V 8130 4400 50  0001 C CNN
F 4 "0402WGF1000TCE" V 8230 4500 60  0001 C CNN "Part Number"
F 5 "C25076" V 8350 4400 50  0001 C CNN "LCSC"
	1    8050 4400
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:R_100 R23
U 1 1 62092CAB
P 9650 4400
F 0 "R23" V 9650 4400 50  0000 C CNN
F 1 "R_100" V 9750 4400 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 9580 4400 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0402WGF1000TCE_C25076.pdf" V 9730 4400 50  0001 C CNN
F 4 "0402WGF1000TCE" V 9830 4500 60  0001 C CNN "Part Number"
F 5 "C25076" V 9950 4400 50  0001 C CNN "LCSC"
	1    9650 4400
	0    1    1    0   
$EndComp
Connection ~ 6500 4400
$EndSCHEMATC
