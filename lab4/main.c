#include <stdint.h>
#include "tm4c123gh6pm.h"

#define STCTRL *((volatile long *) 0xE000E010)    // control and status
#define STRELOAD *((volatile long *) 0xE000E014)    // reload value
#define STCURRENT *((volatile long *) 0xE000E018)    // current value

#define COUNT_FLAG  (1 << 16)   // bit 16 of CSR automatically set to 1
                                //   when timer expires
#define ENABLE      (1 << 0)    // bit 0 of CSR to enable the timer
#define TICKINT      (1 << 1)    // bit 1 of CSR An interrupt is generated to the NVIC when SysTick counts to 0
#define CLKINT      (1 << 2)    // bit 2 of CSR to specify CPU clock

#define CLOCK_MHZ 16
#define MASK_BITS 0x11 // pins for which interrupts should be enabled

void SysTick_Init() {
    STCURRENT = 0;
    STRELOAD = 16000*1000;                       // reload value for '1000ms' milliseconds
    STCTRL |= (CLKINT | TICKINT | ENABLE);        // set internal clock, enable the timer
    return;
}

void GPIO_Init() {
    // Initialize GPIO for LED (assuming it's connected to PortF, Pin 2)
    SYSCTL_RCGC2_R |= 0x00000020; // Enable clock to PortF
    GPIO_PORTF_LOCK_R = 0x4C4F434B;     /* unlock commit register */
    GPIO_PORTF_CR_R = 0xFF;             /* make PORTF0 configurable */
    GPIO_PORTF_DIR_R |= 0x04;  // Set Pin 2 as an output
    GPIO_PORTF_DEN_R |= 0x04;  // Digital enable Pin 2

    // Initialize GPIO for Button (assuming it's connected to PortF, Pin 4)
    GPIO_PORTF_DIR_R &= ~0x11; // Set Pin 0, 4 as an input
    GPIO_PORTF_PUR_R |= 0x11;  // Enable the internal pull-up resistor for Pin 0, 4
    GPIO_PORTF_DEN_R |= 0x11;  // Digital enable Pin 0, 4

    //GPIO interrupts
    // Enable interrupt for the button (PortF, Pin 4)
    GPIO_PORTF_IM_R &= ~MASK_BITS; // mask interrupt by clearing bits
    GPIO_PORTF_IS_R &= ~MASK_BITS; // edge sensitive interrupts
    GPIO_PORTF_IBE_R &= ~MASK_BITS; // interrupt NOT on both edges
    GPIO_PORTF_IEV_R &= ~MASK_BITS; // falling edge trigger
    /* Prioritize and enable interrupts in NVIC */
    NVIC_PRI7_R = (NVIC_PRI7_R & 0xF1FFFFFF) | (3 << 21);
     // interrupt priority register 7
     // bits 21-23 for interrupt 30 (port F)
    NVIC_EN0_R |= 1 << 30; // enable interrupt 30 (port F)
    /* Enable interrupt generation in GPIO */
    GPIO_PORTF_ICR_R = MASK_BITS; // clear any prior interrupt
    GPIO_PORTF_IM_R |= MASK_BITS; // unmask interrupt by setting bits
}

void SysTickIntHandler(void) {
    GPIO_PORTF_DATA_R &= ~0x04; // Clear Pin 2 (LED)
    STCTRL = 0x00; // Disable SysTick Timer
}

void GPIOF_Handler(void) {
    GPIO_PORTF_ICR_R = 0x11; // Clear the interrupt flag
    GPIO_PORTF_DATA_R ^= 0x04; // Set Pin 2 (LED)
    //STCTRL = 0x07; // Enable SysTick Timer
}

int main(void) {
    GPIO_Init(); // Initialize GPIO pins
    //SysTick_Init(); // Initialize SysTick Timer

    while (1) {
        // Your main application code here
    }
}
