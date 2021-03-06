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
DIP_BUTTON1 equ P3.0 ; When low (switch thrown), in settings mode
PLAY_BUTTON equ P0.3 ; Currently acts as screen reset button
SOUND equ P2.7 ; Connects to the input of the sound op amp
PWM_PIN equ P0.2 ; Connects to transistor for PWM output, oven is on when low
STARTSTOP equ P2.1 ; Start button
ABORT equ P0.1 ; Abort button
FLASH_CE    EQU P2.4


dseg at 30H
	w:   ds 3 ; 24-bit play counter.  Decremented in CCU ISR.
  x:   ds 4
  y:   ds 4
  bcd: ds 5
  sound_start: ds 3 ; Directs sound playing code where to start
  pwm_percentage: ds 1 ; Set to determine PWM percentage on; range 1-99
  state: ds 1 ; Current state: options are waiting, ramp to soak, preheat, ramp to peak, reflow, and cooling
  overall_time: ds 2 ; Time since the start button was pressed, stays at zero in waiting state
  state_time: ds 2 ; Time since state transition, stays at zero in waiting state
  lmtemp: ds 4 ; Temperature at the cold junction
  thermotemp: ds 4 ; Temperature difference between sides of the thermocouple
  totaltemp: ds 4 ; Temperature at end of thermocouple
  pwm_counter: ds 1 ; Counts down number of PWM cycles, gets reloaded when it gets to zero with pwm_percentage or 100-pwm_percentage
  Count1ms: ds 2 ; Milliseconds counter for overall timing
  talker_counter: ds 1 ; Counts number of cycles through number-saying FSM, more details in sound.inc
  temp_truncated: ds 1 ; Temperature integer
  temp_bcd: ds 2 ; Temperature in BCD
  Talk_temp: ds 2 ; Temperature that is being said -- notusing temp_bcd because temperature can change in the middle of talking
  param_state: ds 1
  soak_temp: ds 1
  soak_time: ds 1
  reflow_temp: ds 1
  reflow_time: ds 1
  language: ds 1 ; Left as an excersise for the reader
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
$include(sound.inc) ; Obama talks here
$include(temppb.inc) ; Reading temperatures
$include(pwm.inc) ; PWM for oven control
$include(fsm.inc) ; Main FSM for oven control
$include(secinc.inc) ; Seconds increment
$include(settings.inc) ; Settings menu
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
  
  mov P0M1, #00H
  mov P0M2, #00H
  mov P1M1, #00H
  mov P1M2, #00H ; WARNING: P1.2 and P1.3 need 1kohm pull-up resistors!
  mov P2M1, #00H
  mov P2M2, #00H
  mov P3M1, #00H
  mov P3M2, #00H


  mov state, #0 ; Initial state is waiting
  mov param_state, #0

  ; Default parameters usually work
  mov soak_temp, #140
  mov soak_time, #60
  mov reflow_temp, #225
  mov reflow_time, #45
  mov language, #0 ; ö
  
  lcall Sound_Start_Init ; Initialize the beginning place in flash to start playing
  lcall Ports_Init ; Default all pins as bidirectional I/O. See Table 42.
  lcall Double_Clk ; Use fast clock
  lcall InitDAC ; Call after 'Ports_Init
	
  lcall CCU_Init ; Initialize talking timer
  lcall Init_SPI ; Initialize SPI for sound flash

  lcall Timer0_Init ; Initialize Timer0 for PWM
  lcall Timer1_Init ; Initialize Timer1 for seconds incrementing
  
  lcall InitSerialPort ; Initialize serial port for communication with computer
  lcall InitADC0 ; ADC0 used for push buttons and LM335
  lcall LCD_4BIT ; Initialize LCD

  mov state_time+0, #0
	mov state_time+1, #0
	mov overall_time+0, #0
	mov overall_time+1, #0
  mov temp_truncated, #0
  mov temp_bcd+0, #0x02
  mov temp_bcd+1, #0
  ; Set beginning message on LCD
  Set_Cursor(1, 1)
  Send_Constant_String(#Title)
  Set_Cursor(2, 1)
  Send_Constant_String(#Title2)

  lcall Wait1S ; Wait a bit so PUTTy has a chance to start

  clr SOUND ; Disable speaker
  clr TMOD20 ; Stop CCU timer
	setb EA ; Enable global interrupts.

  mov a, #34
  lcall Play_Numbered ; Startup sound "ha'ha"!

forever:
  jb DIP_BUTTON1, next
  ; If we pass the dipswitch then go to the settings menu
  clr EA ; Stop interrupts, we don't need to play sound, increment seconds, or use PWM during settings setting
  lcall ADC_to_PB ; Read the ADC pushbuttons
	lcall settings ; Open the settings menu
  sjmp forever

next:
  setb EA ; We definitely want interrupts during normal running
  jb Abort, moveon ; ABORT BUTTON CHECK - Immediately exit if abort buttom is pushed
  jnb ABort, $  
  mov a, state
  jz moveon
  mov state, #0x0  ; Move to state zero, fsm will take care of turning stuff off
 
moveon:
  ;mov a, temp_truncated
  ;clr c
  ;subb a, #250
  ;jc moveon2
  ;mov state, #0
  ;clr SOUND
moveon2:
  lcall ADC_to_PB ; KEEP THIS LINE IN - IT DOESN'T WORK IF IT ISN'T THERE
  lcall fsm_update ; Go through the FSM
  lcall update_lcd ; Show stuff on the LCD
	
  jb second_flag, Every_Second_Stuff ; If it's exactly a second, then we do stuff that happens every second
  jb PLAY_BUTTON, forever ; Reset the screen if button is pressed
  ;Wait_Milli_Seconds(#50)
  ;jb PLAY_BUTTON, forever
  jnb PLAY_BUTTON, $
  lcall LCD_4BIT
  Wait_Milli_Seconds(#1)
  ljmp forever
Check_Temperatures:
  ;mov dptr, #Total
  ;lcall SendString
  lcall Get_Thermocouple ; Read the thermocouple
  lcall Read_Temperature ; Read the temperature at the LM335
  lcall get_total_temp ; Read overall temperature by combining thermocouple and cold junction data
  ;mov b, temp_truncated
  ;lcall SendHex
  ljmp Every_Second_2


;next:
	;Set_Cursor(2, 1)
    ;Send_Constant_String(#blank)
    ;ljmp next_check
Every_Second_Stuff:
  ;mov bcd+1, #0x02
  ;mov bcd+0, #0x53
  clr second_flag ; Clear the seconds flag so that we don't do this stuff every time
  ;mov b, state_time
  ;lcall SendHex
  ;mov b, reflow_time
  ;lcall SendHex
  ; Clear the screen
  Set_Cursor(1,1)
  Send_Constant_String(#clear)
  Set_Cursor(2,1)
  Send_Constant_String(#clear)
  ljmp Check_Temperatures
  ;jnb say_a_number, Check_Temperatures
Every_Second_2:
  lcall BCD_To_Sound ; Actually say stuff
  lcall Say_Stuff_FSM ; Set flag if we want to say stuff
  inc state_time ; Increase state_time
  inc overall_time+0 ; Increase overall time
  mov a, overall_time+0
  jnz End_Every_Second
increment_upper_seconds:
  inc overall_time+1
End_Every_Second:
  ljmp forever
end