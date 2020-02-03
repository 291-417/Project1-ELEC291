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

LCD_RS equ P1.6
LCD_RW equ P1.4
LCD_E  equ P1.3
LCD_D4 equ P0.7
LCD_D5 equ P0.6
LCD_D6 equ P0.5
LCD_D7 equ P0.3

PLAY_BUTTON equ P3.0

FLASH_CE    EQU P2.4

dseg at 30H
	w:   ds 3 ; 24-bit play counter.  Decremented in CCU ISR.
  x:   ds 4
  y:   ds 4
  bcd: ds 5
 
bseg

mf: dbit 1
PB0: dbit 1 ; Variable to store the state of pushbutton 0 after calling ADC_to_PB below
PB1: dbit 1 ; Variable to store the state of pushbutton 1 after calling ADC_to_PB below
PB2: dbit 1 ; Variable to store the state of pushbutton 2 after calling ADC_to_PB below
PB3: dbit 1 ; Variable to store the state of pushbutton 3 after calling ADC_to_PB below
PB4: dbit 1 ; Variable to store the state of pushbutton 4 after calling ADC_to_PB below
PB5: dbit 1 ; Variable to store the state of pushbutton 5 after calling ADC_to_PB below
PB6: dbit 1 ; Variable to store the state of pushbutton 6 after calling ADC_to_PB below

cseg

org 0x0000 ; Reset vector
    ljmp MainProgram

org 0x0003 ; External interrupt 0 vector (not used in this code)
	reti

org 0x000B ; Timer/Counter 0 overflow interrupt vector (not used in this code)
	reti

org 0x0013 ; External interrupt 1 vector (not used in this code)
	reti

org 0x001B ; Timer/Counter 1 overflow interrupt vector (not used in this code
	reti

org 0x0023 ; Serial port receive/transmit interrupt vector (not used in this code)
	reti

org 0x005b ; CCU interrupt vector.  Used in this code to replay the wave file.
	ljmp CCU_ISR



$NOLIST
$INCLUDE(math32.inc)
$include(LCD_4bit_LPC9351.inc) ; A library of LCD related functions and utility macros
$include(sound.inc)
$include(temppb.inc)
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

MainProgram:
  mov SP, #0x7F
  
  lcall Ports_Init ; Default all pins as bidirectional I/O. See Table 42.
  lcall Double_Clk
  lcall InitDAC ; Call after 'Ports_Init
	lcall CCU_Init
	lcall Init_SPI
  
  lcall InitSerialPort
	lcall InitADC0
  lcall LCD_4BIT

  ; Set beginning message on LCD
  Set_Cursor(1, 1)
  Send_Constant_String(#Title)

  lcall Wait1S ; Wait a bit so PUTTy has a chance to start

  mov dptr, #InitialMessage
	lcall SendString

  clr TMOD20 ; Stop CCU timer
	setb EA ; Enable global interrupts.

forever:
  lcall Read_Temperature
  lcall ADC_to_PB
	lcall Display_PushButtons_ADC
	jb PLAY_BUTTON, forever
  Wait_Milli_Seconds(#50)
  jb PLAY_BUTTON, forever
  jnb PLAY_BUTTON, $
  lcall Play_Whole_Memory
  ljmp forever

end