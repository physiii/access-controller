/* GPIO Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define R1_IO    12
#define R2_IO    33
#define R3_IO    15
#define R4_IO    27

#define C1_IO    4
#define C2_IO    23
#define C3_IO    26
#define C4_IO    0


#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<R1_IO) | (1ULL<<R2_IO) | (1ULL<<R3_IO) | (1ULL<<R4_IO))
#define GPIO_INPUT_PIN_SEL  ((1ULL<<C1_IO) | (1ULL<<C2_IO) | (1ULL<<C3_IO) | (1ULL<<C4_IO))
#define ESP_INTR_FLAG_DEFAULT 0

static xQueueHandle keypad_evt_queue = NULL;
bool new_key_ready = true;

static void IRAM_ATTR keypad_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(keypad_evt_queue, &gpio_num, NULL);
}

int check_column(int pin)
{
    gpio_config_t io_conf;
    int row = 0;
    // set rows to input
    io_conf.intr_type = GPIO_PIN_INTR_ANYEDGE;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = 1;
    gpio_config(&io_conf);

    // set columns to output
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.pull_down_en =  1;
    gpio_config(&io_conf);

    gpio_set_level(pin, 1);

    if (gpio_get_level(R1_IO)) {
      row = 1;
    } else if (gpio_get_level(R2_IO)) {
      row = 2;
    } else if (gpio_get_level(R3_IO)) {
      row = 3;
    } else if (gpio_get_level(R4_IO)) {
      row = 4;
    }

    // set rows as output
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en =  1;
    gpio_config(&io_conf);

    // set columns as input
    io_conf.intr_type = GPIO_PIN_INTR_ANYEDGE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = 1;
    gpio_config(&io_conf);

    gpio_set_level(R1_IO, 1);
    gpio_set_level(R2_IO, 1);
    gpio_set_level(R3_IO, 1);
    gpio_set_level(R4_IO, 1);

    return row;
}

int new_key_entered ()
{
  int value = new_key_ready;
  new_key_ready = false;
  return value;
}

int check_pad (int io_num)
{
  int press_found = check_column(io_num);

  if (io_num == C1_IO) {
    if (press_found == 1) return 1;
    if (press_found == 2) return 4;
    if (press_found == 3) return 7;
    if (press_found == 4) return 10;
  }

  if (io_num == C2_IO) {
    if (press_found == 1) return 2;
    if (press_found == 2) return 5;
    if (press_found == 3) return 8;
    if (press_found == 4) return 0;
  }

  if (io_num == C3_IO) {
    if (press_found == 1) return 3;
    if (press_found == 2) return 6;
    if (press_found == 3) return 9;
    if (press_found == 4) return 11;
  }

  if (io_num == C4_IO) {
    if (press_found == 1) return 12;
    if (press_found == 2) return 13;
    if (press_found == 3) return 14;
    if (press_found == 4) return 15;
  }

  return -1;
}

static void keypad_task_example(void* arg)
{
    uint32_t io_num;
    int prev_cnt = 0;
    int prev_num = 0;
    int num_count = 0;
    int notify = false;
    int code_count = 0;

    for(;;) {
        if(xQueueReceive(keypad_evt_queue, &io_num, portMAX_DELAY)) {
            int num = check_pad(io_num);

            if (num >= 0 && prev_num == num) {
              num_count++;
            }

            if (num_count > 2) {

              if (notify) {
                printf("Key Pressed: %d\n", num);

                if (num == 11 && code_count >= code_size) {
                    printf("Sending code: %d %d %d %d\n",
                    keypad_code[0], keypad_code[1], keypad_code[2], keypad_code[3]);
                    code_count = 0;
                    new_key_ready = true;
                    vTaskDelay(2000 / portTICK_RATE_MS);
                } else {
                  keypad_code[code_count] = num;
                  code_count++;
                }

                notify = false;
              }

              if (prev_num != num) {
                num_count = 0;
              }
            } else {
              notify = true;
            }

            prev_num = num;
            vTaskDelay(20 / portTICK_RATE_MS);
        }
    }
}

void keypad_driver_main()
{
    gpio_config_t io_conf;

    // set rows as output
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en =  1;
    gpio_config(&io_conf);

    // set columns as input
    io_conf.intr_type = GPIO_PIN_INTR_ANYEDGE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = 1;
    gpio_config(&io_conf);

    gpio_set_level(R1_IO, 1);
    gpio_set_level(R2_IO, 1);
    gpio_set_level(R3_IO, 1);
    gpio_set_level(R4_IO, 1);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(C1_IO, keypad_isr_handler, (void*) C1_IO);
    gpio_isr_handler_add(C2_IO, keypad_isr_handler, (void*) C2_IO);
    gpio_isr_handler_add(C3_IO, keypad_isr_handler, (void*) C3_IO);
    gpio_isr_handler_add(C4_IO, keypad_isr_handler, (void*) C4_IO);


    //create a queue to handle gpio event from isr
    keypad_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(keypad_task_example, "keypad_task_example", 2048, NULL, 10, NULL);


    // int cnt = 0;
    // while(1) {
    //     printf("cnt: %d\n", cnt++);
    //     vTaskDelay(1000 / portTICK_RATE_MS);
    // }
}
