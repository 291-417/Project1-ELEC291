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



fsm_update:

mov a, state

state0:
    cjne a, #0, state1 ; check if in state 0
    Set_PWM_Percentage(#0) ; power set to zero
    jb STARTSTOP, state0_done
    Wait_Milli_Seconds(#50)
    jb STARTSTOP, state0_done
    mov a, #0
    mov state_time+0, #0x0
    mov state, #1
state0_done:
    ret

state1:
    cjne a, #1, state2
    Set_PWM_Percentage(#100) ; power set to 100%
    mov a, temp
    clr c
    subb a, #50 ; if less than 50 carry is set
    jnc state1_done
    mov a, state_time
    clr c
    subb a, #60 ; if greater than 60, carry is NOT set
    ;if carry state change 0

state1_done:
    ret




end