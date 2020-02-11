; Settable parameters
; state0: Soak Temp
; state1: Soak Time
; state2: Reflow Temp
; state3: Reflow Time

state_change0:
    mov param_state, #0x0
    ljmp pstate0

state_change1:
    mov param_state, #0x1
    ljmp pstate0

state_change2:
    mov param_state, #0x2
    ljmp pstate0

state_change3:
    mov param_state, #0x3
    ljmp pstate0



settings:                   ;MAIN CODE START
clr a
mov a, param_state
Set_Cursor(2,14)
Display_BCD(param_state) 

lcall ADC_to_PB
jnb PB0, state_change0
jnb PB1, state_change1
jnb PB2, state_change2
jnb PB3, state_change3




pstate0:
    mov a, param_state
    cjne a, #0x0, pstate1 ; check if in state 0
    lcall update_lcd_param
    mov a, soak_temp
    mov b, soak_temp
    lcall check_increment
    mov soak_temp, a

pstate0_done:
    ret

pstate1:
    cjne a, #0x1, pstate2 ; check if in state 0
    lcall update_lcd_param
    mov a, soak_time
    mov b, soak_time
    lcall check_increment
    mov soak_time, a

pstate1_done:
    ret

pstate2:
    cjne a, #0x2, pstate3 ; check if in state 0
    lcall update_lcd_param
    mov a, reflow_temp
    mov b, reflow_temp
    lcall check_increment
    mov reflow_temp, a

pstate2_done:
    ret

pstate3:
    lcall update_lcd_param
    mov a, reflow_time
    mov b, reflow_time
    lcall check_increment
    mov reflow_time, a

pstate3_done:
    ret

;----------------------------------------------------------------


check_increment:
    jnb PB4, increment
    jnb PB5, decrement
    jnb PB6, default
    ret

increment:
    clr c
    subb a, #0xFF ; if less than 255 carry is set
    jc check_button4_done

    ;cjne a, #255, check_button4_done
    mov a, #0x0
    ret

decrement:

    clr c
    addc a, #0x99 ; if less than 0 carry is set
    da a
    mov a, b
    jnc check_button5_done

    ;cjne a, #0, check_button5_done
    mov a, #0xFF
    da a
    ret

default:
    mov soak_temp, #0x96 ;150
    mov soak_time, #0x3C ;60
    mov reflow_temp, #0xDC ;220
    mov reflow_time, #0x3C ;60
    Wait_Milli_Seconds(#255)
    ret

check_button4_done:
    add a, #0x1     ;if PB4 is pressed, increment a
    da a
    Wait_Milli_Seconds(#255)
    ret

check_button5_done:
    add a, #0x99     ;if PB5 is pressed, increment a
    da a
    Wait_Milli_Seconds(#255)
    ret


param0message: db 'Soak Temp   ', 0
param1message: db 'Soak Time   ', 0
param2message: db 'Reflow Temp ', 0
param3message: db 'Reflow Time ', 0
param4message: db 'C ', 0
param5message: db ' s ', 0

update_lcd_param:
    ;Set_Cursor(1,1)
    ;Send_Constant_String(#clear)
    mov a, param_state
    cjne a, #0, check_state1_param
    Set_Cursor(1,1)
    Send_Constant_String(#param0message)
    Set_Cursor(1,14)
    Display_char(#11011111b)
    ;lcall ?WriteCommand
    Set_Cursor(1,15)
    Send_Constant_String(#param4message)
    Set_Cursor(2,1)
    Display_BCD(soak_temp)
    ret
 check_state1_param:  
    cjne a, #1, check_state2_param
    Set_Cursor(1,1)
    Send_Constant_String(#param1message)
    Set_Cursor(1,14)
    ;lcall ?WriteCommand
    Send_Constant_String(#param5message)
    Set_Cursor(2,1)
    Display_BCD(soak_time)  
    ret    
check_state2_param:
    cjne a, #2, check_state3_param
    Set_Cursor(1,1)
    Send_Constant_String(#param2message)
    Set_Cursor(1,14)
    Display_char(#11011111b)
    Set_Cursor(1,15)
    ;lcall ?WriteCommand
    Send_Constant_String(#param4message)
    Set_Cursor(2,1)
    Display_BCD(reflow_temp)  
    ret    
check_state3_param:
    Set_Cursor(1,1)
    Send_Constant_String(#param3message)
    Set_Cursor(1,14)
    ;lcall ?WriteCommand
    Send_Constant_String(#param5message)
    Set_Cursor(2,1)
    Display_BCD(reflow_time)  
    ret 

end