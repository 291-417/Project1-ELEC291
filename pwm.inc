TIMER0_RATE   EQU 4096     ; 2048Hz squarewave (peak amplitude of CEM-1203 speaker)
TIMER0_RELOAD EQU ((65536-(CLK/(2*TIMER0_RATE))))/128
DEFAULT_PWM EQU 5

Update_PWM_Percentages:
  clr ET0
  clr PWM_PIN
  ;mov a,  #low(TIMER0_RELOAD)
  ;mov b, pwm_percentage
  ;mul ab
  ;mov t0_reload_big, b ; t0_reload_big has period for high
  mov t0_reload_big, #high(2*TIMER0_RELOAD)

  mov t0_reload_small, #high(2*TIMER0_RELOAD)
  ;clr c
  ;mov a, #10
  ;subb a, pwm_percentage
  ;mov b, a
  ;mov a,  #low(TIMER0_RELOAD)
  ;mul ab
  ;mov t0_reload_small, b ; t0_reload_small has period for low
  setb ET0
  ret

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
	mov TH0, #0
  mov pwm_percentage, #DEFAULT_PWM
  lcall Update_PWM_Percentages
	mov TH0, t0_reload_small
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
  jb PWM_PIN, Reload_With_Big
  mov TL0, #0
  mov TH0, t0_reload_small
  setb PWM_PIN
  reti
Reload_With_Big:
	mov TL0, #0
	mov TH0, t0_reload_big
	clr PWM_PIN ; Connect speaker to this pin
	reti
end