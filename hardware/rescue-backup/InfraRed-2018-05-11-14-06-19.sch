EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:open-automation
LIBS:liger-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 3 9
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text Label 3725 4300 0    60   ~ 0
IR-Receive
$Comp
L GND #PWR04
U 1 1 59098AF2
P 3525 3650
F 0 "#PWR04" H 3525 3400 50  0001 C CNN
F 1 "GND" H 3525 3500 50  0000 C CNN
F 2 "" H 3525 3650 50  0000 C CNN
F 3 "" H 3525 3650 50  0000 C CNN
	1    3525 3650
	1    0    0    -1  
$EndComp
Text GLabel 4875 3550 2    60   Input ~ 0
IR_RX
$Comp
L LED D1
U 1 1 59098AF4
P 7625 3100
F 0 "D1" H 7625 3200 50  0000 C CNN
F 1 "LED" H 7625 3000 50  0000 C CNN
F 2 "LEDs:LED_D3.0mm" H 7625 3100 50  0001 C CNN
F 3 "http://www.mouser.com/ds/2/149/QED123-890063.pdf" H 7625 3100 50  0001 C CNN
F 4 "QED123" H 7625 3100 60  0001 C CNN "Part Number"
	1    7625 3100
	0    -1   -1   0   
$EndComp
$Comp
L R R5
U 1 1 59098AF5
P 7625 2650
F 0 "R5" V 7705 2650 50  0000 C CNN
F 1 "36" V 7625 2650 50  0000 C CNN
F 2 "Resistors_SMD:R_0603_HandSoldering" V 7555 2650 50  0001 C CNN
F 3 "http://www.yageo.com.tw/exep/pages/download/literatures/PYu-R_INT-thick_7.pdf" H 7625 2650 50  0001 C CNN
F 4 "RC0603JR-0736RL" V 7625 2650 60  0001 C CNN "Part Number"
	1    7625 2650
	1    0    0    -1  
$EndComp
$Comp
L Q_NPN_BEC Q1
U 1 1 59098AF8
P 7525 3600
F 0 "Q1" H 7825 3650 50  0000 R CNN
F 1 "Q_NPN_BEC" H 8125 3550 50  0000 R CNN
F 2 "TO_SOT_Packages_SMD:SOT-23" H 7725 3700 50  0001 C CNN
F 3 "http://www.onsemi.com/pub/Collateral/MMBT3904LT1-D.PDF" H 7525 3600 50  0001 C CNN
F 4 "MMBT3904LT1GOSCT-ND" H 7525 3600 60  0001 C CNN "Part Number"
	1    7525 3600
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR05
U 1 1 59098AF9
P 7625 4000
F 0 "#PWR05" H 7625 3750 50  0001 C CNN
F 1 "GND" H 7625 3850 50  0000 C CNN
F 2 "" H 7625 4000 50  0000 C CNN
F 3 "" H 7625 4000 50  0000 C CNN
	1    7625 4000
	1    0    0    -1  
$EndComp
Text GLabel 6475 3600 0    60   Input ~ 0
IR_TX
Text Label 6925 4400 0    60   ~ 0
IR-Transmit
Wire Wire Line
	6775 3600 6475 3600
Wire Wire Line
	7625 3800 7625 4000
Wire Wire Line
	7075 3600 7325 3600
Wire Wire Line
	7625 2350 7625 2500
Wire Wire Line
	7625 2800 7625 2950
Wire Wire Line
	7625 3250 7625 3400
Text GLabel 7625 2350 1    60   Input ~ 0
3V3
Text GLabel 3525 3475 0    60   Input ~ 0
3V3
$Comp
L R_1k R3
U 1 1 59BA28B3
P 4625 3550
F 0 "R3" V 4625 3550 50  0000 C CNN
F 1 "R_1k" V 4550 3550 50  0000 C CNN
F 2 "Resistors_SMD:R_0603_HandSoldering" V 4555 3550 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/yageo/RC0603JR-071KL/311-1.0KGRCT-ND/729624" V 4705 3550 50  0001 C CNN
F 4 "RC0603JR-071KL" V 4805 3650 60  0001 C CNN "Part Number"
	1    4625 3550
	0    1    1    0   
$EndComp
$Comp
L R_1k R4
U 1 1 59BA28E7
P 6925 3600
F 0 "R4" V 6925 3600 50  0000 C CNN
F 1 "R_1k" V 6850 3600 50  0000 C CNN
F 2 "Resistors_SMD:R_0603_HandSoldering" V 6855 3600 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/yageo/RC0603JR-071KL/311-1.0KGRCT-ND/729624" V 7005 3600 50  0001 C CNN
F 4 "RC0603JR-071KL" V 7105 3700 60  0001 C CNN "Part Number"
	1    6925 3600
	0    1    1    0   
$EndComp
Wire Wire Line
	4875 3550 4775 3550
Wire Wire Line
	4475 3550 4400 3550
Wire Wire Line
	3525 3650 3525 3625
Wire Wire Line
	3525 3625 3575 3625
Wire Wire Line
	3525 3475 3575 3475
$Comp
L IR-RX-TSOP34438 U2
U 1 1 5A1853BB
P 3925 3150
F 0 "U2" H 4125 2575 60  0000 C CNN
F 1 "IR-RX-TSOP34438" H 3975 3000 60  0000 C CNN
F 2 "open-automation:TSOP75338TR" H 3925 2500 60  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/vishay-semiconductor-opto-division/TSOP75338TR/TSOP75338CT-ND/6596741" H 3950 2400 60  0001 C CNN
F 4 "TSOP75338TR" H 3950 3100 60  0001 C CNN "Part Number"
	1    3925 3150
	1    0    0    -1  
$EndComp
$EndSCHEMATC
