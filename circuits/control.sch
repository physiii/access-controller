EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr User 8661 5906
encoding utf-8
Sheet 2 12
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
	1150 1050 1200 1050
Wire Wire Line
	1200 1200 1150 1200
Wire Wire Line
	1150 1300 1200 1300
Wire Wire Line
	1200 1400 1150 1400
Wire Wire Line
	1150 1500 1200 1500
Text HLabel 1150 1500 0    50   Input ~ 0
OUT1
Text HLabel 1150 1400 0    50   Input ~ 0
OUT0
Text HLabel 1150 1300 0    50   Input ~ 0
IN1
Text HLabel 1150 1200 0    50   Input ~ 0
IN0
Text HLabel 1150 1050 0    50   Input ~ 0
V_IN
Text Label 1200 1500 0    50   ~ 0
OUT1
Text Label 1200 1400 0    50   ~ 0
OUT0
Text Label 1200 1050 0    50   ~ 0
V_IN
Text Label 1200 1300 0    50   ~ 0
IN1
Text Label 1200 1200 0    50   ~ 0
IN0
Text Label 3250 2850 2    50   ~ 0
V_IN
Text Label 3350 1900 2    50   ~ 0
V_IN
Wire Wire Line
	2450 2600 2500 2600
Text Label 2450 2600 2    50   ~ 0
IN1
Wire Wire Line
	2450 2250 2500 2250
Text Label 2450 2250 2    50   ~ 0
IN0
Wire Wire Line
	3250 2850 3300 2850
Wire Wire Line
	3800 1900 3800 2150
Wire Wire Line
	3700 2550 3800 2550
Wire Wire Line
	3700 2850 3700 2550
Wire Wire Line
	3400 2150 3800 2150
Connection ~ 3700 2550
Wire Wire Line
	3650 2550 3700 2550
Wire Wire Line
	3650 2500 3650 2550
Wire Wire Line
	3400 2500 3650 2500
$Comp
L Open_Automation:R_10K R?
U 1 1 600435CF
P 3500 2850
AR Path="/5FBF1F61/600435CF" Ref="R?"  Part="1" 
AR Path="/5FFBD8EA/600435CF" Ref="R?"  Part="1" 
AR Path="/601F1685/600435CF" Ref="R?"  Part="1" 
AR Path="/601F213A/600435CF" Ref="R?"  Part="1" 
AR Path="/5FE8FE83/600435CF" Ref="R?"  Part="1" 
AR Path="/5FE5EA71/600435CF" Ref="R?"  Part="1" 
AR Path="/60030996/600435CF" Ref="R6"  Part="1" 
AR Path="/600E925B/600F0FE9/600435CF" Ref="R?"  Part="1" 
AR Path="/600F4C7B/600435CF" Ref="R?"  Part="1" 
AR Path="/605FA739/600435CF" Ref="R10"  Part="1" 
F 0 "R10" V 3500 2850 50  0000 C CNN
F 1 "R_10K" V 3400 2850 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad0.98x0.95mm_HandSolder" V 3430 2850 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0603WAF1002T5E_C25804.pdf" V 3580 2850 50  0001 C CNN
F 4 "0603WAF1002T5E" V 3680 2950 60  0001 C CNN "Part Number"
F 5 "C25804" H -4850 -1700 50  0001 C CNN "LCSC"
	1    3500 2850
	0    -1   -1   0   
$EndComp
Wire Wire Line
	2950 2250 2900 2250
Wire Wire Line
	2900 2600 2950 2600
$Comp
L Open_Automation:R_10K R?
U 1 1 600435D9
P 2700 2600
AR Path="/5FBF1F61/600435D9" Ref="R?"  Part="1" 
AR Path="/5FFBD8EA/600435D9" Ref="R?"  Part="1" 
AR Path="/601F1685/600435D9" Ref="R?"  Part="1" 
AR Path="/601F213A/600435D9" Ref="R?"  Part="1" 
AR Path="/5FE8FE83/600435D9" Ref="R?"  Part="1" 
AR Path="/5FE5EA71/600435D9" Ref="R?"  Part="1" 
AR Path="/60030996/600435D9" Ref="R5"  Part="1" 
AR Path="/600E925B/600F0FE9/600435D9" Ref="R?"  Part="1" 
AR Path="/600F4C7B/600435D9" Ref="R?"  Part="1" 
AR Path="/605FA739/600435D9" Ref="R9"  Part="1" 
F 0 "R9" V 2700 2600 50  0000 C CNN
F 1 "R_10K" V 2600 2600 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad0.98x0.95mm_HandSolder" V 2630 2600 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0603WAF1002T5E_C25804.pdf" V 2780 2600 50  0001 C CNN
F 4 "0603WAF1002T5E" V 2880 2700 60  0001 C CNN "Part Number"
F 5 "C25804" H -4850 -1700 50  0001 C CNN "LCSC"
	1    2700 2600
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3400 1900 3350 1900
$Comp
L Open_Automation:R_10K R?
U 1 1 600435E2
P 2700 2250
AR Path="/5FBF1F61/600435E2" Ref="R?"  Part="1" 
AR Path="/5FFBD8EA/600435E2" Ref="R?"  Part="1" 
AR Path="/601F1685/600435E2" Ref="R?"  Part="1" 
AR Path="/601F213A/600435E2" Ref="R?"  Part="1" 
AR Path="/5FE8FE83/600435E2" Ref="R?"  Part="1" 
AR Path="/5FE5EA71/600435E2" Ref="R?"  Part="1" 
AR Path="/60030996/600435E2" Ref="R4"  Part="1" 
AR Path="/600E925B/600F0FE9/600435E2" Ref="R?"  Part="1" 
AR Path="/600F4C7B/600435E2" Ref="R?"  Part="1" 
AR Path="/605FA739/600435E2" Ref="R8"  Part="1" 
F 0 "R8" V 2700 2250 50  0000 C CNN
F 1 "R_10K" V 2600 2250 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad0.98x0.95mm_HandSolder" V 2630 2250 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0603WAF1002T5E_C25804.pdf" V 2780 2250 50  0001 C CNN
F 4 "0603WAF1002T5E" V 2880 2350 60  0001 C CNN "Part Number"
F 5 "C25804" H -4850 -1700 50  0001 C CNN "LCSC"
	1    2700 2250
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3450 2700 3400 2700
Wire Wire Line
	3450 2350 3400 2350
Wire Wire Line
	4250 2400 4200 2400
Text Label 4250 2400 0    50   ~ 0
OUT1
Wire Wire Line
	4250 2700 4200 2700
Wire Wire Line
	4250 2300 4200 2300
Wire Wire Line
	4250 2000 4200 2000
Text Label 4250 2000 0    50   ~ 0
OUT0
$Comp
L Open_Automation:R_10K R?
U 1 1 6004360A
P 3600 1900
AR Path="/5FBF1F61/6004360A" Ref="R?"  Part="1" 
AR Path="/5FFBD8EA/6004360A" Ref="R?"  Part="1" 
AR Path="/601F1685/6004360A" Ref="R?"  Part="1" 
AR Path="/601F213A/6004360A" Ref="R?"  Part="1" 
AR Path="/5FE8FE83/6004360A" Ref="R?"  Part="1" 
AR Path="/5FE5EA71/6004360A" Ref="R?"  Part="1" 
AR Path="/60030996/6004360A" Ref="R7"  Part="1" 
AR Path="/600E925B/600F0FE9/6004360A" Ref="R?"  Part="1" 
AR Path="/600F4C7B/6004360A" Ref="R?"  Part="1" 
AR Path="/605FA739/6004360A" Ref="R11"  Part="1" 
F 0 "R11" V 3600 1900 50  0000 C CNN
F 1 "R_10K" V 3500 1900 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad0.98x0.95mm_HandSolder" V 3530 1900 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0603WAF1002T5E_C25804.pdf" V 3680 1900 50  0001 C CNN
F 4 "0603WAF1002T5E" V 3780 2000 60  0001 C CNN "Part Number"
F 5 "C25804" H -4850 -1700 50  0001 C CNN "LCSC"
	1    3600 1900
	0    -1   -1   0   
$EndComp
$Comp
L Open_Automation:Q_MBT3904 Q?
U 1 1 60043612
P 3150 2250
AR Path="/5FBF1F61/60043612" Ref="Q?"  Part="1" 
AR Path="/601F1685/60043612" Ref="Q?"  Part="1" 
AR Path="/601F213A/60043612" Ref="Q?"  Part="1" 
AR Path="/5FE8FE83/60043612" Ref="Q?"  Part="1" 
AR Path="/5FE5EA71/60043612" Ref="Q?"  Part="1" 
AR Path="/60030996/60043612" Ref="Q1"  Part="1" 
AR Path="/600E925B/600F0FE9/60043612" Ref="Q?"  Part="1" 
AR Path="/600F4C7B/60043612" Ref="Q?"  Part="1" 
AR Path="/605FA739/60043612" Ref="Q3"  Part="1" 
F 0 "Q3" H 3175 2531 50  0000 C CNN
F 1 "Q_MBT3904" H 3175 2440 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-363_SC-70-6_Handsoldering" H 2300 2450 50  0001 L CIN
F 3 "https://datasheet.lcsc.com/szlcsc/Leshan-Radio-LMBT3904DW1T1G_C131781.pdf" H 3150 2250 50  0001 L CNN
F 4 "LMBT3904DW1T1G" H 3200 2650 50  0001 C CNN "Part Number"
F 5 "C131781" H -4850 -1700 50  0001 C CNN "LCSC"
	1    3150 2250
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:Q_IRF7907TRPBF Q2
U 1 1 6055AEB9
P 4100 2400
AR Path="/60030996/6055AEB9" Ref="Q2"  Part="1" 
AR Path="/605FA739/6055AEB9" Ref="Q4"  Part="1" 
F 0 "Q4" H 4150 1900 50  0000 C CNN
F 1 "Q_IRF7907TRPBF" H 4150 2000 50  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 4150 3000 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/1808281653_Infineon-Technologies-IRF7907TRPBF_C168886.pdf" H 4000 2250 50  0001 C CNN
F 4 "IRF7907TRPBF" H 4100 3100 50  0001 C CNN "Part Number"
F 5 "C168886" H 4100 3200 50  0001 C CNN "LCSC"
	1    4100 2400
	1    0    0    -1  
$EndComp
Connection ~ 3800 2150
$Comp
L power:GND #PWR?
U 1 1 5FDE4427
P 4250 2300
AR Path="/5ABD187E/5FDE4427" Ref="#PWR?"  Part="1" 
AR Path="/5FDE4427" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/5FDE4427" Ref="#PWR?"  Part="1" 
AR Path="/60030996/5FDE4427" Ref="#PWR023"  Part="1" 
AR Path="/605FA739/5FDE4427" Ref="#PWR027"  Part="1" 
F 0 "#PWR027" H 4250 2050 50  0001 C CNN
F 1 "GND" V 4300 2150 50  0000 C CNN
F 2 "" H 4250 2300 50  0001 C CNN
F 3 "" H 4250 2300 50  0001 C CNN
	1    4250 2300
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5FDE59F9
P 4250 2700
AR Path="/5ABD187E/5FDE59F9" Ref="#PWR?"  Part="1" 
AR Path="/5FDE59F9" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/5FDE59F9" Ref="#PWR?"  Part="1" 
AR Path="/60030996/5FDE59F9" Ref="#PWR024"  Part="1" 
AR Path="/605FA739/5FDE59F9" Ref="#PWR028"  Part="1" 
F 0 "#PWR028" H 4250 2450 50  0001 C CNN
F 1 "GND" V 4300 2550 50  0000 C CNN
F 2 "" H 4250 2700 50  0001 C CNN
F 3 "" H 4250 2700 50  0001 C CNN
	1    4250 2700
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5FDE8400
P 3450 2700
AR Path="/5ABD187E/5FDE8400" Ref="#PWR?"  Part="1" 
AR Path="/5FDE8400" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/5FDE8400" Ref="#PWR?"  Part="1" 
AR Path="/60030996/5FDE8400" Ref="#PWR022"  Part="1" 
AR Path="/605FA739/5FDE8400" Ref="#PWR026"  Part="1" 
F 0 "#PWR026" H 3450 2450 50  0001 C CNN
F 1 "GND" V 3400 2550 50  0000 C CNN
F 2 "" H 3450 2700 50  0001 C CNN
F 3 "" H 3450 2700 50  0001 C CNN
	1    3450 2700
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5FDEC0EE
P 3450 2350
AR Path="/5ABD187E/5FDEC0EE" Ref="#PWR?"  Part="1" 
AR Path="/5FDEC0EE" Ref="#PWR?"  Part="1" 
AR Path="/5A7BADB8/5FDEC0EE" Ref="#PWR?"  Part="1" 
AR Path="/60030996/5FDEC0EE" Ref="#PWR021"  Part="1" 
AR Path="/605FA739/5FDEC0EE" Ref="#PWR025"  Part="1" 
F 0 "#PWR025" H 3450 2100 50  0001 C CNN
F 1 "GND" V 3400 2200 50  0000 C CNN
F 2 "" H 3450 2350 50  0001 C CNN
F 3 "" H 3450 2350 50  0001 C CNN
	1    3450 2350
	0    -1   -1   0   
$EndComp
$EndSCHEMATC
