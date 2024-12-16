#include "gpio.h"
#include "rcc.h"

#define EXTI_BASE 0x40010400
#define EXTI ((EXTI_t *)EXTI_BASE)

#define EXTI15_10_IRQn 40
#define NVIC_ISER1 ((uint32_t *)(0xE000E104)) // NVIC Interrupt Set-Enable Register

#define SYSCFG_BASE 0x40010000
#define SYSCFG ((SYSCFG_t *)SYSCFG_BASE)

#define GPIOA ((GPIO_t *)0x48000000) // Base address of GPIOA
#define GPIOC ((GPIO_t *)0x48000800) // Base address of GPIOC

#define LED_PIN 5 // Pin 5 of GPIOA
#define LED2_PIN 4      // Pin 4 of GPIOC (New LED) left
#define LED3_PIN 6      // Pin 6 of GPIOC (New LED) right
#define BUTTON_PIN 13 // Pin 13 of GPIOC
#define BUTTON_PIN2 12 // Pin 12 of GPIOC           left
#define BUTTON_PIN3 10 // Pin 10 of GPIOC           right

#define BUTTON_IS_PRESSED()    (!(GPIOC->IDR & (1 << BUTTON_PIN)))
#define BUTTON_IS_RELEASED()   (GPIOC->IDR & (1 << BUTTON_PIN))
#define BUTTON_IS_PRESSED2()    (!(GPIOC->IDR & (1 << BUTTON_PIN2))) // New macro for button 2
#define BUTTON_IS_RELEASED2()   (GPIOC->IDR & (1 << BUTTON_PIN2)) 
#define BUTTON_IS_PRESSED3()    (!(GPIOC->IDR & (1 << BUTTON_PIN3))) // New macro for button 3
#define BUTTON_IS_RELEASED3()   (GPIOC->IDR & (1 << BUTTON_PIN3)) 
#define TOGGLE_LED()           (GPIOA->ODR ^= (1 << LED_PIN))
#define TOGGLE_LED2()          (GPIOC->ODR ^= (1 << LED2_PIN)) // New macro for toggling LED2
#define TOGGLE_LED3()          (GPIOC->ODR ^= (1 << LED3_PIN)) // New macro for toggling LED3
#define OFF_LED2()            (GPIOC->ODR &= ~(1 << LED2_PIN)) //   New macro for off LED2
#define OFF_LED3()            (GPIOC->ODR &= ~(1 << LED3_PIN)) //   New macro for off LED3
volatile uint8_t button_pressed = 0; // Flag to indicate button press
volatile uint8_t button_pressed2 = 0; // Flag to indicate button 2 press 
volatile uint8_t button_pressed3 = 0; // Flag to indicate button 3 press

void configure_gpio_for_usart() {
    // Enable GPIOA clock
    *RCC_AHB2ENR |= (1 << 0);

    // Configure PA2 (TX) as alternate function
    GPIOA->MODER &= ~(3U << (2 * 2)); // Clear mode bits for PA2
    GPIOA->MODER |= (2U << (2 * 2));  // Set alternate function mode for PA2

    // Configure PA3 (RX) as alternate function
    GPIOA->MODER &= ~(3U << (3 * 2)); // Clear mode bits for PA3
    GPIOA->MODER |= (2U << (3 * 2));  // Set alternate function mode for PA3

    // Set alternate function to AF7 for PA2 and PA3
    GPIOA->AFR[0] &= ~(0xF << (4 * 2)); // Clear AFR bits for PA2
    GPIOA->AFR[0] |= (7U << (4 * 2));   // Set AFR to AF7 for PA2
    GPIOA->AFR[0] &= ~(0xF << (4 * 3)); // Clear AFR bits for PA3
    GPIOA->AFR[0] |= (7U << (4 * 3));   // Set AFR to AF7 for PA3

    // Configure PA2 and PA3 as very high speed
    GPIOA->OSPEEDR |= (3U << (2 * 2)); // Very high speed for PA2
    GPIOA->OSPEEDR |= (3U << (3 * 2)); // Very high speed for PA3

    // Configure PA2 and PA3 as no pull-up, no pull-down
    GPIOA->PUPDR &= ~(3U << (2 * 2)); // No pull-up, no pull-down for PA2
    GPIOA->PUPDR &= ~(3U << (3 * 2)); // No pull-up, no pull-down for PA3
}

void init_gpio_pin(GPIO_t *GPIOx, uint8_t pin, uint8_t mode)
{
    GPIOx->MODER &= ~(0x3 << (pin * 2)); // Clear MODER bits for this pin
    GPIOx->MODER |= (mode << (pin * 2)); // Set MODER bits for this pin
}

void configure_gpio(void)
{
    *RCC_AHB2ENR |= (1 << 0) | (1 << 2); // Enable clock for GPIOA and GPIOC

    // Enable clock for SYSCFG
    *RCC_APB2ENR |= (1 << 0); // RCC_APB2ENR_SYSCFGEN

    // Configure SYSCFG EXTICR to map EXTI13 to PC13
    SYSCFG->EXTICR[3] &= ~(0xF << 4); // Clear bits for EXTI13
    SYSCFG->EXTICR[3] |= (0x2 << 4);  // Map EXTI13 to Port C
    SYSCFG->EXTICR[3] &= ~(0xF << 0); // Clear bits for EXTI12
    SYSCFG->EXTICR[3] |= (0x2 << 0);  // Map EXTI12 to Port C
    SYSCFG->EXTICR[2] &= ~(0xF << 8);  // Clear bits for EXTI10
    SYSCFG->EXTICR[2] |= (0x2 << 8);   // Map EXTI10 to Port C
    
    // Configure EXTI13 for falling edge trigger
    EXTI->FTSR1 |= (1 << BUTTON_PIN);  // Enable falling trigger
    EXTI->RTSR1 &= ~(1 << BUTTON_PIN); // Disable rising trigger

    // Configure EXTI12 for falling edge trigger
    EXTI->FTSR1 |= (1 << BUTTON_PIN2);  // Enable falling trigger
    EXTI->RTSR1 &= ~(1 << BUTTON_PIN2); // Disable rising trigger

    // Configure EXTI10 for falling edge trigger
    EXTI->FTSR1 |= (1 << BUTTON_PIN3);  // Enable falling trigger
    EXTI->RTSR1 &= ~(1 << BUTTON_PIN3); // Disable rising trigger

    // Unmask EXTI13
    EXTI->IMR1 |= (1 << BUTTON_PIN);

    // Unmask EXTI12
    EXTI->IMR1 |= (1 << BUTTON_PIN2);

    // Unmask EXTI14
    EXTI->IMR1 |= (1 << BUTTON_PIN3);

    init_gpio_pin(GPIOA, LED_PIN, 0x1); // Set LED pin as output
    init_gpio_pin(GPIOC, BUTTON_PIN, 0x0); // Set BUTTON pin as input
    init_gpio_pin(GPIOC, BUTTON_PIN2, 0x0); // Set BUTTON 2 pin as input
    init_gpio_pin(GPIOC, BUTTON_PIN3, 0x0); // Set BUTTON 2 pin as input
    init_gpio_pin(GPIOC, LED2_PIN, 0x1); // Configure PC4 as output for new LED
    init_gpio_pin(GPIOC, LED3_PIN, 0x1); // Configure PC6 as output for new LED

    // Enable EXTI15_10 interrupt
    *NVIC_ISER1 |= (1 << (EXTI15_10_IRQn - 32));

    configure_gpio_for_usart();
}

uint8_t gpio_button_is_pressed(void)
{
    return BUTTON_IS_PRESSED();
}

uint8_t gpio_button_is_pressed2(void) // New function to button 2
{
    return BUTTON_IS_PRESSED2();
}

uint8_t gpio_button_is_pressed3(void) // New function to button 3
{
    return BUTTON_IS_PRESSED3();
}

void gpio_toggle_led(void)
{
    TOGGLE_LED();
}
void gpio_toggle_led2(void) { // New function to toggle the second LED
    TOGGLE_LED2();
}

void gpio_toggle_led3(void) { // New function to toggle the third LED
    TOGGLE_LED3();
}

void gpio_off_led2(void) { // New function to turn off the second LED
    OFF_LED2();
}

void gpio_off_led3(void) { // New function to turn off the third LED
    OFF_LED3();
}

void EXTI15_10_IRQHandler(void)
{
    if (EXTI->PR1 & (1 << BUTTON_PIN)) {
        EXTI->PR1 = (1 << BUTTON_PIN); // Clear pending bit
        button_pressed = 1; // Set button pressed flag  
    }

    if (EXTI->PR1 & (1 << BUTTON_PIN2)) {
        EXTI->PR1 = (1 << BUTTON_PIN2); // Clear pending bit
        button_pressed2 = 1; // Set button pressed flag  
    }

    if (EXTI->PR1 & (1 << BUTTON_PIN3)) {
        EXTI->PR1 = (1 << BUTTON_PIN3); // Clear pending bit
        button_pressed3 = 1; // Set button pressed flag  
    }
}
