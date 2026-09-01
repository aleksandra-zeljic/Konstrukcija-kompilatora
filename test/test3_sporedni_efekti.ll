; test3_sporedni_efekti.ll
; store i call imaju sporedne efekte -> ostaju. %mrtva se uklanja.

declare void @spoljna()

define void @sa_efektima(ptr %p) {
entry:
  %mrtva = add i32 1, 2
  store i32 42, ptr %p
  call void @spoljna()
  ret void
}
