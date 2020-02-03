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

FLASH_CE    EQU P2.4

dseg at 30H
	w:   ds 3 ; 24-bit play counter.  Decremented in CCU ISR.

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

bseg

$NOLIST
$include(sound.inc)
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

  clr TMOD20 ; Stop CCU timer
	setb EA ; Enable global interrupts.

forever:

  ljmp forever