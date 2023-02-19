EESchema Schematic File Version 4
LIBS:open-dash-daughterboard-cache
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
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
L open-automation:Adapter-TE-52 U1
U 1 1 5CD21B6F
P 4550 2750
F 0 "U1" H 4500 3115 50  0000 C CNN
F 1 "Adapter-TE-52" H 4500 3024 50  0000 C CNN
F 2 "open-automation:Adapter-TE-52" H 4500 3300 50  0001 C CNN
F 3 "https://www.mouser.com/datasheet/2/418/NG_CD_2098978_D1-790821.pdf" H 6100 2800 50  0001 C CNN
F 4 "https://www.mouser.com/ProductDetail/TE-Connectivity-AMP/2098978-1?qs=sGAEpiMZZMs7eK6h2EBtKpfjTZYxfnESj8gtjNM%252Bntw%3D" H 5400 3200 50  0001 C CNN "Website"
F 5 " 2098978-1" H 4550 3100 50  0001 C CNN "Part Number"
	1    4550 2750
	1    0    0    -1  
$EndComp
$Comp
L open-automation:TE-AUTO-CONN-52 U2
U 1 1 5CCC1469
P 7000 2800
F 0 "U2" H 7400 1900 50  0000 C CNN
F 1 "TE-AUTO-CONN-52" H 6950 3050 50  0000 C CNN
F 2 "open-automation:TE-AUTO-CONN-52" H 7850 3300 50  0001 C CNN
F 3 "https://www.mouser.com/ProductDetail/TE-Connectivity-AMP/2098978-1?qs=sGAEpiMZZMs7eK6h2EBtKpfjTZYxfnESj8gtjNM%252Bntw%3D" H 8550 2850 50  0001 C CNN
F 4 " 2098978-1" H 8050 2250 50  0001 C CNN "Part Number"
	1    7000 2800
	1    0    0    -1  
$EndComp
Text Label 7650 2900 0    50   ~ 0
LF+
Wire Wire Line
	7650 2900 7600 2900
Text Label 7650 3000 0    50   ~ 0
LR+
Text Label 7650 2700 0    50   ~ 0
RR+
Wire Wire Line
	7650 2700 7600 2700
Text Label 7650 2800 0    50   ~ 0
RF+
Text Label 7650 3100 0    50   ~ 0
RR-
Wire Wire Line
	7650 3100 7600 3100
Text Label 7650 3200 0    50   ~ 0
RF-
Text Label 7650 3300 0    50   ~ 0
LF-
Wire Wire Line
	7650 3300 7600 3300
Text Label 7650 3400 0    50   ~ 0
LR-
Wire Wire Line
	7650 2800 7600 2800
Wire Wire Line
	7600 3000 7650 3000
Wire Wire Line
	7650 3200 7600 3200
Wire Wire Line
	7600 3400 7650 3400
$Comp
L open-automation:GND #PWR?
U 1 1 5CCCF2F4
P 6250 3450
AR Path="/5ABD187E/5CCCF2F4" Ref="#PWR?"  Part="1" 
AR Path="/5CCCF2F4" Ref="#PWR0101"  Part="1" 
F 0 "#PWR0101" H 6250 3200 50  0001 C CNN
F 1 "GND" H 6250 3300 50  0000 C CNN
F 2 "" H 6250 3450 50  0000 C CNN
F 3 "" H 6250 3450 50  0000 C CNN
	1    6250 3450
	1    0    0    -1  
$EndComp
Wire Wire Line
	6250 3450 6250 3400
Wire Wire Line
	6250 3400 6300 3400
Text Label 6250 3300 2    50   ~ 0
BATTERY
Wire Wire Line
	6250 3300 6300 3300
Text Label 6250 3050 2    50   ~ 0
CAN+
Text Label 6250 3150 2    50   ~ 0
CAN-
Wire Wire Line
	6250 3150 6300 3150
Wire Wire Line
	6300 3050 6250 3050
Text Label 6250 2700 2    50   ~ 0
CAMERA+
Wire Wire Line
	6250 2700 6300 2700
Text Label 6250 2800 2    50   ~ 0
CAMERA-
Wire Wire Line
	6250 2800 6300 2800
$Comp
L open-automation:GND #PWR?
U 1 1 5CCD3124
P 6250 2900
AR Path="/5ABD187E/5CCD3124" Ref="#PWR?"  Part="1" 
AR Path="/5CCD3124" Ref="#PWR0102"  Part="1" 
F 0 "#PWR0102" H 6250 2650 50  0001 C CNN
F 1 "GND" V 6200 2750 50  0000 C CNN
F 2 "" H 6250 2900 50  0000 C CNN
F 3 "" H 6250 2900 50  0000 C CNN
	1    6250 2900
	0    1    1    0   
$EndComp
Wire Wire Line
	6250 2900 6300 2900
Text Label 5200 2850 0    50   ~ 0
LF+
Wire Wire Line
	5200 2850 5150 2850
Text Label 5200 2950 0    50   ~ 0
LR+
Text Label 5200 2650 0    50   ~ 0
RR+
Wire Wire Line
	5200 2650 5150 2650
Text Label 5200 2750 0    50   ~ 0
RF+
Text Label 5200 3050 0    50   ~ 0
RR-
Wire Wire Line
	5200 3050 5150 3050
Text Label 5200 3150 0    50   ~ 0
RF-
Text Label 5200 3250 0    50   ~ 0
LF-
Wire Wire Line
	5200 3250 5150 3250
Text Label 5200 3350 0    50   ~ 0
LR-
Wire Wire Line
	5200 2750 5150 2750
Wire Wire Line
	5150 2950 5200 2950
Wire Wire Line
	5200 3150 5150 3150
Wire Wire Line
	5150 3350 5200 3350
$Comp
L open-automation:GND #PWR?
U 1 1 5CD2B935
P 3800 3400
AR Path="/5ABD187E/5CD2B935" Ref="#PWR?"  Part="1" 
AR Path="/5CD2B935" Ref="#PWR0103"  Part="1" 
F 0 "#PWR0103" H 3800 3150 50  0001 C CNN
F 1 "GND" H 3800 3250 50  0000 C CNN
F 2 "" H 3800 3400 50  0000 C CNN
F 3 "" H 3800 3400 50  0000 C CNN
	1    3800 3400
	1    0    0    -1  
$EndComp
Wire Wire Line
	3800 3400 3800 3350
Wire Wire Line
	3800 3350 3850 3350
Text Label 3800 3250 2    50   ~ 0
BATTERY
Wire Wire Line
	3800 3250 3850 3250
Text Label 3800 3000 2    50   ~ 0
CAN+
Text Label 3800 3100 2    50   ~ 0
CAN-
Wire Wire Line
	3800 3100 3850 3100
Wire Wire Line
	3850 3000 3800 3000
Text Label 3800 2650 2    50   ~ 0
CAMERA+
Wire Wire Line
	3800 2650 3850 2650
Text Label 3800 2750 2    50   ~ 0
CAMERA-
Wire Wire Line
	3800 2750 3850 2750
$Comp
L open-automation:GND #PWR?
U 1 1 5CD2B94B
P 3800 2850
AR Path="/5ABD187E/5CD2B94B" Ref="#PWR?"  Part="1" 
AR Path="/5CD2B94B" Ref="#PWR0104"  Part="1" 
F 0 "#PWR0104" H 3800 2600 50  0001 C CNN
F 1 "GND" V 3750 2700 50  0000 C CNN
F 2 "" H 3800 2850 50  0000 C CNN
F 3 "" H 3800 2850 50  0000 C CNN
	1    3800 2850
	0    1    1    0   
$EndComp
Wire Wire Line
	3800 2850 3850 2850
$EndSCHEMATC
