#define STCTRL *((volatile long *) 0xE000E010)          // control and status
#define STRELOAD *((volatile long *) 0xE000E014)        // reload value
#define STCURRENT *((volatile long *) 0xE000E018)       // current value

#define COUNT_FLAG  (1 << 16)                           // bit 16 of CSR automatically set to 1
#define ENABLE      (1 << 0)                            // bit 0 of CSR to enable the timer
#define CLKINT      (1 << 2)                            // bit 2 of CSR to specify CPU clock
#define CLOCK_HZ    16000000                            // Timer clock frequency
#define MAX_RELOAD  16777215
#define PWM_Freq    1                              // Frequency of the PWM waveform in Hz

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

void PORT_E_init( void );
void PORT_F_init( void );
void DynamicPWM(float duty_cycle, float on_time, float off_time);

int main(void)
{
    float duty_cycle = 50 ;    // Start with 50% duty cycle
    float on_time = duty_cycle / (100.0 * PWM_Freq) ;            // On time in seconds
    float off_time = (100 - duty_cycle) / (100.0 * PWM_Freq) ;   // Off time in seconds
    PORT_E_init();
    PORT_F_init();

    while(1) {
        // Set the duty cycle
        if ((~GPIO_PORTF_DATA_R) & 0x10){
            // SW1 increases the duty cycle by 5 %
            if (duty_cycle >= 100){
                duty_cycle = 99 ;
            }
            else{
                duty_cycle += 5 ;
                on_time = duty_cycle / (100.0 * PWM_Freq) ;            // On time in seconds
                off_time = (100 - duty_cycle) / (100.0 * PWM_Freq) ;   // Off time in seconds
            }
        }
        if ((~GPIO_PORTF_DATA_R) & 0x01){
            // SW2 decreases the duty cycle by 5 %
            if (duty_cycle <= 0){
                duty_cycle = 1 ;
            }
            else{
                duty_cycle -= 5 ;
                on_time = duty_cycle / (100.0 * PWM_Freq) ;            // On time in seconds
                off_time = (100 - duty_cycle) / (100.0 * PWM_Freq) ;   // Off time in seconds
            }
        }

        DynamicPWM(duty_cycle, on_time, off_time) ;
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
    GPIO_PORTF_DATA_R = 0x00 ;                  // Initialize the LEDs to off state
}

void DynamicPWM(float duty_cycle, float on_time, float off_time)
{
    unsigned long int time = CLOCK_HZ * on_time;
        // Complete the residue:
    STRELOAD = time;                                 // Set reload value
    STCTRL |= (CLKINT | ENABLE);                        // Set internal clock, enable the timer
    GPIO_PORTE_DATA_R = 0x01 ;
    while ((STCTRL & COUNT_FLAG) == 0) {            // Wait until flag is set
        STRELOAD = 0;// do nothing
    }
    // Stop the timer
    STCTRL = 0;
    // LOW/ Turn OFF
    time = CLOCK_HZ * off_time ;
    // Complete the residue:
    STRELOAD = time;                                 // Set reload value
    STCTRL |= (CLKINT | ENABLE);                        // Set internal clock, enable the timer
    GPIO_PORTE_DATA_R = 0x00 ;
    while ((STCTRL & COUNT_FLAG) == 0) {            // Wait until flag is set
        STRELOAD = 0;// do nothing
    }
        // Stop the timer
    STCTRL = 0;
}
