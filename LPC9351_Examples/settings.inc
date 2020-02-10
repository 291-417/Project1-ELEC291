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
    mov state, #0           ;if PB0 is pressed, change to state 0
    
check_button1:
    jnb PB0, check_button1
    Wait_Milli_Seconds(#50)
    jnb PB0, check_button1
    jb PB0, $
    mov state, #0           ;if PB0 is pressed, change to state 0
    
check_button2:
    jnb PB0, check_button1
    Wait_Milli_Seconds(#50)
    jnb PB0, check_button1
    jb PB0, $
    mov state, #0           ;if PB0 is pressed, change to state 0
    
check_button3:
    jnb PB0, check_button1
    Wait_Milli_Seconds(#50)
    jnb PB0, check_button1
    jb PB0, $
    mov state, #0           ;if PB0 is pressed, change to state 0






pstate0:
    cjne a, #0, param_state1 ; check if in state 0
    lcall update_lcd_param










state0message: db 'Waiting...', 0
state1message: db 'Ramp Soak ', 0
state2message: db 'Preheat', 0
state3message: db 'Peak Ramp', 0
state4message: db 'Reflow', 0
state5message: db 'Cooling', 0
clear:  db '               ', 0

update_lcd_param:
    Set_Cursor(1,1)
    Send_Constant_String(#clear)
    mov a, state
    cjne a, #0, check_state1_param
    Set_Cursor(1,1)
    Send_Constant_String(#param0message)
    ret
 check_state1_param:  
    cjne a, #1, check_state2_param
    Set_Cursor(1,1)
    Send_Constant_String(#param1message)  
    ret    
check_state2_param:
    cjne a, #2, check_state3_param
    Set_Cursor(1,1)
    Send_Constant_String(#param2message)  
    ret    
check_state3_param:
    Set_Cursor(1,1)
    Send_Constant_String(#param3message)
    ret 
end