EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr USLetter 11000 8500
encoding utf-8
Sheet 5 6
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text HLabel 1250 1550 0    60   Input ~ 0
RXD
Text HLabel 1250 1450 0    60   Input ~ 0
TXD
Text HLabel 1250 2050 0    60   Input ~ 0
IO5
Text HLabel 1250 950  0    60   Input ~ 0
3V3
Text Label 1300 950  0    50   ~ 0
3V3
Wire Wire Line
	1300 950  1250 950 
Text HLabel 1250 1950 0    60   Input ~ 0
IO4
Text Label 1300 1950 0    50   ~ 0
IO4
Wire Wire Line
	1300 1950 1250 1950
Text HLabel 1250 1700 0    60   Input ~ 0
SCL
Text HLabel 1250 1800 0    60   Input ~ 0
SDA
Text Label 1300 1800 0    50   ~ 0
SDA
Wire Wire Line
	1300 1800 1250 1800
Text Label 1300 1700 0    50   ~ 0
SCL
Wire Wire Line
	1300 1700 1250 1700
Text Label 1300 2050 0    50   ~ 0
IO5
Wire Wire Line
	1300 2050 1250 2050
Text Label 1300 1550 0    50   ~ 0
RXD
Wire Wire Line
	1300 1550 1250 1550
Text Label 1300 1450 0    50   ~ 0
TXD
Wire Wire Line
	1300 1450 1250 1450
Text HLabel 1250 3450 0    60   Input ~ 0
IO33
Text HLabel 1250 3350 0    60   Input ~ 0
IO32
Wire Wire Line
	1300 3450 1250 3450
Wire Wire Line
	1300 3350 1250 3350
Text Label 1300 3450 0    50   ~ 0
IO33
Text Label 1300 3350 0    50   ~ 0
IO32
Text HLabel 1250 2550 0    60   Input ~ 0
IO18
Wire Wire Line
	1300 2550 1250 2550
Text Label 1300 2550 0    50   ~ 0
IO18
Text Label 1300 1100 0    50   ~ 0
EN
Text HLabel 1250 1100 0    60   Input ~ 0
ESP_EN
Wire Wire Line
	1300 1100 1250 1100
Text HLabel 1250 1200 0    60   Input ~ 0
PROG
Wire Wire Line
	1300 1200 1250 1200
Text HLabel 1250 3650 0    60   Input ~ 0
IO35
Wire Wire Line
	1300 3650 1250 3650
Text HLabel 1250 3550 0    60   Input ~ 0
IO34
Wire Wire Line
	1300 3550 1250 3550
Wire Wire Line
	1300 2350 1250 2350
Wire Wire Line
	1300 2450 1250 2450
Text HLabel 1250 2350 0    60   Input ~ 0
IO16
Text Label 1300 2350 0    50   ~ 0
IO16
Text HLabel 1250 2450 0    60   Input ~ 0
IO17
Text Label 1300 2450 0    50   ~ 0
IO17
Text Label 1300 3550 0    50   ~ 0
IO34
Text Label 1300 3650 0    50   ~ 0
IO35
Text Label 1300 1200 0    50   ~ 0
PROG
Text HLabel 1250 2150 0    60   Input ~ 0
IO13
Wire Wire Line
	1300 2150 1250 2150
Text Label 1300 2150 0    50   ~ 0
IO13
Text HLabel 1250 2250 0    60   Input ~ 0
IO15
Wire Wire Line
	1300 2250 1250 2250
Text Label 1300 2250 0    50   ~ 0
IO15
Text HLabel 1250 2850 0    60   Input ~ 0
IO25
Wire Wire Line
	1300 2850 1250 2850
Text Label 1300 2850 0    50   ~ 0
IO25
Wire Wire Line
	1300 2650 1250 2650
Wire Wire Line
	1300 2750 1250 2750
Text HLabel 1250 2650 0    60   Input ~ 0
IO19
Text Label 1300 2650 0    50   ~ 0
IO19
Text HLabel 1250 2750 0    60   Input ~ 0
IO23
Text Label 1300 2750 0    50   ~ 0
IO23
Text HLabel 1250 2950 0    60   Input ~ 0
IO26
Wire Wire Line
	1300 2950 1250 2950
Text HLabel 1250 3250 0    60   Input ~ 0
IO33
Wire Wire Line
	1300 3250 1250 3250
Text Label 1300 3250 0    50   ~ 0
IO33
Wire Wire Line
	1300 3050 1250 3050
Wire Wire Line
	1300 3150 1250 3150
Text HLabel 1250 3050 0    60   Input ~ 0
IO27
Text Label 1300 3050 0    50   ~ 0
IO27
Text HLabel 1250 3150 0    60   Input ~ 0
IO32
Text Label 1300 3150 0    50   ~ 0
IO32
Text Label 1300 2950 0    50   ~ 0
IO26
Text Label 2600 4100 2    50   ~ 0
EN
Wire Wire Line
	2600 4800 2600 4750
Wire Wire Line
	2600 4050 2600 4150
Text Label 2600 3250 2    50   ~ 0
3V3
Wire Wire Line
	2600 3250 2600 3300
Wire Wire Line
	2600 3650 2600 3600
Text Label 3200 4100 2    50   ~ 0
PROG
Wire Wire Line
	3200 4800 3200 4750
Wire Wire Line
	3200 4050 3200 4150
Text Label 3200 3250 2    50   ~ 0
3V3
Wire Wire Line
	3200 3250 3200 3300
Text Label 3750 4100 2    50   ~ 0
FACT
Wire Wire Line
	3750 4800 3750 4750
Wire Wire Line
	3750 4050 3750 4150
Text Label 3750 3250 2    50   ~ 0
3V3
Wire Wire Line
	3750 3250 3750 3300
Wire Wire Line
	3750 3650 3750 3600
$Comp
L Open_Automation:SW_PUSH SW?
U 1 1 5CC69855
P 3750 4450
AR Path="/5ABD187E/5CC69855" Ref="SW?"  Part="1" 
AR Path="/5A7BADDB/5CC69855" Ref="SW?"  Part="1" 
AR Path="/6077D777/5CC69855" Ref="FACT1"  Part="1" 
F 0 "FACT1" H 3750 4625 50  0000 C CNN
F 1 "SW_PUSH" H 3725 4350 50  0000 C CNN
F 2 "Button_Switch_SMD:SW_SPST_PTS645" H 3750 4450 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/1811082113_Korean-Hroparts-Elec-K2-1102SP-C4SC-04_C127509.pdf" H 3750 4450 50  0001 C CNN
F 4 "K2-1102SP-C4SC-04" H 3750 4450 60  0001 C CNN "Part Number"
F 5 "C127509" H 0   0   50  0001 C CNN "LCSC"
	1    3750 4450
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:SW_PUSH SW?
U 1 1 5CC68117
P 3200 4450
AR Path="/5ABD187E/5CC68117" Ref="SW?"  Part="1" 
AR Path="/5A7BADDB/5CC68117" Ref="SW?"  Part="1" 
AR Path="/6077D777/5CC68117" Ref="PROG1"  Part="1" 
F 0 "PROG1" H 3200 4625 50  0000 C CNN
F 1 "SW_PUSH" H 3175 4350 50  0000 C CNN
F 2 "Button_Switch_SMD:SW_SPST_PTS645" H 3200 4450 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/1811082113_Korean-Hroparts-Elec-K2-1102SP-C4SC-04_C127509.pdf" H 3200 4450 50  0001 C CNN
F 4 "K2-1102SP-C4SC-04" H 3200 4450 60  0001 C CNN "Part Number"
F 5 "C127509" H 0   0   50  0001 C CNN "LCSC"
	1    3200 4450
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:SW_PUSH SW?
U 1 1 5CC89C1F
P 2600 4450
AR Path="/5ABD187E/5CC89C1F" Ref="SW?"  Part="1" 
AR Path="/5A7BADDB/5CC89C1F" Ref="SW?"  Part="1" 
AR Path="/6077D777/5CC89C1F" Ref="EN1"  Part="1" 
F 0 "EN1" H 2600 4625 50  0000 C CNN
F 1 "SW_PUSH" H 2575 4350 50  0000 C CNN
F 2 "Button_Switch_SMD:SW_SPST_PTS645" H 2600 4450 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/1811082113_Korean-Hroparts-Elec-K2-1102SP-C4SC-04_C127509.pdf" H 2600 4450 50  0001 C CNN
F 4 "K2-1102SP-C4SC-04" H 2600 4450 60  0001 C CNN "Part Number"
F 5 "C127509" H 0   0   50  0001 C CNN "LCSC"
	1    2600 4450
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:R_10K R?
U 1 1 5CC2B523
P 3750 3850
AR Path="/5A7BADDB/5CC2B523" Ref="R?"  Part="1" 
AR Path="/6077D777/5CC2B523" Ref="R75"  Part="1" 
F 0 "R75" V 3543 3850 50  0000 C CNN
F 1 "R_10K" V 3634 3850 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad0.98x0.95mm_HandSolder" V 3680 3850 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0603WAF1002T5E_C25804.pdf" V 3830 3850 50  0001 C CNN
F 4 "0603WAF1002T5E" V 3930 3950 60  0001 C CNN "Part Number"
F 5 "C25804" H 0   0   50  0001 C CNN "LCSC"
	1    3750 3850
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:R_10K R?
U 1 1 5CC2D1AD
P 3200 3850
AR Path="/5A7BADDB/5CC2D1AD" Ref="R?"  Part="1" 
AR Path="/6077D777/5CC2D1AD" Ref="R74"  Part="1" 
F 0 "R74" V 2993 3850 50  0000 C CNN
F 1 "R_10K" V 3084 3850 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad0.98x0.95mm_HandSolder" V 3130 3850 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0603WAF1002T5E_C25804.pdf" V 3280 3850 50  0001 C CNN
F 4 "0603WAF1002T5E" V 3380 3950 60  0001 C CNN "Part Number"
F 5 "C25804" H 0   0   50  0001 C CNN "LCSC"
	1    3200 3850
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:R_10K R?
U 1 1 5CC2EC19
P 2600 3850
AR Path="/5A7BADDB/5CC2EC19" Ref="R?"  Part="1" 
AR Path="/6077D777/5CC2EC19" Ref="R73"  Part="1" 
F 0 "R73" V 2393 3850 50  0000 C CNN
F 1 "R_10K" V 2484 3850 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad0.98x0.95mm_HandSolder" V 2530 3850 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0603WAF1002T5E_C25804.pdf" V 2680 3850 50  0001 C CNN
F 4 "0603WAF1002T5E" V 2780 3950 60  0001 C CNN "Part Number"
F 5 "C25804" H 0   0   50  0001 C CNN "LCSC"
	1    2600 3850
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:LED_G D?
U 1 1 5CC255BE
P 2600 3450
AR Path="/5A7BADDB/5CC255BE" Ref="D?"  Part="1" 
AR Path="/6077D777/5CC255BE" Ref="D10"  Part="1" 
F 0 "D10" V 2639 3333 50  0000 R CNN
F 1 "LED_G" V 2548 3333 50  0000 R CNN
F 2 "Diode_SMD:D_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 2600 3450 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Everlight-Elec-19-217-BHC-ZL1M2RY-3T_C72041.pdf" H 2600 3450 50  0001 C CNN
F 4 "19-217/BHC-ZL1M2RY/3T" H 2600 3450 50  0001 C CNN "Part Number"
F 5 "C72041" H 0   0   50  0001 C CNN "LCSC"
	1    2600 3450
	0    -1   -1   0   
$EndComp
$Comp
L Open_Automation:LED_G D?
U 1 1 5CC26ACF
P 3200 3450
AR Path="/5A7BADDB/5CC26ACF" Ref="D?"  Part="1" 
AR Path="/6077D777/5CC26ACF" Ref="D11"  Part="1" 
F 0 "D11" V 3239 3333 50  0000 R CNN
F 1 "LED_G" V 3148 3333 50  0000 R CNN
F 2 "Diode_SMD:D_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 3200 3450 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Everlight-Elec-19-217-BHC-ZL1M2RY-3T_C72041.pdf" H 3200 3450 50  0001 C CNN
F 4 "19-217/BHC-ZL1M2RY/3T" H 3200 3450 50  0001 C CNN "Part Number"
F 5 "C72041" H 0   0   50  0001 C CNN "LCSC"
	1    3200 3450
	0    -1   -1   0   
$EndComp
$Comp
L Open_Automation:LED_G D?
U 1 1 5CC27619
P 3750 3450
AR Path="/5A7BADDB/5CC27619" Ref="D?"  Part="1" 
AR Path="/6077D777/5CC27619" Ref="D12"  Part="1" 
F 0 "D12" V 3789 3333 50  0000 R CNN
F 1 "LED_G" V 3698 3333 50  0000 R CNN
F 2 "Diode_SMD:D_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 3750 3450 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Everlight-Elec-19-217-BHC-ZL1M2RY-3T_C72041.pdf" H 3750 3450 50  0001 C CNN
F 4 "19-217/BHC-ZL1M2RY/3T" H 3750 3450 50  0001 C CNN "Part Number"
F 5 "C72041" H 0   0   50  0001 C CNN "LCSC"
	1    3750 3450
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3200 3600 3200 3650
$Comp
L power:GND #PWR?
U 1 1 601E8A98
P 2600 4800
AR Path="/5A7BADDB/601E8A98" Ref="#PWR?"  Part="1" 
AR Path="/6077D777/601E8A98" Ref="#PWR086"  Part="1" 
F 0 "#PWR086" H 2600 4550 50  0001 C CNN
F 1 "GND" H 2600 4650 50  0000 C CNN
F 2 "" H 2600 4800 50  0000 C CNN
F 3 "" H 2600 4800 50  0000 C CNN
	1    2600 4800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 601E9002
P 3200 4800
AR Path="/5A7BADDB/601E9002" Ref="#PWR?"  Part="1" 
AR Path="/6077D777/601E9002" Ref="#PWR087"  Part="1" 
F 0 "#PWR087" H 3200 4550 50  0001 C CNN
F 1 "GND" H 3200 4650 50  0000 C CNN
F 2 "" H 3200 4800 50  0000 C CNN
F 3 "" H 3200 4800 50  0000 C CNN
	1    3200 4800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 601E9321
P 3750 4800
AR Path="/5A7BADDB/601E9321" Ref="#PWR?"  Part="1" 
AR Path="/6077D777/601E9321" Ref="#PWR089"  Part="1" 
F 0 "#PWR089" H 3750 4550 50  0001 C CNN
F 1 "GND" H 3750 4650 50  0000 C CNN
F 2 "" H 3750 4800 50  0000 C CNN
F 3 "" H 3750 4800 50  0000 C CNN
	1    3750 4800
	1    0    0    -1  
$EndComp
Text HLabel 1250 1300 0    60   Input ~ 0
FACT
Wire Wire Line
	1300 1300 1250 1300
Text Label 1300 1300 0    50   ~ 0
FACT
$Comp
L Open_Automation:C_1uF C?
U 1 1 5B2CA474
P 3400 2050
AR Path="/5A7BADDB/5B2CA474" Ref="C?"  Part="1" 
AR Path="/6077D777/5B2CA474" Ref="C22"  Part="1" 
F 0 "C22" H 3500 2050 50  0000 L CNN
F 1 "C_1uF" H 3400 1950 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric_Pad1.08x0.95mm_HandSolder" H 3500 2350 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL10A106MA8NRNC_C96446.pdf" H 3400 1800 50  0001 C CNN
F 4 "CL10A106MA8NRNC" H 3525 2250 60  0001 C CNN "Part Number"
F 5 "C96446" H -550 100 50  0001 C CNN "LCSC"
	1    3400 2050
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:C_10uF C?
U 1 1 5B2706A8
P 3100 2050
AR Path="/5A7BADDB/5B2706A8" Ref="C?"  Part="1" 
AR Path="/6077D777/5B2706A8" Ref="C21"  Part="1" 
F 0 "C21" H 2850 2050 50  0000 L CNN
F 1 "C_10uF" H 2800 1950 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric_Pad1.08x0.95mm_HandSolder" H 3200 2350 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL10A106MA8NRNC_C96446.pdf" H 3100 1800 50  0001 C CNN
F 4 "CL10A106MA8NRNC" H 3225 2250 60  0001 C CNN "Part Number"
F 5 "C96446" H -550 100 50  0001 C CNN "LCSC"
	1    3100 2050
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5CC8C67A
P 3400 2250
AR Path="/5A7BADDB/5CC8C67A" Ref="#PWR?"  Part="1" 
AR Path="/6077D777/5CC8C67A" Ref="#PWR090"  Part="1" 
F 0 "#PWR090" H 3400 2000 50  0001 C CNN
F 1 "GND" H 3400 2100 50  0000 C CNN
F 2 "" H 3400 2250 50  0000 C CNN
F 3 "" H 3400 2250 50  0000 C CNN
	1    3400 2250
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5CC8CF13
P 3100 2250
AR Path="/5A7BADDB/5CC8CF13" Ref="#PWR?"  Part="1" 
AR Path="/6077D777/5CC8CF13" Ref="#PWR088"  Part="1" 
F 0 "#PWR088" H 3100 2000 50  0001 C CNN
F 1 "GND" H 3100 2100 50  0000 C CNN
F 2 "" H 3100 2250 50  0000 C CNN
F 3 "" H 3100 2250 50  0000 C CNN
	1    3100 2250
	1    0    0    -1  
$EndComp
Wire Wire Line
	3400 2200 3400 2250
Wire Wire Line
	3400 1850 3400 1900
Wire Wire Line
	3100 1900 3100 1850
Wire Wire Line
	3100 2250 3100 2200
Text Label 3100 1850 2    50   ~ 0
3V3
Text Label 3400 1850 0    50   ~ 0
3V3
$Comp
L power:GND #PWR?
U 1 1 601E9D28
P 8350 4800
AR Path="/5A7BADDB/601E9D28" Ref="#PWR?"  Part="1" 
AR Path="/6077D777/601E9D28" Ref="#PWR094"  Part="1" 
F 0 "#PWR094" H 8350 4550 50  0001 C CNN
F 1 "GND" H 8350 4650 50  0000 C CNN
F 2 "" H 8350 4800 50  0000 C CNN
F 3 "" H 8350 4800 50  0000 C CNN
	1    8350 4800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 601E974F
P 7450 4700
AR Path="/5A7BADDB/601E974F" Ref="#PWR?"  Part="1" 
AR Path="/6077D777/601E974F" Ref="#PWR093"  Part="1" 
F 0 "#PWR093" H 7450 4450 50  0001 C CNN
F 1 "GND" H 7450 4550 50  0000 C CNN
F 2 "" H 7450 4700 50  0000 C CNN
F 3 "" H 7450 4700 50  0000 C CNN
	1    7450 4700
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:C_0.1uF C?
U 1 1 5CC805AD
P 7450 4500
AR Path="/5A8CA1FA/5CC805AD" Ref="C?"  Part="1" 
AR Path="/5A7BADDB/5CC805AD" Ref="C?"  Part="1" 
AR Path="/6077D777/5CC805AD" Ref="C23"  Part="1" 
F 0 "C23" H 7200 4500 50  0000 L CNN
F 1 "C_0.1uF" H 7150 4400 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric_Pad1.08x0.95mm_HandSolder" H 7550 4800 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/YAGEO-CC0603KRX7R9BB104_C14663.pdf" H 7450 4250 50  0001 C CNN
F 4 "CC0603KRX7R9BB104" H 7575 4700 60  0001 C CNN "Part Number"
F 5 "C14663" H -750 500 50  0001 C CNN "LCSC"
	1    7450 4500
	1    0    0    -1  
$EndComp
Wire Wire Line
	7450 4650 7450 4700
Wire Wire Line
	7700 2650 7700 2700
Text Label 7700 2650 0    50   ~ 0
IO2
$Comp
L power:GND #PWR?
U 1 1 601E053E
P 7700 3150
AR Path="/5A7BADDB/601E053E" Ref="#PWR?"  Part="1" 
AR Path="/6077D777/601E053E" Ref="#PWR0109"  Part="1" 
F 0 "#PWR0109" H 7700 2900 50  0001 C CNN
F 1 "GND" H 7700 3000 50  0000 C CNN
F 2 "" H 7700 3150 50  0000 C CNN
F 3 "" H 7700 3150 50  0000 C CNN
	1    7700 3150
	1    0    0    -1  
$EndComp
Wire Wire Line
	7700 3150 7700 3100
$Comp
L Open_Automation:R_10K R?
U 1 1 5CC28727
P 8200 2900
AR Path="/5A7BADDB/5CC28727" Ref="R?"  Part="1" 
AR Path="/6077D777/5CC28727" Ref="R76"  Part="1" 
F 0 "R76" V 8200 2900 50  0000 C CNN
F 1 "R_10K" V 8084 2900 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad0.98x0.95mm_HandSolder" V 8130 2900 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0603WAF1002T5E_C25804.pdf" V 8280 2900 50  0001 C CNN
F 4 "0603WAF1002T5E" V 8380 3000 60  0001 C CNN "Part Number"
F 5 "C25804" H 3350 950 50  0001 C CNN "LCSC"
	1    8200 2900
	-1   0    0    1   
$EndComp
$Comp
L Open_Automation:R_10K R?
U 1 1 5CC267DD
P 8050 2900
AR Path="/5A7BADDB/5CC267DD" Ref="R?"  Part="1" 
AR Path="/6077D777/5CC267DD" Ref="R77"  Part="1" 
F 0 "R77" V 8050 2900 50  0000 C CNN
F 1 "R_10K" V 8150 2900 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad0.98x0.95mm_HandSolder" V 7980 2900 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0603WAF1002T5E_C25804.pdf" V 8130 2900 50  0001 C CNN
F 4 "0603WAF1002T5E" V 8230 3000 60  0001 C CNN "Part Number"
F 5 "C25804" H 3200 800 50  0001 C CNN "LCSC"
	1    8050 2900
	-1   0    0    1   
$EndComp
Text Label 8050 2650 1    50   ~ 0
3V3
Wire Wire Line
	8050 2650 8050 2700
Wire Wire Line
	8200 2650 8200 2700
Wire Wire Line
	8200 3150 8200 3100
Text Label 8200 2650 1    50   ~ 0
3V3
Text Label 8200 3150 3    50   ~ 0
SCL
Wire Wire Line
	8050 3150 8050 3100
Text Label 8050 3150 3    50   ~ 0
SDA
Wire Wire Line
	6350 2350 6300 2350
Text Label 6350 2350 0    50   ~ 0
PROG
Wire Wire Line
	5750 2150 5700 2150
Text Label 5750 2150 0    50   ~ 0
3V3
Wire Wire Line
	5050 2350 5100 2350
Text Label 5050 2350 2    50   ~ 0
EN
Wire Wire Line
	6300 3250 6350 3250
Text Label 6350 3250 0    50   ~ 0
IO15
Wire Wire Line
	6350 3350 6300 3350
Wire Wire Line
	6300 3450 6350 3450
Text Label 6350 3450 0    50   ~ 0
IO17
Text Label 6350 3350 0    50   ~ 0
IO16
Wire Wire Line
	6300 3650 6350 3650
Wire Wire Line
	6350 3550 6300 3550
Text Label 6350 3650 0    50   ~ 0
IO19
Text Label 6350 3550 0    50   ~ 0
IO18
Wire Wire Line
	6350 3750 6300 3750
Text Label 6350 3750 0    50   ~ 0
SDA
Text Label 6350 3850 0    50   ~ 0
SCL
Text Label 6350 3950 0    50   ~ 0
IO23
Wire Wire Line
	6300 4350 6350 4350
Wire Wire Line
	6350 4450 6300 4450
Wire Wire Line
	6300 4550 6350 4550
Wire Wire Line
	6350 4650 6300 4650
Wire Wire Line
	6350 3850 6300 3850
Wire Wire Line
	6350 3950 6300 3950
Wire Wire Line
	6350 4050 6300 4050
Wire Wire Line
	6350 4150 6300 4150
Wire Wire Line
	6350 4250 6300 4250
Wire Wire Line
	6350 3150 6300 3150
Text Label 6350 4050 0    50   ~ 0
IO25
Text Label 6350 4150 0    50   ~ 0
IO26
Text Label 6350 4250 0    50   ~ 0
IO27
Text Label 6350 4550 0    50   ~ 0
IO34
Text Label 6350 4650 0    50   ~ 0
IO35
Text Label 6350 4350 0    50   ~ 0
IO32
Text Label 6350 4450 0    50   ~ 0
IO33
Text Label 6350 3150 0    50   ~ 0
FACT
Wire Wire Line
	6350 3050 6300 3050
Text Label 6350 3050 0    50   ~ 0
IO13
Wire Wire Line
	6350 2850 6300 2850
Text Label 6350 2850 0    50   ~ 0
IO5
Wire Wire Line
	6350 2750 6300 2750
Text Label 6350 2750 0    50   ~ 0
IO4
Wire Wire Line
	6350 2650 6300 2650
Wire Wire Line
	6300 2450 6350 2450
Wire Wire Line
	6350 2550 6300 2550
Text Label 6350 2550 0    50   ~ 0
IO2
Text Label 6350 2650 0    50   ~ 0
RXD
Text Label 6350 2450 0    50   ~ 0
TXD
Wire Wire Line
	5700 4950 5700 5000
$Comp
L power:GND #PWR?
U 1 1 601D8E89
P 5700 5000
AR Path="/5A7BADDB/601D8E89" Ref="#PWR?"  Part="1" 
AR Path="/6077D777/601D8E89" Ref="#PWR0110"  Part="1" 
F 0 "#PWR0110" H 5700 4750 50  0001 C CNN
F 1 "GND" H 5700 4850 50  0000 C CNN
F 2 "" H 5700 5000 50  0000 C CNN
F 3 "" H 5700 5000 50  0000 C CNN
	1    5700 5000
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:ESP32-WROOM-32 U13
U 1 1 601C2110
P 5700 3550
F 0 "U13" H 5250 4900 50  0000 C CNN
F 1 "ESP32-WROOM-32" H 5600 3950 50  0000 C CNN
F 2 "RF_Module:ESP32-WROOM-32" H 5700 2050 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/2007061615_Espressif-Systems-ESP32-WROOM-32_C82899.pdf" H 5400 3600 50  0001 C CNN
F 4 "C82899" H 0   0   50  0001 C CNN "LCSC"
	1    5700 3550
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:R_10K R?
U 1 1 5CC3139A
P 7700 2900
AR Path="/5A7BADDB/5CC3139A" Ref="R?"  Part="1" 
AR Path="/6077D777/5CC3139A" Ref="R78"  Part="1" 
F 0 "R78" V 7700 2900 50  0000 C CNN
F 1 "R_10K" V 7584 2900 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad0.98x0.95mm_HandSolder" V 7630 2900 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0603WAF1002T5E_C25804.pdf" V 7780 2900 50  0001 C CNN
F 4 "0603WAF1002T5E" V 7880 3000 60  0001 C CNN "Part Number"
F 5 "C25804" H 1050 -2200 50  0001 C CNN "LCSC"
	1    7700 2900
	1    0    0    -1  
$EndComp
Text Label 7450 4300 2    50   ~ 0
3V3
Wire Wire Line
	7450 4300 7450 4350
Text Label 8350 4100 2    50   ~ 0
3V3
Wire Wire Line
	8350 4100 8350 4150
Wire Wire Line
	8250 4750 8350 4750
Wire Wire Line
	8350 4800 8350 4750
Connection ~ 8350 4750
Wire Wire Line
	8350 4750 8450 4750
Text Label 7800 4350 2    50   ~ 0
SDA
Wire Wire Line
	7800 4350 7850 4350
Text Label 7800 4550 2    50   ~ 0
SCL
Wire Wire Line
	7800 4550 7850 4550
$Comp
L Sensor_Humidity:Si7020-A20 U14
U 1 1 602D690F
P 8350 4450
F 0 "U14" H 8650 4700 50  0000 L CNN
F 1 "Si7020-A20" H 8500 4200 50  0000 L CNN
F 2 "Package_DFN_QFN:DFN-6-1EP_3x3mm_P1mm_EP1.5x2.4mm" H 8350 4050 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/2005151605_SILICON-LABS-SI7020-A20-GM1_C430229.pdf" H 8150 4750 50  0001 C CNN
F 4 "SI7020-A20-GM1" H 0   0   50  0001 C CNN "Part Number"
F 5 "C430229" H 0   0   50  0001 C CNN "LCSC"
	1    8350 4450
	1    0    0    -1  
$EndComp
$EndSCHEMATC
