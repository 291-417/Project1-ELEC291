TIMER0_RATE   EQU 250     ; 2048Hz squarewave (peak amplitude of CEM-1203 speaker)
TIMER0_RELOAD EQU ((65536-(CLK/(2*TIMER0_RATE))))
DEFAULT_PWM EQU 10 ; Make sure to set PWM in range from 1-99
MAX_PWM EQU 100

;---------------------------------;
; Routine to initialize the ISR   ;
; for timer 0                     ;
;---------------------------------;
Timer0_Init:
	mov a, TMOD
	anl a, #0xf0 ; Clear the bits for timer 0
	orl a, #0x02 ; Configure timer 0 as 16-timer
	mov TMOD, a
  mov a, TAMOD
  anl a, #0xf0
  orl a, #0x01 ; configure t0m2
  mov TAMOD, a
  mov pwm_percentage, #DEFAULT_PWM
  mov pwm_counter, #DEFAULT_PWM
	mov TH0, #high(TIMER0_RELOAD)
  mov TL0, #low(TIMER0_RELOAD)
	; Enable the timer and interrupts
    setb ET0  ; Enable timer 0 interrupt
    setb TR0  ; Start timer 0
	ret

;---------------------------------;
; ISR for timer 0.  Set to execute;
; every 1/4096Hz to generate a    ;
; 2048 Hz square wave at pin P3.7 ;
;---------------------------------;
Timer0_ISR:
  push acc
  push ac
  mov TL0, #low(TIMER0_RELOAD)
  mov TH0, #high(TIMER0_RELOAD)
  dec pwm_counter
  mov a, pwm_counter
  jnz Return_From_PWM ; if pwm_counter is not zero, we still want to wait a few cycles; just return
  cpl PWM_PIN
  jnb PWM_PIN, PWM_Was_High
PWM_Was_Low:
  mov pwm_counter, pwm_percentage
  sjmp Return_From_PWM
PWM_Was_High:
  mov a, #MAX_PWM
  clr c
  subb a, pwm_percentage
  mov pwm_counter, a
Return_From_PWM:
  pop ac
  pop acc
	reti

end