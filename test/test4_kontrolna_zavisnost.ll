; test4_kontrolna_zavisnost.ll
; store u then bloku je ziv; then kontrolno zavisi od grananja u entry,
; pa grananje i uslov ostaju zivi. Nista se ne uklanja.

define void @kontrola(i32 %x, ptr %p) {
entry:
  %cond = icmp sgt i32 %x, 0
  br i1 %cond, label %then, label %end
then:
  store i32 1, ptr %p
  br label %end
end:
  ret void
}
