global intr_get
intr_get:
  pushfq
  pop eax
  shr rax, 9
  and rax, 1
  ret

