; This program shows how to read many push buttons using just one analog input.
; The idea is to make a voltage divider with many resistors and the push buttons
; connect the diferent voltages to an analog input.  In this example we have seven push
; buttons.  The diagram is in this image: push_button_adc.jpg.  The common pin of all
; the push buttons is connected to one of the analog input pins of ADC0.  Warning:
; since P2.0 and P2.1 are used with the LCD we can not use those channels with ADC0.
; The common input for all the push buttons is AD0DAT1 which is P1.7.
;

$MOD9351

XTAL EQU 7373000
BAUD EQU 115200
BRVAL EQU ((XTAL/BAUD)-16)

CSEG at 0x0000
ljmp	MainProgram

DSEG at 0x30
x:   ds 4
y:   ds 4
bcd: ds 5


BSEG
mf: dbit 1
PB0: dbit 1 ; Variable to store the state of pushbutton 0 after calling ADC_to_PB below
PB1: dbit 1 ; Variable to store the state of pushbutton 1 after calling ADC_to_PB below
PB2: dbit 1 ; Variable to store the state of pushbutton 2 after calling ADC_to_PB below
PB3: dbit 1 ; Variable to store the state of pushbutton 3 after calling ADC_to_PB below
PB4: dbit 1 ; Variable to store the state of pushbutton 4 after calling ADC_to_PB below
PB5: dbit 1 ; Variable to store the state of pushbutton 5 after calling ADC_to_PB below
PB6: dbit 1 ; Variable to store the state of pushbutton 6 after calling ADC_to_PB below

cseg
; These 'equ' must match the wiring between the microcontroller and the LCD!
LCD_RS equ P0.7
LCD_RW equ P3.0
LCD_E  equ P3.1
LCD_D4 equ P2.3
LCD_D5 equ P2.4
LCD_D6 equ P2.5
LCD_D7 equ P2.6
$NOLIST
$include(LCD_4bit_LPC9351.inc) ; A library of LCD related functions and utility macros
$INCLUDE(math32.inc)
$LIST


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
	mov	ADMODB,#0x20 ;ADC0 clock is 7.3728MHz/2
	mov	ADINS,#0x0F ; Select four channels of ADC0 for conversion
	mov	ADCON0,#0x05 ; Enable the converter and start immediately
	; Wait for first conversion to complete
InitADC0_L1:
	mov	a,ADCON0
	jnb	acc.3,InitADC0_L1
	ret

HexAscii: db '0123456789ABCDEF'
SendTemp:
	mov dptr, #HexAscii 
	
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

Title: db 'ADC0 push buttons', 0
InitialMessage: db '\r\nADC0 push buttons.  The push buttons voltage divider is connected to P1.7\r\n', 0
	
MainProgram:
    mov SP, #0x7F

    ; Configure all the ports in bidirectional mode:
    mov P0M1, #00H
    mov P0M2, #00H
    mov P1M1, #00H
    mov P1M2, #00H ; WARNING: P1.2 and P1.3 need 1kohm pull-up resistors!
    mov P2M1, #00H
    mov P2M2, #00H
    mov P3M1, #00H
    mov P3M2, #00H
	
	lcall InitSerialPort
	lcall InitADC0
	
    lcall LCD_4BIT
    ; For convenience a few handy macros are included in 'LCD_4bit_LPC9351.inc':
	Set_Cursor(1, 1)
    Send_Constant_String(#Title)

	lcall Wait1S ; Wait a bit so PUTTy has a chance to start
	mov dptr, #InitialMessage
	lcall SendString

forever_loop:
    ; Send the conversion results via the serial port to putty.
	mov a, #'\r' ; move cursor all the way to the left
    lcall putchar
    ; Display converted value from P1.7
	mov	b, AD0DAT0
	lcall SendHex
	
	mov	b, AD0DAT1
	lcall SendHex
	
	; lm355 measurement starts here
	Load_x(0)
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
	Load_Y(16)
	lcall div32
	; x has now the 12-bit representation of the temperature
	
	; Convert to temperature (C)
	Load_Y(43000) ; Vref is 4.3V
	lcall mul32
	Load_Y(((1<<12)-1)) ; 2^12-1
	lcall div32
	Load_Y(27300)
	lcall sub32
	
	lcall hex2bcd
	
	lcall SendTemp 

	; lm355 temp sent and calc finished here
	
	lcall ADC_to_PB
	lcall Display_PushButtons_ADC
	lcall Wait1S
	
	ljmp forever_loop


end
