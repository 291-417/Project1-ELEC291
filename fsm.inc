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
    jnb STARTSTOP, state0_done
    Wait_Milli_Seconds(#50)
    jnb STARTSTOP, state0_done
    jb STARTSTOP, $
    mov overall_time+0, #0x0
    mov overall_time+1, #0x0
    mov state_time, #0x0
    mov state, #1
state0_done:
    ret

state1:
    cjne a, #1, state2
    Set_PWM_Percentage(#100) ; power set to 100%
    lcall divsadtimes ; grabs temp into a
    clr c
    subb a, #50 ; if less than 50 carry is set
    jc check_time_state1
continue_state1:
    lcall divsadtimes
    clr c
    subb a, #150 ; if less that 150 carry is set
    jc state1_done
    mov state_time, #0
    mov state, #2
state1_done:
    ret
check_time_state1:
    mov a, state_time
    mov b, a
    lcall sendhex
    mov a, state_time
    clr c
    subb a, #60 ; if greater than 60 carry is not set
    jc continue_state1 
    mov state, #0
    sjmp state1_done

state2:
    cjne a, #2, state3
    Set_PWM_Percentage(#20)
    mov a, state_time
    clr c
    subb a, #59 ; if less than 60 carry is set
    jc state2_done
    mov state, #3
    mov state_time, #0
state2_done:    
    ret

state3:
    cjne a, #3, state4
    Set_PWM_Percentage(#100)
    lcall divsadtimes
    clr c
    subb a, #220 ; if less than 220 carry is set
    jc state3_done
    mov state_time, #0
    mov state, #4
state3_done:
    ret

state4:
    cjne a, #4, state5
    Set_PWM_Percentage(#20)
    mov a, state_time
    clr c
    subb a, #45 ; if less than 45 carry is set
    jc state4_done
    mov state_time, #0
    mov state, #5
state4_done:
    ret

state5:
    cjne a, #5, abort_error
    Set_PWM_Percentage(#0)
    lcall divsadtimes
    clr c
    subb a, #60 ; if less than 60 carry is set
    jc state5_done
    mov state_time, #0
    mov state, #0
state5_done:
    ret

abort_error:
    mov state, #0
    ret

divsadtimes:
    mov x+0, totaltemp+0
    mov x+1, totaltemp+1
    mov x+2, totaltemp+2
    mov x+3, totaltemp+3
    load_Y(100)
    lcall div32

    mov a, x+0
    ret



state0message: db 'Waiting...', 0
state1message: db 'Ramp Soak ', 0
state2message: db 'Preheat', 0
state3message: db 'Peak Ramp', 0
state4message: db 'Reflow', 0
state5message: db 'Cooling', 0
clear:  db '               ', 0

update_lcd:
    Set_Cursor(1,1)
    Send_Constant_String(#clear)
    mov a, state
    cjne a, #0, check_state1
    Set_Cursor(1,1)
    Send_Constant_String(#state0message)
    ret
 check_state1:  
    cjne a, #1, check_state2
    Set_Cursor(1,1)
    Send_Constant_String(#state1message)  
    ret    
check_state2:
    cjne a, #2, check_state3
    Set_Cursor(1,1)
    Send_Constant_String(#state2message)  
    ret    
check_state3:
    cjne a, #3, check_state4
    Set_Cursor(1,1)
    Send_Constant_String(#state3message)
    ret 
check_state4:
    cjne a, #4, check_state5
    Set_Cursor(1,1)
    Send_Constant_String(#state4message)   
    ret
check_state5:
    Set_Cursor(1,1)
    Send_Constant_String(#state5message)  
    ret
end