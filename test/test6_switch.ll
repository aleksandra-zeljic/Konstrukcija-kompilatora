define i32 @sa_switchem(i32 %n, i32 %x) {
entry:
  switch i32 %x, label %default [
    i32 1, label %blokA
    i32 2, label %blokA
    i32 3, label %blokB
  ]

blokA:
  %a = add i32 %x, 10
  br label %exit

blokB:
  %b = mul i32 %x, 3
  br label %exit

default:
  %d = sub i32 %x, 1
  br label %exit

exit:
  ret i32 %n
}