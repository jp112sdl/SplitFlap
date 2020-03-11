EESchema Schematic File Version 4
EELAYER 30 0
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
L Transistor_Array:ULN2003 U1
U 1 1 5E616674
P 5000 1260
F 0 "U1" H 5000 1927 50  0000 C CNN
F 1 "ULN2003" H 5000 1836 50  0000 C CNN
F 2 "Housings_DIP:DIP-16_W7.62mm_LongPads" H 5050 710 50  0001 L CNN
F 3 "http://www.ti.com/lit/ds/symlink/uln2003a.pdf" H 5100 1060 50  0001 C CNN
	1    5000 1260
	1    0    0    -1  
$EndComp
$Comp
L Interface_Expansion:MCP23017_SP U2
U 1 1 5E6198D4
P 3140 2510
F 0 "U2" H 3140 3791 50  0000 C CNN
F 1 "MCP23017_SP" H 3140 3700 50  0000 C CNN
F 2 "Package_DIP:DIP-28_W7.62mm_LongPads" H 3340 1510 50  0001 L CNN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/20001952C.pdf" H 3340 1410 50  0001 L CNN
	1    3140 2510
	1    0    0    -1  
$EndComp
$Comp
L Interface_Expansion:MCP23017_SP U?
U 1 1 5E61CC9B
P 3140 5150
F 0 "U?" H 3140 6431 50  0000 C CNN
F 1 "MCP23017_SP" H 3140 6340 50  0000 C CNN
F 2 "Package_DIP:DIP-28_W7.62mm_LongPads" H 3340 4150 50  0001 L CNN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/20001952C.pdf" H 3340 4050 50  0001 L CNN
	1    3140 5150
	1    0    0    -1  
$EndComp
$Comp
L Transistor_Array:ULN2003 U?
U 1 1 5E61E2DF
P 5000 2660
F 0 "U?" H 5000 3327 50  0000 C CNN
F 1 "ULN2003" H 5000 3236 50  0000 C CNN
F 2 "Housings_DIP:DIP-16_W7.62mm_LongPads" H 5050 2110 50  0001 L CNN
F 3 "http://www.ti.com/lit/ds/symlink/uln2003a.pdf" H 5100 2460 50  0001 C CNN
	1    5000 2660
	1    0    0    -1  
$EndComp
$Comp
L Transistor_Array:ULN2003 U?
U 1 1 5E61F266
P 4990 4050
F 0 "U?" H 4990 4717 50  0000 C CNN
F 1 "ULN2003" H 4990 4626 50  0000 C CNN
F 2 "Housings_DIP:DIP-16_W7.62mm_LongPads" H 5040 3500 50  0001 L CNN
F 3 "http://www.ti.com/lit/ds/symlink/uln2003a.pdf" H 5090 3850 50  0001 C CNN
	1    4990 4050
	1    0    0    -1  
$EndComp
$Comp
L Transistor_Array:ULN2003 U?
U 1 1 5E620475
P 4950 5460
F 0 "U?" H 4950 6127 50  0000 C CNN
F 1 "ULN2003" H 4950 6036 50  0000 C CNN
F 2 "Housings_DIP:DIP-16_W7.62mm_LongPads" H 5000 4910 50  0001 L CNN
F 3 "http://www.ti.com/lit/ds/symlink/uln2003a.pdf" H 5050 5260 50  0001 C CNN
	1    4950 5460
	1    0    0    -1  
$EndComp
$Comp
L Transistor_Array:ULN2003 U?
U 1 1 5E6276E0
P 4920 6960
F 0 "U?" H 4920 7627 50  0000 C CNN
F 1 "ULN2003" H 4920 7536 50  0000 C CNN
F 2 "Housings_DIP:DIP-16_W7.62mm_LongPads" H 4970 6410 50  0001 L CNN
F 3 "http://www.ti.com/lit/ds/symlink/uln2003a.pdf" H 5020 6760 50  0001 C CNN
	1    4920 6960
	1    0    0    -1  
$EndComp
Wire Wire Line
	3840 1710 3840 1060
Wire Wire Line
	3840 1060 4600 1060
Wire Wire Line
	4600 1160 3890 1160
Wire Wire Line
	3890 1160 3890 1810
Wire Wire Line
	3890 1810 3840 1810
Wire Wire Line
	3840 1910 3940 1910
Wire Wire Line
	3940 1910 3940 1260
Wire Wire Line
	3940 1260 4600 1260
Wire Wire Line
	3840 2010 3990 2010
Wire Wire Line
	3990 2010 3990 1360
Wire Wire Line
	3990 1360 4600 1360
Wire Wire Line
	4600 1460 4050 1460
Wire Wire Line
	4050 1460 4050 2110
Wire Wire Line
	4050 2110 3840 2110
Wire Wire Line
	4600 1560 4110 1560
Wire Wire Line
	4110 1560 4110 2210
Wire Wire Line
	4110 2210 3840 2210
Wire Wire Line
	4600 1660 4170 1660
Wire Wire Line
	4170 1660 4170 2310
Wire Wire Line
	4170 2310 3840 2310
Wire Wire Line
	3840 2410 4170 2410
Wire Wire Line
	4170 2410 4170 2460
Wire Wire Line
	4170 2460 4600 2460
Wire Wire Line
	3840 2610 4170 2610
Wire Wire Line
	4170 2610 4170 2560
Wire Wire Line
	4170 2560 4600 2560
Wire Wire Line
	3840 2710 4210 2710
Wire Wire Line
	4210 2710 4210 2660
Wire Wire Line
	4210 2660 4600 2660
Wire Wire Line
	3840 2810 4240 2810
Wire Wire Line
	4240 2810 4240 2760
Wire Wire Line
	4240 2760 4600 2760
Wire Wire Line
	3840 2910 4280 2910
Wire Wire Line
	4280 2910 4280 2860
Wire Wire Line
	4280 2860 4600 2860
Wire Wire Line
	3840 3010 4320 3010
Wire Wire Line
	4320 3010 4320 2960
Wire Wire Line
	4320 2960 4600 2960
Wire Wire Line
	3840 3110 4360 3110
Wire Wire Line
	4360 3110 4360 3060
Wire Wire Line
	4360 3060 4600 3060
Wire Wire Line
	3840 3210 4410 3210
Wire Wire Line
	4410 3210 4410 3850
Wire Wire Line
	4410 3850 4590 3850
Wire Wire Line
	3840 4350 3950 4350
Wire Wire Line
	3950 4350 3950 3950
Wire Wire Line
	3950 3950 4590 3950
Wire Wire Line
	3840 4450 3990 4450
Wire Wire Line
	3990 4450 3990 4050
Wire Wire Line
	3990 4050 4590 4050
Wire Wire Line
	3840 4550 4040 4550
Wire Wire Line
	4040 4550 4040 4150
Wire Wire Line
	4040 4150 4590 4150
Wire Wire Line
	3840 4650 4100 4650
Wire Wire Line
	4100 4650 4100 4250
Wire Wire Line
	4100 4250 4590 4250
Wire Wire Line
	3840 4750 4160 4750
Wire Wire Line
	4160 4750 4160 4350
Wire Wire Line
	4160 4350 4590 4350
Wire Wire Line
	3840 4850 4220 4850
Wire Wire Line
	4220 4850 4220 4450
Wire Wire Line
	4220 4450 4590 4450
Wire Wire Line
	3840 4950 4460 4950
Wire Wire Line
	4460 4950 4460 5260
Wire Wire Line
	4460 5260 4550 5260
Wire Wire Line
	3840 5050 4400 5050
Wire Wire Line
	4400 5050 4400 5360
Wire Wire Line
	4400 5360 4550 5360
Wire Wire Line
	3840 5250 4340 5250
Wire Wire Line
	4340 5250 4340 5460
Wire Wire Line
	4340 5460 4550 5460
Wire Wire Line
	3840 5350 4280 5350
Wire Wire Line
	4280 5350 4280 5560
Wire Wire Line
	4280 5560 4550 5560
Wire Wire Line
	3840 5450 4210 5450
Wire Wire Line
	4210 5450 4210 5660
Wire Wire Line
	4210 5660 4550 5660
Wire Wire Line
	3840 5550 4140 5550
Wire Wire Line
	4140 5550 4140 5760
Wire Wire Line
	4140 5760 4550 5760
Wire Wire Line
	3840 5650 4070 5650
Wire Wire Line
	4070 5650 4070 5860
Wire Wire Line
	4070 5860 4550 5860
Wire Wire Line
	3840 5750 4000 5750
Wire Wire Line
	4000 5750 4000 6760
Wire Wire Line
	4000 6760 4520 6760
Wire Wire Line
	3840 5850 3920 5850
Wire Wire Line
	3920 5850 3920 6860
Wire Wire Line
	3920 6860 4520 6860
Wire Wire Line
	3840 5950 3840 6960
Wire Wire Line
	3840 6960 4520 6960
$EndSCHEMATC
