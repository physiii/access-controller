EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr USLetter 11000 8500
encoding utf-8
Sheet 2 5
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
Text HLabel 1250 1900 0    60   Input ~ 0
IO5
Text HLabel 1250 950  0    60   Input ~ 0
3V3
Text Label 1300 950  0    50   ~ 0
3V3
Wire Wire Line
	1300 950  1250 950 
Text HLabel 1250 1800 0    60   Input ~ 0
IO4
Text Label 1300 1800 0    50   ~ 0
IO4
Wire Wire Line
	1300 1800 1250 1800
Text Label 1300 1900 0    50   ~ 0
IO5
Wire Wire Line
	1300 1900 1250 1900
Text Label 1300 1550 0    50   ~ 0
RXD
Wire Wire Line
	1300 1550 1250 1550
Text Label 1300 1450 0    50   ~ 0
TXD
Wire Wire Line
	1300 1450 1250 1450
Text HLabel 1250 3700 0    60   Input ~ 0
IO33
Text HLabel 1250 3600 0    60   Input ~ 0
IO32
Wire Wire Line
	1300 3700 1250 3700
Wire Wire Line
	1300 3600 1250 3600
Text Label 1300 3700 0    50   ~ 0
IO33
Text Label 1300 3600 0    50   ~ 0
IO32
Text HLabel 1250 2600 0    60   Input ~ 0
IO18
Wire Wire Line
	1300 2600 1250 2600
Text Label 1300 2600 0    50   ~ 0
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
Text HLabel 1250 3900 0    60   Input ~ 0
IO35
Wire Wire Line
	1300 3900 1250 3900
Text HLabel 1250 3800 0    60   Input ~ 0
IO34
Wire Wire Line
	1300 3800 1250 3800
Wire Wire Line
	1300 2400 1250 2400
Wire Wire Line
	1300 2500 1250 2500
Text HLabel 1250 2400 0    60   Input ~ 0
IO16
Text Label 1300 2400 0    50   ~ 0
IO16
Text HLabel 1250 2500 0    60   Input ~ 0
IO17
Text Label 1300 2500 0    50   ~ 0
IO17
Text Label 1300 3800 0    50   ~ 0
IO34
Text Label 1300 3900 0    50   ~ 0
IO35
Text Label 1300 1200 0    50   ~ 0
PROG
Text HLabel 1250 2100 0    60   Input ~ 0
IO13
Wire Wire Line
	1300 2100 1250 2100
Text Label 1300 2100 0    50   ~ 0
IO13
Text HLabel 1250 2300 0    60   Input ~ 0
IO15
Wire Wire Line
	1300 2300 1250 2300
Text Label 1300 2300 0    50   ~ 0
IO15
Text HLabel 1250 3100 0    60   Input ~ 0
IO25
Wire Wire Line
	1300 3100 1250 3100
Text Label 1300 3100 0    50   ~ 0
IO25
Wire Wire Line
	1300 2700 1250 2700
Wire Wire Line
	1300 3000 1250 3000
Text HLabel 1250 2700 0    60   Input ~ 0
IO19
Text Label 1300 2700 0    50   ~ 0
IO19
Text HLabel 1250 3000 0    60   Input ~ 0
IO23
Text Label 1300 3000 0    50   ~ 0
IO23
Text HLabel 1250 3200 0    60   Input ~ 0
IO26
Wire Wire Line
	1300 3200 1250 3200
Text HLabel 1250 3500 0    60   Input ~ 0
IO33
Wire Wire Line
	1300 3500 1250 3500
Text Label 1300 3500 0    50   ~ 0
IO33
Wire Wire Line
	1300 3300 1250 3300
Wire Wire Line
	1300 3400 1250 3400
Text HLabel 1250 3300 0    60   Input ~ 0
IO27
Text Label 1300 3300 0    50   ~ 0
IO27
Text HLabel 1250 3400 0    60   Input ~ 0
IO32
Text Label 1300 3400 0    50   ~ 0
IO32
Text Label 1300 3200 0    50   ~ 0
IO26
$Comp
L Open_Automation:C_10uF C?
U 1 1 5B2706A8
P 4100 2350
AR Path="/5A7BADDB/5B2706A8" Ref="C?"  Part="1" 
AR Path="/6077D777/5B2706A8" Ref="C?"  Part="1" 
AR Path="/61E4D8AE/5B2706A8" Ref="C1"  Part="1" 
F 0 "C1" H 3850 2350 50  0000 L CNN
F 1 "C_10uF" H 3800 2250 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric_Pad1.08x0.95mm_HandSolder" H 4200 2650 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL10A106MA8NRNC_C96446.pdf" H 4100 2100 50  0001 C CNN
F 4 "CL10A106MA8NRNC" H 4225 2550 60  0001 C CNN "Part Number"
F 5 "C96446" H 450 400 50  0001 C CNN "LCSC"
	1    4100 2350
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5CC8C67A
P 4400 2550
AR Path="/5A7BADDB/5CC8C67A" Ref="#PWR?"  Part="1" 
AR Path="/6077D777/5CC8C67A" Ref="#PWR?"  Part="1" 
AR Path="/61E4D8AE/5CC8C67A" Ref="#PWR04"  Part="1" 
F 0 "#PWR04" H 4400 2300 50  0001 C CNN
F 1 "GND" H 4400 2400 50  0000 C CNN
F 2 "" H 4400 2550 50  0000 C CNN
F 3 "" H 4400 2550 50  0000 C CNN
	1    4400 2550
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5CC8CF13
P 4100 2550
AR Path="/5A7BADDB/5CC8CF13" Ref="#PWR?"  Part="1" 
AR Path="/6077D777/5CC8CF13" Ref="#PWR?"  Part="1" 
AR Path="/61E4D8AE/5CC8CF13" Ref="#PWR02"  Part="1" 
F 0 "#PWR02" H 4100 2300 50  0001 C CNN
F 1 "GND" H 4100 2400 50  0000 C CNN
F 2 "" H 4100 2550 50  0000 C CNN
F 3 "" H 4100 2550 50  0000 C CNN
	1    4100 2550
	1    0    0    -1  
$EndComp
Wire Wire Line
	4400 2500 4400 2550
Wire Wire Line
	4400 2150 4400 2200
Wire Wire Line
	4100 2200 4100 2150
Wire Wire Line
	4100 2550 4100 2500
Text Label 4100 2150 2    50   ~ 0
3V3
Text Label 4400 2150 0    50   ~ 0
3V3
Wire Wire Line
	7350 2650 7300 2650
Text Label 7350 2650 0    50   ~ 0
PROG
Wire Wire Line
	6750 2450 6700 2450
Text Label 6750 2450 0    50   ~ 0
3V3
Wire Wire Line
	6050 2650 6100 2650
Text Label 6050 2650 2    50   ~ 0
EN
Wire Wire Line
	7300 3550 7350 3550
Text Label 7350 3550 0    50   ~ 0
IO15
Wire Wire Line
	7350 3650 7300 3650
Wire Wire Line
	7300 3750 7350 3750
Text Label 7350 3750 0    50   ~ 0
IO17
Text Label 7350 3650 0    50   ~ 0
IO16
Wire Wire Line
	7300 3950 7350 3950
Wire Wire Line
	7350 3850 7300 3850
Text Label 7350 3950 0    50   ~ 0
IO19
Text Label 7350 3850 0    50   ~ 0
IO18
Wire Wire Line
	7350 4050 7300 4050
Text Label 7350 4250 0    50   ~ 0
IO23
Wire Wire Line
	7300 4650 7350 4650
Wire Wire Line
	7350 4750 7300 4750
Wire Wire Line
	7300 4850 7350 4850
Wire Wire Line
	7350 4950 7300 4950
Wire Wire Line
	7350 4150 7300 4150
Wire Wire Line
	7350 4250 7300 4250
Wire Wire Line
	7350 4350 7300 4350
Wire Wire Line
	7350 4450 7300 4450
Wire Wire Line
	7350 4550 7300 4550
Wire Wire Line
	7350 3450 7300 3450
Text Label 7350 4350 0    50   ~ 0
IO25
Text Label 7350 4450 0    50   ~ 0
IO26
Text Label 7350 4550 0    50   ~ 0
IO27
Text Label 7350 4850 0    50   ~ 0
IO34
Text Label 7350 4950 0    50   ~ 0
IO35
Text Label 7350 4650 0    50   ~ 0
IO32
Text Label 7350 4750 0    50   ~ 0
IO33
Text Label 7350 3450 0    50   ~ 0
IO14
Wire Wire Line
	7350 3350 7300 3350
Text Label 7350 3350 0    50   ~ 0
IO13
Wire Wire Line
	7350 3150 7300 3150
Text Label 7350 3150 0    50   ~ 0
IO5
Wire Wire Line
	7350 3050 7300 3050
Text Label 7350 3050 0    50   ~ 0
IO4
Wire Wire Line
	7350 2950 7300 2950
Wire Wire Line
	7300 2750 7350 2750
Wire Wire Line
	7350 2850 7300 2850
Text Label 7350 2850 0    50   ~ 0
IO2
Text Label 7350 2950 0    50   ~ 0
RXD
Text Label 7350 2750 0    50   ~ 0
TXD
Wire Wire Line
	6700 5250 6700 5300
$Comp
L power:GND #PWR?
U 1 1 601D8E89
P 6700 5300
AR Path="/5A7BADDB/601D8E89" Ref="#PWR?"  Part="1" 
AR Path="/6077D777/601D8E89" Ref="#PWR?"  Part="1" 
AR Path="/61E4D8AE/601D8E89" Ref="#PWR07"  Part="1" 
F 0 "#PWR07" H 6700 5050 50  0001 C CNN
F 1 "GND" H 6700 5150 50  0000 C CNN
F 2 "" H 6700 5300 50  0000 C CNN
F 3 "" H 6700 5300 50  0000 C CNN
	1    6700 5300
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:ESP32-WROOM-32 U1
U 1 1 601C2110
P 6700 3850
F 0 "U1" H 6250 5200 50  0000 C CNN
F 1 "ESP32-WROOM-32" H 6600 4250 50  0000 C CNN
F 2 "RF_Module:ESP32-WROOM-32" H 6700 2350 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/2007061615_Espressif-Systems-ESP32-WROOM-32_C82899.pdf" H 6400 3900 50  0001 C CNN
F 4 "C82899" H 1000 300 50  0001 C CNN "LCSC"
	1    6700 3850
	1    0    0    -1  
$EndComp
Text HLabel 1250 1700 0    60   Input ~ 0
IO2
Text Label 1300 1700 0    50   ~ 0
IO2
Wire Wire Line
	1300 1700 1250 1700
Text Label 7350 3250 0    50   ~ 0
IO12
Wire Wire Line
	7350 3250 7300 3250
Text Label 7350 4050 0    50   ~ 0
IO21
Text Label 7350 4150 0    50   ~ 0
IO22
Text HLabel 1250 2900 0    60   Input ~ 0
IO22
Wire Wire Line
	1300 2900 1250 2900
Text Label 1300 2900 0    50   ~ 0
IO22
Wire Wire Line
	1300 2800 1250 2800
Text HLabel 1250 2800 0    60   Input ~ 0
IO21
Text Label 1300 2800 0    50   ~ 0
IO21
Text HLabel 1250 2200 0    60   Input ~ 0
IO14
Wire Wire Line
	1300 2200 1250 2200
Text Label 1300 2200 0    50   ~ 0
IO14
Text Label 6050 2850 2    50   ~ 0
FACT
Wire Wire Line
	6050 2850 6100 2850
$Comp
L power:GND #PWR?
U 1 1 6195C511
P 4850 2550
AR Path="/5A7BADDB/6195C511" Ref="#PWR?"  Part="1" 
AR Path="/6077D777/6195C511" Ref="#PWR?"  Part="1" 
AR Path="/61E4D8AE/6195C511" Ref="#PWR06"  Part="1" 
F 0 "#PWR06" H 4850 2300 50  0001 C CNN
F 1 "GND" H 4850 2400 50  0000 C CNN
F 2 "" H 4850 2550 50  0000 C CNN
F 3 "" H 4850 2550 50  0000 C CNN
	1    4850 2550
	1    0    0    -1  
$EndComp
Wire Wire Line
	4850 2500 4850 2550
Wire Wire Line
	4850 2150 4850 2200
Text Label 4850 2150 0    50   ~ 0
PROG
Text Label 4150 4250 2    50   ~ 0
EN
Wire Wire Line
	4150 4950 4150 4900
Wire Wire Line
	4150 4200 4150 4300
Text Label 4150 3400 2    50   ~ 0
3V3
Wire Wire Line
	4150 3400 4150 3450
Wire Wire Line
	4150 3800 4150 3750
Text Label 4750 4250 2    50   ~ 0
PROG
Wire Wire Line
	4750 4950 4750 4900
Wire Wire Line
	4750 4200 4750 4300
Text Label 4750 3400 2    50   ~ 0
3V3
Wire Wire Line
	4750 3400 4750 3450
$Comp
L Open_Automation:LED_G D?
U 1 1 5CC255BE
P 4150 3600
AR Path="/5A7BADDB/5CC255BE" Ref="D?"  Part="1" 
AR Path="/6077D777/5CC255BE" Ref="D?"  Part="1" 
AR Path="/61E4D8AE/5CC255BE" Ref="D1"  Part="1" 
F 0 "D1" V 4189 3483 50  0000 R CNN
F 1 "LED_G" V 4098 3483 50  0000 R CNN
F 2 "Diode_SMD:D_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 4150 3600 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Everlight-Elec-19-217-BHC-ZL1M2RY-3T_C72041.pdf" H 4150 3600 50  0001 C CNN
F 4 "19-217/BHC-ZL1M2RY/3T" H 4150 3600 50  0001 C CNN "Part Number"
F 5 "C72041" H 1550 150 50  0001 C CNN "LCSC"
	1    4150 3600
	0    -1   -1   0   
$EndComp
$Comp
L Open_Automation:LED_G D?
U 1 1 5CC26ACF
P 4750 3600
AR Path="/5A7BADDB/5CC26ACF" Ref="D?"  Part="1" 
AR Path="/6077D777/5CC26ACF" Ref="D?"  Part="1" 
AR Path="/61E4D8AE/5CC26ACF" Ref="D2"  Part="1" 
F 0 "D2" V 4789 3483 50  0000 R CNN
F 1 "LED_G" V 4698 3483 50  0000 R CNN
F 2 "Diode_SMD:D_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 4750 3600 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Everlight-Elec-19-217-BHC-ZL1M2RY-3T_C72041.pdf" H 4750 3600 50  0001 C CNN
F 4 "19-217/BHC-ZL1M2RY/3T" H 4750 3600 50  0001 C CNN "Part Number"
F 5 "C72041" H 1550 150 50  0001 C CNN "LCSC"
	1    4750 3600
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4750 3750 4750 3800
$Comp
L power:GND #PWR?
U 1 1 601E8A98
P 4150 4950
AR Path="/5A7BADDB/601E8A98" Ref="#PWR?"  Part="1" 
AR Path="/6077D777/601E8A98" Ref="#PWR?"  Part="1" 
AR Path="/61E4D8AE/601E8A98" Ref="#PWR01"  Part="1" 
F 0 "#PWR01" H 4150 4700 50  0001 C CNN
F 1 "GND" H 4150 4800 50  0000 C CNN
F 2 "" H 4150 4950 50  0000 C CNN
F 3 "" H 4150 4950 50  0000 C CNN
	1    4150 4950
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 601E9002
P 4750 4950
AR Path="/5A7BADDB/601E9002" Ref="#PWR?"  Part="1" 
AR Path="/6077D777/601E9002" Ref="#PWR?"  Part="1" 
AR Path="/61E4D8AE/601E9002" Ref="#PWR03"  Part="1" 
F 0 "#PWR03" H 4750 4700 50  0001 C CNN
F 1 "GND" H 4750 4800 50  0000 C CNN
F 2 "" H 4750 4950 50  0000 C CNN
F 3 "" H 4750 4950 50  0000 C CNN
	1    4750 4950
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:SW_PUSH_SM EN1
U 1 1 61F22C20
P 4150 4600
F 0 "EN1" V 4150 4718 50  0000 L CNN
F 1 "SW_PUSH_SM" H 4150 4520 50  0001 C CNN
F 2 "Button_Switch_SMD:SW_SPST_TL3342" H 4100 4800 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/lcsc/2002271431_XKB-Connectivity-TS-1187A-B-A-B_C318884.pdf" H 4150 4600 50  0001 C CNN
F 4 "TS-1187A-B-A-B" H 4100 4900 50  0001 C CNN "Part Number"
F 5 "C318884" H 4150 5000 50  0001 C CNN "LCSC"
	1    4150 4600
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:SW_PUSH_SM PROG1
U 1 1 61F24274
P 4750 4600
F 0 "PROG1" V 4750 4718 50  0000 L CNN
F 1 "SW_PUSH_SM" H 4750 4520 50  0001 C CNN
F 2 "Button_Switch_SMD:SW_SPST_TL3342" H 4700 4800 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/lcsc/2002271431_XKB-Connectivity-TS-1187A-B-A-B_C318884.pdf" H 4750 4600 50  0001 C CNN
F 4 "TS-1187A-B-A-B" H 4700 4900 50  0001 C CNN "Part Number"
F 5 "C318884" H 4750 5000 50  0001 C CNN "LCSC"
	1    4750 4600
	0    1    1    0   
$EndComp
$Comp
L Open_Automation:R_10K_0402 R1
U 1 1 61F38F2E
P 4150 4000
F 0 "R1" V 4150 3950 50  0000 L CNN
F 1 "R_10K_0402" V 4050 3800 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 4080 4000 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0402WGF1002TCE_C25744.pdf" V 4230 4000 50  0001 C CNN
F 4 "0402WGF1002TCE" V 4330 4100 60  0001 C CNN "Part Number"
F 5 "C25744" V 4400 4000 50  0001 C CNN "LCSC"
	1    4150 4000
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:R_10K_0402 R2
U 1 1 61F3DA6C
P 4750 4000
F 0 "R2" V 4750 3950 50  0000 L CNN
F 1 "R_10K_0402" V 4650 3800 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric_Pad0.72x0.64mm_HandSolder" V 4680 4000 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Uniroyal-Elec-0402WGF1002TCE_C25744.pdf" V 4830 4000 50  0001 C CNN
F 4 "0402WGF1002TCE" V 4930 4100 60  0001 C CNN "Part Number"
F 5 "C25744" V 5000 4000 50  0001 C CNN "LCSC"
	1    4750 4000
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:C_1uF C3
U 1 1 61F52FBE
P 4850 2350
F 0 "C3" H 4965 2396 50  0000 L CNN
F 1 "C_1uF" H 4965 2305 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric_Pad0.74x0.62mm_HandSolder" H 4950 2650 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL05A105KA5NQNC_C52923.pdf" H 4850 2100 50  0001 C CNN
F 4 "C52923" H 4850 2750 50  0001 C CNN "LCSC"
F 5 "CL05A105KA5NQNC" H 4975 2550 60  0001 C CNN "Part Number"
	1    4850 2350
	1    0    0    -1  
$EndComp
$Comp
L Open_Automation:C_1uF C2
U 1 1 61F55F9B
P 4400 2350
F 0 "C2" H 4515 2396 50  0000 L CNN
F 1 "C_1uF" H 4515 2305 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric_Pad0.74x0.62mm_HandSolder" H 4500 2650 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/Samsung-Electro-Mechanics-CL05A105KA5NQNC_C52923.pdf" H 4400 2100 50  0001 C CNN
F 4 "C52923" H 4400 2750 50  0001 C CNN "LCSC"
F 5 "CL05A105KA5NQNC" H 4525 2550 60  0001 C CNN "Part Number"
	1    4400 2350
	1    0    0    -1  
$EndComp
$EndSCHEMATC
