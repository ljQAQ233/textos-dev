[bits 64]

global intr_get
intr_get:
  pushf
  pop rax
  shr rax, 9
  and rax, 1
  ret

