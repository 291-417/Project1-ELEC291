; Settable parameters
; state0: Soak Temp
; state1: Soak Time
; state2: Reflow Temp
; state3: Reflow Time


settings:

mov a, param_state

check_button0:
    jnb PB0, check_button1
    Wait_Milli_Seconds(#50)
    jnb PB0, check_button1
    jb PB0, $
    mov param_state, #0           ;if PB0 is pressed, change to state 0
    ljmp pstate0

check_button1:
    jnb PB1, check_button2
    Wait_Milli_Seconds(#50)
    jnb PB1, check_button2
    jb PB1, $
    mov param_state, #1           ;if PB1 is pressed, change to state 1
    ljmp pstate0
    
check_button2:
    jnb PB2, check_button3
    Wait_Milli_Seconds(#50)
    jnb PB2, check_button3
    jb PB2, $
    mov param_state, #2           ;if PB2 is pressed, change to state 2
    ljmp pstate0

check_button3:
    jnb PB3, pstate0
    Wait_Milli_Seconds(#50)
    jnb PB3, pstate0
    jb PB3, $
    mov param_state, #3      ;if PB3 is pressed, change to state 3
    ljmp pstate0





pstate0:
    cjne a, #0, pstate1 ; check if in state 0
    lcall update_lcd_param
    mov a, soak_temp
    lcall check_increment
    mov soak_temp, a

pstate0_done:
    ret

pstate1:
    cjne a, #0, pstate2 ; check if in state 0
    lcall update_lcd_param
    mov a, soak_time
    lcall check_increment
    mov soak_time, a

pstate1_done:
    ret

pstate2:
    cjne a, #0, pstate3 ; check if in state 0
    lcall update_lcd_param
    mov a, reflow_temp
    lcall check_increment
    mov reflow_temp, a

pstate2_done:
    ret

pstate3:
    lcall update_lcd_param
    mov a, reflow_time
    lcall check_increment
    mov reflow_time, a

pstate3_done:
    ret


check_increment:
    jnb PB4, check_button5
    Wait_Milli_Seconds(#50)
    jnb PB4, check_button5
    jb PB4, $
    cjne a, #255, check_button4_done
    mov a, #0
    ret

check_button4_done:
    ADD a, #0x01     ;if PB4 is pressed, increment a
    da a
    ret

check_button5:
    jnb PB5, check_button5_return
    Wait_Milli_Seconds(#50)
    jnb PB5, check_button5_return
    jb PB5, $
    cjne a, #0, check_button5_done
    mov a, #255
    ret

check_button5_done:
    sub a, #0x01     ;if PB5 is pressed, increment a
    da a
    ret

check_button5_return:
    ret


param0message: db 'Soak Temp ', 0
param1message: db 'Soak Time ', 0
param2message: db 'Reflow Temp ', 0
param3message: db 'Reflow Time ', 0
clear:  db '               ', 0

update_lcd_param:
    Set_Cursor(1,1)
    Send_Constant_String(#clear)
    mov a, param_state
    cjne a, #0, check_state1_param
    Set_Cursor(1,1)
    Send_Constant_String(#param0message)
    Set_Cursor(1,14)
    Display_char(#11011111b)
    lcall ?WriteCommand
    Set_Cursor(1,15)
    WriteData(#'C')
    Set_Cursor(2,1)
    Display_BCD(soak_temp)
    ret
 check_state1_param:  
    cjne a, #1, check_state2_param
    Set_Cursor(1,1)
    Send_Constant_String(#param1message)
    Set_Cursor(1,15)
    lcall ?WriteCommand
    WriteData(#'s')
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
    lcall ?WriteCommand
    WriteData(#'C')
    Set_Cursor(2,1)
    Display_BCD(reflow_temp)  
    ret    
check_state3_param:
    Set_Cursor(1,1)
    Send_Constant_String(#param3message)
    Set_Cursor(1,15)
    lcall ?WriteCommand
    WriteData(#'s')
    Set_Cursor(2,1)
    Display_BCD(reflow_time)  
    ret 
end