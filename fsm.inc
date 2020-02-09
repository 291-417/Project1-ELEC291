; fsm with following states
; state0: waiting state for start signal
; state1: ramp to soak, keep power high, reset timer to 0 seconds
; state2: preheat, bring back power to 20% for 60 seconds
; state3: ramp to peak, keep power at 100% until temp == 220
; state4: reflow, bring power down to 20% for 45 seconds
; state5: cooling, power down to 0% until temperature down to 60 degrees
; abort conditions:
    ; stop button presses
    ; time > 50s after start and temp < 50 degrees


mov a, state


