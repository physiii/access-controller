EESchema Schematic File Version 4
LIBS:liger-cache
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 7 9
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text Label 4525 1325 0    394  ~ 0
Relay
$Comp
L liger-rescue:R_10k R26
U 1 1 59B9995F
P 5275 2275
F 0 "R26" V 5275 2275 50  0000 C CNN
F 1 "R_10k" V 5200 2275 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 5205 2275 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/yageo/RC0603JR-0710KL/311-10KGRCT-ND/729647" V 5355 2275 50  0001 C CNN
F 4 "MCT06030C1002FP500" V 5455 2375 60  0001 C CNN "Part Number"
	1    5275 2275
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR028
U 1 1 59B99A98
P 5475 2275
F 0 "#PWR028" H 5475 2025 50  0001 C CNN
F 1 "GND" H 5475 2125 50  0000 C CNN
F 2 "" H 5475 2275 50  0000 C CNN
F 3 "" H 5475 2275 50  0000 C CNN
	1    5475 2275
	0    -1   -1   0   
$EndComp
Wire Wire Line
	5475 2275 5425 2275
Wire Wire Line
	5000 2275 5125 2275
Text HLabel 5000 2275 0    60   Input ~ 0
RELAY
$EndSCHEMATC
