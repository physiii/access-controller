EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr User 5906 4724
encoding utf-8
Sheet 6 16
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
	1100 1250 1150 1250
Wire Wire Line
	1150 1400 1100 1400
Wire Wire Line
	1100 1500 1150 1500
Wire Wire Line
	1150 1600 1100 1600
Wire Wire Line
	1100 1700 1150 1700
Text HLabel 1100 1700 0    50   Input ~ 0
OUT1
Text HLabel 1100 1600 0    50   Input ~ 0
OUT0
Text HLabel 1100 1500 0    50   Input ~ 0
IN1
Text HLabel 1100 1400 0    50   Input ~ 0
IN0
Text HLabel 1100 1250 0    50   Input ~ 0
V_IN
Text Label 1150 1700 0    50   ~ 0
OUT1
Text Label 1150 1600 0    50   ~ 0
OUT0
Text Label 1150 1250 0    50   ~ 0
V_IN
Text Label 1150 1500 0    50   ~ 0
IN1
Text Label 1150 1400 0    50   ~ 0
IN0
Text Label 3000 2100 2    50   ~ 0
V_IN
Text Label 3100 1150 2    50   ~ 0
V_IN
Wire Wire Line
	2200 1850 2250 1850
Text Label 2200 1850 2    50   ~ 0
IN1
Wire Wire Line
	2200 1500 2250 1500
Text Label 2200 1500 2    50   ~ 0
IN0
Wire Wire Line
	3000 2100 3050 2100
Wire Wire Line
	3550 1150 3550 1400
Wire Wire Line
	3450 1800 3550 1800
Wire Wire Line
	3450 2100 3450 1800
Wire Wire Line
	3150 1400 3550 1400
Connection ~ 3450 1800
Wire Wire Line
	3400 1800 3450 1800
Wire Wire Line
	3400 1750 3400 1800
Wire Wire Line
	3150 1750 3400 1750
$Comp
L Open_Automation:R_10K R?
U 1 1 5FE626EF
P 3250 2100
AR Path="/5FBF1F61/5FE626EF" Ref="R?"  Part="1" 
AR Path="/5FFBD8EA/5FE626EF" Ref="R?"  Part="1" 
AR Path="/601F1685/5FE626EF" Ref="R?"  Part="1" 
AR Path="/601F213A/5FE626EF" Ref="R?"  Part="1" 
AR Path="/5FE8FE83/5FE626EF" Ref="R?"  Part="1" 
AR Path="/5FE5EA71/5FE626EF" Ref="R?"  Part="1" 
F 0 "R?" V 3250 2100 50  0000 C CNN
F 1 "R_10K" V 3150 2100 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad0.98x0.95mm_HandSolder" V 3180 2100 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0603WAF1002T5E_C25804.pdf" V 3330 2100 50  0001 C CNN
F 4 "0603WAF1002T5E" V 3430 2200 60  0001 C CNN "Part Number"
F 5 "C25804" H -5100 -2450 50  0001 C CNN "LCSC"
	1    3250 2100
	0    -1   -1   0   
$EndComp
Wire Wire Line
	2700 1500 2650 1500
Wire Wire Line
	2650 1850 2700 1850
$Comp
L Open_Automation:R_10K R?
U 1 1 5FE626E5
P 2450 1850
AR Path="/5FBF1F61/5FE626E5" Ref="R?"  Part="1" 
AR Path="/5FFBD8EA/5FE626E5" Ref="R?"  Part="1" 
AR Path="/601F1685/5FE626E5" Ref="R?"  Part="1" 
AR Path="/601F213A/5FE626E5" Ref="R?"  Part="1" 
AR Path="/5FE8FE83/5FE626E5" Ref="R?"  Part="1" 
AR Path="/5FE5EA71/5FE626E5" Ref="R?"  Part="1" 
F 0 "R?" V 2450 1850 50  0000 C CNN
F 1 "R_10K" V 2350 1850 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad0.98x0.95mm_HandSolder" V 2380 1850 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0603WAF1002T5E_C25804.pdf" V 2530 1850 50  0001 C CNN
F 4 "0603WAF1002T5E" V 2630 1950 60  0001 C CNN "Part Number"
F 5 "C25804" H -5100 -2450 50  0001 C CNN "LCSC"
	1    2450 1850
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3150 1150 3100 1150
$Comp
L Open_Automation:R_10K R?
U 1 1 5FE626DC
P 2450 1500
AR Path="/5FBF1F61/5FE626DC" Ref="R?"  Part="1" 
AR Path="/5FFBD8EA/5FE626DC" Ref="R?"  Part="1" 
AR Path="/601F1685/5FE626DC" Ref="R?"  Part="1" 
AR Path="/601F213A/5FE626DC" Ref="R?"  Part="1" 
AR Path="/5FE8FE83/5FE626DC" Ref="R?"  Part="1" 
AR Path="/5FE5EA71/5FE626DC" Ref="R?"  Part="1" 
F 0 "R?" V 2450 1500 50  0000 C CNN
F 1 "R_10K" V 2350 1500 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad0.98x0.95mm_HandSolder" V 2380 1500 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0603WAF1002T5E_C25804.pdf" V 2530 1500 50  0001 C CNN
F 4 "0603WAF1002T5E" V 2630 1600 60  0001 C CNN "Part Number"
F 5 "C25804" H -5100 -2450 50  0001 C CNN "LCSC"
	1    2450 1500
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3200 1950 3150 1950
$Comp
L open-automation:GND #PWR?
U 1 1 5FE626D3
P 3200 1950
AR Path="/5ABD187E/5FE626D3" Ref="#PWR?"  Part="1" 
AR Path="/5FE626D3" Ref="#PWR?"  Part="1" 
AR Path="/5FBF1F61/5FE626D3" Ref="#PWR?"  Part="1" 
AR Path="/601F1685/5FE626D3" Ref="#PWR?"  Part="1" 
AR Path="/601F213A/5FE626D3" Ref="#PWR?"  Part="1" 
AR Path="/5FE8FE83/5FE626D3" Ref="#PWR?"  Part="1" 
AR Path="/5FE5EA71/5FE626D3" Ref="#PWR?"  Part="1" 
F 0 "#PWR?" H 3200 1700 50  0001 C CNN
F 1 "GND" V 3250 1800 50  0000 C CNN
F 2 "" H 3200 1950 50  0000 C CNN
F 3 "" H 3200 1950 50  0000 C CNN
	1    3200 1950
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3200 1600 3150 1600
$Comp
L open-automation:GND #PWR?
U 1 1 5FE626CC
P 3200 1600
AR Path="/5ABD187E/5FE626CC" Ref="#PWR?"  Part="1" 
AR Path="/5FE626CC" Ref="#PWR?"  Part="1" 
AR Path="/5FBF1F61/5FE626CC" Ref="#PWR?"  Part="1" 
AR Path="/601F1685/5FE626CC" Ref="#PWR?"  Part="1" 
AR Path="/601F213A/5FE626CC" Ref="#PWR?"  Part="1" 
AR Path="/5FE8FE83/5FE626CC" Ref="#PWR?"  Part="1" 
AR Path="/5FE5EA71/5FE626CC" Ref="#PWR?"  Part="1" 
F 0 "#PWR?" H 3200 1350 50  0001 C CNN
F 1 "GND" V 3250 1450 50  0000 C CNN
F 2 "" H 3200 1600 50  0000 C CNN
F 3 "" H 3200 1600 50  0000 C CNN
	1    3200 1600
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4000 1650 3950 1650
Text Label 4000 1650 0    50   ~ 0
OUT1
Wire Wire Line
	4000 1950 3950 1950
$Comp
L open-automation:GND #PWR?
U 1 1 5FE626C3
P 4000 1950
AR Path="/5ABD187E/5FE626C3" Ref="#PWR?"  Part="1" 
AR Path="/5FE626C3" Ref="#PWR?"  Part="1" 
AR Path="/5FBF1F61/5FE626C3" Ref="#PWR?"  Part="1" 
AR Path="/601F1685/5FE626C3" Ref="#PWR?"  Part="1" 
AR Path="/601F213A/5FE626C3" Ref="#PWR?"  Part="1" 
AR Path="/5FE8FE83/5FE626C3" Ref="#PWR?"  Part="1" 
AR Path="/5FE5EA71/5FE626C3" Ref="#PWR?"  Part="1" 
F 0 "#PWR?" H 4000 1700 50  0001 C CNN
F 1 "GND" V 4050 1800 50  0000 C CNN
F 2 "" H 4000 1950 50  0000 C CNN
F 3 "" H 4000 1950 50  0000 C CNN
	1    4000 1950
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4000 1550 3950 1550
$Comp
L open-automation:GND #PWR?
U 1 1 5FE626BC
P 4000 1550
AR Path="/5ABD187E/5FE626BC" Ref="#PWR?"  Part="1" 
AR Path="/5FE626BC" Ref="#PWR?"  Part="1" 
AR Path="/5FBF1F61/5FE626BC" Ref="#PWR?"  Part="1" 
AR Path="/601F1685/5FE626BC" Ref="#PWR?"  Part="1" 
AR Path="/601F213A/5FE626BC" Ref="#PWR?"  Part="1" 
AR Path="/5FE8FE83/5FE626BC" Ref="#PWR?"  Part="1" 
AR Path="/5FE5EA71/5FE626BC" Ref="#PWR?"  Part="1" 
F 0 "#PWR?" H 4000 1300 50  0001 C CNN
F 1 "GND" V 4050 1400 50  0000 C CNN
F 2 "" H 4000 1550 50  0000 C CNN
F 3 "" H 4000 1550 50  0000 C CNN
	1    4000 1550
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4000 1250 3950 1250
Text Label 4000 1250 0    50   ~ 0
OUT0
$Comp
L Open_Automation:R_10K R?
U 1 1 5FE626B4
P 3350 1150
AR Path="/5FBF1F61/5FE626B4" Ref="R?"  Part="1" 
AR Path="/5FFBD8EA/5FE626B4" Ref="R?"  Part="1" 
AR Path="/601F1685/5FE626B4" Ref="R?"  Part="1" 
AR Path="/601F213A/5FE626B4" Ref="R?"  Part="1" 
AR Path="/5FE8FE83/5FE626B4" Ref="R?"  Part="1" 
AR Path="/5FE5EA71/5FE626B4" Ref="R?"  Part="1" 
F 0 "R?" V 3350 1150 50  0000 C CNN
F 1 "R_10K" V 3250 1150 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad0.98x0.95mm_HandSolder" V 3280 1150 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0603WAF1002T5E_C25804.pdf" V 3430 1150 50  0001 C CNN
F 4 "0603WAF1002T5E" V 3530 1250 60  0001 C CNN "Part Number"
F 5 "C25804" H -5100 -2450 50  0001 C CNN "LCSC"
	1    3350 1150
	0    -1   -1   0   
$EndComp
$Comp
L Open_Automation:Q_MBT3904 Q?
U 1 1 5FE626AC
P 2900 1500
AR Path="/5FBF1F61/5FE626AC" Ref="Q?"  Part="1" 
AR Path="/601F1685/5FE626AC" Ref="Q?"  Part="1" 
AR Path="/601F213A/5FE626AC" Ref="Q?"  Part="1" 
AR Path="/5FE8FE83/5FE626AC" Ref="Q?"  Part="1" 
AR Path="/5FE5EA71/5FE626AC" Ref="Q?"  Part="1" 
F 0 "Q?" H 2925 1781 50  0000 C CNN
F 1 "Q_MBT3904" H 2925 1690 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-363_SC-70-6_Handsoldering" H 2050 1700 50  0001 L CIN
F 3 "https://datasheet.lcsc.com/szlcsc/Leshan-Radio-LMBT3904DW1T1G_C131781.pdf" H 2900 1500 50  0001 L CNN
F 4 "LMBT3904DW1T1G" H 2950 1900 50  0001 C CNN "Part Number"
F 5 "C131781" H -5100 -2450 50  0001 C CNN "LCSC"
	1    2900 1500
	1    0    0    -1  
$EndComp
Connection ~ 3550 1400
$Comp
L Open_Automation:Q_DMT3020 Q?
U 1 1 5FE626A4
P 3850 1650
AR Path="/5FBF1F61/5FE626A4" Ref="Q?"  Part="1" 
AR Path="/601F1685/5FE626A4" Ref="Q?"  Part="1" 
AR Path="/601F213A/5FE626A4" Ref="Q?"  Part="1" 
AR Path="/5FE8FE83/5FE626A4" Ref="Q?"  Part="1" 
AR Path="/5FE5EA71/5FE626A4" Ref="Q?"  Part="1" 
F 0 "Q?" H 3850 1150 50  0000 C CNN
F 1 "Q_DMT3020" H 3850 1250 50  0000 C CNN
F 2 "Package_SO:SO-8_3.9x4.9mm_P1.27mm" H 3900 2250 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/1808280417_Diodes-Incorporated-DMT3020LSD-13_C268033.pdf" H 3750 1500 50  0001 C CNN
F 4 "DMT3020LSD-13" H 3850 2350 50  0001 C CNN "Part Number"
F 5 "C268033" H -5100 -2450 50  0001 C CNN "LCSC"
	1    3850 1650
	1    0    0    -1  
$EndComp
Wire Wire Line
	5600 1250 5600 1200
Text Label 5150 900  2    50   ~ 0
_LOCK0
Wire Wire Line
	5150 900  5200 900 
Wire Wire Line
	5150 600  5200 600 
Text Label 5600 350  0    50   ~ 0
3V3
Wire Wire Line
	5600 350  5600 400 
Wire Wire Line
	6000 800  6050 800 
Wire Wire Line
	6050 800  6050 750 
Text Label 6050 900  0    50   ~ 0
FAULT
Wire Wire Line
	6050 900  6000 900 
Text Label 6100 800  0    50   ~ 0
VIOUT
Wire Wire Line
	6100 800  6050 800 
Connection ~ 6050 800 
Wire Wire Line
	5600 350  6050 350 
$Comp
L Open_Automation:R_10K R?
U 1 1 60022CE0
P 6050 550
AR Path="/5FBF1F61/60022CE0" Ref="R?"  Part="1" 
AR Path="/5FFBD8EA/60022CE0" Ref="R?"  Part="1" 
AR Path="/6063934A/60022CE0" Ref="R?"  Part="1" 
AR Path="/5A7BADB8/60022CE0" Ref="R?"  Part="1" 
AR Path="/60022CE0" Ref="R?"  Part="1" 
F 0 "R?" H 6200 600 50  0000 C CNN
F 1 "R_10K" H 6200 700 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad0.98x0.95mm_HandSolder" V 5980 550 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0603WAF1002T5E_C25804.pdf" V 6130 550 50  0001 C CNN
F 4 "0603WAF1002T5E" V 6230 650 60  0001 C CNN "Part Number"
F 5 "C25804" H 650 -4250 50  0001 C CNN "LCSC"
	1    6050 550 
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:ACS711 U?
U 1 1 60022CE8
P 5600 800
F 0 "U?" H 5800 1050 50  0000 C CNN
F 1 "ACS711" H 5800 1150 50  0000 C CNN
F 2 "Sensor_Current:Allegro_QFN-12-10-1EP_3x3mm_P0.5mm" H 4550 1450 50  0001 L CIN
F 3 "https://datasheet.lcsc.com/szlcsc/Allegro-MicroSystems-LLC-ACS711KEXLT-15AB-T_C150824.pdf" H 5600 800 50  0001 C CNN
F 4 "ACS711KEXLT-15AB-T" H 5600 800 50  0001 C CNN "Part Number"
F 5 "C150824" H 5600 800 50  0001 C CNN "LCSC"
	1    5600 800 
	1    0    0    -1  
$EndComp
$Comp
L open-automation:GND #PWR?
U 1 1 60022CEE
P 5600 1250
F 0 "#PWR?" H 5600 1000 50  0001 C CNN
F 1 "GND" H 5600 1100 50  0000 C CNN
F 2 "" H 5600 1250 50  0000 C CNN
F 3 "" H 5600 1250 50  0000 C CNN
	1    5600 1250
	1    0    0    -1  
$EndComp
Text Label 5150 600  2    50   ~ 0
LOCK0
Wire Wire Line
	5600 2700 5600 2650
Text Label 5150 2350 2    50   ~ 0
_LOCK1
Wire Wire Line
	5150 2350 5200 2350
Wire Wire Line
	5150 2050 5200 2050
Text Label 5600 1800 0    50   ~ 0
3V3
Wire Wire Line
	5600 1800 5600 1850
Wire Wire Line
	6000 2250 6050 2250
Wire Wire Line
	6050 2250 6050 2200
Text Label 6050 2350 0    50   ~ 0
FAULT
Wire Wire Line
	6050 2350 6000 2350
Text Label 6100 2250 0    50   ~ 0
VIOUT
Wire Wire Line
	6100 2250 6050 2250
Connection ~ 6050 2250
Wire Wire Line
	5600 1800 6050 1800
$Comp
L Open_Automation:R_10K R?
U 1 1 60022D05
P 6050 2000
AR Path="/5FBF1F61/60022D05" Ref="R?"  Part="1" 
AR Path="/5FFBD8EA/60022D05" Ref="R?"  Part="1" 
AR Path="/6063934A/60022D05" Ref="R?"  Part="1" 
AR Path="/5A7BADB8/60022D05" Ref="R?"  Part="1" 
AR Path="/60022D05" Ref="R?"  Part="1" 
F 0 "R?" H 6200 2050 50  0000 C CNN
F 1 "R_10K" H 6200 2150 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad0.98x0.95mm_HandSolder" V 5980 2000 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0603WAF1002T5E_C25804.pdf" V 6130 2000 50  0001 C CNN
F 4 "0603WAF1002T5E" V 6230 2100 60  0001 C CNN "Part Number"
F 5 "C25804" H 650 -2800 50  0001 C CNN "LCSC"
	1    6050 2000
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:ACS711 U?
U 1 1 60022D0D
P 5600 2250
F 0 "U?" H 5800 2500 50  0000 C CNN
F 1 "ACS711" H 5800 2600 50  0000 C CNN
F 2 "Sensor_Current:Allegro_QFN-12-10-1EP_3x3mm_P0.5mm" H 4550 2900 50  0001 L CIN
F 3 "https://datasheet.lcsc.com/szlcsc/Allegro-MicroSystems-LLC-ACS711KEXLT-15AB-T_C150824.pdf" H 5600 2250 50  0001 C CNN
F 4 "ACS711KEXLT-15AB-T" H 5600 2250 50  0001 C CNN "Part Number"
F 5 "C150824" H 5600 2250 50  0001 C CNN "LCSC"
	1    5600 2250
	1    0    0    -1  
$EndComp
$Comp
L open-automation:GND #PWR?
U 1 1 60022D13
P 5600 2700
F 0 "#PWR?" H 5600 2450 50  0001 C CNN
F 1 "GND" H 5600 2550 50  0000 C CNN
F 2 "" H 5600 2700 50  0000 C CNN
F 3 "" H 5600 2700 50  0000 C CNN
	1    5600 2700
	1    0    0    -1  
$EndComp
Text Label 5150 2050 2    50   ~ 0
LOCK1
$EndSCHEMATC
