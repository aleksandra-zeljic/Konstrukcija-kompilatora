; test2_lanac.ll
; Lanac mrtvih instrukcija - rezultat se nigde ne koristi, sve tri se uklanjaju.

define void @lanac(i32 %x) {
entry:
  %t1 = add i32 %x, 1
  %t2 = mul i32 %t1, 2
  %t3 = sub i32 %t2, 3
  ret void
}
