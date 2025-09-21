/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>


#include <stdio.h>
#include <string.h> 
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

QueueHandle_t xQueueBtn;

SemaphoreHandle_t xSemaphoreR;
SemaphoreHandle_t xSemaphoreY;

void btn_callback(uint gpio, uint32_t events) {
    int botao;

    if (events == 0x4 && gpio == BTN_PIN_R) { // fall edge
        botao = 1;     
        xQueueSendFromISR(xQueueBtn, &botao, 0);
    }

    if (events == 0x4 && gpio == BTN_PIN_Y) { // fall edge
        botao = 2;
        xQueueSendFromISR(xQueueBtn, &botao, 0);
    }


    
}


void btn_task(void* p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);
    

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true,
                                       &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);

    int gpio = 0;
    while (true) {
        if (xQueueReceive(xQueueBtn, &gpio,  pdMS_TO_TICKS(100))){
            
            if (gpio == 1){
                // printf("Liguei o botão vermelho \n");
                xSemaphoreGive(xSemaphoreR);

            }

            else if (gpio == 2){
                // printf("Liguei no botão amarelo \n");
                xSemaphoreGive(xSemaphoreY);
                vTaskDelay(pdMS_TO_TICKS(1));
            }

        }
        vTaskDelay(pdMS_TO_TICKS(100));

    }
}

void led_r_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    // printf("cai aqui 1 \n");

    int delay = 100;
    int led_pisca_R = 0;

    while (true) {

        if (xSemaphoreTake(xSemaphoreR, pdMS_TO_TICKS(500)) == pdTRUE) {
            // printf("cai aqui 2 \n");
            led_pisca_R = !led_pisca_R;

        }

        if(led_pisca_R){
            // printf("Clique mais uma vez para desligar (red) \n");
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }

        else{
            // printf("Red desligado \n");
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }

        
  }
}

void led_y_task(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    int delay = 0;
    int led_pisca_Y = 0;

    while (true) {

        if (xSemaphoreTake(xSemaphoreY, pdMS_TO_TICKS(500)) == pdTRUE) {
            // printf("cai aqui 2 \n");
            led_pisca_Y = !led_pisca_Y;

        }

        if(led_pisca_Y){
            // printf("Clique mais uma vez para desligar (yellow) \n");
            gpio_put(LED_PIN_Y, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
        
        else{
            // printf("Yellow desligado \n");
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}



int main() {
    stdio_init_all();

    xSemaphoreR = xSemaphoreCreateBinary();
    xSemaphoreY = xSemaphoreCreateBinary();
    xQueueBtn = xQueueCreate(32, sizeof(int) );

    xTaskCreate(btn_task, "BTN_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(led_r_task,"LED_R_Task",256, NULL, 1, NULL);
    xTaskCreate(led_y_task,"LED_Y_Task",256, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1){}

    return 0;
}