.include "xc.inc"
    
.text
.global _delay_1ms
.global _delay_200us
    
; Wait for exactly 1ms, including the call and return cycles.
_delay_1ms:
    repeat #15994
    nop
    return
    
_delay_200us:
    repeat #2994
    nop
    return