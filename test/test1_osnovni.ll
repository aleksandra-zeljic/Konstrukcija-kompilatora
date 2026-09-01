; test1_osnovni.ll
; Osnovni primer: %mrtva se nigde ne koristi -> ADCE je uklanja.
; %ziva se koristi u ret -> ostaje.
;
; Pokretanje (out-of-tree, apt LLVM):
;   opt -load ./build/libOurAggressiveDCE.so -enable-new-pm=0 \
;       -aggressive-dead-code-elimination -S test/test1_osnovni.ll

define i32 @osnovni(i32 %a, i32 %b) {
entry:
  %mrtva = add i32 %a, 100
  %ziva  = add i32 %a, %b
  ret i32 %ziva
}
