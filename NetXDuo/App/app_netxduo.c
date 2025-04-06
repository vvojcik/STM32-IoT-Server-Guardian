/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_netxduo.c
  * @author  MCD Application Team
  * @brief   NetXDuo applicative file
  ******************************************************************************
    * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_netxduo.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_azure_rtos.h"
#include "nx_ip.h"
#include "mx_wifi.h"
#include  MOSQUITTO_CERT_FILE
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern RNG_HandleTypeDef hrng;
/* Replace with your target PC's MAC address */
uint8_t PC_MAC_ADDRESS[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
NX_UDP_SOCKET wol_socket;

TX_THREAD AppMainThread;
TX_THREAD AppMQTTClientThread;
TX_THREAD AppSNTPThread;

TX_QUEUE  MsgQueueOne;

TX_SEMAPHORE Semaphore;

NX_PACKET_POOL  AppPool;
NX_IP           IpInstance;
NX_DHCP         DhcpClient;
NXD_MQTT_CLIENT MqttClient;
NX_SNTP_CLIENT  SntpClient;
static NX_DNS   DnsClient;

TX_EVENT_FLAGS_GROUP     SntpFlags;

ULONG   IpAddress;
ULONG   NetMask;

ULONG mqtt_client_stack[MQTT_CLIENT_STACK_SIZE];

TX_EVENT_FLAGS_GROUP mqtt_app_flag;

/* Declare buffers to hold message and topic. */
static char message[NXD_MQTT_MAX_MESSAGE_LENGTH];
static UCHAR message_buffer[NXD_MQTT_MAX_MESSAGE_LENGTH];
static UCHAR topic_buffer[NXD_MQTT_MAX_TOPIC_NAME_LENGTH];

/* TLS buffers and certificate containers. */
//extern const NX_SECURE_TLS_CRYPTO nx_crypto_tls_ciphers;
//static CHAR crypto_metadata_client[11600];
//UCHAR tls_packet_buffer[8000];

ULONG                    current_time;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static VOID App_Main_Thread_Entry(ULONG thread_input);
static VOID App_MQTT_Client_Thread_Entry(ULONG thread_input);
static VOID App_SNTP_Thread_Entry(ULONG thread_input);
static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr);
static VOID time_update_callback(NX_SNTP_TIME_MESSAGE *time_update_ptr, NX_SNTP_TIME *local_time);
static UINT dns_create(NX_DNS *dns_ptr);
/* USER CODE END PFP */

/**
  * @brief  Application NetXDuo Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT MX_NetXDuo_Init(VOID *memory_ptr)
{
  UINT ret = NX_SUCCESS;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

   /* USER CODE BEGIN App_NetXDuo_MEM_POOL */

  /* USER CODE END App_NetXDuo_MEM_POOL */
  /* USER CODE BEGIN 0 */

  /* USER CODE END 0 */

  /* USER CODE BEGIN MX_NetXDuo_Init */
#if (USE_STATIC_ALLOCATION == 1)
  printf("Nx_MQTT_Client application started..\n");

  CHAR *pointer;

  /* Initialize the NetX system. */
  nx_system_initialize();

  /* Allocate the memory for packet_pool.  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,  NX_PACKET_POOL_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the Packet pool to be used for packet allocation */
  ret = nx_packet_pool_create(&AppPool, "Main Packet Pool", PAYLOAD_SIZE, pointer, NX_PACKET_POOL_SIZE);

  if (ret != NX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  /* Allocate the memory for Ip_Instance */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, 2 * DEFAULT_MEMORY_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the main NX_IP instance */
  ret = nx_ip_create(&IpInstance, "Main Ip instance", NULL_ADDRESS, NULL_ADDRESS, &AppPool, nx_driver_emw3080_entry,
                     pointer, 2 * DEFAULT_MEMORY_SIZE, DEFAULT_MAIN_PRIORITY);

  if (ret != NX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  /* create the DHCP client */
  ret = nx_dhcp_create(&DhcpClient, &IpInstance, "DHCP Client");

  if (ret != NX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  /* Allocate the memory for ARP */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, ARP_MEMORY_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Enable the ARP protocol and provide the ARP cache size for the IP instance */
  ret = nx_arp_enable(&IpInstance, (VOID *)pointer, ARP_MEMORY_SIZE);

  if (ret != NX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  /* Enable the ICMP */
  ret = nx_icmp_enable(&IpInstance);

  if (ret != NX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  /* Enable the UDP protocol required for DHCP communication */
  ret = nx_udp_enable(&IpInstance);

  if (ret != NX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  /* Enable the TCP protocol required for DNS, MQTT.. */
  ret = nx_tcp_enable(&IpInstance);

  if (ret != NX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  /* Allocate the memory for main thread   */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, THREAD_MEMORY_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the main thread */
  ret = tx_thread_create(&AppMainThread, "App Main thread", App_Main_Thread_Entry, 0, pointer, THREAD_MEMORY_SIZE,
                         DEFAULT_MAIN_PRIORITY, DEFAULT_MAIN_PRIORITY, TX_NO_TIME_SLICE, TX_AUTO_START);

  if (ret != TX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  /* Allocate the memory for SNTP client thread   */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, SNTP_CLIENT_THREAD_MEMORY, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* create the SNTP client thread */
  ret = tx_thread_create(&AppSNTPThread, "App SNTP Thread", App_SNTP_Thread_Entry, 0, pointer, SNTP_CLIENT_THREAD_MEMORY,
                         SNTP_PRIORITY, SNTP_PRIORITY, TX_NO_TIME_SLICE, TX_DONT_START);

  if (ret != TX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  /* Create the event flags. */
  ret = tx_event_flags_create(&SntpFlags, "SNTP event flags");

  /* Check for errors */
  if (ret != NX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  /* Allocate the memory for MQTT client thread   */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, THREAD_MEMORY_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* create the MQTT client thread */
  ret = tx_thread_create(&AppMQTTClientThread, "App MQTT Thread", App_MQTT_Client_Thread_Entry, 0, pointer, THREAD_MEMORY_SIZE,
                         MQTT_PRIORITY, MQTT_PRIORITY, TX_NO_TIME_SLICE, TX_DONT_START);

  if (ret != TX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  /* Allocate the MsgQueueOne.  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, APP_QUEUE_SIZE*sizeof(ULONG), TX_NO_WAIT) != TX_SUCCESS)
  {
    ret = TX_POOL_ERROR;
  }

  /* Create the MsgQueueOne shared by MsgSenderThreadOne and MsgReceiverThread */
  if (tx_queue_create(&MsgQueueOne, "Message Queue One",TX_1_ULONG, pointer, APP_QUEUE_SIZE*sizeof(ULONG)) != TX_SUCCESS)
  {
    ret = TX_QUEUE_ERROR;
  }

  /* set DHCP notification callback  */
  tx_semaphore_create(&Semaphore, "DHCP Semaphore", 0);
#endif
  /* USER CODE END MX_NetXDuo_Init */

  return ret;
}

/* USER CODE BEGIN 1 */
void send_wake_on_lan(uint8_t *mac)
{
  NX_PACKET *packet_ptr;
  uint8_t wol_payload[102];

  memset(wol_payload, 0xFF, 6);
  for (int i = 0; i < 16; i++) {
    memcpy(&wol_payload[6 + (i * 6)], mac, 6);
  }

  if (nx_packet_allocate(&AppPool, &packet_ptr, NX_UDP_PACKET, TX_WAIT_FOREVER) == NX_SUCCESS)
  {
    nx_packet_data_append(packet_ptr, wol_payload, 102, &AppPool, TX_WAIT_FOREVER);
    nx_udp_socket_send(&wol_socket, packet_ptr, IP_ADDRESS(255, 255, 255, 255), 9);
    printf("Pakiet wybudzajacy wyslany do sieci lokalnej w Gdansku!\n");
  }
}
/* USER CODE END 1 */
/**
* @brief  ip address change callback.
* @param ip_instance: NX_IP instance
* @param ptr: user data
* @retval none
*/
static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr)
{
  /* release the semaphore as soon as an IP address is available */
  tx_semaphore_put(&Semaphore);
}

/**
* @brief  DNS Create Function.
* @param dns_ptr
* @retval ret
*/
UINT dns_create(NX_DNS *dns_ptr)
{
  UINT ret = NX_SUCCESS;

  /* Create a DNS instance for the Client */
  ret = nx_dns_create(dns_ptr, &IpInstance, (UCHAR *)"DNS Client");
  if (ret)
  {
    Error_Handler();
  }
  /* Initialize DNS instance with a dummy server */
  ret = nx_dns_server_add(dns_ptr, USER_DNS_ADDRESS);
  if (ret)
  {
    Error_Handler();
  }

  return ret;
}

/**
* @brief  Main thread entry.
* @param thread_input: ULONG user argument used by the thread entry
* @retval none
*/
static VOID App_Main_Thread_Entry(ULONG thread_input)
{
  UINT ret = NX_SUCCESS;
  /* Dodaj te linie: */
  extern MX_WIFIObject_t *wifi_obj_get(void);
  MX_WIFIObject_t *pWifi = wifi_obj_get();

  ret = dns_create(&DnsClient);
  if (ret != NX_SUCCESS)
  {
    /* Wypisz kod błędu w HEX - to nam powie wszystko! */
    printf("\nBŁĄD POŁĄCZENIA: 0x%02X | Broker: %s\n", ret, MQTT_BROKER_NAME);
    Error_Handler();
  }

  ret = nx_ip_address_change_notify(&IpInstance, ip_address_change_notify_callback, NULL);
  if (ret != NX_SUCCESS)
  {
    /* Wypisz kod błędu w HEX - to nam powie wszystko! */
    printf("\nBŁĄD POŁĄCZENIA: 0x%02X | Broker: %s\n", ret, MQTT_BROKER_NAME);
    Error_Handler();
  }

  /* --- TU JEST KLUCZ DO SUKCESU --- */
  printf("Laczenie z WiFi: %s...\n", WIFI_SSID);

  /* Rozkaz polaczenia do modulu Wi-Fi */
  if (MX_WIFI_Connect(pWifi, WIFI_SSID, WIFI_PASSWORD, MX_WIFI_SEC_WPA2_AES) != MX_WIFI_STATUS_OK)
  {
      printf("KATASTROFA: Nie udalo sie polaczyc z landryneczka!\n");
      Error_Handler();
  }
  /* ------------------------------- */

  ret = nx_dhcp_start(&DhcpClient);
  if (ret != NX_SUCCESS)
  {
    /* Wypisz kod błędu w HEX - to nam powie wszystko! */
    printf("\nBŁĄD POŁĄCZENIA: 0x%02X | Broker: %s\n", ret, MQTT_BROKER_NAME);
    Error_Handler();
  }

  if(tx_semaphore_get(&Semaphore, TX_WAIT_FOREVER) != TX_SUCCESS) Error_Handler();

  ret = nx_ip_address_get(&IpInstance, &IpAddress, &NetMask);
  if (ret != TX_SUCCESS) Error_Handler();

  PRINT_IP_ADDRESS(IpAddress);

  nx_udp_socket_create(&IpInstance, &wol_socket, "WOL Socket", NX_IP_NORMAL, NX_FRAGMENT_OKAY, 0x80, 5);
  nx_udp_socket_bind(&wol_socket, 9, TX_WAIT_FOREVER);

  tx_thread_resume(&AppSNTPThread);
  tx_thread_relinquish();
  return;
}

/* Declare the disconnect notify function. */
static VOID my_disconnect_func(NXD_MQTT_CLIENT *client_ptr)
{
  NX_PARAMETER_NOT_USED(client_ptr);

  printf("client disconnected from broker < %s >.\n", MQTT_BROKER_NAME);
}

/* Declare the notify function. */
static VOID my_notify_func(NXD_MQTT_CLIENT* client_ptr, UINT number_of_messages)
{
  NX_PARAMETER_NOT_USED(client_ptr);
  /* Dodaj ten printf, żeby widzieć samo nadejście pakietu */
  printf("Dostałem powiadomienie o %u nowych wiadomościach!\n", number_of_messages);

  tx_event_flags_set(&mqtt_app_flag, DEMO_MESSAGE_EVENT, TX_OR);
}

/**
* @brief  message generation Function.
* @param  RandomNbr
* @retval none
*/
UINT message_generate()
{
  uint32_t RandomNbr = 0;

  HAL_RNG_Init(&hrng);

  /* generate a random number */
  if(HAL_RNG_GenerateRandomNumber(&hrng, &RandomNbr) != HAL_OK)
  {
    Error_Handler();
  }

  return RandomNbr %= 50;
}

/* Function (set by user) to call when TLS needs the current time. */

/* Callback to setup TLS parameters for secure MQTT connection. */

/* This application defined handler for notifying SNTP time update event.  */
static VOID time_update_callback(NX_SNTP_TIME_MESSAGE *time_update_ptr, NX_SNTP_TIME *local_time)
{
  NX_PARAMETER_NOT_USED(time_update_ptr);
  NX_PARAMETER_NOT_USED(local_time);

  tx_event_flags_set(&SntpFlags, SNTP_UPDATE_EVENT, TX_OR);
}

/** @brief  SNTP Client thread entry.
* @param thread_input: ULONG user argument used by the thread entry
* @retval none
*/
static VOID App_SNTP_Thread_Entry(ULONG thread_input)
{
  UINT ret;
  ULONG  fraction;
  ULONG  events = 0;
  UINT   server_status;
  NXD_ADDRESS sntp_server_ip;

  sntp_server_ip.nxd_ip_version = 4;

  /* Look up SNTP Server address. */
  ret = nx_dns_host_by_name_get(&DnsClient, (UCHAR *)SNTP_SERVER_NAME, &sntp_server_ip.nxd_ip_address.v4, DEFAULT_TIMEOUT);

  /* Check for error. */
  if (ret != NX_SUCCESS)
  {
    /* Wypisz kod błędu w HEX - to nam powie wszystko! */
    printf("\nBŁĄD POŁĄCZENIA: 0x%02X | Broker: %s\n", ret, MQTT_BROKER_NAME);
    Error_Handler();
  }

  /* Create the SNTP Client */
  ret =  nx_sntp_client_create(&SntpClient, &IpInstance, 0, &AppPool, NULL, NULL, NULL);

  /* Check for error. */
  if (ret != NX_SUCCESS)
  {
    /* Wypisz kod błędu w HEX - to nam powie wszystko! */
    printf("\nBŁĄD POŁĄCZENIA: 0x%02X | Broker: %s\n", ret, MQTT_BROKER_NAME);
    Error_Handler();
  }

  /* Setup time update callback function. */
  nx_sntp_client_set_time_update_notify(&SntpClient, time_update_callback);

  /* Use the IPv4 service to set up the Client and set the IPv4 SNTP server. */
  ret = nx_sntp_client_initialize_unicast(&SntpClient, sntp_server_ip.nxd_ip_address.v4);

  if (ret != NX_SUCCESS)
  {
    /* Wypisz kod błędu w HEX - to nam powie wszystko! */
    printf("\nBŁĄD POŁĄCZENIA: 0x%02X | Broker: %s\n", ret, MQTT_BROKER_NAME);
    Error_Handler();
  }

  /* Run whichever service the client is configured for. */
  ret = nx_sntp_client_run_unicast(&SntpClient);

  if (ret != NX_SUCCESS)
  {
    /* Wypisz kod błędu w HEX - to nam powie wszystko! */
    printf("\nBŁĄD POŁĄCZENIA: 0x%02X | Broker: %s\n", ret, MQTT_BROKER_NAME);
    Error_Handler();
  }

  /* Wait for a server update event. */
  tx_event_flags_get(&SntpFlags, SNTP_UPDATE_EVENT, TX_OR_CLEAR, &events, PERIODIC_CHECK_INTERVAL);

  if (events == SNTP_UPDATE_EVENT)
  {
    /* Check for valid SNTP server status. */
    ret = nx_sntp_client_receiving_updates(&SntpClient, &server_status);

    if (ret != NX_SUCCESS)
    {
      /* Wypisz kod błędu w HEX - to nam powie wszystko! */
      printf("\nBŁĄD POŁĄCZENIA: 0x%02X | Broker: %s\n", ret, MQTT_BROKER_NAME);
      Error_Handler();
    }
    /* We have a valid update.  Get the SNTP Client time.  */
    ret = nx_sntp_client_get_local_time_extended(&SntpClient, &current_time, &fraction, NX_NULL, 0);
    current_time -= EPOCH_TIME_DIFF;
    printf("Zsynchronizowany czas (Unix): %lu\n", current_time);

  }
  else
  {
    Error_Handler();
  }

  /* start the MQTT client thread */
  tx_thread_resume(&AppMQTTClientThread);

}

/**
* @brief  MQTT Client thread entry.
* @param thread_input: ULONG user argument used by the thread entry
* @retval none
*/
static VOID App_MQTT_Client_Thread_Entry(ULONG thread_input)
{
  UINT ret = NX_SUCCESS;
  NXD_ADDRESS mqtt_server_ip;
  ULONG events;
  UINT aRandom32bit;
  UINT topic_length, message_length;
  UINT remaining_msg = NB_MESSAGE;
  UINT message_count = 0;
  UINT unlimited_publish = NX_FALSE;

  mqtt_server_ip.nxd_ip_version = 4;

  /* Look up MQTT Server address. */
  ret = nx_dns_host_by_name_get(&DnsClient, (UCHAR *)MQTT_BROKER_NAME,
                                &mqtt_server_ip.nxd_ip_address.v4, DEFAULT_TIMEOUT);

  /* Check status.  */
  if (ret != NX_SUCCESS)
  {
    /* Wypisz kod błędu w HEX - to nam powie wszystko! */
    printf("\nBŁĄD POŁĄCZENIA: 0x%02X | Broker: %s\n", ret, MQTT_BROKER_NAME);
    Error_Handler();
  }


  /* Create MQTT client instance. */
  ret = nxd_mqtt_client_create(&MqttClient, "my_client", CLIENT_ID_STRING, STRLEN(CLIENT_ID_STRING),
                               &IpInstance, &AppPool, (VOID*)mqtt_client_stack, MQTT_CLIENT_STACK_SIZE,
                               MQTT_THREAD_PRIORTY, NX_NULL, 0);

  if (ret != NX_SUCCESS)
  {
    /* Wypisz kod błędu w HEX - to nam powie wszystko! */
    printf("\nBŁĄD POŁĄCZENIA: 0x%02X | Broker: %s\n", ret, MQTT_BROKER_NAME);
    Error_Handler();
  }

  /* Register the disconnect notification function. */
  nxd_mqtt_client_disconnect_notify_set(&MqttClient, my_disconnect_func);

  /* Set the receive notify function. */
  nxd_mqtt_client_receive_notify_set(&MqttClient, my_notify_func);

  /* Create an MQTT flag */
  ret = tx_event_flags_create(&mqtt_app_flag, "my app event");
  if (ret != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Start a secure connection to the server. */
  ret = nxd_mqtt_client_connect(&MqttClient, &mqtt_server_ip, MQTT_PORT,
                                MQTT_KEEP_ALIVE_TIMER, CLEAN_SESSION, 10 * NX_IP_PERIODIC_RATE);

  if (ret != NX_SUCCESS)
  {
    /* Wypisz kod błędu w HEX - to nam powie wszystko! */
    printf("\nBŁĄD POŁĄCZENIA: 0x%02X | Broker: %s\n", ret, MQTT_BROKER_NAME);
    Error_Handler();
  }
  else
  {
    printf("\nMQTT client connected to broker < %s > at PORT %d :\n",MQTT_BROKER_NAME, MQTT_PORT);
  }

  /* Subscribe to the topic with QoS level 1. */
  ret = nxd_mqtt_client_subscribe(&MqttClient, TOPIC_NAME, STRLEN(TOPIC_NAME), QOS1);

  if (ret != NX_SUCCESS)
  {
    /* Wypisz kod błędu w HEX - to nam powie wszystko! */
    printf("\nBŁĄD POŁĄCZENIA: 0x%02X | Broker: %s\n", ret, MQTT_BROKER_NAME);
    Error_Handler();
  }

  if (NB_MESSAGE ==0)
    unlimited_publish = NX_TRUE;

    printf("\n--- SYSTEM GOTOWY ---\n");
    printf("Czekam na komende WAKE na topiku: %s\n", TOPIC_NAME);

    while(1)
    {
      tx_event_flags_get(&mqtt_app_flag, DEMO_ALL_EVENTS, TX_OR_CLEAR, &events, TX_WAIT_FOREVER);

      /* Zmień ten fragment wewnątrz while(1) */
      if (events & DEMO_MESSAGE_EVENT)
      {
        UINT status;
        // Próbujemy wyciągnąć wiadomość
        status = nxd_mqtt_client_message_get(&MqttClient, topic_buffer, sizeof(topic_buffer), &topic_length,
                                              message_buffer, sizeof(message_buffer), &message_length);

        printf("Próba odczytu wiadomości... Status: 0x%02X\n", status);

        if (status == NXD_MQTT_SUCCESS)
        {
          message_buffer[message_length] = '\0';
          printf("SUROWA WIADOMOŚĆ: [%s] (Długość: %u)\n", message_buffer, message_length);

          if (strcmp((char*)message_buffer, "WAKE") == 0)
          {
            printf("!!! AKCJA: WYSYLAM MAGIC PACKET DO PC !!!\n");
            send_wake_on_lan(PC_MAC_ADDRESS);
          }
          else
          {
             printf("Wiadomość nie pasuje do 'WAKE'. Sprawdź wielkość liter lub spacje!\n");
          }
        }
      }
    }

  /* Now unsubscribe from topic. */
  ret = nxd_mqtt_client_unsubscribe(&MqttClient, TOPIC_NAME, STRLEN(TOPIC_NAME));

  if (ret != NX_SUCCESS)
  {
    /* Wypisz kod błędu w HEX - to nam powie wszystko! */
    printf("\nBŁĄD POŁĄCZENIA: 0x%02X | Broker: %s\n", ret, MQTT_BROKER_NAME);
    Error_Handler();
  }

  /* Disconnect from the broker. */
  ret = nxd_mqtt_client_disconnect(&MqttClient);

  if (ret != NX_SUCCESS)
  {
    /* Wypisz kod błędu w HEX - to nam powie wszystko! */
    printf("\nBŁĄD POŁĄCZENIA: 0x%02X | Broker: %s\n", ret, MQTT_BROKER_NAME);
    Error_Handler();
  }

  /* Delete the client instance, release all the resources. */
  ret = nxd_mqtt_client_delete(&MqttClient);

  if (ret != NX_SUCCESS)
  {
    /* Wypisz kod błędu w HEX - to nam powie wszystko! */
    printf("\nBŁĄD POŁĄCZENIA: 0x%02X | Broker: %s\n", ret, MQTT_BROKER_NAME);
    Error_Handler();
  }

  /* test OK -> success Handler */
  Success_Handler();
}
/* USER CODE END 1 */
