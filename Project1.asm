; Project 1: Reflow oven controller
; ELEC 291 UBC
; Oakley Bach-Raabe, Jashan Brar, Trevor MacKay, Ryan Meshulam, Anusika Nijher
; Includes modified code from Jesus Calvino-Fraga
; GPL v3
$NOLIST
$MOD9351
$LIST

CLK         EQU 14746000  ; Microcontroller system clock frequency in Hz
CCU_RATE    EQU 22050     ; 22050Hz is the sampling rate of the wav file we are playing
CCU_RELOAD  EQU ((65536-((CLK/(2*CCU_RATE)))))
BAUD        EQU 115200
BRVAL       EQU ((CLK/BAUD)-16)
XTAL        EQU 7373000

LCD_RS equ P0.5
LCD_RW equ P0.6
LCD_E  equ P0.7
LCD_D4 equ P1.2
LCD_D5 equ P1.3
LCD_D6 equ P1.4
LCD_D7 equ P1.6
DIP_BUTTON1 equ P3.0
PLAY_BUTTON equ P0.3
SOUND equ P2.7
PWM_PIN equ P0.2
STARTSTOP equ P2.1

FLASH_CE    EQU P2.4

dseg at 30H
	w:   ds 3 ; 24-bit play counter.  Decremented in CCU ISR.
  x:   ds 4
  y:   ds 4
  bcd: ds 5
  sound_start: ds 3
  pwm_percentage: ds 1
  state: ds 1
  overall_time: ds 2
  state_time: ds 1
  lmtemp: ds 4
  thermotemp: ds 4
  totaltemp: ds 4
  pwm_counter: ds 1
  Count1ms: ds 2
  talker_counter: ds 1
  temp_truncated: ds 1
  temp_bcd: ds 2
bseg

mf: dbit 1
PB0: dbit 1 ; Variable to store the state of pushbutton 0 after calling ADC_to_PB below
PB1: dbit 1 ; Variable to store the state of pushbutton 1 after calling ADC_to_PB below
PB2: dbit 1 ; Variable to store the state of pushbutton 2 after calling ADC_to_PB below
PB3: dbit 1 ; Variable to store the state of pushbutton 3 after calling ADC_to_PB below
PB4: dbit 1 ; Variable to store the state of pushbutton 4 after calling ADC_to_PB below
PB5: dbit 1 ; Variable to store the state of pushbutton 5 after calling ADC_to_PB below
PB6: dbit 1 ; Variable to store the state of pushbutton 6 after calling ADC_to_PB below

second_flag: dbit 1 ; a second has passed, must update state_time and overall_time
say_a_number: dbit 1 ; Say the number in BCD if this is high

cseg

org 0x0000 ; Reset vector
    ljmp MainProgram

org 0x0003 ; External interrupt 0 vector (not used in this code)
	reti

org 0x000B ; Timer/Counter 0 overflow interrupt vector (not used in this code)
	ljmp Timer0_ISR

org 0x0013 ; External interrupt 1 vector (not used in this code)
	reti

org 0x001B ; Timer/Counter 1 overflow interrupt vector
	ljmp Timer1_ISR

org 0x0023 ; Serial port receive/transmit interrupt vector (not used in this code)
	reti

org 0x005b ; CCU interrupt vector.  Used in this code to replay the wave file.
	ljmp CCU_ISR



$NOLIST
$INCLUDE(math32.inc)
$include(LCD_4bit_LPC9351.inc) ; A library of LCD related functions and utility macros
$include(sound.inc)
$include(temppb.inc)
$include(pwm.inc)
$include(fsm.inc)
$include(secinc.inc)
$LIST

;---------------------------------;
; Initial configuration of ports. ;
; After reset the default for the ;
; pins is 'Open Drain'.  This     ;
; routine changes them pins to    ;
; Quasi-bidirectional like in the ;
; original 8051.                  ;
; Notice that P1.2 and P1.3 are   ;
; always 'Open Drain'. If those   ;
; pins are to be used as output   ;
; they need a pull-up resistor.   ;
;---------------------------------;
Ports_Init:
    ; Configure all the ports in bidirectional mode:
    mov P0M1, #00H
    mov P0M2, #00H
    mov P1M1, #00H
    mov P1M2, #00H ; WARNING: P1.2 and P1.3 need 1 kohm pull-up resistors if used as outputs!
    mov P2M1, #00H
    mov P2M2, #00H
    mov P3M1, #00H
    mov P3M2, #00H
	ret

; Display a 3-digit BCD number in the LCD
LCD_3BCD:
	mov a, bcd+1
	anl a, #0x0f
	orl a, #'0'
	lcall ?WriteData
	mov a, bcd+0
	swap a
	anl a, #0x0f
	orl a, #'0'
	lcall ?WriteData
	mov a, bcd+0
	anl a, #0x0f
	orl a, #'0'
	lcall ?WriteData
	ret

Sound_Start_Init:
  mov sound_start+0, #0xff
  mov sound_start+1, #0x00
  mov sound_start+2, #0x00
  ret

MainProgram:
  mov SP, #0x7F
  
  mov state, #0
  lcall Sound_Start_Init
  lcall Ports_Init ; Default all pins as bidirectional I/O. See Table 42.
  lcall Double_Clk
  lcall InitDAC ; Call after 'Ports_Init
	
  lcall CCU_Init
  lcall Init_SPI

  lcall Timer0_Init
  lcall Timer1_Init
  
  lcall InitSerialPort
  lcall InitADC0
  lcall LCD_4BIT

	mov state_time, #0
	mov overall_time+0, #0
	mov overall_time+1, #0
  mov temp_truncated, #0
  ; Set beginning message on LCD
  Set_Cursor(1, 1)
  Send_Constant_String(#Title)

  lcall Wait1S ; Wait a bit so PUTTy has a chance to start

  mov dptr, #InitialMessage
	lcall SendString

  clr SOUND ; Disable speaker
  clr TMOD20 ; Stop CCU timer
	setb EA ; Enable global interrupts.

forever:
  mov dptr, #Temp
	lcall SendString
  
  mov dptr, #Thermocouple
	lcall SendString
  
  mov dptr, #Total
  lcall SendString
  lcall get_total_temp
  ;lcall ADC_to_PB
  lcall fsm_update
  lcall update_lcd
	;lcall Wait1S
	;jb DIP_BUTTON1, next
	;Wait_Milli_Seconds(#50)	
	;jb DIP_BUTTON1, next
	;lcall Display_PushButtons_ADC
;next_check:
  jb second_flag, Every_Second_Stuff
  ;jb PLAY_BUTTON, forever
  ;Wait_Milli_Seconds(#50)
  ;jb PLAY_BUTTON, forever
  ;jnb PLAY_BUTTON, $
  jnb say_a_number, Check_Temperatures
  ljmp forever
Check_Temperatures:
  lcall Get_Thermocouple
  lcall Read_Temperature
  ljmp forever


;next:
	;Set_Cursor(2, 1)
    ;Send_Constant_String(#blank)
    ;ljmp next_check
Every_Second_Stuff:
  ;mov bcd+1, #0x02
  ;mov bcd+0, #0x53
  clr second_flag
  lcall BCD_To_Sound
  lcall Say_Stuff_FSM
  inc state_time
  inc overall_time+0
  mov a, overall_time+0
  jnz End_Every_Second
increment_upper_seconds:
  inc overall_time+1
End_Every_Second:
  ljmp forever

Play_Sounds:
  ;mov a, #2
  ;lcall Play_Numbered
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  ;Set_PWM_Percentage(#80)

  ;mov a, #28
  ;lcall Play_Numbered
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Set_PWM_Percentage(#100)
  
  ;mov a, #11
  ;lcall Play_Numbered
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  ;Set_PWM_Percentage(#0)

  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  Wait_Milli_Seconds(#255)
  ;Set_PWM_Percentage(#DEFAULT_PWM)
  ret
end