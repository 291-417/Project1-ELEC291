;EXTRN code (Load_X, Load_y)
Wait10us:
    mov R0, #18
    djnz R0, $ ; 2 machine cycles-> 2*0.27126us*18=10us
	ret

putchar:
	jbc	TI,putchar_L1
	sjmp putchar
putchar_L1:
	mov	SBUF,a
	ret

getchar:
	jbc	RI,getchar_L1
	sjmp getchar
getchar_L1:
	mov	a,SBUF
	ret

Wait1S:
	mov R2, #40
M3:	mov R1, #250
M2:	mov R0, #184
M1:	djnz R0, M1 ; 2 machine cycles-> 2*0.27126us*184=100us
	djnz R1, M2 ; 100us*250=0.025s
	djnz R2, M3 ; 0.025s*40=1s
	ret

InitSerialPort:
	mov	BRGCON,#0x00
	mov	BRGR1,#high(BRVAL)
	mov	BRGR0,#low(BRVAL)
	mov	BRGCON,#0x03 ; Turn-on the baud rate generator
	mov	SCON,#0x52 ; Serial port in mode 1, ren, txrdy, rxempty
	mov	P1M1,#0x00 ; Enable pins RxD and TXD
	mov	P1M2,#0x00 ; Enable pins RxD and TXD
	ret

InitADC0:
	; ADC0_0 is connected to P1.7
	; ADC0_1 is connected to P0.0
    ; Configure pins P1.7 and P0.0  as inputs
    orl P0M1, #00000001b
    anl P0M2, #11111110b
    orl P1M1, #10000000b
    anl P1M2, #01111111b
    orl P2M1, #00000011b
    anl P2M2, #11111100b
	; Setup ADC0
	setb BURST0 ; Autoscan continuos conversion mode
	orl	ADMODB,#0x20 ;ADC0 clock is 7.3728MHz/2
	mov	ADINS,#0x0B ; Select four channels of ADC0 for conversion
	mov	ADCON0,#0x05 ; Enable the converter and start immediately
	; Wait for first conversion to complete
InitADC0_L1:
	mov	a,ADCON0
	jnb	acc.3,InitADC0_L1
	ret

HexAscii: db '0123456789ABCDEF'
SendTemp:
	mov dptr, #HexAscii 
	
	mov a, bcd+2
	swap a
	anl a, #0xf
	movc a, @a+dptr
	lcall putchar
	mov a, bcd+2
	anl a, #0xf
	movc a, @a+dptr
	lcall putchar

	mov a, bcd+1
	swap a
	anl a, #0xf
	movc a, @a+dptr
	lcall putchar
	mov a, bcd+1
	anl a, #0xf
	movc a, @a+dptr
	lcall putchar

	mov a, #'.'
	lcall putchar

	mov a, bcd+0
	swap a
	anl a, #0xf
	movc a, @a+dptr
	lcall putchar
	mov a, bcd+0
	anl a, #0xf
	movc a, @a+dptr
	lcall putchar
	
	mov a, #'\r'
	lcall putchar
	mov a, #'\n'
	lcall putchar	
	ret
	
SendHex:
	mov a, #'0'
	lcall putchar
	mov a, #'x'
	lcall putchar
	mov dptr, #HexAscii 
	mov a, b
	swap a
	anl a, #0xf
	movc a, @a+dptr
	lcall putchar
	mov a, b
	anl a, #0xf
	movc a, @a+dptr
	lcall putchar
	mov a, #' '
	lcall putchar
	ret
	
SendString:
    clr a
    movc a, @a+dptr
    jz SendString_L1
    lcall putchar
    inc dptr
    sjmp SendString  
SendString_L1:
	ret

ADC_to_PB:
	setb PB6
	setb PB5
	setb PB4
	setb PB3
	setb PB2
	setb PB1
	setb PB0
	; Check PB6
	clr c
	mov a, AD0DAT0
	subb a, #(206-10) ; 2.8V=216*(3.3/255); the -10 is to prevent false readings
	jc ADC_to_PB_L6
	clr PB6
	ret
ADC_to_PB_L6:
	; Check PB5
	clr c
	mov a, AD0DAT0
	subb a, #(185-10) ; 2.4V=185*(3.3/255); the -10 is to prevent false readings
	jc ADC_to_PB_L5
	clr PB5
	ret
ADC_to_PB_L5:
	; Check PB4
	clr c
	mov a, AD0DAT0
	subb a, #(154-10) ; 2.0V=154*(3.3/255); the -10 is to prevent false readings
	jc ADC_to_PB_L4
	clr PB4
	ret
ADC_to_PB_L4:
	; Check PB3
	clr c
	mov a, AD0DAT0
	subb a, #(123-10) ; 1.6V=123*(3.3/255); the -10 is to prevent false readings
	jc ADC_to_PB_L3
	clr PB3
	ret
ADC_to_PB_L3:
	; Check PB2
	clr c
	mov a, AD0DAT0
	subb a, #(92-10) ; 1.2V=92*(3.3/255); the -10 is to prevent false readings
	jc ADC_to_PB_L2
	clr PB2
	ret
ADC_to_PB_L2:
	; Check PB1
	clr c
	mov a, AD0DAT0
	subb a, #(61-10) ; 0.8V=61*(3.3/255); the -10 is to prevent false readings
	jc ADC_to_PB_L1
	clr PB1
	ret
ADC_to_PB_L1:
	; Check PB1
	clr c
	mov a, AD0DAT0
	subb a, #(30-10) ; 0.4V=30*(3.3/255); the -10 is to prevent false readings
	jc ADC_to_PB_L0
	clr PB0
	ret
ADC_to_PB_L0:
	; No pusbutton pressed	
	ret



Title: db 'ADC0 push buttons', 0
InitialMessage: db '\r\nADC0 push buttons.  The push buttons voltage divider is connected to P1.7\r\n', 0
Thermocouple: db 'Thermocouple difference: ', 0
Total: db 'Total Temp: ',0 
Temp: db 'Temp from LM335: ', 0
blank: db '                 ', 0
Read_Temperature:
	clr EA
  	Load_X(0)
    mov x+0, AD0DAT1
	mov R7, #255
    lcall Wait10us
accumulate_loop:
    mov y+0, AD0DAT1
    mov y+1, #0
    mov y+2, #0
    mov y+3, #0
    lcall add32
    lcall Wait10us
	djnz R7, accumulate_loop
	
	; Now divide by 16 (2^4)
	Load_y(16)
	lcall div32
	; x has now the 12-bit representation of the temperature
	
	; Convert to temperature (C)
	Load_y(33000) ; Vref is 3.3V
	lcall mul32
	Load_y(((1<<12)-1)) ; 2^12-1
	lcall div32
	Load_y(27300)
	lcall sub32
	mov lmtemp+0, x+0
	mov lmtemp+1, x+1
	mov lmtemp+2, x+2
	mov lmtemp+3, x+3
	lcall hex2bcd
	
	lcall SendTemp 
	setb EA
  ret

Display_PushButtons_ADC:
	Set_Cursor(2, 1)
	mov a, #'0'
	mov c, PB6
	addc a, #0
    lcall ?WriteData	
	mov a, #'0'
	mov c, PB5
	addc a, #0
    lcall ?WriteData	
	mov a, #'0'
	mov c, PB4
	addc a, #0
    lcall ?WriteData	
	mov a, #'0'
	mov c, PB3
	addc a, #0
    lcall ?WriteData	
	mov a, #'0'
	mov c, PB2
	addc a, #0
    lcall ?WriteData	
	mov a, #'0'
	mov c, PB1
	addc a, #0
    lcall ?WriteData	
	mov a, #'0'
	mov c, PB0
	addc a, #0
    lcall ?WriteData	
	ret

Get_Thermocouple:
	clr EA
  	Load_X(0)
    mov x+0, AD0DAT3
	mov R7, #255
    lcall Wait10us	
accumulate_loop_Thermocouple:
    mov y+0, AD0DAT3
    mov y+1, #0
    mov y+2, #0
    mov y+3, #0
    lcall add32
    lcall Wait10us
	djnz R7, accumulate_loop_Thermocouple
	; Now divide by 16 (2^4)
	Load_y(16)
	lcall div32
	; x has now the 12-bit representation of the temperature

	; Convert to temperature (C)
	Load_y(33000) ; Vref is 3.3V
	lcall mul32
	Load_y(((1<<12)-1)) ; 2^12-1
	lcall div32	

	mov thermotemp+0, x+0
	mov thermotemp+1, x+1
	mov thermotemp+2, x+2
	mov thermotemp+3, x+3	
	lcall hex2bcd
	
	lcall SendTemp 
	setb EA
  ret


get_total_temp:
	mov x+0, thermotemp+0	
	mov x+1, thermotemp+1	
	mov x+2, thermotemp+2	
	mov x+3, thermotemp+3
	mov y+0, lmtemp+0	
	mov y+1, lmtemp+1	
	mov y+2, lmtemp+2	
	mov y+3, lmtemp+3	
	lcall add32

	mov totaltemp+0, x+0
	mov totaltemp+1, x+1
	mov totaltemp+2, x+2
	mov totaltemp+3, x+3
	lcall hex2bcd
	lcall SendTemp
	ret
end