#include <stdint.h>

#include "systick.h"
#include "gpio.h"
#include "uart.h"


int main(void)
{
    configure_systick_and_start();
    configure_gpio();
    
    UART_Init(USART2);

    uint8_t state = 0; // state of the FSM;
    uint32_t heartbeat=systick_GetTick();
    UART_send_string(USART2, "Hello World, from main!\r\n");

    uint8_t buffer[1];
    UART_receive_string(USART2, buffer, 1);

    UART_send_string(USART2, "Received: ");
    UART_send_string(USART2, (char *)buffer);
    UART_send_string(USART2, "\r\n");

    UART_receive_it(USART2, buffer, 1);
    while (1) {
        if (rx_ready != 0) {
            UART_send_string(USART2, "Received: ");
            UART_send_string(USART2, (char *)buffer);
            UART_send_string(USART2, "\r\n");
            UART_receive_it(USART2, buffer, 1);
            rx_ready = 0;
        }
        //if (systick_GetTick() >= 500) {
        //    gpio_toggle_led();
        //    systick_reset();
        //}
        //if (buffer[0] == 'L') {
        //    gpio_toggle_led3(); // Turn on LED 3
        //} else if (buffer[0] == 'R') {
        //    gpio_toggle_led2(); // Turn on LED 2
        //}

        // Prepare to receive the next character
        //UART_receive_it(USART2, buffer, 1);
        //rx_ready = 0;


        switch (state) {
        case 0: // idle 
            if (systick_GetTick() >= heartbeat+500){ // Heartbeating LED
                state = 1; // End timing for 500 ms
                heartbeat = systick_GetTick(); // Reset timer
            } if (gpio_button_is_pressed() != 0) { // Press central button 
                state = 2;
            }
            //} else if (gpio_button_is_pressed2() != 0) { // Press left button
            //    state = 6;
            //} else if (gpio_button_is_pressed3() != 0) { // Press right button
            //    state = 8;
            break;

        case 1: // Toggle Led (Heartbeating LED)
            gpio_toggle_led(); 
            state = 0;
            break;
        case 2: 
            if (systick_GetTick() >= heartbeat+500){
                state=3;
                heartbeat = systick_GetTick(); // Reset timer
            }
            if (gpio_button_is_pressed2() != 0 || gpio_button_is_pressed3()){
                gpio_off_led2(); // Apaga LED 2
                gpio_off_led3(); // Apaga LED 3
                state=0;
            }
            break;
        case 3: //Stationary
            gpio_toggle_led();
            gpio_toggle_led2(); // Toggle LED 2 left
            gpio_toggle_led3(); // Toggle LED 3 right
            state = 2;
            break;
        default:
            break;
        }
    }
}

