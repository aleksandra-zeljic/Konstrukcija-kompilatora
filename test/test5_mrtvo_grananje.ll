; test5_mrtvo_grananje.ll
; Grane racunaju %a/%b, spojene u %res koji se NE koristi (ret vraca %n).
; ADCE uklanja %a, %b, %res. Grananje se pojednostavljuje (uz simplifycfg
; nestaje potpuno).

define i32 @mrtvo_grananje(i32 %n, i32 %x) {
entry:
  %cond = icmp sgt i32 %x, 0
  br i1 %cond, label %then, label %else
then:
  %a = add i32 %x, 10
  br label %merge
else:
  %b = mul i32 %x, 3
  br label %merge
merge:
  %res = phi i32 [ %a, %then ], [ %b, %else ]
  ret i32 %n
}
