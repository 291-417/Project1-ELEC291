; Settable parameters
; state0: Soak Temp
; state1: Soak Time
; state2: Reflow Temp
; state3: Reflow Time

param6message: db 'LAN', 0

state_change0:
    mov param_state, #0
    ljmp pstate0

state_change1:
    mov param_state, #1
    ljmp pstate0

state_change2:
    mov param_state, #2
    ljmp pstate0

state_change3:
    mov param_state, #3
    ljmp pstate0

lang_change:
    mov a, language
    clr c
    subb a, #3      ;if less than 3, carry is set
    jnc reset_lan
    mov a, language
    add a, #0x01
    mov language, a
    
    Wait_Milli_Seconds(#255)
    Wait_Milli_Seconds(#255)
    ljmp pstate0

reset_lan:
    Wait_Milli_Seconds(#255)
    Wait_Milli_Seconds(#255)
    mov language, #0x00
    ljmp pstate0

settings:                   ;MAIN CODE START
lcall langfsm
mov a, param_state 
Set_Cursor(1,14)
Send_Constant_String(#param6message)



lcall ADC_to_PB
jnb PB0, state_change0
jnb PB1, state_change1
jnb PB2, state_change2
jnb PB3, state_change3
jnb PB6, lang_change



pstate0:
    mov a, param_state
    cjne a, #0, pstate1 ; check if in state 0
    lcall update_lcd_param
    mov a, soak_temp
    mov b, soak_temp
    lcall check_increment
    mov soak_temp, a

pstate0_done:
    ret

pstate1:
    cjne a, #1, pstate2 ; check if in state 0
    lcall update_lcd_param
    mov a, soak_time
    mov b, soak_time
    lcall check_increment
    mov soak_time, a

pstate1_done:
    ret

pstate2:
    cjne a, #2, pstate3 ; check if in state 0
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
    jnb PB4, decrement
    jnb PB5, increment
    ret

increment:
    clr c
    subb a, #255 ; if less than 255 carry is set
    mov a, b
    jc check_button4_done
    mov a, #0
    ;cjne a, #255, check_button4_done
    ret

decrement:
    clr c
    ;subb a, #255 ; if less than 255 carry is set
    ;mov a, b
    ljmp check_button5_done
    mov a, #255

    ;cjne a, #0, check_button5_done
    ret



check_button4_done:
    add a, #0x01
	;da a
    Wait_Milli_Seconds(#155)
    ret

check_button5_done:
    add a, #0xFF   ;if PB5 is pressed, increment a
    ;da a
    Wait_Milli_Seconds(#155)
    ret
;-----------------------------------------------------------------------

param0message: db 'Soak Temp   ', 0
param1message: db 'Soak Time   ', 0
param2message: db 'Reflow Temp', 0
param3message: db 'Reflow Time', 0
param4message: db 'C ', 0
param5message: db 's ', 0
spacebar: db ' ', 0

param0messageNor: db 'Bløt Temp     ', 0
param1messageNor: db 'Bløt Tid      ', 0
param2messageNor: db 'Gjenflyt Temp', 0
param3messageNor: db 'Gjenflyt Tid ', 0
param4messageNor: db 'C ', 0
param5messageNor: db 's ', 0

check_state0_param_nor_jump:
    ljmp check_state0_param_nor
;-----------------------------------------------------------------------
update_lcd_param:

    Set_Cursor(2,9)
    Display_char(#01111100b)
    
    mov a, language
    cjne a, #0, check_state0_param_nor_jump

    mov a, language
    cjne a, #3, check_state0_param_nor_jump

    mov a, param_state
    cjne a, #0, check_state1_param

    Set_Cursor(1,1)
    Send_Constant_String(#param0message)
    Set_Cursor(2,4)
    Display_char(#11011111b)
    Set_Cursor(2,5)
    Send_Constant_String(#param4message)

    Set_Cursor(2,1)
    mov x+0, soak_temp
    mov x+1, #0
    mov x+2, #0
    mov x+3, #0

    lcall hex2bcd
    lcall LCD_3BCD
    
    ret

 check_state1_param:  
    cjne a, #1, check_state2_param
    Set_Cursor(1,1)
    Send_Constant_String(#param1message)
    Set_Cursor(2,4)
    Send_Constant_String(#param5message)
    Set_Cursor(2,1)
    mov x+0, soak_time
    mov x+1, #0
    mov x+2, #0
    mov x+3, #0

    lcall hex2bcd
    lcall LCD_3BCD
    ret  

check_state2_param:
    cjne a, #2, check_state3_param
    Set_Cursor(1,1)
    Send_Constant_String(#param2message)
    Set_Cursor(2,4)
    Display_char(#11011111b)
    Set_Cursor(2,5)
    ;lcall ?WriteCommand
    Send_Constant_String(#param4message)
    Set_Cursor(2,1)
    mov x+0, reflow_temp
    mov x+1, #0
    mov x+2, #0
    mov x+3, #0
    lcall hex2bcd
    lcall LCD_3BCD
    ret    

check_state3_param:
    Set_Cursor(1,1)
    Send_Constant_String(#param3message)
    Set_Cursor(2,4)
    ;lcall ?WriteCommand
    Send_Constant_String(#param5message)
    Set_Cursor(2,1)
    mov x+0, reflow_time
    mov x+1, #0
    mov x+2, #0
    mov x+3, #0
    
    lcall hex2bcd
    lcall LCD_3BCD
    ret 
;------------------------------------------ NOR -----------------------------------------
check_state0_param_nor:
    mov a, param_state
    cjne a, #0, check_state1_param_nor

    Set_Cursor(1,1)
    Send_Constant_String(#param0messageNor)
    Set_Cursor(2,4)
    Display_char(#11011111b)
    Set_Cursor(2,5)
    Send_Constant_String(#param4messageNor)

    Set_Cursor(2,1)
    mov x+0, soak_temp
    mov x+1, #0
    mov x+2, #0
    mov x+3, #0

    lcall hex2bcd
    lcall LCD_3BCD
    ret
    

check_state1_param_nor:  
    mov a, language
    cjne a, #1, check_state2_param_nor
    Set_Cursor(1,1)
    Send_Constant_String(#param1messageNor)
    Set_Cursor(2,4)
    Send_Constant_String(#param5messageNor)
    Set_Cursor(2,1)
    mov x+0, soak_time
    mov x+1, #0
    mov x+2, #0
    mov x+3, #0

    lcall hex2bcd
    lcall LCD_3BCD
    ret  

check_state2_param_nor:
    cjne a, #2, check_state3_param_nor
    Set_Cursor(1,1)
    Send_Constant_String(#param2messageNor)
    Set_Cursor(2,4)
    Display_char(#11011111b)
    Set_Cursor(2,5)
    ;lcall ?WriteCommand
    Send_Constant_String(#param4messageNor)
    Set_Cursor(2,1)
    mov x+0, reflow_temp
    mov x+1, #0
    mov x+2, #0
    mov x+3, #0
    lcall hex2bcd
    lcall LCD_3BCD
    ret    

check_state3_param_nor:
    Set_Cursor(1,1)
    Send_Constant_String(#param3messageNor)
    Set_Cursor(2,4)
    ;lcall ?WriteCommand
    Send_Constant_String(#param5messageNor)
    Set_Cursor(2,1)
    mov x+0, reflow_time
    mov x+1, #0
    mov x+2, #0
    mov x+3, #0
    
    lcall hex2bcd
    lcall LCD_3BCD
    ret 

;------------------------------------------------------------------------------
language1: db ' Eng', 0
language2: db ' Nor', 0
language3: db 'p00n', 0

langfsm:
    mov a, language
    cjne a, #0, lang2 ; check if in state 0
    Set_Cursor(2,13)
    Send_Constant_String(#language1)
    ret

lang2:
    cjne a, #1, lang3 ; check if in state 1
    Set_Cursor(2,13)
    Send_Constant_String(#language2)
    ret

lang3:                 ;then state 2
    Set_Cursor(2,13)
    Send_Constant_String(#language3)
    ret




end