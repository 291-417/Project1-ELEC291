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
    mov overall_time+0, #0x0
    mov overall_time+1, #0x0
    mov state_time, #0x0
    ;Set_PWM_Percentage(#0) ; power set to zero
    clr PWM_PIN
    clr TR0 ; power set to zero
    jnb STARTSTOP, state0_done
    Wait_Milli_Seconds(#50)
    jnb STARTSTOP, state0_done
    jb STARTSTOP, $
    mov state, #1
    mov state_time, #0x0
state0_done:
    ret

state1:
    cjne a, #1, state2
    clr TR0
    setb PWM_PIN ; power set to 100%
    lcall divsadtimes ; grabs temp into temp_truncated
    mov a, temp_truncated
    clr c
    subb a, #50 ; if less than 50 carry is set
    jc check_time_state1
continue_state1:
    lcall divsadtimes
    mov a, temp_truncated
    clr c
    subb a, soak_temp ; if less that soak_temp carry is set
    jc state1_done
    ;mov dptr, #title
	;lcall SendString
    mov state_time, #0
    mov state, #2
state1_done:
    ret
check_time_state1:
    mov a, state_time
    clr c
    subb a, #60 ; if greater than 60 carry is not set
    jc continue_state1 
    mov state, #0
    sjmp state1_done

state2:
    cjne a, #2, state3

    Set_Cursor(1,10)            ;display soak time debugging 
    
    mov x+0, soak_time
    mov x+1, #0
    mov x+2, #0
    mov x+3, #0

    lcall hex2bcd
    lcall LCD_3BCD

    mov pwm_percentage, #20
    setb TR0
    mov a, state_time+0
    clr c
    subb a, soak_time ; if less than soak_time carry is set
    jc state2_done
    mov state, #3
    mov state_time, #0
state2_done:    
    ret

state3:
    cjne a, #3, state4
    clr TR0
    setb PWM_PIN
    lcall divsadtimes
    mov a, temp_truncated
    clr c
    subb a, reflow_temp ; if less than reflow_temp carry is set
    jc state3_done
    mov state_time, #0
    mov state, #4
state3_done:
    ret
;                           REFLOW state
state4:
    cjne a, #4, state5
    mov pwm_percentage, #20
    setb TR0
    mov a, state_time
    clr c
    subb a, reflow_time ; if less than 45 carry is set
    jc state4_done
    mov state_time, #0
    mov state, #5
state4_done:
    ret

state5:
    cjne a, #5, abort_error
    clr TR0
    clr PWM_PIN
    lcall divsadtimes
    mov a, temp_truncated
    clr c
    subb a, #60 ; if less than 60 carry is set
    jnc state5_done
    mov state_time, #0
    mov state, #0
state5_done:
    ret

abort_error:
    Set_Cursor(1,1)                         ;debugging 
    Send_Constant_String(#aborterrormessage)
    Set_Cursor(2,1)
    Send_Constant_String(#aborterrormessage2)
    mov state, #0
    ljmp abort_error

aborterrormessage: db 'Abort error', 0
aborterrormessage2: db 'Returning to Wait', 0


MOVATT MAC
    mov a, totaltemp+%0
    mov x+%0, a
ENDMAC

divsadtimes:
    ;MOVATT(0)
    ;MOVATT(1)
    ;MOVATT(2)
    ;MOVATT(3)
    ;load_Y(100)
    ;lcall div32
    ;mov a, x+0
    ;mov temp_truncated, a
    ;lcall hex2bcd
    ;mov temp_truncated, bcd+1
    push ar0
    push acc
    
    mov a, temp_bcd+0
    anl a, #0x0f
    mov r0, a
    mov a, temp_bcd+0
    anl a, #0xf0
    swap a
    mov b, #10
    mul ab
    add a, r0
    mov r0, a
    mov a, temp_bcd+1
    anl a, #0x0f
    mov b, #100
    mul ab
    add a, r0
    mov temp_truncated, a
    pop acc
    pop ar0
    ret


runningTimeMessage: db 'OT:', 0
stateTimeMessage: db 'ST:', 0
state0message: db 'Waiting...', 0
state1message: db 'Ramp Soak', 0
state2message: db 'Preheat', 0
state3message: db 'Peak Ramp', 0
state4message: db 'Reflow', 0
state5message: db 'Cooling', 0
clear:  db '                                   ', 0
clear3:  db '   ', 0

update_lcd:
    Set_Cursor(2,7)
    Send_Constant_String(#clear3)

    Set_Cursor(2,1)
    Send_Constant_String(#runningTimeMessage)
    Set_Cursor(2,4)
    mov x+0, overall_time+0
    mov x+1, overall_time+1
    mov x+2, #0
    mov x+3, #0
    lcall hex2bcd
    lcall LCD_3BCD

    Set_Cursor(2,10)
    Send_Constant_String(#stateTimeMessage)
    Set_Cursor(2,14)
    mov x+0, state_time+0
    mov x+1, state_time+1
    mov x+2, #0
    mov x+3, #0
    lcall hex2bcd
    lcall LCD_3BCD

    Set_Cursor(1,14)
    mov x+0, temp_bcd+0
    mov x+1, temp_bcd+1
    mov x+2, #0
    mov x+3, #0
    lcall hex2bcd
    lcall LCD_3BCD

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