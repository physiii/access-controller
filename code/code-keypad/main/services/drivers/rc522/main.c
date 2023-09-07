/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/****************************************************************************
*
* This file is for gatt server. It can send adv data, be connected by client.
* Run the gatt_client demo, the client demo will automatically connect to the gatt_server demo.
* Client demo will enable gatt_server's notify after connection. Then two devices will exchange
* data.
*
****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include "sdkconfig.h"

#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/semphr.h"

#include "rc522_2.h"
#include "rc522_2.c"

#ifndef MAIN_MAINAPP_H_
#define MAIN_MAINAPP_H_
#include "myTypeDef.h"
#endif /* MAIN_MAINAPP_H_ */


void rc522_init()
{
    initRc522();
    return 0;
}
