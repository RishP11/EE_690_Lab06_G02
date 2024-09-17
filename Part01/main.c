//#define STCTRL *((volatile long *) 0xE000E010)          // control and status
//#define STRELOAD *((volatile long *) 0xE000E014)        // reload value
//#define STCURRENT *((volatile long *) 0xE000E018)       // current value
//
//#define COUNT_FLAG  (1 << 16)                           // bit 16 of CSR automatically set to 1
//#define ENABLE      (1 << 0)                            // bit 0 of CSR to enable the timer
//#define CLKINT      (1 << 2)                            // bit 2 of CSR to specify CPU clock
#define CLOCK_HZ    16000000                            // Timer clock frequency
//#define MAX_RELOAD  16777215

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

void PORT_E_init( void );
void PORT_F_init( void );
void Delay(float seconds);

int main(void)
{
    return 0;
    int frequency = 1;               // f_PWM + delta (delta>0 to compensate for the overhead calculations in the Delay function)
    float duty_cycle = 0.2;
    float on_time = duty_cycle / frequency;        // On time in seconds
    float off_time = (1.0 - duty_cycle) / frequency; // Off time in seconds

    PORT_E_init();
    PORT_F_init();

    while(1) {
        // HIGH :: ON
        GPIO_PORTF_DATA_R = 0x02 ;
        // Wait
        Delay(on_time);
       // LOW :: OFF
        GPIO_PORTF_DATA_R = 0x00;
       // Wait
        Delay(off_time);
    }
}

void PORT_E_init( void )
{
    SYSCTL_RCGC2_R |= 0x00000010;      // Enable clock to PORT_E
    GPIO_PORTE_LOCK_R = 0x4C4F434B;    // Unlock commit register
    GPIO_PORTE_CR_R = 0x01;            // Make PORT_E0 configurable
    GPIO_PORTE_DEN_R = 0x01;           // Set PE0 pin as digital
    GPIO_PORTE_DIR_R = 0x01;           // Set PE0 pin as output
    return ;
}

void PORT_F_init( void )
{
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF ;      // Enable clock to GPIO_F
    GPIO_PORTF_LOCK_R = 0x4C4F434B ;            // Unlock commit register
    GPIO_PORTF_CR_R = 0x1F ;                    // Make PORTF0 configurable
    GPIO_PORTF_DEN_R = 0x1F ;                   // Set PORTF pins 4 pin
    GPIO_PORTF_DIR_R = 0x0E ;                   // Set PORTF4 pin as input user switch pin
    GPIO_PORTF_PUR_R = 0x11 ;                   // Set the switches for Pull Up
    GPIO_PORTF_DATA_R = 0x00 ;                  // Initialize the LEDs to off state
}

void Delay(float seconds)
{
    unsigned long int counter_high = CLOCK_HZ * seconds ;
    // GPTMT = General Purpose Timer Module
    // Provide clock access to GPTM:
    SYSCTL_RCGCTIMER_R |= 0x01 ;
    // GPTMT Control Register :: Ensure that the timer is disabled.
    TIMER0_CTL_R = 0x00 ;
    // GPTMT Configuration Register
    TIMER0_CFG_R = 0x00 ;
    // GPTMT Timer and Mode Register :: Selecting the mode
    // Also configure to count-down using TnCDIR
    TIMER0_TAMR_R = 0x1 ;
    TIMER0_TAMR_R &= (1 << 4) ;
    // GPTMT Interval Load Register :: Load the counter value
    TIMER0_TAILR_R = counter_high ;
    // Set the TAEN bit = 1 to enable the timer A.
    TIMER0_CTL_R = 0x01 ;
    while (TIMER0_TAV_R > 0){

    }
    return ;
}
