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
    uint32_t heartbeat=systick_GetTick(); // heartbeat
    uint8_t i=0; // count number 
    uint8_t lrf=0; //Left - Right flag
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

            //UART STATES
            if (buffer[0] == 'S') {
                UART_send_string(USART2, "'Stationary'!\r\n");
                state = 2; // Cambia al estado 2 si 'S' fue recibido
            }

            if (buffer[0] == 'L') {
                UART_send_string(USART2, "'Left'!\r\n");
                state = 6; // Cambia al estado 6 si 'L' fue recibido
            }

            if (buffer[0] == 'l') {
                UART_send_string(USART2, "'Left infinite'!\r\n");
                state = 5; // Cambia al estado 6 si 'L' fue recibido
            }

            if (buffer[0] == 'R') {
                UART_send_string(USART2, "'Right'!\r\n");
                state = 10; // Cambia al estado 10 si 'R' fue recibido
            }

            if (buffer[0] == 'r') {
                UART_send_string(USART2, "'Right infinite'!\r\n");
                state = 9; // Cambia al estado 10 si 'R' fue recibido
            }

            if (buffer[0] == 'O') {
                UART_send_string(USART2, "'IDLE'!\r\n");
                state = 0; // Cambia al estado 0 si 'O' fue recibido
                gpio_off_led2(); // Apaga LED 2
                gpio_off_led3(); // Apaga LED 3
            }
        }
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
                lrf=0;
            } if (gpio_button_is_pressed() != 0) { // Press central button 
                state = 2;
                
            } else if (gpio_button_is_pressed2() != 0 && lrf==0) { // Press left button
                state = 4;
                heartbeat = systick_GetTick();
            } else if (gpio_button_is_pressed3() != 0 && lrf==0) { // Press right button
                state = 8;
                heartbeat = systick_GetTick();
            }
            break;

        case 1: // Toggle Led (Heartbeating LED)
            gpio_toggle_led(); 
            state = 0;
            break;
        case 2:  // Press central button
            if (systick_GetTick() >= heartbeat+500){
                state=3;
                heartbeat = systick_GetTick(); // Toggle LEDS
            }
            if (gpio_button_is_pressed() != 0){
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
        case 4: // Press left button
            //if (gpio_button_is_pressed2() != 0) {
            //    uint32_t current_time = systick_GetTick(); // Si el botón 2 está presionado
            //    if (current_time - heartbeat <= 500) { // Si el tiempo desde la última presión es menor a 500 ms
            //        state = 5; // Cambia a estado 5 si es una doble presión
            //        heartbeat = systick_GetTick();
            //} else {
            state=6;
            heartbeat = systick_GetTick();
            //}
            //}
            break;
        case 5: //Left infinite loop
            if (systick_GetTick() >= heartbeat+500){ // Heartbeating LED
                state = 12; // End timing for 500 ms
                heartbeat = systick_GetTick(); // Reset timer
            }
            if (gpio_button_is_pressed3() != 0){
                gpio_off_led2(); // Apaga LED 2
                state=0;
                heartbeat = systick_GetTick();
                lrf=1;
            }
            break;
        case 12: // Toggle Led (Heartbeating LED)
            gpio_toggle_led(); 
            gpio_toggle_led2(); 
            state = 5;
            break;
        case 6: //Left light toggle 6 times
            if (systick_GetTick() >= heartbeat + 500 && i < 6) {
                state = 7;
                heartbeat = systick_GetTick(); // Reset timer
            }
            if (i == 6) {
                state = 0;
                i = 0;
                gpio_off_led2(); // Apaga LED 2
            }     
            break;  
        case 7:
            gpio_toggle_led();
            gpio_toggle_led2();
            i++; // Incrementar el contador de parpadeos
            state = 6;
            break;  
        case 8: //Press right button
            //if (gpio_button_is_pressed2() != 0) {
            //    uint32_t current_time = systick_GetTick(); // Si el botón 2 está presionado
            //    if (current_time - heartbeat <= 500) { // Si el tiempo desde la última presión es menor a 500 ms
            //        state = 9; // Cambia a estado 5 si es una doble presión
            //        heartbeat = systick_GetTick();
            //} else {
            state=10;
            heartbeat = systick_GetTick();
            //}
            //}
            break;
        case 9: //Right infinite loop
            if (systick_GetTick() >= heartbeat+500){ // Heartbeating LED
                state = 13; // End timing for 500 ms
                heartbeat = systick_GetTick(); // Reset timer
            }
            if (gpio_button_is_pressed2() != 0){
                gpio_off_led3(); // Apaga LED 3
                state=0;
                lrf=1;
                heartbeat = systick_GetTick();
            }
            break;
        case 13: // Toggle Led (Heartbeating LED)
            gpio_toggle_led(); 
            gpio_toggle_led3(); 
            state = 9;
            break;
        case 10: // Right light toggle 6 times
            if (systick_GetTick() >= heartbeat + 500 && i < 6) {
                state = 11;
                heartbeat = systick_GetTick(); // Reset timer
            }
            if (i == 6) {
                state = 0;
                i = 0;
                gpio_off_led3(); // Apaga LED 2
            }     
            break;  
        case 11:
            gpio_toggle_led();
            gpio_toggle_led3();
            i++; // Incrementar el contador de parpadeos
            state = 10;
            break;  
        default:
            break;
        }
    }
}

