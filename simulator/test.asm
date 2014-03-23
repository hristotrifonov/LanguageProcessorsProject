 0 add
 1 sub
 2 mult
 3 div
 4 neg
 5 br 20
 6 bgz 20
 7 bg  20
 8 blz    20
 9 bl    20
10 bz    20
10 call 120
11 ret
12 bsf
13 rsf
14 ldp 103
15 rdp 103
16 inc 5
17 dec 5
18 push fp
19 load fp + 5
19 load fp - 5
20 load #1
21 load #-232
22 load 1
23 load [sp]+5
24 load [sp]-5
25 store 40
26 store fp+40
27 store fp-50
28 store [sp]+20
29 store [sp]-30
30 read
31 write
32 load fp
33 load [sp]
34 store fp
35 store [sp]
36 bnz  20
