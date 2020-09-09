/*
 * Copyright (c) 2017, George Oikonomou - http://www.spd.gr
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "batmon-sensor.h"
#include "board-peripherals.h"
#include "lwsec-pubsub-system.h"

#include <string.h>
#include <stdio.h>

#include "os/sys/log.h"

/*---------------------------------------------------------------------------*/
#define LOG_MODULE "LWPubSub"
#ifdef MQTT_CLIENT_CONF_LOG_LEVEL
#define LOG_LEVEL MQTT_CLIENT_CONF_LOG_LEVEL
#else
#define LOG_LEVEL LOG_LEVEL_NONE
#endif
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
#define TMP_BUF_SZ 32
/*---------------------------------------------------------------------------*/
char tmp_buf[TMP_BUF_SZ];
/*---------------------------------------------------------------------------*/
static char *
temp_reading(void)
{
  memset(tmp_buf, 0, TMP_BUF_SZ);
  int value;
  // snprintf(tmp_buf, TMP_BUF_SZ, "%d",
  //          batmon_sensor.value(BATMON_SENSOR_TYPE_TEMP));


 /* TMP_007_SENSOR TEST */
           // value = tmp_007_sensor.value(TMP_007_SENSOR_TYPE_ALL);
           //
           // if(value == CC26XX_SENSOR_READING_ERROR) {
           //   printf("TMP: Ambient Read Error\n");
           //   snprintf(tmp_buf, TMP_BUF_SZ, "%d",
           //             tmp_007_sensor.value(TMP_007_SENSOR_TYPE_AMBIENT));
           // }
           //
           // value = tmp_007_sensor.value(TMP_007_SENSOR_TYPE_AMBIENT);
           // printf("\nTMP: Ambient=%d.%02d C\n", value / 1000, value % 1000);
           // snprintf(tmp_buf, TMP_BUF_SZ, "%d.%02d", value / 1000, value % 1000);
           //
           // value = tmp_007_sensor.value(TMP_007_SENSOR_TYPE_OBJECT);
           // printf("TMP: Object=%d.%02d C\n", value / 1000, value % 1000);

           //SENSORS_DEACTIVATE(tmp_007_sensor);


           value = hdc_1000_sensor.value(HDC_1000_SENSOR_TYPE_TEMP);

           if(value != CC26XX_SENSOR_READING_ERROR) {
             LOG_DBG("HDC: Temp=%d.%02d C\n", value / 100, value % 100);
           } else {
//             printf("HDC: Temp Read Error\n");
             LOG_ERR("HDC: ERROR! -> %d.%02d C\n", value / 100, value % 100);
           }
           snprintf(tmp_buf, TMP_BUF_SZ, "%d.%02d", value / 100, value % 100);
           //SENSORS_DEACTIVATE(batmon_sensor);
         //  SENSORS_ACTIVATE(tmp_007_sensor);
           //SENSORS_DEACTIVATE(hdc_1000_sensor);


  return tmp_buf;
}
/*---------------------------------------------------------------------------*/
static void
temp_init(void)
{
  SENSORS_ACTIVATE(batmon_sensor);
//  SENSORS_ACTIVATE(tmp_007_sensor);
  SENSORS_ACTIVATE(hdc_1000_sensor);
}
/*---------------------------------------------------------------------------*/
const mqtt_client_extension_t builtin_sensors_batmon_temp = {
  temp_init,
  temp_reading,
};
