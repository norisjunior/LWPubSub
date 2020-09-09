/*
13-03-2020
Vamos ver
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (c) 2017, George Oikonomou - http://www.spd.gr
 * Copyright (c) 2020, Noris Junior - norisjunior@{usp.br, gmail.com)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
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
#include "net/routing/routing.h"
#include "mqtt.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-icmp6.h"
#include "net/ipv6/sicslowpan.h"
#include "sys/etimer.h"
#include "sys/ctimer.h"
#include "lib/sensors.h"
#include "dev/button-hal.h"
#include "dev/leds.h"
#include "os/sys/log.h"
#include "lwsec-pubsub-system.h"
#if BOARD_SENSORTAG
#include "board-peripherals.h"
#include "ti-lib.h"
#endif


//LWPubSub - AES functions
#include "lib/tiny-aes.h"

#include <stdio.h> /* For printf() */

#include <string.h>
#include <strings.h>

#include "lib/random.h"

//Noris
#if (ENERGEST_CONF_ON == 1)
  #include "sys/energest.h"
#endif /* ENERGEST_ON */

/*---------------------------------------------------------------------------*/
#define LOG_MODULE "LWPubSub"
#ifdef MQTT_CLIENT_CONF_LOG_LEVEL
#define LOG_LEVEL MQTT_CLIENT_CONF_LOG_LEVEL
#else
#define LOG_LEVEL LOG_LEVEL_NONE
#endif
/*---------------------------------------------------------------------------*/
/* Controls whether the example will work in IBM Watson IoT platform mode */
#ifdef MQTT_CLIENT_CONF_WITH_IBM_WATSON
#define MQTT_CLIENT_WITH_IBM_WATSON MQTT_CLIENT_CONF_WITH_IBM_WATSON
#else
#define MQTT_CLIENT_WITH_IBM_WATSON 0
#endif
/*---------------------------------------------------------------------------*/
/* MQTT broker address. Ignored in Watson mode */
#ifdef MQTT_CLIENT_CONF_BROKER_IP_ADDR
#define MQTT_CLIENT_BROKER_IP_ADDR MQTT_CLIENT_CONF_BROKER_IP_ADDR
#else
#define MQTT_CLIENT_BROKER_IP_ADDR "fd00::1"
#endif
/*---------------------------------------------------------------------------*/
/*
 * MQTT Org ID.
 *
 * If it equals "quickstart", the client will connect without authentication.
 * In all other cases, the client will connect with authentication mode.
 *
 * In Watson mode, the username will be "use-token-auth". In non-Watson mode
 * the username will be MQTT_CLIENT_USERNAME.
 *
 * In all cases, the password will be MQTT_CLIENT_AUTH_TOKEN.
 */
#ifdef MQTT_CLIENT_CONF_ORG_ID
#define MQTT_CLIENT_ORG_ID MQTT_CLIENT_CONF_ORG_ID
#else
#define MQTT_CLIENT_ORG_ID "LWPubSub"
#endif
/*---------------------------------------------------------------------------*/
/* MQTT token */
#ifdef MQTT_CLIENT_CONF_AUTH_TOKEN
#define MQTT_CLIENT_AUTH_TOKEN MQTT_CLIENT_CONF_AUTH_TOKEN
#else
#define MQTT_CLIENT_AUTH_TOKEN "password"
#endif
/*---------------------------------------------------------------------------*/
#if MQTT_CLIENT_WITH_IBM_WATSON
/* With IBM Watson support */
static const char *broker_ip = "0064:ff9b:0000:0000:0000:0000:b8ac:7cbd";
#define MQTT_CLIENT_USERNAME "use-token-auth"

#else /* MQTT_CLIENT_WITH_IBM_WATSON */
/* Without IBM Watson support. To be used with other brokers, e.g. Mosquitto */
static const char *broker_ip = MQTT_CLIENT_BROKER_IP_ADDR;

/*
#ifdef MQTT_CLIENT_CONF_USERNAME
#define MQTT_CLIENT_USERNAME MQTT_CLIENT_CONF_USERNAME
#else
#define MQTT_CLIENT_USERNAME "use-token-auth"
#endif
*/

#endif /* MQTT_CLIENT_WITH_IBM_WATSON */
/*---------------------------------------------------------------------------*/
#ifdef MQTT_CLIENT_CONF_STATUS_LED
#define MQTT_CLIENT_STATUS_LED MQTT_CLIENT_CONF_STATUS_LED
#else
#define MQTT_CLIENT_STATUS_LED LEDS_GREEN
#endif
/*---------------------------------------------------------------------------*/
#ifdef MQTT_CLIENT_CONF_WITH_EXTENSIONS
#define MQTT_CLIENT_WITH_EXTENSIONS MQTT_CLIENT_CONF_WITH_EXTENSIONS
#else
#define MQTT_CLIENT_WITH_EXTENSIONS 0
#endif
/*---------------------------------------------------------------------------*/
/*
 * A timeout used when waiting for something to happen (e.g. to connect or to
 * disconnect)
 */
#define STATE_MACHINE_PERIODIC     (CLOCK_SECOND >> 1)
/*---------------------------------------------------------------------------*/
/* Provide visible feedback via LEDS during various states */
/* When connecting to broker */
#define CONNECTING_LED_DURATION    (CLOCK_SECOND >> 2)

/* Each time we try to publish */
#define PUBLISH_LED_ON_DURATION    (CLOCK_SECOND)
/*---------------------------------------------------------------------------*/
/* Connections and reconnections */
#define RETRY_FOREVER              0xFF
#define RECONNECT_INTERVAL         (CLOCK_SECOND * 2)

/*
 * Number of times to try reconnecting to the broker.
 * Can be a limited number (e.g. 3, 10 etc) or can be set to RETRY_FOREVER
 */
#define RECONNECT_ATTEMPTS         RETRY_FOREVER
#define CONNECTION_STABLE_TIME     (CLOCK_SECOND * 5)
static struct timer connection_life;
static uint8_t connect_attempt;
/*---------------------------------------------------------------------------*/
/* Various states */
static uint8_t state;
#define STATE_INIT            0
#define STATE_REGISTERED      1
#define STATE_CONNECTING      2
#define STATE_CONNECTED       3
#define STATE_PUBLISHING      4
#define STATE_DISCONNECTED    5
#define STATE_NEWCONFIG       6
#define STATE_CONFIG_ERROR 0xFE
#define STATE_ERROR        0xFF
/*---------------------------------------------------------------------------*/
#define CONFIG_ORG_ID_LEN        32
#define CONFIG_TYPE_ID_LEN       32
#define CONFIG_AUTH_TOKEN_LEN    32
#define CONFIG_EVENT_TYPE_ID_LEN 32
#define CONFIG_CMD_TYPE_LEN       8
#define CONFIG_IP_ADDR_STR_LEN   64
/*---------------------------------------------------------------------------*/
/* A timeout used when waiting to connect to a network */
#define NET_CONNECT_PERIODIC        (CLOCK_SECOND >> 2)
#define NO_NET_LED_DURATION         (NET_CONNECT_PERIODIC >> 1)
/*---------------------------------------------------------------------------*/
/* Default configuration values */
#define DEFAULT_TYPE_ID             "mqtt-client"
#define DEFAULT_EVENT_TYPE_ID       "status"
#define DEFAULT_SUBSCRIBE_CMD_TYPE  "+"
#define DEFAULT_BROKER_PORT         1883
//Noris
//A variável passada no Makefile é: LWPUBSUB_POLLFREQUENCY
//#define DEFAULT_PUBLISH_INTERVAL    (30 * CLOCK_SECOND)
/*---------------------------------------------------------------------------*/
#ifndef LWPUBSUB_POLLFREQUENCY
#define LWPUBSUB_POLLFREQUENCY 15
#endif
/*---------------------------------------------------------------------------*/
#define DEFAULT_PUBLISH_INTERVAL    (LWPUBSUB_POLLFREQUENCY * CLOCK_SECOND)

#define DEFAULT_KEEP_ALIVE_TIMER    14400

#define DEFAULT_RSSI_MEAS_INTERVAL  (CLOCK_SECOND * 30)
/*---------------------------------------------------------------------------*/
#define MQTT_CLIENT_SENSOR_NONE     (void *)0xFFFFFFFF
/*---------------------------------------------------------------------------*/
/* Payload length of ICMPv6 echo requests used to measure RSSI with def rt */
#define ECHO_REQ_PAYLOAD_LEN   20
/*---------------------------------------------------------------------------*/
/* Variable used to inform publish() to do not print energest values When
   startting from commandReceived */
static int publish_from_command = 0;





/*---------------------------------------------------------------------------*/
/************************ SECURITY FUNCTIONS *********************************/
static uint8_t iv[16] = {0};

#ifdef LWPUBSUB_IS_ENCRYPTED
static void generate_iv()
{
  //Random initialization
  random_init(random_rand() + sicslowpan_get_last_rssi());

  for (int i = 0; i < 16; i++) {
    iv[i] = rand();
  }
}
#endif

#if (LOG_LEVEL == LOG_LEVEL_DBG)
static void dump(uint8_t* str, int len)
{
  unsigned char i;
  for (i = 0; i < len; ++i) {
    if (i % 4 == 0) { printf("  "); }
    if (i % 16 == 0) { printf("\n"); }
    printf("%.2x ", str[i]);
  }
  printf("\n");
}

static void phex(uint8_t* str)
{
  #ifdef LWPUBSUB_IS_ENCRYPTED
  unsigned char i;
  for (i = 0; i < KEYSIZE/8; ++i) {
    if (i % 4 == 0) { printf("  "); }
    if (i % 16 == 0) { printf("\n"); }
    printf("%.2x ", str[i]);
	}
  #endif
  printf("\n");
}
#endif  /* IF LOG_LEVEL_DBG - PHEX and DUMP */


/**** CRYPTO MODES ****/
#if (CRYPTOMODE == 1) //ECB
static void encrypt_ecb(uint8_t *AESkey, uint8_t *plain, int plain_size)
{
    struct AES_ctx ctx;
    uint8_t i;
    int iterations = 0;

    iterations = plain_size / 16;

    AES_init_ctx(&ctx, AESkey);
    for (i = 0; i < iterations; ++i)
    {
      AES_ECB_encrypt(&ctx, plain + (i * 16));
    }
    //GaD
}
#elif (CRYPTOMODE == 2) //CBC
static void encrypt_cbc(uint8_t *AESkey, uint8_t *plain, uint8_t *iniv, int plain_size)
{

  #if (ENERGEST_CONF_ON == 1)
    energest_flush();
    LOG_INFO("EncryptCBC start ");
    printf("E_CPU %llu E_LPM %llu E_DEEP_LPM %llu E_TX %llu E_RX %llu E_Total: %llu\n",
      energest_type_time(ENERGEST_TYPE_CPU), energest_type_time(ENERGEST_TYPE_LPM), energest_type_time(ENERGEST_TYPE_DEEP_LPM),
      energest_type_time(ENERGEST_TYPE_TRANSMIT), energest_type_time(ENERGEST_TYPE_LISTEN), ENERGEST_GET_TOTAL_TIME());
  #endif /* ENERGEST_CONF_ON */

  struct AES_ctx ctx;

  AES_init_ctx_iv(&ctx, AESkey, iniv);
  AES_CBC_encrypt_buffer(&ctx, plain, plain_size);

  //GaD
  #if (ENERGEST_CONF_ON == 1)
    energest_flush();
    LOG_INFO("EncryptCBC finish ");
    printf("E_CPU %llu E_LPM %llu E_DEEP_LPM %llu E_TX %llu E_RX %llu E_Total: %llu\n",
      energest_type_time(ENERGEST_TYPE_CPU), energest_type_time(ENERGEST_TYPE_LPM), energest_type_time(ENERGEST_TYPE_DEEP_LPM),
      energest_type_time(ENERGEST_TYPE_TRANSMIT), energest_type_time(ENERGEST_TYPE_LISTEN), ENERGEST_GET_TOTAL_TIME());
  #endif /* ENERGEST_CONF_ON */
}
#elif (CRYPTOMODE == 3) //CTR
static void encrypt_ctr(uint8_t *AESkey, uint8_t *plain, uint8_t *iniv, int plain_size)
{

  #if (ENERGEST_CONF_ON == 1)
    energest_flush();
    LOG_INFO("EncryptCTR start ");
    printf("E_CPU %llu E_LPM %llu E_DEEP_LPM %llu E_TX %llu E_RX %llu E_Total: %llu\n",
      energest_type_time(ENERGEST_TYPE_CPU), energest_type_time(ENERGEST_TYPE_LPM), energest_type_time(ENERGEST_TYPE_DEEP_LPM),
      energest_type_time(ENERGEST_TYPE_TRANSMIT), energest_type_time(ENERGEST_TYPE_LISTEN), ENERGEST_GET_TOTAL_TIME());
  #endif /* ENERGEST_CONF_ON */

  struct AES_ctx ctx;

  AES_init_ctx_iv(&ctx, AESkey, iniv);
  AES_CTR_xcrypt_buffer(&ctx, plain, plain_size);

  //GaD
  #if (ENERGEST_CONF_ON == 1)
    energest_flush();
    LOG_INFO("EncryptCTR finish ");
    printf("E_CPU %llu E_LPM %llu E_DEEP_LPM %llu E_TX %llu E_RX %llu E_Total: %llu\n",
      energest_type_time(ENERGEST_TYPE_CPU), energest_type_time(ENERGEST_TYPE_LPM), energest_type_time(ENERGEST_TYPE_DEEP_LPM),
      energest_type_time(ENERGEST_TYPE_TRANSMIT), energest_type_time(ENERGEST_TYPE_LISTEN), ENERGEST_GET_TOTAL_TIME());
  #endif /* ENERGEST_CONF_ON */
}

#endif //CRYPTOMODE
/**** CRYPTO MODES ****/


//Decryption
static void decrypt_cbc(uint8_t *AESkey, uint8_t *ciphered, uint8_t *iniv, int size)
{

  #if (ENERGEST_CONF_ON == 1)
    energest_flush();
    LOG_INFO("DecryptCBC start ");
    printf("E_CPU %llu E_LPM %llu E_DEEP_LPM %llu E_TX %llu E_RX %llu E_Total: %llu\n",
      energest_type_time(ENERGEST_TYPE_CPU), energest_type_time(ENERGEST_TYPE_LPM), energest_type_time(ENERGEST_TYPE_DEEP_LPM),
      energest_type_time(ENERGEST_TYPE_TRANSMIT), energest_type_time(ENERGEST_TYPE_LISTEN), ENERGEST_GET_TOTAL_TIME());
  #endif /* ENERGEST_CONF_ON */

  struct AES_ctx ctx;

  AES_init_ctx_iv(&ctx, AESkey, iniv);
  AES_CBC_decrypt_buffer(&ctx, ciphered, size);

  //GaD
  #if (ENERGEST_CONF_ON == 1)
    energest_flush();
    LOG_INFO("DecryptCBC finish ");
    printf("E_CPU %llu E_LPM %llu E_DEEP_LPM %llu E_TX %llu E_RX %llu E_Total: %llu\n",
      energest_type_time(ENERGEST_TYPE_CPU), energest_type_time(ENERGEST_TYPE_LPM), energest_type_time(ENERGEST_TYPE_DEEP_LPM),
      energest_type_time(ENERGEST_TYPE_TRANSMIT), energest_type_time(ENERGEST_TYPE_LISTEN), ENERGEST_GET_TOTAL_TIME());
  #endif /* ENERGEST_CONF_ON */
}


static void decrypt_ctr(uint8_t *AESkey, uint8_t *ciphered, uint8_t *iniv, int size)
{

  #if (ENERGEST_CONF_ON == 1)
    energest_flush();
    LOG_INFO("DecryptCTR start ");
    printf("E_CPU %llu E_LPM %llu E_DEEP_LPM %llu E_TX %llu E_RX %llu E_Total: %llu\n",
      energest_type_time(ENERGEST_TYPE_CPU), energest_type_time(ENERGEST_TYPE_LPM), energest_type_time(ENERGEST_TYPE_DEEP_LPM),
      energest_type_time(ENERGEST_TYPE_TRANSMIT), energest_type_time(ENERGEST_TYPE_LISTEN), ENERGEST_GET_TOTAL_TIME());
  #endif /* ENERGEST_CONF_ON */

  struct AES_ctx ctx;

  AES_init_ctx_iv(&ctx, AESkey, iniv);
  AES_CTR_xcrypt_buffer(&ctx, ciphered, size);

  //GaD
  #if (ENERGEST_CONF_ON == 1)
    energest_flush();
    LOG_INFO("DecryptCTR finish ");
    printf("E_CPU %llu E_LPM %llu E_DEEP_LPM %llu E_TX %llu E_RX %llu E_Total: %llu\n",
      energest_type_time(ENERGEST_TYPE_CPU), energest_type_time(ENERGEST_TYPE_LPM), energest_type_time(ENERGEST_TYPE_DEEP_LPM),
      energest_type_time(ENERGEST_TYPE_TRANSMIT), energest_type_time(ENERGEST_TYPE_LISTEN), ENERGEST_GET_TOTAL_TIME());
  #endif /* ENERGEST_CONF_ON */
}



//#endif /* DEFINING_ENCRIPTION_FUNCTION */

//LWPubSub - AES Keys -- DEFINE YOUR OWN KEYS!

// static uint8_t lwpubsub_aes128_key[16] = {0x4e, 0x6f, 0x72, 0x69, 0x73, 0x50, 0x41, 0x44, 0x50, 0x6f, 0x6c, 0x69, 0x55, 0x53, 0x50, 0x21}; //AES 128 Key
// static uint8_t lwpubsub_aes192_key[24] = {0x4e, 0x6f, 0x72, 0x69, 0x73, 0x4a, 0x75, 0x6e, 0x69, 0x6f, 0x72, 0x50, 0x41, 0x44, 0x4c, 0x53, 0x49, 0x50, 0x6f, 0x6c, 0x69, 0x55, 0x53, 0x50}; //AES 192 Key
// static uint8_t lwpubsub_aes256_key[36] = {0x4e, 0x6f, 0x72, 0x69, 0x73, 0x4a, 0x75, 0x6e, 0x69, 0x6f, 0x72, 0x50, 0x41, 0x44, 0x4c, 0x53, 0x49, 0x50, 0x6f, 0x6c, 0x69, 0x74, 0x65, 0x63, 0x6e, 0x69, 0x63, 0x61, 0x55, 0x53, 0x50, 0x21}; // AES 256 Key

#if defined (AES128)
  static uint8_t lwpubsub_aes_key[KEYSIZE/8] = {0x4e, 0x6f, 0x72, 0x69, 0x73, 0x50, 0x41, 0x44, 0x50, 0x6f, 0x6c, 0x69, 0x55, 0x53, 0x50, 0x21}; //AES 128 Key
#elif defined (AES192)
  static uint8_t lwpubsub_aes_key[KEYSIZE/8] = {0x4e, 0x6f, 0x72, 0x69, 0x73, 0x4a, 0x75, 0x6e, 0x69, 0x6f, 0x72, 0x50, 0x41, 0x44, 0x4c, 0x53, 0x49, 0x50, 0x6f, 0x6c, 0x69, 0x55, 0x53, 0x50}; //AES 192 Key
#elif defined (AES256)
  static uint8_t lwpubsub_aes_key[KEYSIZE/8] = {0x4e, 0x6f, 0x72, 0x69, 0x73, 0x4a, 0x75, 0x6e, 0x69, 0x6f, 0x72, 0x50, 0x41, 0x44, 0x4c, 0x53, 0x49, 0x50, 0x6f, 0x6c, 0x69, 0x74, 0x65, 0x63, 0x6e, 0x69, 0x63, 0x61, 0x55, 0x53, 0x50, 0x21}; // AES 256 Key
#else
  static uint8_t lwpubsub_aes_key[1] = {0};
#endif /* lwpubsub_aes_key */


/************************ SECURITY FUNCTIONS *********************************/
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/

static char metadata[20];
static char objectID[5];
static char instanceID[1];
static char commandReceived[1];


static void parsePayload(uint8_t* mqttPayload, int mqttPayload_len)
{
  int separator = 0;
  int start = 0;
  int i = 0;

#if (LOG_LEVEL == LOG_LEVEL_DBG)
  LOG_DBG("mqttPayload decrypted: "); phex(mqttPayload); printf("\n");
#endif


  if(mqttPayload[0] == 0x30) { // No security - for testing purposes
    start = 1;
    LOG_DBG("parsePayload function - NO SECURITY MQTT payload\n");
  }

  for (i = start; i < mqttPayload_len; ++i) {
    if(mqttPayload[i] == 0x7c) {
      separator = 1;
      continue;
    } else {
      if(separator == 0 && start == 0) {
        metadata[i] = (char)mqttPayload[i];
      } else if(separator == 0 && start == 1) {
        metadata[i-1] = (char)mqttPayload[i];
      } else if(separator == 1){
        //printf("Received command (commandReceived): %c, i = %d\n", (char)mqttPayload[i], i);
        metadata[i] = (char)mqttPayload[i];
        //commandReceived[0] = (char)mqttPayload[i];
        break; //command of only one Byte size
      }
    }
  }

  objectID[0] = metadata[0];
  objectID[1] = metadata[1];
  objectID[2] = metadata[2];
  objectID[3] = metadata[3];
  objectID[4] = '\0';
  instanceID[0] = metadata[4];
  commandReceived[0] = metadata[6];

}


/*---------------------------------------------------------------------------*/










/*---------------------------------------------------------------------------*/
PROCESS_NAME(mqtt_client_process);
AUTOSTART_PROCESSES(&mqtt_client_process);
/*---------------------------------------------------------------------------*/
/**
 * \brief Data structure declaration for the MQTT client configuration
 */
typedef struct mqtt_client_config {
  char org_id[CONFIG_ORG_ID_LEN];
  char type_id[CONFIG_TYPE_ID_LEN];
  char auth_token[CONFIG_AUTH_TOKEN_LEN];
  char event_type_id[CONFIG_EVENT_TYPE_ID_LEN];
  char broker_ip[CONFIG_IP_ADDR_STR_LEN];
  char cmd_type[CONFIG_CMD_TYPE_LEN];
  clock_time_t pub_interval;
  int def_rt_ping_interval;
  uint16_t broker_port;
} mqtt_client_config_t;
/*---------------------------------------------------------------------------*/
/* Maximum TCP segment size for outgoing segments of our socket */
// Noris Max from 32 to 100
#define MAX_TCP_SEGMENT_SIZE    100
/*---------------------------------------------------------------------------*/
/*
 * Buffers for Client ID and Topic.
 * Make sure they are large enough to hold the entire respective string
 *
 * We also need space for the null termination
 */
//LWPubSub - Buffer size at least 120
#define BUFFER_SIZE 120
static char client_id[BUFFER_SIZE];
static char pub_topic[BUFFER_SIZE];
static char sub_topic[BUFFER_SIZE];
/*---------------------------------------------------------------------------*/
/*
 * The main MQTT buffers.
 * We will need to increase if we start publishing more data.
 */
#define APP_BUFFER_SIZE 512
static struct mqtt_connection conn;
static char app_buffer[APP_BUFFER_SIZE];
/*---------------------------------------------------------------------------*/
#define QUICKSTART "quickstart"
/*---------------------------------------------------------------------------*/
static struct mqtt_message *msg_ptr = 0;
static struct etimer publish_periodic_timer;
static struct ctimer ct;
static char *buf_ptr;
static uint16_t seq_nr_value = 0;
/*---------------------------------------------------------------------------*/
/* Parent RSSI functionality */
static struct uip_icmp6_echo_reply_notification echo_reply_notification;
static struct etimer echo_request_timer;
static int def_rt_rssi = 0;
/*---------------------------------------------------------------------------*/
static mqtt_client_config_t conf;
/*---------------------------------------------------------------------------*/
#if MQTT_CLIENT_WITH_EXTENSIONS
extern const mqtt_client_extension_t *mqtt_client_extensions[];
extern const uint8_t mqtt_client_extension_count;
#else
static const mqtt_client_extension_t *mqtt_client_extensions[] = { NULL };
static const uint8_t mqtt_client_extension_count = 0;
#endif
/*---------------------------------------------------------------------------*/
//LWPubSub
PROCESS(mqtt_client_process, "LWPubSub System for IoT - MQTT client process");
/*---------------------------------------------------------------------------*/
static bool
have_connectivity(void)
{
  if(uip_ds6_get_global(ADDR_PREFERRED) == NULL ||
     uip_ds6_defrt_choose() == NULL) {
    return false;
  }
  return true;
}
/*---------------------------------------------------------------------------*/
/*
static int
ipaddr_sprintf(char *buf, uint8_t buf_len, const uip_ipaddr_t *addr)
{
  uint16_t a;
  uint8_t len = 0;
  int i, f;
  for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
    a = (addr->u8[i] << 8) + addr->u8[i + 1];
    if(a == 0 && f >= 0) {
      if(f++ == 0) {
        len += snprintf(&buf[len], buf_len - len, "::");
      }
    } else {
      if(f > 0) {
        f = -1;
      } else if(i > 0) {
        len += snprintf(&buf[len], buf_len - len, ":");
      }
      len += snprintf(&buf[len], buf_len - len, "%x", a);
    }
  }

  return len;
}
*/
/*---------------------------------------------------------------------------*/
static void
echo_reply_handler(uip_ipaddr_t *source, uint8_t ttl, uint8_t *data,
                   uint16_t datalen)
{
  if(uip_ip6addr_cmp(source, uip_ds6_defrt_choose())) {
    def_rt_rssi = sicslowpan_get_last_rssi();
  }
}
/*---------------------------------------------------------------------------*/
static void
publish_led_off(void *d)
{
  leds_off(MQTT_CLIENT_STATUS_LED);
}
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
static void
publish(void)
{

  if (!publish_from_command) {
    #if (ENERGEST_CONF_ON == 1)
      energest_flush();
      LOG_INFO("Publish start ");
      printf("E_CPU %llu E_LPM %llu E_DEEP_LPM %llu E_TX %llu E_RX %llu E_Total: %llu\n",
        energest_type_time(ENERGEST_TYPE_CPU), energest_type_time(ENERGEST_TYPE_LPM), energest_type_time(ENERGEST_TYPE_DEEP_LPM),
        energest_type_time(ENERGEST_TYPE_TRANSMIT), energest_type_time(ENERGEST_TYPE_LISTEN), ENERGEST_GET_TOTAL_TIME());
    #endif /* ENERGEST_CONF_ON */
  }

  /* LWPubSub System for IoT */
  int len;
  int remaining = APP_BUFFER_SIZE;
  int payload_size = 0;

  seq_nr_value++;

  buf_ptr = app_buffer;

  //LWPubSub - Experiment uses only temperature objectID/instanceID
  if(LWPUBSUB_IS_ENCRYPTED) {
    len = snprintf(buf_ptr, remaining, "33030|");
  } else {
    len = snprintf(buf_ptr, remaining, "033030|");
  }

  if(len < 0 || len >= remaining) {
    LOG_ERR("Buffer too short. Have %d, need %d + \\0\n", remaining,
            len);
    return;
  }

  remaining -= len;
  buf_ptr += len;
  payload_size += len;



  //LWPubSub - check board/extensions
  if (mqtt_client_extension_count != 0) {
    len = snprintf(buf_ptr, remaining, "%s", mqtt_client_extensions[0]->value());
    LOG_DBG("Client has extensions\n");
  } else {
    // Native target - send dummy measurement
    len = snprintf(buf_ptr, remaining, "%d.%d", (10+rand()%10), (rand()%10));
  }


//#if (LOG_LEVEL == LOG_LEVEL_DBG)
  LOG_DBG("App_Buffer.: %s\n", app_buffer); //printf("\n");
//#endif  /* IF LOG_LEVEL_DBG */


  if(len < 0 || len >= remaining) {
    LOG_ERR("Buffer too short. Have %d, need %d + \\0\n", remaining,
            len);
    return;
  }

  remaining -= len;
  buf_ptr += len;
  payload_size += len;


  /* Cipher payload */
  uint8_t plain_text[64] = {0};

  int plain_text_size = 0;

  uint8_t i = 0;
  uint8_t j = 0;

  for (i = 0; i < payload_size; i++) {
  	plain_text[i] = app_buffer[i];
  }

  if (payload_size >= 64) {
    LOG_ERR("Buffer too long, plain_text > 64 bytes\n");
    return;
  } else {
    if (payload_size <= 16) {
      plain_text_size = 16;
    } else if (payload_size > 16 && payload_size <= 32) {
      plain_text_size = 32;
    } else if (payload_size > 32 && payload_size <= 48) {
      plain_text_size = 48;
    } else if (payload_size > 48 && payload_size <= 64) {
      plain_text_size = 64;
    }
  }

  /* Debugging */

#if (LOG_LEVEL == LOG_LEVEL_DBG)
  LOG_DBG("Key: "); phex(lwpubsub_aes_key); printf("\n");
  LOG_DBG("Plaintext size: %d\n", plain_text_size); printf("\n");
  LOG_DBG("Plaintext before encr.: "); dump(plain_text, plain_text_size); printf("\n");
#endif /* IF LOG_LEVEL_DBG */

#if (CRYPTOMODE == 1) //ECB
  encrypt_ecb(lwpubsub_aes_key, plain_text, plain_text_size);
#elif (CRYPTOMODE == 2) //CBC
  generate_iv();
  #if (LOG_LEVEL == LOG_LEVEL_DBG)
    LOG_DBG("CBC\nIV: "); dump(iv, 16); printf("\n");
  #endif /* IF LOG_LEVEL_DBG */
  encrypt_cbc(lwpubsub_aes_key, plain_text, iv, plain_text_size);
#elif (CRYPTOMODE == 3) //CTR
  generate_iv();
  #if (LOG_LEVEL == LOG_LEVEL_DBG)
    LOG_DBG("CTR\nIV: "); dump(iv, 16); printf("\n");
  #endif /* IF LOG_LEVEL_DBG */
  encrypt_ctr(lwpubsub_aes_key, plain_text, iv, plain_text_size);
#endif /* ENCRYPTION_TYPE */

#if (LOG_LEVEL == LOG_LEVEL_DBG)
  LOG_DBG("\nPlaintext after encr.: "); dump(plain_text, plain_text_size); printf("\n");
#endif

  /* Secure MQTT Payload construction
     App_buffer (LWPubSub):
       1st byte: encryption type
       2nd to 17th byte: IV
       18th to end: encrypted msg
  */

  if(LWPUBSUB_IS_ENCRYPTED) {

    app_buffer[0] = LWPUBSUB_CRYPTO_MODE;
    //#if (LOG_LEVEL == LOG_LEVEL_DBG)
      LOG_DBG("Crypto - LWPUBSUB_CRYPTO_MODE: %02X\n", (char)app_buffer[0]);
    //#endif

    for (i = 1; i < 17; i++) {
      app_buffer[i] = iv[i-1];
    }

    for (i = 17, j = 0; i < (plain_text_size+17); i++, j++) {
      app_buffer[i] = (char)plain_text[j];
    }

  }

#if (LOG_LEVEL == LOG_LEVEL_DBG) && (LWPUBSUB_IS_ENCRYPTED == 1)
  LOG_DBG("App_Buffer.: "); dump((uint8_t *)app_buffer, (plain_text_size+17)); printf("\n");
#endif

  //LWPubSub:
  //1. 1st byte:
  //   00 - no security
  //   01 - AES-CBC-128
  //   02 - AES-CBC-192
  //   03 - AES-CBC-256
  //   11 - AES-CTR-128
  //   12 - AES-CTR-192
  //   13 - AES-CTR-256
  //2. Cloud platform IoT Agent:
  //   Byte  1 --------- (security algorithm) +
  //   Bytes 2 a 17 ---- (random IV) +
  //   Bytes 18 a 64  -- (MQTT payload - LWSec PubSub System for IoT message)

  //GaD

  if(LWPUBSUB_IS_ENCRYPTED) {
    mqtt_publish(&conn, NULL, pub_topic, (uint8_t *)app_buffer,
                 (plain_text_size+17), MQTT_QOS_LEVEL_0, MQTT_RETAIN_OFF);
  } else {
    mqtt_publish(&conn, NULL, pub_topic, (uint8_t *)app_buffer,
                 strlen(app_buffer), MQTT_QOS_LEVEL_0, MQTT_RETAIN_OFF);
  }

  LOG_DBG("Publish!\n");

  printf("RTIMER_NOW: %lu\n", RTIMER_NOW());

}
/*---------------------------------------------------------------------------*/



/*---CommandReceived - aka Publish Handler (receiving publish from Broker)---*/
/*---------------------------------------------------------------------------*/
static void
pub_handler(const char *topic, uint16_t topic_len, const uint8_t *chunk,
            uint16_t chunk_len)
{

  #if (ENERGEST_CONF_ON == 1)
    energest_flush();
    LOG_INFO("CommandReceived start ");
    printf("E_CPU %llu E_LPM %llu E_DEEP_LPM %llu E_TX %llu E_RX %llu E_Total: %llu\n",
      energest_type_time(ENERGEST_TYPE_CPU), energest_type_time(ENERGEST_TYPE_LPM), energest_type_time(ENERGEST_TYPE_DEEP_LPM),
      energest_type_time(ENERGEST_TYPE_TRANSMIT), energest_type_time(ENERGEST_TYPE_LISTEN), ENERGEST_GET_TOTAL_TIME());
  #endif /* ENERGEST_CONF_ON */

  LOG_DBG("Pub Handler: topic='%s' (len=%u), chunk_len=%u\n", topic,
          topic_len, chunk_len);
  //LWPubSub
  //Example topic: /99/4b001205257a/cmd ('cmd' to receive command from the cloud)
  //This is defined in the subscribe function

  /* If we don't like the length, ignore */
  /* Fixed topic_len for the article experiment */
  if(topic_len != 20 || chunk_len < 16) {
  //if(topic_len != 20) {
    //LWPubSub message has at least 16 bytes, due to use of AES on encryption
    LOG_ERR("Incorrect topic or chunk len. Ignored\n");
    return;
  }

  /* If the command != cmd, ignore */
  if(strncmp(&topic[topic_len - 3], "cmd", 3) != 0) {
    LOG_ERR("Incorrect command format\n");
  }

  if(strncmp(&topic[17], "cmd", 3) == 0) {
    //We accept only cmd in the last topic part

    #if (LOG_LEVEL == LOG_LEVEL_DBG)
      LOG_DBG("Received message - Chunk: "); dump((uint8_t *)chunk, chunk_len); printf("\n");
      //printf("LWPUBSUB_CRYPTO_MODE: %02X\n", (char)app_buffer[0]);
    #endif


    /* Decrypt the payload */
    uint8_t decrypt_key128[16] = {0x4e, 0x6f, 0x72, 0x69, 0x73, 0x50, 0x41, 0x44, 0x50, 0x6f, 0x6c, 0x69, 0x55, 0x53, 0x50, 0x21}; //AES 128 Key
    uint8_t decrypt_key192[24] = {0x4e, 0x6f, 0x72, 0x69, 0x73, 0x4a, 0x75, 0x6e, 0x69, 0x6f, 0x72, 0x50, 0x41, 0x44, 0x4c, 0x53, 0x49, 0x50, 0x6f, 0x6c, 0x69, 0x55, 0x53, 0x50}; //AES 192 Key
    uint8_t decrypt_key256[32] = {0x4e, 0x6f, 0x72, 0x69, 0x73, 0x4a, 0x75, 0x6e, 0x69, 0x6f, 0x72, 0x50, 0x41, 0x44, 0x4c, 0x53, 0x49, 0x50, 0x6f, 0x6c, 0x69, 0x74, 0x65, 0x63, 0x6e, 0x69, 0x63, 0x61, 0x55, 0x53, 0x50, 0x21}; // AES 256 Key
    uint8_t crypto_mode[1] = {0};
    uint8_t decrypt_iv[16] = {0};
    uint8_t decrypt_message[16] = {0};
    //int keysize = 16; //default keysize for aes-128-cbc


    crypto_mode[0] = chunk[0];


    /* ***** Print the payload ****** */

    /* Split message */
    int i = 0;
    for (i = 0; i < 16; ++i) {
      decrypt_iv[i] = chunk[i+1];
      decrypt_message[i] = chunk[i+17];
    }

    #if (LOG_LEVEL == LOG_LEVEL_DBG)
        LOG_DBG("Crypto_mode: "); dump((uint8_t *)crypto_mode, 1); printf("\n");

        LOG_DBG("IV: "); dump(decrypt_iv, 16); printf("\n");

        LOG_DBG("Encrypted message: "); dump(decrypt_message, 16); printf("\n");
    #endif

    /* ***** Print the payload ****** */



    switch(crypto_mode[0]) {
      case 0x30:
        LOG_INFO("No security\n");
        break;

      case LWPUBSUB_AES_CBC_128:
        LOG_INFO("LWPubSub Message: CBC 128 bit\n");
        #if (LOG_LEVEL == LOG_LEVEL_DBG)
          LOG_DBG("Decrypt_key: "); dump(decrypt_key128, 16); printf("\n");
        #endif
        decrypt_cbc(decrypt_key128, decrypt_message, decrypt_iv, 16); //for now, only supported 16-Byte message size
        break;

      case LWPUBSUB_AES_CBC_192:
        LOG_INFO("LWPubSub Message: CBC 192 bit\n");
        #if (LOG_LEVEL == LOG_LEVEL_DBG)
          LOG_DBG("Decrypt_key: "); dump(decrypt_key192, 24); printf("\n");
        #endif
        decrypt_cbc(decrypt_key192, decrypt_message, decrypt_iv, 16); //for now, only supported 16-Byte message size
        break;

      case LWPUBSUB_AES_CBC_256:
        LOG_INFO("LWPubSub Message: CBC 256 bit\n");
        #if (LOG_LEVEL == LOG_LEVEL_DBG)
          LOG_DBG("Decrypt_key: "); dump(decrypt_key256, 32); printf("\n");
        #endif
        decrypt_cbc(decrypt_key256, decrypt_message, decrypt_iv, 16); //for now, only supported 16-Byte message size
        break;

      case LWPUBSUB_AES_CTR_128:
        LOG_INFO("LWPubSub Message: CTR 128 bit\n");
        #if (LOG_LEVEL == LOG_LEVEL_DBG)
          LOG_DBG("Decrypt_key: "); dump(decrypt_key128, 16); printf("\n");
        #endif
        decrypt_ctr(decrypt_key128, decrypt_message, decrypt_iv, 16); //for now, only supported 16-Byte message size
        break;

      case LWPUBSUB_AES_CTR_192:
        LOG_INFO("LWPubSub Message: CTR 192 bit\n");
        #if (LOG_LEVEL == LOG_LEVEL_DBG)
          LOG_DBG("Decrypt_key: "); dump(decrypt_key192, 24); printf("\n");
        #endif
        decrypt_ctr(decrypt_key192, decrypt_message, decrypt_iv, 16); //for now, only supported 16-Byte message size
        break;

      case LWPUBSUB_AES_CTR_256:
        LOG_INFO("LWPubSub Message: CTR 256 bit\n");
        #if (LOG_LEVEL == LOG_LEVEL_DBG)
          LOG_DBG("Decrypt_key: "); dump(decrypt_key256, 32); printf("\n");
        #endif
        decrypt_ctr(decrypt_key256, decrypt_message, decrypt_iv, 16); //for now, only supported 16-Byte message size
        break;

      default:
        LOG_ERR("Unknown cipher\n");
    }

    parsePayload(decrypt_message, 16);

    LOG_DBG("Chunk - objectID: %s\n", objectID);
    LOG_DBG("Chunk - instanceID: %c\n", instanceID[0]);
    LOG_DBG("Chunk - commandReceived: %c\n\n", commandReceived[0]);

    //http://www.openmobilealliance.org/wp/OMNA/LwM2M/LwM2MRegistry.html

//3311 - Start
    if(strncmp(objectID, "3311", 4) == 0) { //IPSO Light Control
      if(strncmp(instanceID, "0", 1) == 0) { //red LED
        if(strncmp(commandReceived, "1", 1) == 0) {
          LOG_INFO("CommandReceived type execute - red led (3311 0)\n");
          printf("\n> RED Led On <\n\n");
          leds_on(LEDS_RED);
        } else if(strncmp(commandReceived, "0", 1) == 0) {
          LOG_INFO("CommandReceived type execute - red led (3311 0)\n");
          printf("\n> RED Led Off <\n\n");
          leds_off(LEDS_RED);
        } else {
          LOG_ERR("--Wrong command--\n");
        }
      } else if(strncmp(instanceID, "1", 1) == 0) { //green LED {

        if(strncmp(commandReceived, "1", 1) == 0) {
          LOG_INFO("CommandReceived type execute - green led (3311 1)\n");
          printf("\n> GREEN Led On <\n\n");
          leds_on(LEDS_GREEN);
        } else if(strncmp(commandReceived, "0", 1) == 0) {
          LOG_INFO("CommandReceived type execute - green led (3311 1)\n");
          printf("\n> GREEN Led Off <\n\n");
          leds_off(LEDS_GREEN);
        } else {
          LOG_ERR("--Wrong command--\n");
        }
      } else {
        LOG_ERR("Only leds: instanceID 0 - Red and instanceID 1 - Green\n");
      }
//3311 - End

//3338 - Start
    } else if(strncmp(objectID, "3338", 4) == 0) { //IPSO Buzzer/Alarm
      if(strncmp(instanceID, "0", 1) == 0) { //The only buzzer in Sensortag
        if(strncmp(commandReceived, "1", 1) == 0) {
          LOG_INFO("CommandReceived type execute - buzzer (3338)\n");
          printf("\n> Buzzer On <\n\n");
#if BOARD_SENSORTAG
          buzzer_start(1000);
#endif
        } else if(strncmp(commandReceived, "0", 1) == 0) {
          LOG_INFO("CommandReceived type execute - buzzer (3338)\n");
          printf("\n> Buzzer Off <\n\n");
#if BOARD_SENSORTAG
          if(buzzer_state()) {
            buzzer_stop();
          }
#endif
        } else {
          LOG_ERR("--Wrong command--\n");
        }
      } else {
        LOG_ERR("Only one Buzzer instance - instanceID 0\n");
      }
//3338 - End

//3303 - Start
    } else if(strncmp(objectID, "3303", 4) == 0) { //commandReceived request measurement
      if(strncmp(instanceID, "0", 1) == 0) { //The only temperature instance implemented
        if(strncmp(commandReceived, "0", 1) == 0) { //empty data
          LOG_INFO("CommandReceived type request - temperature (3303)\n");
          /* Fire publish from here! */
          LOG_INFO("Publish from commandReceived started\n");
          publish_from_command = 1;
          publish();
          LOG_INFO("Publish from commandReceived finished\n");
        } else {
          LOG_ERR("--Wrong commandReceived--\n");
        }
      } else {
        LOG_ERR("Only one temperature instance implemented - instanceID 0\n");
      }
//3303 - End

    } else {
      LOG_ERR("Not implemented other objectID different than 3311 (led), 3338 (buzzer) and 3303 (temperature)!\n");
    }
//IPSO 3311 and 3338 devices end

#if (ENERGEST_CONF_ON == 1)
    energest_flush();
    LOG_INFO("CommandReceived finish ");
    printf("E_CPU %llu E_LPM %llu E_DEEP_LPM %llu E_TX %llu E_RX %llu E_Total: %llu\n",
      energest_type_time(ENERGEST_TYPE_CPU), energest_type_time(ENERGEST_TYPE_LPM), energest_type_time(ENERGEST_TYPE_DEEP_LPM),
      energest_type_time(ENERGEST_TYPE_TRANSMIT), energest_type_time(ENERGEST_TYPE_LISTEN), ENERGEST_GET_TOTAL_TIME());
#endif /* ENERGEST_CONF_ON */

    publish_from_command = 0;

    return;

  } else {
    LOG_DBG("MQTT CommandReceived 'cmd' incorrect position\n");
    return;
  }
}
/*---------------------------------------------------------------------------*/
/*---------- CommandReceived end ---------*/

/*---------------------------------------------------------------------------*/
static void
mqtt_event(struct mqtt_connection *m, mqtt_event_t event, void *data)
{
  switch(event) {
  case MQTT_EVENT_CONNECTED: {
    LOG_DBG("Application has a MQTT connection\n");
    timer_set(&connection_life, CONNECTION_STABLE_TIME);
    state = STATE_CONNECTED;

    #if (ENERGEST_CONF_ON == 1)
      energest_flush();
      LOG_INFO("Connect finish ");
      printf("E_CPU %llu E_LPM %llu E_DEEP_LPM %llu E_TX %llu E_RX %llu E_Total: %llu\n",
        energest_type_time(ENERGEST_TYPE_CPU), energest_type_time(ENERGEST_TYPE_LPM), energest_type_time(ENERGEST_TYPE_DEEP_LPM),
        energest_type_time(ENERGEST_TYPE_TRANSMIT), energest_type_time(ENERGEST_TYPE_LISTEN), ENERGEST_GET_TOTAL_TIME());
    #endif /* ENERGEST_CONF_ON */

    break;
  }
  case MQTT_EVENT_DISCONNECTED: {
    LOG_DBG("MQTT Disconnect. Reason %u\n", *((mqtt_event_t *)data));

    state = STATE_DISCONNECTED;
    process_poll(&mqtt_client_process);
    break;
  }
  case MQTT_EVENT_PUBLISH: {
    msg_ptr = data;

    /* Implement first_flag in publish message? */
    if(msg_ptr->first_chunk) {
      msg_ptr->first_chunk = 0;
      LOG_DBG("Application received publish for topic '%s'. Payload "
              "size is %i bytes.\n", msg_ptr->topic, msg_ptr->payload_length);
    }

    pub_handler(msg_ptr->topic, strlen(msg_ptr->topic),
                msg_ptr->payload_chunk, msg_ptr->payload_length);
    break;
  }
  case MQTT_EVENT_SUBACK: {
#if MQTT_311
    mqtt_suback_event_t *suback_event = (mqtt_suback_event_t *)data;

    if(suback_event->success) {
      LOG_DBG("Application is subscribed to topic successfully\n");
    } else {
      LOG_DBG("Application failed to subscribe to topic (ret code %x)\n", suback_event->return_code);
    }
#else
    LOG_DBG("Application is subscribed to topic successfully\n");
#endif
#if (ENERGEST_CONF_ON == 1)
    energest_flush();
    LOG_INFO("Subscribe finish ");
    printf("E_CPU %llu E_LPM %llu E_DEEP_LPM %llu E_TX %llu E_RX %llu E_Total: %llu\n",
      energest_type_time(ENERGEST_TYPE_CPU), energest_type_time(ENERGEST_TYPE_LPM), energest_type_time(ENERGEST_TYPE_DEEP_LPM),
      energest_type_time(ENERGEST_TYPE_TRANSMIT), energest_type_time(ENERGEST_TYPE_LISTEN), ENERGEST_GET_TOTAL_TIME());
#endif /* ENERGEST_CONF_ON */

    break;
  }
  case MQTT_EVENT_UNSUBACK: {
    LOG_DBG("Application is unsubscribed to topic successfully\n");
    break;
  }
  case MQTT_EVENT_PUBACK: {
    LOG_DBG("Publishing complete.\n");
    break;
  }
  default:
    LOG_DBG("Application got a unhandled MQTT event: %i\n", event);
    break;
  }
}
/*---------------------------------------------------------------------------*/
static int
construct_pub_topic(void)
{
  //LWPubSub - The same topic used in the article scenario: /99/<client_id>
  int len = snprintf(pub_topic, BUFFER_SIZE, "/99/%s", client_id);
  LOG_INFO("Pub Topic: %s\n", pub_topic);

  /* len < 0: Error. Len >= BUFFER_SIZE: Buffer too small */
  if(len < 0 || len >= BUFFER_SIZE) {
    LOG_INFO("Pub Topic: %d, Buffer %d\n", len, BUFFER_SIZE);
    return 0;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
construct_sub_topic(void)
{
  //LWPubSub - subscribe topic: /99/<client_id>/cmd
  int len = snprintf(sub_topic, BUFFER_SIZE, "/99/%02x%02x%02x%02x%02x%02x/cmd",
                     linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
                     linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
                     linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);
  LOG_INFO("Sub Topic: %s\n", sub_topic);

  /* len < 0: Error. Len >= BUFFER_SIZE: Buffer too small */
  if(len < 0 || len >= BUFFER_SIZE) {
    LOG_INFO("Sub Topic: %d, Buffer %d\n", len, BUFFER_SIZE);
    return 0;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
construct_client_id(void)
{
  int len = snprintf(client_id, BUFFER_SIZE, "%02x%02x%02x%02x%02x%02x",
                     linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
                     linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
                     linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);

  /* len < 0: Error. Len >= BUFFER_SIZE: Buffer too small */
  if(len < 0 || len >= BUFFER_SIZE) {
    LOG_ERR("Client ID: %d, Buffer %d\n", len, BUFFER_SIZE);
    return 0;
  }
  LOG_DBG("Client ID: %s\n", client_id);

  #define MQTT_CLIENT_USERNAME client_id

  return 1;
}
/*---------------------------------------------------------------------------*/
static void
update_config(void)
{
  if(construct_client_id() == 0) {
    /* Fatal error. Client ID larger than the buffer */
    state = STATE_CONFIG_ERROR;
    return;
  }

  if(construct_sub_topic() == 0) {
    /* Fatal error. Topic larger than the buffer */
    state = STATE_CONFIG_ERROR;
    return;
  }

  if(construct_pub_topic() == 0) {
    /* Fatal error. Topic larger than the buffer */
    state = STATE_CONFIG_ERROR;
    return;
  }

  /* Reset the counter */
  seq_nr_value = 0;

  state = STATE_INIT;

  /*
   * Schedule next timer event ASAP
   *
   * If we entered an error state then we won't do anything when it fires.
   *
   * Since the error at this stage is a config error, we will only exit this
   * error state if we get a new config.
   */
  etimer_set(&publish_periodic_timer, 0);

  return;
}
/*---------------------------------------------------------------------------*/
static int
init_config()
{
  /* Populate configuration with default values */
  memset(&conf, 0, sizeof(mqtt_client_config_t));

  memcpy(conf.org_id, MQTT_CLIENT_ORG_ID, strlen(MQTT_CLIENT_ORG_ID));
  memcpy(conf.type_id, DEFAULT_TYPE_ID, strlen(DEFAULT_TYPE_ID));
  memcpy(conf.auth_token, MQTT_CLIENT_AUTH_TOKEN,
         strlen(MQTT_CLIENT_AUTH_TOKEN));
  memcpy(conf.event_type_id, DEFAULT_EVENT_TYPE_ID,
         strlen(DEFAULT_EVENT_TYPE_ID));
  memcpy(conf.broker_ip, broker_ip, strlen(broker_ip));
  memcpy(conf.cmd_type, DEFAULT_SUBSCRIBE_CMD_TYPE, 1);

  conf.broker_port = DEFAULT_BROKER_PORT;
  conf.pub_interval = DEFAULT_PUBLISH_INTERVAL;
  conf.def_rt_ping_interval = DEFAULT_RSSI_MEAS_INTERVAL;

  return 1;
}
/*---------------------------------------------------------------------------*/
static void
subscribe(void)
{
  /* Publish MQTT topic in IBM quickstart format */
  mqtt_status_t status;

  #if (ENERGEST_CONF_ON == 1)
    energest_flush();
    LOG_INFO("Subscribe start ");
    printf("E_CPU %llu E_LPM %llu E_DEEP_LPM %llu E_TX %llu E_RX %llu E_Total: %llu\n",
      energest_type_time(ENERGEST_TYPE_CPU), energest_type_time(ENERGEST_TYPE_LPM), energest_type_time(ENERGEST_TYPE_DEEP_LPM),
      energest_type_time(ENERGEST_TYPE_TRANSMIT), energest_type_time(ENERGEST_TYPE_LISTEN), ENERGEST_GET_TOTAL_TIME());
  #endif /* ENERGEST_CONF_ON */

  status = mqtt_subscribe(&conn, NULL, sub_topic, MQTT_QOS_LEVEL_0);

  LOG_DBG("Subscribing!\n");
  if(status == MQTT_STATUS_OUT_QUEUE_FULL) {
    LOG_ERR("Tried to subscribe but command queue was full!\n");
  }
}
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/

static void
connect_to_broker(void)
{
  /* Connect to MQTT server */

  #if (ENERGEST_CONF_ON == 1)
    energest_flush();
    LOG_INFO("Connect start ");
    printf("E_CPU %llu E_LPM %llu E_DEEP_LPM %llu E_TX %llu E_RX %llu E_Total: %llu\n",
      energest_type_time(ENERGEST_TYPE_CPU), energest_type_time(ENERGEST_TYPE_LPM), energest_type_time(ENERGEST_TYPE_DEEP_LPM),
      energest_type_time(ENERGEST_TYPE_TRANSMIT), energest_type_time(ENERGEST_TYPE_LISTEN), ENERGEST_GET_TOTAL_TIME());
  #endif /* ENERGEST_CONF_ON */

  // mqtt_connect(&conn, conf.broker_ip, conf.broker_port,
  //              (conf.pub_interval * 3) / CLOCK_SECOND,
  //              MQTT_CLEAN_SESSION_ON);

  mqtt_connect(&conn, conf.broker_ip, conf.broker_port,
               DEFAULT_KEEP_ALIVE_TIMER,
               MQTT_CLEAN_SESSION_ON);

  state = STATE_CONNECTING;
}
/*---------------------------------------------------------------------------*/
static void
ping_parent(void)
{
  if(have_connectivity()) {
    uip_icmp6_send(uip_ds6_defrt_choose(), ICMP6_ECHO_REQUEST, 0,
                   ECHO_REQ_PAYLOAD_LEN);
  } else {
    LOG_WARN("ping_parent() is called while we don't have connectivity\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
state_machine(void)
{
  switch(state) {
  case STATE_INIT:
    /* If we have just been configured register MQTT connection */
    mqtt_register(&conn, &mqtt_client_process, client_id, mqtt_event,
                  MAX_TCP_SEGMENT_SIZE);

    /*
     * If we are not using the quickstart service (thus we are an IBM
     * registered device), we need to provide user name and password
     */
    if(strncasecmp(conf.org_id, QUICKSTART, strlen(conf.org_id)) != 0) {
      if(strlen(conf.auth_token) == 0) {
        LOG_ERR("User name set, but empty auth token\n");
        state = STATE_ERROR;
        break;
      } else {
        mqtt_set_username_password(&conn, MQTT_CLIENT_USERNAME,
                                   conf.auth_token);
      }
    }

    /* _register() will set auto_reconnect. We don't want that. */
    conn.auto_reconnect = 0;
    connect_attempt = 1;

    state = STATE_REGISTERED;
    LOG_DBG("Init\n");
    /* Continue */
  case STATE_REGISTERED:
    if(have_connectivity()) {
      /* Registered and with a public IP. Connect */
      LOG_DBG("Registered. Connect attempt %u\n", connect_attempt);
      ping_parent();
      connect_to_broker();
    } else {
      leds_on(MQTT_CLIENT_STATUS_LED);
      ctimer_set(&ct, NO_NET_LED_DURATION, publish_led_off, NULL);
    }
    etimer_set(&publish_periodic_timer, NET_CONNECT_PERIODIC);
    return;
    break;
  case STATE_CONNECTING:
    leds_on(MQTT_CLIENT_STATUS_LED);
    ctimer_set(&ct, CONNECTING_LED_DURATION, publish_led_off, NULL);
    /* Not connected yet. Wait */
    LOG_DBG("Connecting (%u)\n", connect_attempt);
    break;
  case STATE_CONNECTED:
    /* Don't subscribe unless we are a registered device */
    if(strncasecmp(conf.org_id, QUICKSTART, strlen(conf.org_id)) == 0) {
      LOG_DBG("Using 'quickstart': Skipping subscribe\n");
      state = STATE_PUBLISHING;
    }
    /* Continue */
  case STATE_PUBLISHING:
    /* If the timer expired, the connection is stable. */
    if(timer_expired(&connection_life)) {
      /*
       * Intentionally using 0 here instead of 1: We want RECONNECT_ATTEMPTS
       * attempts if we disconnect after a successful connect
       */
      connect_attempt = 0;
    }

    if(mqtt_ready(&conn) && conn.out_buffer_sent) {
      /* Connected. Publish */
      if(state == STATE_CONNECTED) {
        subscribe();
        state = STATE_PUBLISHING;
      } else {
        leds_on(MQTT_CLIENT_STATUS_LED);
        ctimer_set(&ct, PUBLISH_LED_ON_DURATION, publish_led_off, NULL);
        LOG_DBG("Publishing\n");
        publish();
      }
      etimer_set(&publish_periodic_timer, conf.pub_interval);
      /* Return here so we don't end up rescheduling the timer */
      return;
    } else {
      /*
       * Our publish timer fired, but some MQTT packet is already in flight
       * (either not sent at all, or sent but not fully ACKd).
       *
       * This can mean that we have lost connectivity to our broker or that
       * simply there is some network delay. In both cases, we refuse to
       * trigger a new message and we wait for TCP to either ACK the entire
       * packet after retries, or to timeout and notify us.
       */
      LOG_DBG("Publishing... (MQTT state=%d, q=%u)\n", conn.state,
              conn.out_queue_full);
    }
    break;
  case STATE_DISCONNECTED:
    LOG_DBG("Disconnected\n");
    if(connect_attempt < RECONNECT_ATTEMPTS ||
       RECONNECT_ATTEMPTS == RETRY_FOREVER) {
      /* Disconnect and backoff */
      clock_time_t interval;
      mqtt_disconnect(&conn);
      connect_attempt++;

      interval = connect_attempt < 3 ? RECONNECT_INTERVAL << connect_attempt :
        RECONNECT_INTERVAL << 3;

      LOG_DBG("Disconnected. Attempt %u in %lu ticks\n", connect_attempt, interval);

      etimer_set(&publish_periodic_timer, interval);

      state = STATE_REGISTERED;
      return;
    } else {
      /* Max reconnect attempts reached. Enter error state */
      state = STATE_ERROR;
      LOG_DBG("Aborting connection after %u attempts\n", connect_attempt - 1);
    }
    break;
  case STATE_CONFIG_ERROR:
    /* Idle away. The only way out is a new config */
    LOG_ERR("Bad configuration.\n");
    return;
  case STATE_ERROR:
  default:
    leds_on(MQTT_CLIENT_STATUS_LED);
    /*
     * 'default' should never happen.
     *
     * If we enter here it's because of some error. Stop timers. The only thing
     * that can bring us out is a new config event
     */
    LOG_ERR("Default case: State=0x%02x\n", state);
    return;
  }

  /* If we didn't return so far, reschedule ourselves */
  etimer_set(&publish_periodic_timer, STATE_MACHINE_PERIODIC);
}
/*---------------------------------------------------------------------------*/
static void
init_extensions(void)
{
  int i;

  for(i = 0; i < mqtt_client_extension_count; i++) {
    if(mqtt_client_extensions[i]->init) {
      mqtt_client_extensions[i]->init();
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mqtt_client_process, ev, data)
{

  PROCESS_BEGIN();

  printf("\nCWIoT - LWSec PubSub System for IoT - MQTT client process\n\n");

  /* PRINT USED CRYPTO in LOG */
  if(LWPUBSUB_CRYPTO_MODE == LWPUBSUB_AES_CBC_128) {
    LOG_INFO("Crypto Algorithm: AES-CBC-128\n");
  } else if(LWPUBSUB_CRYPTO_MODE == LWPUBSUB_AES_CBC_192) {
    LOG_INFO("Crypto Algorithm: AES-CBC-192\n");
  } else if(LWPUBSUB_CRYPTO_MODE == LWPUBSUB_AES_CBC_256) {
    LOG_INFO("Crypto Algorithm: AES-CBC-256\n");
  } else if(LWPUBSUB_CRYPTO_MODE == LWPUBSUB_AES_CTR_128) {
    LOG_INFO("Crypto Algorithm: AES-CTR-128\n");
  } else if(LWPUBSUB_CRYPTO_MODE == LWPUBSUB_AES_CTR_192) {
    LOG_INFO("Crypto Algorithm: AES-CTR-192\n");
  } else if(LWPUBSUB_CRYPTO_MODE == LWPUBSUB_AES_CTR_256) {
    LOG_INFO("Crypto Algorithm: AES-CTR-256\n");
  } else {
    LOG_ERR("Wrong crypto algorithm\n");
  }

  /* PRINT POLL FREQUENCY */
  //LOG_INFO("Poll frequency: %lu \n", LWPUBSUB_POLLFREQUENCY);

  if(LWPUBSUB_POLLFREQUENCY == 5) {
    LOG_INFO("Poll frequency: VERYHIGH\n");
  } else if(LWPUBSUB_POLLFREQUENCY == 15) {
    LOG_INFO("Poll frequency: HIGH\n");
  } else if(LWPUBSUB_POLLFREQUENCY == 900) {
    LOG_INFO("Poll frequency: MEDIUM\n");
  } else if(LWPUBSUB_POLLFREQUENCY == 21600) {
    LOG_INFO("Poll frequency: LOW\n");
  } else if(LWPUBSUB_POLLFREQUENCY == 86400) {
    LOG_INFO("Poll frequency: VERYLOW\n");
  } else if(LWPUBSUB_POLLFREQUENCY == 30) {
    LOG_INFO("Poll frequency: 30 secs.\n");
  } else {
    LOG_ERR("Wrong poll frequency\n");
  }

  /* PRINT TSCH SCHEDULE */
#ifdef LWPUBSUB_ORCHESTRA
  LOG_INFO("TSCH Schedule: Orchestra\n");
#else
  LOG_INFO("TSCH Schedule: Minimal\n");
#endif



  if(init_config() != 1) {
    PROCESS_EXIT();
  }

  init_extensions();

  update_config();

  def_rt_rssi = 0x8000000;
  uip_icmp6_echo_reply_callback_add(&echo_reply_notification,
                                    echo_reply_handler);
  etimer_set(&echo_request_timer, conf.def_rt_ping_interval);

  /* Main loop */
  while(1) {

    PROCESS_YIELD();

    if(ev == button_hal_release_event &&
       ((button_hal_button_t *)data)->unique_id == BUTTON_HAL_ID_BUTTON_ZERO) {
      if(state == STATE_ERROR) {
        connect_attempt = 1;
        state = STATE_REGISTERED;
      }
    }

    if((ev == PROCESS_EVENT_TIMER && data == &publish_periodic_timer) ||
       ev == PROCESS_EVENT_POLL ||
       (ev == button_hal_release_event &&
        ((button_hal_button_t *)data)->unique_id == BUTTON_HAL_ID_BUTTON_ZERO)) {

#if BOARD_SENSORTAG
      if(buzzer_state()) {
        buzzer_stop();
      }
#endif

      state_machine();
    }

    if(ev == PROCESS_EVENT_TIMER && data == &echo_request_timer) {


      ping_parent();
      etimer_set(&echo_request_timer, conf.def_rt_ping_interval);


      // #if (ENERGEST_CONF_ON == 1)
      //   energest_flush();
      //   LOG_INFO("Energy while_process ");
      //   printf("E_CPU %llu E_LPM %llu E_DEEP_LPM %llu E_TX %llu E_RX %llu E_Total: %llu\n",
      //     energest_type_time(ENERGEST_TYPE_CPU), energest_type_time(ENERGEST_TYPE_LPM), energest_type_time(ENERGEST_TYPE_DEEP_LPM),
      //     energest_type_time(ENERGEST_TYPE_TRANSMIT), energest_type_time(ENERGEST_TYPE_LISTEN), ENERGEST_GET_TOTAL_TIME());
      // #endif /* ENERGEST_CONF_ON */
    }

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
