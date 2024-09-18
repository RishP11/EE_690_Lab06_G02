#define CLOCK_HZ    16000000                            // Timer clock frequency
#define PWM_FREQ    100000                              // Frequency of the PWM waveform in Hz
float duty_cycle = 50.0 ;                               // On-time Duty cycle of the PWM waveform

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

void PORT_F_init( void );
void GPTM_SETUP( float duty_cycle );
void GPIOF_SETUP( void );
void GPIOF_ISR( void );

int main(void)
{
    // Setup Port F
    PORT_F_init();
    // Setup PWM Timer
    GPTM_SETUP( duty_cycle );
    // Setup Interrupt for GPIO
    GPIOF_SETUP();
    while(1){
        ;// Handled by interrupts & ISR
    }
}

void PORT_F_init( void )
{
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF ;                  // Enable clock to GPIO_F
    GPIO_PORTF_LOCK_R = 0x4C4F434B ;                        // Unlock commit register
    GPIO_PORTF_CR_R = 0x1F ;                                // Make PORTF0 configurable
    GPIO_PORTF_DEN_R = 0x1F ;                               // Set PORTF pins 4 pin
    GPIO_PORTF_DIR_R = 0x0E ;                               // Set PORTF4 pin as input user switch pin
    GPIO_PORTF_PUR_R = 0x11 ;                               // Set the switches for Pull Up
    GPIO_PORTF_AFSEL_R |= (1 << 1) ;                        // Alternate function assignment
    GPIO_PORTF_PCTL_R |= (1 << 6) | (1 << 5) | (1 << 4) ;   // Assign T0CCP1
}

void GPTM_SETUP( float duty_cycle ){
    unsigned long int count_value = CLOCK_HZ * (1.0 / PWM_FREQ) ;
    unsigned long int match_value = count_value - (duty_cycle * CLOCK_HZ / (100 * PWM_FREQ)) ;
    // Enable clock to GPTM
    SYSCTL_RCGCTIMER_R |= 0x01 ;
    // GPTM Control Register :: Ensure the Timer is OFF
    TIMER0_CTL_R = 0x00 ;
    // GPTM Configuration register :: Single (register) mode
    TIMER0_CFG_R = 0x04 ;
    // GPTM Timer and Mode Register :: PWM Mode
    TIMER0_TBMR_R = 0x00 ;
    TIMER0_TBMR_R |= (1 << 3) ;
    TIMER0_TBMR_R |= (1 << 1) ;
    // GPTM Control Register :: Non-Inverted Output
    TIMER0_CTL_R |= (1 << 6) ;
    // GPTM Interval Load Register :: Count Top value
    TIMER0_TBILR_R = count_value ;
    // GPTM Match Value register
    TIMER0_TBMATCHR_R = match_value ;
    // GPTM CTL register :: Start the timer:
    TIMER0_CTL_R |= (1 << 8) ;
}

void GPIOF_SETUP( void )
{
    // PORT F = ...|SW1|G|B|R|SW2|
    // Interrupt sense register :: Used to configure whether interrupt is level/ edge sensitive.
    // 1 => Level detection and 0 => Edge detection
    GPIO_PORTF_IS_R = 0x00 ;
    // Interrupt Event Register :: Used to configure whether event to be detected is high/low or rising/falling.
    // 1 => Rising Edge/ High and 0 => Falling Edge/ Low
    GPIO_PORTF_IEV_R = 0x00 ;
    // Interrupt Both Edges Register :: Used to configure whether both edges are to be detected as events for interrupt.
    GPIO_PORTF_IBE_R = 0x00 ;
    // Interrupt Mask Register :: To determine whether to allow the interrupt generated to be passed onto interrupt controller or not.
    // 1 => Send the interrupt and 0 => Do not send the interrupt
    GPIO_PORTF_IM_R = 0x11 ;
    // Interrupt Clear Register :: Used to clear any pending interrupts
    GPIO_PORTF_ICR_R = 0x11 ;
    NVIC_EN0_R |=  (1 << 30) ; // Enable interrupt for GPIO Port F
}

void GPIOF_ISR( void )
{
    // PORT F = ...|SW1|G|B|R|SW2|
    int hold_time = 0;
    int threshold = 1000000 ;
    while(hold_time < threshold){
        hold_time += 1;
    }
    if ((~(GPIO_PORTF_DATA_R) & 0x11) && (duty_cycle >= 5)) {
        // Long press ==> Decrease
        duty_cycle -= 4.99 ;
    }
    else{
        // Short press ==> Increase
        if (duty_cycle <= 95){
            duty_cycle += 4.99 ;
    }
    GPIO_PORTF_ICR_R = 0x11 ;
    unsigned long int count_value = CLOCK_HZ * (1.0 / PWM_FREQ) ;
    unsigned long int match_value = count_value - (duty_cycle * CLOCK_HZ / (100 * PWM_FREQ)) ;
    // Match Value Update
    TIMER0_TBMATCHR_R = match_value ;
    }
}
