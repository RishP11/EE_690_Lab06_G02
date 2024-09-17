#define STCTRL *((volatile long *) 0xE000E010)          // control and status
#define STRELOAD *((volatile long *) 0xE000E014)        // reload value
#define STCURRENT *((volatile long *) 0xE000E018)       // current value
#define COUNT_FLAG  (1 << 16)                           // bit 16 of CSR automatically set to 1
#define ENABLE      (1 << 0)                            // bit 0 of CSR to enable the timer
#define CLKINT      (1 << 2)                            // bit 2 of CSR to specify CPU clock
#define CLOCK_HZ    16000000                            // Timer clock frequency
#define MAX_RELOAD  16777215
#define PWM_FREQ    100000                             // Frequency of the PWM waveform in Hz

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

void PORT_E_init( void );
void PORT_F_init( void );
void GPTM_SETUP( float duty_cycle );

int main(void)
{
    float duty_cycle = 99.0 ;
    // Setup Port F
    PORT_F_init();
    // Setup Port E
    PORT_E_init();
    // Setup PWM Timer
    GPTM_SETUP( duty_cycle );
    // Setup Interrupt for GPIO
    while(1){
        ;// Everything done by the interrupts
    }
}

void PORT_E_init( void )
{
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE ;               // Enable clock to PORT_E
    GPIO_PORTE_LOCK_R = 0x4C4F434B ;             // Unlock commit register
    GPIO_PORTE_CR_R = 0x01 ;                     // Make PORT_E0 configurable
    GPIO_PORTE_DEN_R = 0x01 ;                    // Set PE0 pin as digital
    GPIO_PORTE_DIR_R = 0x01 ;                    // Set PE0 pin as output
    GPIO_PORTE_DATA_R = 0x00 ;
}
void PORT_F_init( void )
{
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF ;      // Enable clock to GPIO_F
    GPIO_PORTF_LOCK_R = 0x4C4F434B ;            // Unlock commit register
    GPIO_PORTF_CR_R = 0x1F ;                    // Make PORTF0 configurable
    GPIO_PORTF_DEN_R = 0x1F ;                   // Set PORTF pins 4 pin
    GPIO_PORTF_DIR_R = 0x0E ;                   // Set PORTF4 pin as input user switch pin
    GPIO_PORTF_PUR_R = 0x11 ;                   // Set the switches for Pull Up
    GPIO_PORTF_AFSEL_R |= (1 << 1) ;                 // Alternate function assignment
    GPIO_PORTF_PCTL_R |= (1 << 6) | (1 << 5) | (1 << 4) ; // Assign T0CCP1
}

void GPTM_SETUP( float duty_cycle ){
    unsigned long int count_value = CLOCK_HZ * (1.0 / PWM_FREQ) ;
    unsigned long int match_value = count_value - (duty_cycle * CLOCK_HZ / (100 * PWM_FREQ)) ;
    // Provide clock access to GPTM
    SYSCTL_RCGCTIMER_R |= 0x01 ;
    // GPTM Control Register :: Ensuring the Timer is OFF
    TIMER0_CTL_R = 0x00 ;
    // GPTM Configuration register :: Single mode
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
