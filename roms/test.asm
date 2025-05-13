CLS               ; 0x0200 Clear screen
LD $3, 4
LD I, $3
CALL Draw
JP GetInput

GetInput:
  LD $2, K        ; 0x0202 Wait for keyboard input, set input value to v2
  LD I, $2        ; 0x0204 Set index to value at v2

Draw:
  LD $0, 0A       ; 0x0206 v0 = 10 
  LD $1, 0A       ; 0x0208 v1 = 10
  CLS             ; 0x020A Clear screen
  DRW $0, $1, 5   ; 0x020C Draw I to (v0, v1), 5 pixels high
  RTS

JP GetInput       ; 0x020E Jump to GetInput
