; define Pow2er of 2  n   
(define-fun Pow2 ((input Int)) Int 
(ite (= input 0) 1 
(ite (= input 1) 2 
(ite (= input 2) 4 
(ite (= input 3) 8 
(ite (= input 4) 16 
(ite (= input 5) 32 
(ite (= input 6) 64 
(ite (= input 7) 128 
(ite (= input 8) 256 
(ite (= input 9) 512 
(ite (= input 10) 1024 
(ite (= input 11) 2048 
(ite (= input 12) 4096 
(ite (= input 13) 8192 
(ite (= input 14) 16384 
(ite (= input 15) 32768 
(ite (= input 16) 65536 
(ite (= input 17) 131072 
(ite (= input 18) 262144 
(ite (= input 19) 524288 
(ite (= input 20) 1048576 
(ite (= input 21) 2097152 
(ite (= input 22) 4194304 
(ite (= input 23) 8388608 
(ite (= input 24) 16777216 
(ite (= input 25) 33554432 
(ite (= input 26) 67108864 
(ite (= input 27) 134217728 
(ite (= input 28) 268435456 
(ite (= input 29) 536870912 
(ite (= input 30) 1073741824 
(ite (= input 31) 2147483648 
(ite (= input 32) 4294967296 
(ite (= input 33) 8589934592 
(ite (= input 34) 17179869184 
(ite (= input 35) 34359738368 
(ite (= input 36) 68719476736 
(ite (= input 37) 137438953472 
(ite (= input 38) 274877906944 
(ite (= input 39) 549755813888 
(ite (= input 40) 1099511627776 
(ite (= input 41) 2199023255552 
(ite (= input 42) 4398046511104 
(ite (= input 43) 8796093022208 
(ite (= input 44) 17592186044416 
(ite (= input 45) 35184372088832 
(ite (= input 46) 70368744177664 
(ite (= input 47) 140737488355328 
(ite (= input 48) 281474976710656 
(ite (= input 49) 562949953421312 
(ite (= input 50) 1125899906842624 
(ite (= input 51) 2251799813685248 
(ite (= input 52) 4503599627370496 
(ite (= input 53) 9007199254740992 
(ite (= input 54) 18014398509481984 
(ite (= input 55) 36028797018963968 
(ite (= input 56) 72057594037927936 
(ite (= input 57) 144115188075855872 
(ite (= input 58) 288230376151711744 
(ite (= input 59) 576460752303423488 
(ite (= input 60) 1152921504606846976 
(ite (= input 61) 2305843009213693952 
(ite (= input 62) 4611686018427387904 
(ite (= input 63) 9223372036854775808 
(ite (= input 64) 18446744073709551616 
(ite (= input 65) 36893488147419103232 
0  
)))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))

(define-fun DecimalToHexadecimal ((input Int)) String
(ite (= input 0) "\x00"
(ite (= input 1) "\x01"
(ite (= input 2) "\x02"
(ite (= input 3) "\x03"
(ite (= input 4) "\x04"
(ite (= input 5) "\x05"
(ite (= input 6) "\x06"
(ite (= input 7) "\x07"
(ite (= input 8) "\x08"
(ite (= input 9) "\x09"
(ite (= input 10) "\x0a"
(ite (= input 11) "\x0b"
(ite (= input 12) "\x0c"
(ite (= input 13) "\x0d"
(ite (= input 14) "\x0e"
(ite (= input 15) "\x0f"
(ite (= input 16) "\x10"
(ite (= input 17) "\x11"
(ite (= input 18) "\x12"
(ite (= input 19) "\x13"
(ite (= input 20) "\x14"
(ite (= input 21) "\x15"
(ite (= input 22) "\x16"
(ite (= input 23) "\x17"
(ite (= input 24) "\x18"
(ite (= input 25) "\x19"
(ite (= input 26) "\x1a"
(ite (= input 27) "\x1b"
(ite (= input 28) "\x1c"
(ite (= input 29) "\x1d"
(ite (= input 30) "\x1e"
(ite (= input 31) "\x1f"
(ite (= input 32) "\x20"
(ite (= input 33) "\x21"
(ite (= input 34) "\x22"
(ite (= input 35) "\x23"
(ite (= input 36) "\x24"
(ite (= input 37) "\x25"
(ite (= input 38) "\x26"
(ite (= input 39) "\x27"
(ite (= input 40) "\x28"
(ite (= input 41) "\x29"
(ite (= input 42) "\x2a"
(ite (= input 43) "\x2b"
(ite (= input 44) "\x2c"
(ite (= input 45) "\x2d"
(ite (= input 46) "\x2e"
(ite (= input 47) "\x2f"
(ite (= input 48) "\x30"
(ite (= input 49) "\x31"
(ite (= input 50) "\x32"
(ite (= input 51) "\x33"
(ite (= input 52) "\x34"
(ite (= input 53) "\x35"
(ite (= input 54) "\x36"
(ite (= input 55) "\x37"
(ite (= input 56) "\x38"
(ite (= input 57) "\x39"
(ite (= input 58) "\x3a"
(ite (= input 59) "\x3b"
(ite (= input 60) "\x3c"
(ite (= input 61) "\x3d"
(ite (= input 62) "\x3e"
(ite (= input 63) "\x3f"
(ite (= input 64) "\x40"
(ite (= input 65) "\x41"
(ite (= input 66) "\x42"
(ite (= input 67) "\x43"
(ite (= input 68) "\x44"
(ite (= input 69) "\x45"
(ite (= input 70) "\x46"
(ite (= input 71) "\x47"
(ite (= input 72) "\x48"
(ite (= input 73) "\x49"
(ite (= input 74) "\x4a"
(ite (= input 75) "\x4b"
(ite (= input 76) "\x4c"
(ite (= input 77) "\x4d"
(ite (= input 78) "\x4e"
(ite (= input 79) "\x4f"
(ite (= input 80) "\x50"
(ite (= input 81) "\x51"
(ite (= input 82) "\x52"
(ite (= input 83) "\x53"
(ite (= input 84) "\x54"
(ite (= input 85) "\x55"
(ite (= input 86) "\x56"
(ite (= input 87) "\x57"
(ite (= input 88) "\x58"
(ite (= input 89) "\x59"
(ite (= input 90) "\x5a"
(ite (= input 91) "\x5b"
(ite (= input 92) "\x5c"
(ite (= input 93) "\x5d"
(ite (= input 94) "\x5e"
(ite (= input 95) "\x5f"
(ite (= input 96) "\x60"
(ite (= input 97) "\x61"
(ite (= input 98) "\x62"
(ite (= input 99) "\x63"
(ite (= input 100) "\x64"
(ite (= input 101) "\x65"
(ite (= input 102) "\x66"
(ite (= input 103) "\x67"
(ite (= input 104) "\x68"
(ite (= input 105) "\x69"
(ite (= input 106) "\x6a"
(ite (= input 107) "\x6b"
(ite (= input 108) "\x6c"
(ite (= input 109) "\x6d"
(ite (= input 110) "\x6e"
(ite (= input 111) "\x6f"
(ite (= input 112) "\x70"
(ite (= input 113) "\x71"
(ite (= input 114) "\x72"
(ite (= input 115) "\x73"
(ite (= input 116) "\x74"
(ite (= input 117) "\x75"
(ite (= input 118) "\x76"
(ite (= input 119) "\x77"
(ite (= input 120) "\x78"
(ite (= input 121) "\x79"
(ite (= input 122) "\x7a"
(ite (= input 123) "\x7b"
(ite (= input 124) "\x7c"
(ite (= input 125) "\x7d"
(ite (= input 126) "\x7e"
(ite (= input 127) "\x7f"
(ite (= input 128) "\x80"
(ite (= input 129) "\x81"
(ite (= input 130) "\x82"
(ite (= input 131) "\x83"
(ite (= input 132) "\x84"
(ite (= input 133) "\x85"
(ite (= input 134) "\x86"
(ite (= input 135) "\x87"
(ite (= input 136) "\x88"
(ite (= input 137) "\x89"
(ite (= input 138) "\x8a"
(ite (= input 139) "\x8b"
(ite (= input 140) "\x8c"
(ite (= input 141) "\x8d"
(ite (= input 142) "\x8e"
(ite (= input 143) "\x8f"
(ite (= input 144) "\x90"
(ite (= input 145) "\x91"
(ite (= input 146) "\x92"
(ite (= input 147) "\x93"
(ite (= input 148) "\x94"
(ite (= input 149) "\x95"
(ite (= input 150) "\x96"
(ite (= input 151) "\x97"
(ite (= input 152) "\x98"
(ite (= input 153) "\x99"
(ite (= input 154) "\x9a"
(ite (= input 155) "\x9b"
(ite (= input 156) "\x9c"
(ite (= input 157) "\x9d"
(ite (= input 158) "\x9e"
(ite (= input 159) "\x9f"
(ite (= input 160) "\xa0"
(ite (= input 161) "\xa1"
(ite (= input 162) "\xa2"
(ite (= input 163) "\xa3"
(ite (= input 164) "\xa4"
(ite (= input 165) "\xa5"
(ite (= input 166) "\xa6"
(ite (= input 167) "\xa7"
(ite (= input 168) "\xa8"
(ite (= input 169) "\xa9"
(ite (= input 170) "\xaa"
(ite (= input 171) "\xab"
(ite (= input 172) "\xac"
(ite (= input 173) "\xad"
(ite (= input 174) "\xae"
(ite (= input 175) "\xaf"
(ite (= input 176) "\xb0"
(ite (= input 177) "\xb1"
(ite (= input 178) "\xb2"
(ite (= input 179) "\xb3"
(ite (= input 180) "\xb4"
(ite (= input 181) "\xb5"
(ite (= input 182) "\xb6"
(ite (= input 183) "\xb7"
(ite (= input 184) "\xb8"
(ite (= input 185) "\xb9"
(ite (= input 186) "\xba"
(ite (= input 187) "\xbb"
(ite (= input 188) "\xbc"
(ite (= input 189) "\xbd"
(ite (= input 190) "\xbe"
(ite (= input 191) "\xbf"
(ite (= input 192) "\xc0"
(ite (= input 193) "\xc1"
(ite (= input 194) "\xc2"
(ite (= input 195) "\xc3"
(ite (= input 196) "\xc4"
(ite (= input 197) "\xc5"
(ite (= input 198) "\xc6"
(ite (= input 199) "\xc7"
(ite (= input 200) "\xc8"
(ite (= input 201) "\xc9"
(ite (= input 202) "\xca"
(ite (= input 203) "\xcb"
(ite (= input 204) "\xcc"
(ite (= input 205) "\xcd"
(ite (= input 206) "\xce"
(ite (= input 207) "\xcf"
(ite (= input 208) "\xd0"
(ite (= input 209) "\xd1"
(ite (= input 210) "\xd2"
(ite (= input 211) "\xd3"
(ite (= input 212) "\xd4"
(ite (= input 213) "\xd5"
(ite (= input 214) "\xd6"
(ite (= input 215) "\xd7"
(ite (= input 216) "\xd8"
(ite (= input 217) "\xd9"
(ite (= input 218) "\xda"
(ite (= input 219) "\xdb"
(ite (= input 220) "\xdc"
(ite (= input 221) "\xdd"
(ite (= input 222) "\xde"
(ite (= input 223) "\xdf"
(ite (= input 224) "\xe0"
(ite (= input 225) "\xe1"
(ite (= input 226) "\xe2"
(ite (= input 227) "\xe3"
(ite (= input 228) "\xe4"
(ite (= input 229) "\xe5"
(ite (= input 230) "\xe6"
(ite (= input 231) "\xe7"
(ite (= input 232) "\xe8"
(ite (= input 233) "\xe9"
(ite (= input 234) "\xea"
(ite (= input 235) "\xeb"
(ite (= input 236) "\xec"
(ite (= input 237) "\xed"
(ite (= input 238) "\xee"
(ite (= input 239) "\xef"
(ite (= input 240) "\xf0"
(ite (= input 241) "\xf1"
(ite (= input 242) "\xf2"
(ite (= input 243) "\xf3"
(ite (= input 244) "\xf4"
(ite (= input 245) "\xf5"
(ite (= input 246) "\xf6"
(ite (= input 247) "\xf7"
(ite (= input 248) "\xf8"
(ite (= input 249) "\xf9"
(ite (= input 250) "\xfa"
(ite (= input 251) "\xfb"
(ite (= input 252) "\xfc"
(ite (= input 253) "\xfd"
(ite (= input 254) "\xfe"
(ite (= input 255) "\xff"
"\x00"
))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))
)))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))
))))))))))))))))))))))))))))))))))))))))))))


(define-fun GetMSB ((input Int) (type Int)) Int
(ite (> (div (rem input (Pow2 type )) (Pow2 (- type 1))) 0) 1 0))
(define-fun GetLSB ((input Int)) Int
(rem input 2)
)


(define-fun Add ((lhs Int) (rhs Int) (type Int)) Int
(rem (+ 
        (rem lhs (Pow2 type))
        (rem rhs (Pow2 type))        
    )
    (Pow2 type)
))

(define-fun Sub ((lhs Int) (rhs Int) (type Int)) Int
(rem (- 
        (rem lhs (Pow2 type))
        (rem rhs (Pow2 type))        
    )
    (Pow2 type)
))


(define-fun Mul ((lhs Int) (rhs Int) (type Int)) Int
(rem (* 
        (rem lhs (Pow2 type))
        (rem rhs (Pow2 type))        
    )
    (Pow2 type)
))

(define-fun UDiv ((lhs Int) (rhs Int) (type Int)) Int
(rem (div
        (rem lhs (Pow2 type))
        (rem rhs (Pow2 type))        
    )
    (Pow2 type)
))

(define-fun URem ((lhs Int) (rhs Int) (type Int)) Int
(rem
        (rem lhs (Pow2 type))
        (rem rhs (Pow2 type))        
    )
)

;#### not sure about signed and unsigned, especially there is not support for float
(define-fun SDiv((lhs Int) (rhs Int) (type Int)) Int
(UDiv lhs rhs type)
)


(define-fun SRem((lhs Int) (rhs Int) (type Int)) Int
(URem lhs rhs type)
)



; bitwise start
; this function defines ZExt n  
(define-fun ZExt ((input  Int) (type Int)) Int  
(ite (< input (Pow2 type)) input
(rem input (Pow2 type))
))

(define-fun MakeupValue ((l Int) (r Int)) Int
(- (Pow2 l) (Pow2 r)))

; define SExt
(define-fun SExt ((input Int) (inputtype Int) (type Int)) Int 
(ite (= 0 (GetMSB input inputtype)) (ZExt input type)
(+ (rem input (Pow2 inputtype)) (MakeupValue type (- type inputtype)))
))



(define-fun Shl ((lhs Int) (rhs Int) (type Int)) Int
(rem (* lhs (Pow2 rhs)) (Pow2 type))
)
(define-fun LShr ((lhs Int) (rhs Int) (type Int)) Int
(div (rem lhs (Pow2 type)) (Pow2 rhs)))


; @bug if there lhs longher than type!!!! overflow???? @todo
(define-fun AShr ((lhs Int) (rhs Int) (type Int)) Int
(ite (= 0  (GetMSB lhs type)) 
    (div (rem lhs (Pow2 type)) (Pow2 rhs))
    (+ 
        (MakeupValue type (- type rhs)) 
        (div (rem lhs (Pow2 type)) (Pow2 rhs))
    )
)
)
; we say type == -1 if it is not specified!!!!
(define-fun notbit ((val Int)) Int
(rem (+ val 1) 2))

(define-fun notallbits ((val Int) (type Int)) Int
(rem 
(+
             (* (notbit (rem (div val (Pow2 0)) 2)) (Pow2 0)) 
(+
             (* (notbit (rem (div val (Pow2 1)) 2)) (Pow2 1)) 
(+
             (* (notbit (rem (div val (Pow2 2)) 2)) (Pow2 2)) 
(+
             (* (notbit (rem (div val (Pow2 3)) 2)) (Pow2 3)) 
(+
             (* (notbit (rem (div val (Pow2 4)) 2)) (Pow2 4)) 
(+
             (* (notbit (rem (div val (Pow2 5)) 2)) (Pow2 5)) 
(+
             (* (notbit (rem (div val (Pow2 6)) 2)) (Pow2 6)) 
(+
             (* (notbit (rem (div val (Pow2 7)) 2)) (Pow2 7)) 
(+
             (* (notbit (rem (div val (Pow2 8)) 2)) (Pow2 8)) 
(+
             (* (notbit (rem (div val (Pow2 9)) 2)) (Pow2 9)) 
(+
             (* (notbit (rem (div val (Pow2 10)) 2)) (Pow2 10)) 
(+
             (* (notbit (rem (div val (Pow2 11)) 2)) (Pow2 11)) 
(+
             (* (notbit (rem (div val (Pow2 12)) 2)) (Pow2 12)) 
(+
             (* (notbit (rem (div val (Pow2 13)) 2)) (Pow2 13)) 
(+
             (* (notbit (rem (div val (Pow2 14)) 2)) (Pow2 14)) 
(+
             (* (notbit (rem (div val (Pow2 15)) 2)) (Pow2 15)) 
(+
             (* (notbit (rem (div val (Pow2 16)) 2)) (Pow2 16)) 
(+
             (* (notbit (rem (div val (Pow2 17)) 2)) (Pow2 17)) 
(+
             (* (notbit (rem (div val (Pow2 18)) 2)) (Pow2 18)) 
(+
             (* (notbit (rem (div val (Pow2 19)) 2)) (Pow2 19)) 
(+
             (* (notbit (rem (div val (Pow2 20)) 2)) (Pow2 20)) 
(+
             (* (notbit (rem (div val (Pow2 21)) 2)) (Pow2 21)) 
(+
             (* (notbit (rem (div val (Pow2 22)) 2)) (Pow2 22)) 
(+
             (* (notbit (rem (div val (Pow2 23)) 2)) (Pow2 23)) 
(+
             (* (notbit (rem (div val (Pow2 24)) 2)) (Pow2 24)) 
(+
             (* (notbit (rem (div val (Pow2 25)) 2)) (Pow2 25)) 
(+
             (* (notbit (rem (div val (Pow2 26)) 2)) (Pow2 26)) 
(+
             (* (notbit (rem (div val (Pow2 27)) 2)) (Pow2 27)) 
(+
             (* (notbit (rem (div val (Pow2 28)) 2)) (Pow2 28)) 
(+
             (* (notbit (rem (div val (Pow2 29)) 2)) (Pow2 29)) 
(+
             (* (notbit (rem (div val (Pow2 30)) 2)) (Pow2 30)) 
(+
             (* (notbit (rem (div val (Pow2 31)) 2)) (Pow2 31)) 
(+
             (* (notbit (rem (div val (Pow2 32)) 2)) (Pow2 32)) 
(+
             (* (notbit (rem (div val (Pow2 33)) 2)) (Pow2 33)) 
(+
             (* (notbit (rem (div val (Pow2 34)) 2)) (Pow2 34)) 
(+
             (* (notbit (rem (div val (Pow2 35)) 2)) (Pow2 35)) 
(+
             (* (notbit (rem (div val (Pow2 36)) 2)) (Pow2 36)) 
(+
             (* (notbit (rem (div val (Pow2 37)) 2)) (Pow2 37)) 
(+
             (* (notbit (rem (div val (Pow2 38)) 2)) (Pow2 38)) 
(+
             (* (notbit (rem (div val (Pow2 39)) 2)) (Pow2 39)) 
(+
             (* (notbit (rem (div val (Pow2 40)) 2)) (Pow2 40)) 
(+
             (* (notbit (rem (div val (Pow2 41)) 2)) (Pow2 41)) 
(+
             (* (notbit (rem (div val (Pow2 42)) 2)) (Pow2 42)) 
(+
             (* (notbit (rem (div val (Pow2 43)) 2)) (Pow2 43)) 
(+
             (* (notbit (rem (div val (Pow2 44)) 2)) (Pow2 44)) 
(+
             (* (notbit (rem (div val (Pow2 45)) 2)) (Pow2 45)) 
(+
             (* (notbit (rem (div val (Pow2 46)) 2)) (Pow2 46)) 
(+
             (* (notbit (rem (div val (Pow2 47)) 2)) (Pow2 47)) 
(+
             (* (notbit (rem (div val (Pow2 48)) 2)) (Pow2 48)) 
(+
             (* (notbit (rem (div val (Pow2 49)) 2)) (Pow2 49)) 
(+
             (* (notbit (rem (div val (Pow2 50)) 2)) (Pow2 50)) 
(+
             (* (notbit (rem (div val (Pow2 51)) 2)) (Pow2 51)) 
(+
             (* (notbit (rem (div val (Pow2 52)) 2)) (Pow2 52)) 
(+
             (* (notbit (rem (div val (Pow2 53)) 2)) (Pow2 53)) 
(+
             (* (notbit (rem (div val (Pow2 54)) 2)) (Pow2 54)) 
(+
             (* (notbit (rem (div val (Pow2 55)) 2)) (Pow2 55)) 
(+
             (* (notbit (rem (div val (Pow2 56)) 2)) (Pow2 56)) 
(+
             (* (notbit (rem (div val (Pow2 57)) 2)) (Pow2 57)) 
(+
             (* (notbit (rem (div val (Pow2 58)) 2)) (Pow2 58)) 
(+
             (* (notbit (rem (div val (Pow2 59)) 2)) (Pow2 59)) 
(+
             (* (notbit (rem (div val (Pow2 60)) 2)) (Pow2 60)) 
(+
             (* (notbit (rem (div val (Pow2 61)) 2)) (Pow2 61)) 
(+
             (* (notbit (rem (div val (Pow2 62)) 2)) (Pow2 62)) 
(+
             (* (notbit (rem (div val (Pow2 63)) 2)) (Pow2 63)) 
))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))
(Pow2 type)
))

(define-fun Not ((val Int) (type Int)) Int
(ite (< type 0) (notallbits val 64)
(notallbits val type)
))
(define-fun andbit ((lb Int) (rb Int)) Int
(ite (and (= lb 1) (= rb 1)) 1 0)
)

(define-fun orbit ((lb Int) (rb Int)) Int
(ite (and (= lb 0) (= rb 0)) 0 1)
)

(define-fun xorbit ((lb Int) (rb Int)) Int
(ite (= lb rb) 0 1)
)

(define-fun Xor ((lhs Int) (rhs Int) (type Int)) Int
(rem
(+
             (* (xorbit (rem (div lhs (Pow2 0)) 2) (rem (div rhs (Pow2 0)) 2)) (Pow2 0)) 
(+
             (* (xorbit (rem (div lhs (Pow2 1)) 2) (rem (div rhs (Pow2 1)) 2)) (Pow2 1)) 
(+
             (* (xorbit (rem (div lhs (Pow2 2)) 2) (rem (div rhs (Pow2 2)) 2)) (Pow2 2)) 
(+
             (* (xorbit (rem (div lhs (Pow2 3)) 2) (rem (div rhs (Pow2 3)) 2)) (Pow2 3)) 
(+
             (* (xorbit (rem (div lhs (Pow2 4)) 2) (rem (div rhs (Pow2 4)) 2)) (Pow2 4)) 
(+
             (* (xorbit (rem (div lhs (Pow2 5)) 2) (rem (div rhs (Pow2 5)) 2)) (Pow2 5)) 
(+
             (* (xorbit (rem (div lhs (Pow2 6)) 2) (rem (div rhs (Pow2 6)) 2)) (Pow2 6)) 
(+
             (* (xorbit (rem (div lhs (Pow2 7)) 2) (rem (div rhs (Pow2 7)) 2)) (Pow2 7)) 
(+
             (* (xorbit (rem (div lhs (Pow2 8)) 2) (rem (div rhs (Pow2 8)) 2)) (Pow2 8)) 
(+
             (* (xorbit (rem (div lhs (Pow2 9)) 2) (rem (div rhs (Pow2 9)) 2)) (Pow2 9)) 
(+
             (* (xorbit (rem (div lhs (Pow2 10)) 2) (rem (div rhs (Pow2 10)) 2)) (Pow2 10)) 
(+
             (* (xorbit (rem (div lhs (Pow2 11)) 2) (rem (div rhs (Pow2 11)) 2)) (Pow2 11)) 
(+
             (* (xorbit (rem (div lhs (Pow2 12)) 2) (rem (div rhs (Pow2 12)) 2)) (Pow2 12)) 
(+
             (* (xorbit (rem (div lhs (Pow2 13)) 2) (rem (div rhs (Pow2 13)) 2)) (Pow2 13)) 
(+
             (* (xorbit (rem (div lhs (Pow2 14)) 2) (rem (div rhs (Pow2 14)) 2)) (Pow2 14)) 
(+
             (* (xorbit (rem (div lhs (Pow2 15)) 2) (rem (div rhs (Pow2 15)) 2)) (Pow2 15)) 
(+
             (* (xorbit (rem (div lhs (Pow2 16)) 2) (rem (div rhs (Pow2 16)) 2)) (Pow2 16)) 
(+
             (* (xorbit (rem (div lhs (Pow2 17)) 2) (rem (div rhs (Pow2 17)) 2)) (Pow2 17)) 
(+
             (* (xorbit (rem (div lhs (Pow2 18)) 2) (rem (div rhs (Pow2 18)) 2)) (Pow2 18)) 
(+
             (* (xorbit (rem (div lhs (Pow2 19)) 2) (rem (div rhs (Pow2 19)) 2)) (Pow2 19)) 
(+
             (* (xorbit (rem (div lhs (Pow2 20)) 2) (rem (div rhs (Pow2 20)) 2)) (Pow2 20)) 
(+
             (* (xorbit (rem (div lhs (Pow2 21)) 2) (rem (div rhs (Pow2 21)) 2)) (Pow2 21)) 
(+
             (* (xorbit (rem (div lhs (Pow2 22)) 2) (rem (div rhs (Pow2 22)) 2)) (Pow2 22)) 
(+
             (* (xorbit (rem (div lhs (Pow2 23)) 2) (rem (div rhs (Pow2 23)) 2)) (Pow2 23)) 
(+
             (* (xorbit (rem (div lhs (Pow2 24)) 2) (rem (div rhs (Pow2 24)) 2)) (Pow2 24)) 
(+
             (* (xorbit (rem (div lhs (Pow2 25)) 2) (rem (div rhs (Pow2 25)) 2)) (Pow2 25)) 
(+
             (* (xorbit (rem (div lhs (Pow2 26)) 2) (rem (div rhs (Pow2 26)) 2)) (Pow2 26)) 
(+
             (* (xorbit (rem (div lhs (Pow2 27)) 2) (rem (div rhs (Pow2 27)) 2)) (Pow2 27)) 
(+
             (* (xorbit (rem (div lhs (Pow2 28)) 2) (rem (div rhs (Pow2 28)) 2)) (Pow2 28)) 
(+
             (* (xorbit (rem (div lhs (Pow2 29)) 2) (rem (div rhs (Pow2 29)) 2)) (Pow2 29)) 
(+
             (* (xorbit (rem (div lhs (Pow2 30)) 2) (rem (div rhs (Pow2 30)) 2)) (Pow2 30)) 
(+
             (* (xorbit (rem (div lhs (Pow2 31)) 2) (rem (div rhs (Pow2 31)) 2)) (Pow2 31)) 
(+
             (* (xorbit (rem (div lhs (Pow2 32)) 2) (rem (div rhs (Pow2 32)) 2)) (Pow2 32)) 
(+
             (* (xorbit (rem (div lhs (Pow2 33)) 2) (rem (div rhs (Pow2 33)) 2)) (Pow2 33)) 
(+
             (* (xorbit (rem (div lhs (Pow2 34)) 2) (rem (div rhs (Pow2 34)) 2)) (Pow2 34)) 
(+
             (* (xorbit (rem (div lhs (Pow2 35)) 2) (rem (div rhs (Pow2 35)) 2)) (Pow2 35)) 
(+
             (* (xorbit (rem (div lhs (Pow2 36)) 2) (rem (div rhs (Pow2 36)) 2)) (Pow2 36)) 
(+
             (* (xorbit (rem (div lhs (Pow2 37)) 2) (rem (div rhs (Pow2 37)) 2)) (Pow2 37)) 
(+
             (* (xorbit (rem (div lhs (Pow2 38)) 2) (rem (div rhs (Pow2 38)) 2)) (Pow2 38)) 
(+
             (* (xorbit (rem (div lhs (Pow2 39)) 2) (rem (div rhs (Pow2 39)) 2)) (Pow2 39)) 
(+
             (* (xorbit (rem (div lhs (Pow2 40)) 2) (rem (div rhs (Pow2 40)) 2)) (Pow2 40)) 
(+
             (* (xorbit (rem (div lhs (Pow2 41)) 2) (rem (div rhs (Pow2 41)) 2)) (Pow2 41)) 
(+
             (* (xorbit (rem (div lhs (Pow2 42)) 2) (rem (div rhs (Pow2 42)) 2)) (Pow2 42)) 
(+
             (* (xorbit (rem (div lhs (Pow2 43)) 2) (rem (div rhs (Pow2 43)) 2)) (Pow2 43)) 
(+
             (* (xorbit (rem (div lhs (Pow2 44)) 2) (rem (div rhs (Pow2 44)) 2)) (Pow2 44)) 
(+
             (* (xorbit (rem (div lhs (Pow2 45)) 2) (rem (div rhs (Pow2 45)) 2)) (Pow2 45)) 
(+
             (* (xorbit (rem (div lhs (Pow2 46)) 2) (rem (div rhs (Pow2 46)) 2)) (Pow2 46)) 
(+
             (* (xorbit (rem (div lhs (Pow2 47)) 2) (rem (div rhs (Pow2 47)) 2)) (Pow2 47)) 
(+
             (* (xorbit (rem (div lhs (Pow2 48)) 2) (rem (div rhs (Pow2 48)) 2)) (Pow2 48)) 
(+
             (* (xorbit (rem (div lhs (Pow2 49)) 2) (rem (div rhs (Pow2 49)) 2)) (Pow2 49)) 
(+
             (* (xorbit (rem (div lhs (Pow2 50)) 2) (rem (div rhs (Pow2 50)) 2)) (Pow2 50)) 
(+
             (* (xorbit (rem (div lhs (Pow2 51)) 2) (rem (div rhs (Pow2 51)) 2)) (Pow2 51)) 
(+
             (* (xorbit (rem (div lhs (Pow2 52)) 2) (rem (div rhs (Pow2 52)) 2)) (Pow2 52)) 
(+
             (* (xorbit (rem (div lhs (Pow2 53)) 2) (rem (div rhs (Pow2 53)) 2)) (Pow2 53)) 
(+
             (* (xorbit (rem (div lhs (Pow2 54)) 2) (rem (div rhs (Pow2 54)) 2)) (Pow2 54)) 
(+
             (* (xorbit (rem (div lhs (Pow2 55)) 2) (rem (div rhs (Pow2 55)) 2)) (Pow2 55)) 
(+
             (* (xorbit (rem (div lhs (Pow2 56)) 2) (rem (div rhs (Pow2 56)) 2)) (Pow2 56)) 
(+
             (* (xorbit (rem (div lhs (Pow2 57)) 2) (rem (div rhs (Pow2 57)) 2)) (Pow2 57)) 
(+
             (* (xorbit (rem (div lhs (Pow2 58)) 2) (rem (div rhs (Pow2 58)) 2)) (Pow2 58)) 
(+
             (* (xorbit (rem (div lhs (Pow2 59)) 2) (rem (div rhs (Pow2 59)) 2)) (Pow2 59)) 
(+
             (* (xorbit (rem (div lhs (Pow2 60)) 2) (rem (div rhs (Pow2 60)) 2)) (Pow2 60)) 
(+
             (* (xorbit (rem (div lhs (Pow2 61)) 2) (rem (div rhs (Pow2 61)) 2)) (Pow2 61)) 
(+
             (* (xorbit (rem (div lhs (Pow2 62)) 2) (rem (div rhs (Pow2 62)) 2)) (Pow2 62)) 
(+
             (* (xorbit (rem (div lhs (Pow2 63)) 2) (rem (div rhs (Pow2 63)) 2)) (Pow2 63))
))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))
(Pow2 type)
))
(define-fun Or ((lhs Int) (rhs Int) (type Int)) Int
(rem
(+
             (* (orbit (rem (div lhs (Pow2 0)) 2) (rem (div rhs (Pow2 0)) 2)) (Pow2 0)) 
(+
             (* (orbit (rem (div lhs (Pow2 1)) 2) (rem (div rhs (Pow2 1)) 2)) (Pow2 1)) 
(+
             (* (orbit (rem (div lhs (Pow2 2)) 2) (rem (div rhs (Pow2 2)) 2)) (Pow2 2)) 
(+
             (* (orbit (rem (div lhs (Pow2 3)) 2) (rem (div rhs (Pow2 3)) 2)) (Pow2 3)) 
(+
             (* (orbit (rem (div lhs (Pow2 4)) 2) (rem (div rhs (Pow2 4)) 2)) (Pow2 4)) 
(+
             (* (orbit (rem (div lhs (Pow2 5)) 2) (rem (div rhs (Pow2 5)) 2)) (Pow2 5)) 
(+
             (* (orbit (rem (div lhs (Pow2 6)) 2) (rem (div rhs (Pow2 6)) 2)) (Pow2 6)) 
(+
             (* (orbit (rem (div lhs (Pow2 7)) 2) (rem (div rhs (Pow2 7)) 2)) (Pow2 7)) 
(+
             (* (orbit (rem (div lhs (Pow2 8)) 2) (rem (div rhs (Pow2 8)) 2)) (Pow2 8)) 
(+
             (* (orbit (rem (div lhs (Pow2 9)) 2) (rem (div rhs (Pow2 9)) 2)) (Pow2 9)) 
(+
             (* (orbit (rem (div lhs (Pow2 10)) 2) (rem (div rhs (Pow2 10)) 2)) (Pow2 10)) 
(+
             (* (orbit (rem (div lhs (Pow2 11)) 2) (rem (div rhs (Pow2 11)) 2)) (Pow2 11)) 
(+
             (* (orbit (rem (div lhs (Pow2 12)) 2) (rem (div rhs (Pow2 12)) 2)) (Pow2 12)) 
(+
             (* (orbit (rem (div lhs (Pow2 13)) 2) (rem (div rhs (Pow2 13)) 2)) (Pow2 13)) 
(+
             (* (orbit (rem (div lhs (Pow2 14)) 2) (rem (div rhs (Pow2 14)) 2)) (Pow2 14)) 
(+
             (* (orbit (rem (div lhs (Pow2 15)) 2) (rem (div rhs (Pow2 15)) 2)) (Pow2 15)) 
(+
             (* (orbit (rem (div lhs (Pow2 16)) 2) (rem (div rhs (Pow2 16)) 2)) (Pow2 16)) 
(+
             (* (orbit (rem (div lhs (Pow2 17)) 2) (rem (div rhs (Pow2 17)) 2)) (Pow2 17)) 
(+
             (* (orbit (rem (div lhs (Pow2 18)) 2) (rem (div rhs (Pow2 18)) 2)) (Pow2 18)) 
(+
             (* (orbit (rem (div lhs (Pow2 19)) 2) (rem (div rhs (Pow2 19)) 2)) (Pow2 19)) 
(+
             (* (orbit (rem (div lhs (Pow2 20)) 2) (rem (div rhs (Pow2 20)) 2)) (Pow2 20)) 
(+
             (* (orbit (rem (div lhs (Pow2 21)) 2) (rem (div rhs (Pow2 21)) 2)) (Pow2 21)) 
(+
             (* (orbit (rem (div lhs (Pow2 22)) 2) (rem (div rhs (Pow2 22)) 2)) (Pow2 22)) 
(+
             (* (orbit (rem (div lhs (Pow2 23)) 2) (rem (div rhs (Pow2 23)) 2)) (Pow2 23)) 
(+
             (* (orbit (rem (div lhs (Pow2 24)) 2) (rem (div rhs (Pow2 24)) 2)) (Pow2 24)) 
(+
             (* (orbit (rem (div lhs (Pow2 25)) 2) (rem (div rhs (Pow2 25)) 2)) (Pow2 25)) 
(+
             (* (orbit (rem (div lhs (Pow2 26)) 2) (rem (div rhs (Pow2 26)) 2)) (Pow2 26)) 
(+
             (* (orbit (rem (div lhs (Pow2 27)) 2) (rem (div rhs (Pow2 27)) 2)) (Pow2 27)) 
(+
             (* (orbit (rem (div lhs (Pow2 28)) 2) (rem (div rhs (Pow2 28)) 2)) (Pow2 28)) 
(+
             (* (orbit (rem (div lhs (Pow2 29)) 2) (rem (div rhs (Pow2 29)) 2)) (Pow2 29)) 
(+
             (* (orbit (rem (div lhs (Pow2 30)) 2) (rem (div rhs (Pow2 30)) 2)) (Pow2 30)) 
(+
             (* (orbit (rem (div lhs (Pow2 31)) 2) (rem (div rhs (Pow2 31)) 2)) (Pow2 31)) 
(+
             (* (orbit (rem (div lhs (Pow2 32)) 2) (rem (div rhs (Pow2 32)) 2)) (Pow2 32)) 
(+
             (* (orbit (rem (div lhs (Pow2 33)) 2) (rem (div rhs (Pow2 33)) 2)) (Pow2 33)) 
(+
             (* (orbit (rem (div lhs (Pow2 34)) 2) (rem (div rhs (Pow2 34)) 2)) (Pow2 34)) 
(+
             (* (orbit (rem (div lhs (Pow2 35)) 2) (rem (div rhs (Pow2 35)) 2)) (Pow2 35)) 
(+
             (* (orbit (rem (div lhs (Pow2 36)) 2) (rem (div rhs (Pow2 36)) 2)) (Pow2 36)) 
(+
             (* (orbit (rem (div lhs (Pow2 37)) 2) (rem (div rhs (Pow2 37)) 2)) (Pow2 37)) 
(+
             (* (orbit (rem (div lhs (Pow2 38)) 2) (rem (div rhs (Pow2 38)) 2)) (Pow2 38)) 
(+
             (* (orbit (rem (div lhs (Pow2 39)) 2) (rem (div rhs (Pow2 39)) 2)) (Pow2 39)) 
(+
             (* (orbit (rem (div lhs (Pow2 40)) 2) (rem (div rhs (Pow2 40)) 2)) (Pow2 40)) 
(+
             (* (orbit (rem (div lhs (Pow2 41)) 2) (rem (div rhs (Pow2 41)) 2)) (Pow2 41)) 
(+
             (* (orbit (rem (div lhs (Pow2 42)) 2) (rem (div rhs (Pow2 42)) 2)) (Pow2 42)) 
(+
             (* (orbit (rem (div lhs (Pow2 43)) 2) (rem (div rhs (Pow2 43)) 2)) (Pow2 43)) 
(+
             (* (orbit (rem (div lhs (Pow2 44)) 2) (rem (div rhs (Pow2 44)) 2)) (Pow2 44)) 
(+
             (* (orbit (rem (div lhs (Pow2 45)) 2) (rem (div rhs (Pow2 45)) 2)) (Pow2 45)) 
(+
             (* (orbit (rem (div lhs (Pow2 46)) 2) (rem (div rhs (Pow2 46)) 2)) (Pow2 46)) 
(+
             (* (orbit (rem (div lhs (Pow2 47)) 2) (rem (div rhs (Pow2 47)) 2)) (Pow2 47)) 
(+
             (* (orbit (rem (div lhs (Pow2 48)) 2) (rem (div rhs (Pow2 48)) 2)) (Pow2 48)) 
(+
             (* (orbit (rem (div lhs (Pow2 49)) 2) (rem (div rhs (Pow2 49)) 2)) (Pow2 49)) 
(+
             (* (orbit (rem (div lhs (Pow2 50)) 2) (rem (div rhs (Pow2 50)) 2)) (Pow2 50)) 
(+
             (* (orbit (rem (div lhs (Pow2 51)) 2) (rem (div rhs (Pow2 51)) 2)) (Pow2 51)) 
(+
             (* (orbit (rem (div lhs (Pow2 52)) 2) (rem (div rhs (Pow2 52)) 2)) (Pow2 52)) 
(+
             (* (orbit (rem (div lhs (Pow2 53)) 2) (rem (div rhs (Pow2 53)) 2)) (Pow2 53)) 
(+
             (* (orbit (rem (div lhs (Pow2 54)) 2) (rem (div rhs (Pow2 54)) 2)) (Pow2 54)) 
(+
             (* (orbit (rem (div lhs (Pow2 55)) 2) (rem (div rhs (Pow2 55)) 2)) (Pow2 55)) 
(+
             (* (orbit (rem (div lhs (Pow2 56)) 2) (rem (div rhs (Pow2 56)) 2)) (Pow2 56)) 
(+
             (* (orbit (rem (div lhs (Pow2 57)) 2) (rem (div rhs (Pow2 57)) 2)) (Pow2 57)) 
(+
             (* (orbit (rem (div lhs (Pow2 58)) 2) (rem (div rhs (Pow2 58)) 2)) (Pow2 58)) 
(+
             (* (orbit (rem (div lhs (Pow2 59)) 2) (rem (div rhs (Pow2 59)) 2)) (Pow2 59)) 
(+
             (* (orbit (rem (div lhs (Pow2 60)) 2) (rem (div rhs (Pow2 60)) 2)) (Pow2 60)) 
(+
             (* (orbit (rem (div lhs (Pow2 61)) 2) (rem (div rhs (Pow2 61)) 2)) (Pow2 61)) 
(+
             (* (orbit (rem (div lhs (Pow2 62)) 2) (rem (div rhs (Pow2 62)) 2)) (Pow2 62)) 
(+
             (* (orbit (rem (div lhs (Pow2 63)) 2) (rem (div rhs (Pow2 63)) 2)) (Pow2 63))
))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))
(Pow2 type)
))

(define-fun And ((lhs Int) (rhs Int) (type Int)) Int
(rem 
(+
             (* (andbit (rem (div lhs (Pow2 0)) 2) (rem (div rhs (Pow2 0)) 2)) (Pow2 0)) 
(+
             (* (andbit (rem (div lhs (Pow2 1)) 2) (rem (div rhs (Pow2 1)) 2)) (Pow2 1)) 
(+
             (* (andbit (rem (div lhs (Pow2 2)) 2) (rem (div rhs (Pow2 2)) 2)) (Pow2 2)) 
(+
             (* (andbit (rem (div lhs (Pow2 3)) 2) (rem (div rhs (Pow2 3)) 2)) (Pow2 3)) 
(+
             (* (andbit (rem (div lhs (Pow2 4)) 2) (rem (div rhs (Pow2 4)) 2)) (Pow2 4)) 
(+
             (* (andbit (rem (div lhs (Pow2 5)) 2) (rem (div rhs (Pow2 5)) 2)) (Pow2 5)) 
(+
             (* (andbit (rem (div lhs (Pow2 6)) 2) (rem (div rhs (Pow2 6)) 2)) (Pow2 6)) 
(+
             (* (andbit (rem (div lhs (Pow2 7)) 2) (rem (div rhs (Pow2 7)) 2)) (Pow2 7)) 
(+
             (* (andbit (rem (div lhs (Pow2 8)) 2) (rem (div rhs (Pow2 8)) 2)) (Pow2 8)) 
(+
             (* (andbit (rem (div lhs (Pow2 9)) 2) (rem (div rhs (Pow2 9)) 2)) (Pow2 9)) 
(+
             (* (andbit (rem (div lhs (Pow2 10)) 2) (rem (div rhs (Pow2 10)) 2)) (Pow2 10)) 
(+
             (* (andbit (rem (div lhs (Pow2 11)) 2) (rem (div rhs (Pow2 11)) 2)) (Pow2 11)) 
(+
             (* (andbit (rem (div lhs (Pow2 12)) 2) (rem (div rhs (Pow2 12)) 2)) (Pow2 12)) 
(+
             (* (andbit (rem (div lhs (Pow2 13)) 2) (rem (div rhs (Pow2 13)) 2)) (Pow2 13)) 
(+
             (* (andbit (rem (div lhs (Pow2 14)) 2) (rem (div rhs (Pow2 14)) 2)) (Pow2 14)) 
(+
             (* (andbit (rem (div lhs (Pow2 15)) 2) (rem (div rhs (Pow2 15)) 2)) (Pow2 15)) 
(+
             (* (andbit (rem (div lhs (Pow2 16)) 2) (rem (div rhs (Pow2 16)) 2)) (Pow2 16)) 
(+
             (* (andbit (rem (div lhs (Pow2 17)) 2) (rem (div rhs (Pow2 17)) 2)) (Pow2 17)) 
(+
             (* (andbit (rem (div lhs (Pow2 18)) 2) (rem (div rhs (Pow2 18)) 2)) (Pow2 18)) 
(+
             (* (andbit (rem (div lhs (Pow2 19)) 2) (rem (div rhs (Pow2 19)) 2)) (Pow2 19)) 
(+
             (* (andbit (rem (div lhs (Pow2 20)) 2) (rem (div rhs (Pow2 20)) 2)) (Pow2 20)) 
(+
             (* (andbit (rem (div lhs (Pow2 21)) 2) (rem (div rhs (Pow2 21)) 2)) (Pow2 21)) 
(+
             (* (andbit (rem (div lhs (Pow2 22)) 2) (rem (div rhs (Pow2 22)) 2)) (Pow2 22)) 
(+
             (* (andbit (rem (div lhs (Pow2 23)) 2) (rem (div rhs (Pow2 23)) 2)) (Pow2 23)) 
(+
             (* (andbit (rem (div lhs (Pow2 24)) 2) (rem (div rhs (Pow2 24)) 2)) (Pow2 24)) 
(+
             (* (andbit (rem (div lhs (Pow2 25)) 2) (rem (div rhs (Pow2 25)) 2)) (Pow2 25)) 
(+
             (* (andbit (rem (div lhs (Pow2 26)) 2) (rem (div rhs (Pow2 26)) 2)) (Pow2 26)) 
(+
             (* (andbit (rem (div lhs (Pow2 27)) 2) (rem (div rhs (Pow2 27)) 2)) (Pow2 27)) 
(+
             (* (andbit (rem (div lhs (Pow2 28)) 2) (rem (div rhs (Pow2 28)) 2)) (Pow2 28)) 
(+
             (* (andbit (rem (div lhs (Pow2 29)) 2) (rem (div rhs (Pow2 29)) 2)) (Pow2 29)) 
(+
             (* (andbit (rem (div lhs (Pow2 30)) 2) (rem (div rhs (Pow2 30)) 2)) (Pow2 30)) 
(+
             (* (andbit (rem (div lhs (Pow2 31)) 2) (rem (div rhs (Pow2 31)) 2)) (Pow2 31)) 
(+
             (* (andbit (rem (div lhs (Pow2 32)) 2) (rem (div rhs (Pow2 32)) 2)) (Pow2 32)) 
(+
             (* (andbit (rem (div lhs (Pow2 33)) 2) (rem (div rhs (Pow2 33)) 2)) (Pow2 33)) 
(+
             (* (andbit (rem (div lhs (Pow2 34)) 2) (rem (div rhs (Pow2 34)) 2)) (Pow2 34)) 
(+
             (* (andbit (rem (div lhs (Pow2 35)) 2) (rem (div rhs (Pow2 35)) 2)) (Pow2 35)) 
(+
             (* (andbit (rem (div lhs (Pow2 36)) 2) (rem (div rhs (Pow2 36)) 2)) (Pow2 36)) 
(+
             (* (andbit (rem (div lhs (Pow2 37)) 2) (rem (div rhs (Pow2 37)) 2)) (Pow2 37)) 
(+
             (* (andbit (rem (div lhs (Pow2 38)) 2) (rem (div rhs (Pow2 38)) 2)) (Pow2 38)) 
(+
             (* (andbit (rem (div lhs (Pow2 39)) 2) (rem (div rhs (Pow2 39)) 2)) (Pow2 39)) 
(+
             (* (andbit (rem (div lhs (Pow2 40)) 2) (rem (div rhs (Pow2 40)) 2)) (Pow2 40)) 
(+
             (* (andbit (rem (div lhs (Pow2 41)) 2) (rem (div rhs (Pow2 41)) 2)) (Pow2 41)) 
(+
             (* (andbit (rem (div lhs (Pow2 42)) 2) (rem (div rhs (Pow2 42)) 2)) (Pow2 42)) 
(+
             (* (andbit (rem (div lhs (Pow2 43)) 2) (rem (div rhs (Pow2 43)) 2)) (Pow2 43)) 
(+
             (* (andbit (rem (div lhs (Pow2 44)) 2) (rem (div rhs (Pow2 44)) 2)) (Pow2 44)) 
(+
             (* (andbit (rem (div lhs (Pow2 45)) 2) (rem (div rhs (Pow2 45)) 2)) (Pow2 45)) 
(+
             (* (andbit (rem (div lhs (Pow2 46)) 2) (rem (div rhs (Pow2 46)) 2)) (Pow2 46)) 
(+
             (* (andbit (rem (div lhs (Pow2 47)) 2) (rem (div rhs (Pow2 47)) 2)) (Pow2 47)) 
(+
             (* (andbit (rem (div lhs (Pow2 48)) 2) (rem (div rhs (Pow2 48)) 2)) (Pow2 48)) 
(+
             (* (andbit (rem (div lhs (Pow2 49)) 2) (rem (div rhs (Pow2 49)) 2)) (Pow2 49)) 
(+
             (* (andbit (rem (div lhs (Pow2 50)) 2) (rem (div rhs (Pow2 50)) 2)) (Pow2 50)) 
(+
             (* (andbit (rem (div lhs (Pow2 51)) 2) (rem (div rhs (Pow2 51)) 2)) (Pow2 51)) 
(+
             (* (andbit (rem (div lhs (Pow2 52)) 2) (rem (div rhs (Pow2 52)) 2)) (Pow2 52)) 
(+
             (* (andbit (rem (div lhs (Pow2 53)) 2) (rem (div rhs (Pow2 53)) 2)) (Pow2 53)) 
(+
             (* (andbit (rem (div lhs (Pow2 54)) 2) (rem (div rhs (Pow2 54)) 2)) (Pow2 54)) 
(+
             (* (andbit (rem (div lhs (Pow2 55)) 2) (rem (div rhs (Pow2 55)) 2)) (Pow2 55)) 
(+
             (* (andbit (rem (div lhs (Pow2 56)) 2) (rem (div rhs (Pow2 56)) 2)) (Pow2 56)) 
(+
             (* (andbit (rem (div lhs (Pow2 57)) 2) (rem (div rhs (Pow2 57)) 2)) (Pow2 57)) 
(+
             (* (andbit (rem (div lhs (Pow2 58)) 2) (rem (div rhs (Pow2 58)) 2)) (Pow2 58)) 
(+
             (* (andbit (rem (div lhs (Pow2 59)) 2) (rem (div rhs (Pow2 59)) 2)) (Pow2 59)) 
(+
             (* (andbit (rem (div lhs (Pow2 60)) 2) (rem (div rhs (Pow2 60)) 2)) (Pow2 60)) 
(+
             (* (andbit (rem (div lhs (Pow2 61)) 2) (rem (div rhs (Pow2 61)) 2)) (Pow2 61)) 
(+
             (* (andbit (rem (div lhs (Pow2 62)) 2) (rem (div rhs (Pow2 62)) 2)) (Pow2 62)) 
(+
             (* (andbit (rem (div lhs (Pow2 63)) 2) (rem (div rhs (Pow2 63)) 2)) (Pow2 63))
))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))
(Pow2 type)
))



; compare 

(define-fun Eq ((lhs Int) (rhs Int) (type Int)) Bool
(ite (= 1 type) (= (GetLSB lhs) (GetLSB rhs))
(= lhs rhs)
))

(define-fun Eq ((lhs Bool) (rhs Bool) (type Int)) Bool
(= lhs rhs)
)

(define-fun Ne ((lhs Int) (rhs Int) (type Int)) Bool
(ite (= 1 type) (not (= (GetLSB lhs) (GetLSB rhs)))
(not(= lhs rhs))
))

(define-fun Ult ((lhs Int) (rhs Int) (type Int)) Bool
(ite (= 1 type) (< (GetLSB lhs) (GetLSB rhs))
(< lhs rhs)
))

(define-fun Ule ((lhs Int) (rhs Int) (type Int)) Bool
(ite (= 1 type) (<= (GetLSB lhs) (GetLSB rhs))
(<= lhs rhs)
))

(define-fun Ugt ((lhs Int) (rhs Int) (type Int)) Bool
(ite (= 1 type) (> (GetLSB lhs) (GetLSB rhs))
(> lhs rhs)
))

(define-fun Uge ((lhs Int) (rhs Int) (type Int)) Bool
(ite (= 1 type) (>= (GetLSB lhs) (GetLSB rhs))
(>= lhs rhs)
))

(define-fun Slt ((lhs Int) (rhs Int) (type Int)) Bool
(Ult lhs rhs type)
)

(define-fun Sle ((lhs Int) (rhs Int) (type Int)) Bool
(Ule lhs rhs type)
)

(define-fun Sgt ((lhs Int) (rhs Int) (type Int)) Bool
(Ugt lhs rhs type)
)

(define-fun Sge ((lhs Int) (rhs Int) (type Int)) Bool
(Uge lhs rhs type)
)


;  Micro

(define-fun Neg ((val Int) (type Int)) Int
(Sub 0 val type)
)

; todo
(define-fun Concat ((msb Int) (msbwidth Int) (lsb Int) (lsbwidth Int) (type Int)) Int
(+ (* msb (Pow2 lsbwidth)) lsb)
)
(define-fun Extract ((val Int) (offset Int) (type Int)) Int
(And (- (Pow2 type) 1) (div val (Pow2 offset)) type)
)

; caution type
(define-fun Select ((cond Bool) (true_val Int) (false_val Int) (type Int)) Int
(ite (= false cond) false_val true_val)
)

; caution type
(define-fun Select ((cond Int) (true_val Int) (false_val Int) (type Int)) Int
(ite (= 0 cond) false_val true_val)
)


;(define-fun DecimalToHexadecimal4 ((input Int)) String
;(ite (< input 10) (int.to.str input) 
;    (ite (= input 10) "a"
;    (ite (= input 11) "b"
;    (ite (= input 12) "c"
;    (ite (= input 13) "d"
;    (ite (= input 14) "e"
;    (ite (= input 15) "f"
;    "0"))))))))
;(define-fun DecimalToHexadecimal ((input Int)) String
;(str.++ (DecimalToHexadecimal4 (rem (div input 16) 16)) (DecimalToHexadecimal4 (rem input 16)))
;)
;(declare-const a Int)
;(declare-const b Int)
;(declare-const arg_3 String)
;(assert (< a 256))
;(assert (>= a 0))
;(assert (= (DecimalToHexadecimal a) "fk"))
;(assert (= (str.at arg_3 0) (DecimalToHexadecimal 58)))
;(assert (= arg_3 (str.++ "\x" (DecimalToHexadecimal 58))))
;(assert (= arg_3 "\xab"))
;(assert (= b (str.len arg_3)))
;(check-sat)
;(get-model)
;below are testing!!!
;(declare-fun x () Int)
;(declare-fun y () Int)
;(assert (>= x 0)) ; something important!!!
;(assert (=
;x
;(Extract 4 2 8) 
;(Concat 1 8 1 8 16)
;(Select (= x y) 1 1 10)
;))
;(check-sat)
;(get-model)
