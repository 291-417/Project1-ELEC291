TIMER1_RATE   EQU 500 ; should fire every millisecond
TIMER1_RELOAD EQU ((65536-(CLK/(2*TIMER0_RATE))))

;---------------------------------;
; Routine to initialize the ISR   ;
; for timer 1                   ;
;---------------------------------;
Timer1_Init:
	mov a, TMOD
	anl a, #0x0f ; Clear the bits for timer 1
	orl a, #0x10 ; Configure timer 1 as 16-timer
	mov TMOD, a
	mov TH1, #high(TIMER1_RELOAD)
  mov TL1, #low(TIMER1_RELOAD)
  mov Count1ms+0, #0
  mov Count1ms+1, #0
  clr second_flag
	; Enable the timer and interrupts
    setb ET1  ; Enable timer 0 interrupt
    setb TR1  ; Start timer 0
	ret

;---------------------------------;
; ISR for timer 1                 ;
;---------------------------------;
Timer1_ISR:
  push acc
  push ac
  mov TL1, #low(TIMER1_RELOAD)
  mov TH1, #high(TIMER1_RELOAD)
  
  ; Increment the 16-bit one mili second counter
	inc Count1ms+0    ; Increment the low 8-bits first
	mov a, Count1ms+0 ; If the low 8-bits overflow, then increment high 8-bits
	jnz Inc_Done
	inc Count1ms+1
Inc_Done:
  mov a, Count1ms+0
  cjne a, #low(2000), Timer1_ISR_done
  mov a, Count1ms+1
  cjne a, #high(2000), Timer1_ISR_done
  ; 1000 milliseconds have passed.  Set a flag so the main program knows
	setb second_flag ; Let the main program know half second had passed
  mov Count1ms+0, #0
  mov Count1ms+1, #0
Timer1_ISR_done:
  pop ac
  pop acc
  reti
end