/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_netxduo.c
  * @author  MCD Application Team
  * @brief   NetXDuo applicative file
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_netxduo.h"
#include "app_azure_rtos.h"
#include "nx_ip.h"
#include "mx_wifi.h"

/* Private typedef -----------------------------------------------------------*/
extern RNG_HandleTypeDef hrng;

TX_THREAD AppMainThread;
TX_THREAD AppTestThread;
TX_SEMAPHORE Semaphore;

NX_PACKET_POOL  AppPool;
NX_IP           IpInstance;
NX_DHCP         DhcpClient;
static NX_DNS   DnsClient;

ULONG   IpAddress;
ULONG   NetMask;

/* Private function prototypes -----------------------------------------------*/
static VOID App_Main_Thread_Entry(ULONG thread_input);
static VOID App_Test_Thread_Entry(ULONG thread_input);
static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr);
static UINT dns_create(NX_DNS *dns_ptr);

/**
  * @brief  Application NetXDuo Initialization.
  */
UINT MX_NetXDuo_Init(VOID *memory_ptr)
{
  UINT ret = NX_SUCCESS;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

#if (USE_STATIC_ALLOCATION == 1)
  printf("\r\n--- INIT NETX DUO ---\r\n");

  CHAR *pointer;

  /* Initialize the NetX system. */
  nx_system_initialize();

  /* Create the Packet pool */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,  NX_PACKET_POOL_SIZE, TX_NO_WAIT) != TX_SUCCESS) return TX_POOL_ERROR;
  ret = nx_packet_pool_create(&AppPool, "Main Packet Pool", PAYLOAD_SIZE, pointer, NX_PACKET_POOL_SIZE);
  if (ret != NX_SUCCESS) return NX_NOT_ENABLED;

  /* Create the main NX_IP instance */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, 2 * DEFAULT_MEMORY_SIZE, TX_NO_WAIT) != TX_SUCCESS) return TX_POOL_ERROR;
  ret = nx_ip_create(&IpInstance, "Main Ip instance", NULL_ADDRESS, NULL_ADDRESS, &AppPool, nx_driver_emw3080_entry, pointer, 2 * DEFAULT_MEMORY_SIZE, DEFAULT_MAIN_PRIORITY);
  if (ret != NX_SUCCESS) return NX_NOT_ENABLED;

  /* Create the DHCP client */
  ret = nx_dhcp_create(&DhcpClient, &IpInstance, "DHCP Client");
  if (ret != NX_SUCCESS) return NX_NOT_ENABLED;

  /* Enable the ARP */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, ARP_MEMORY_SIZE, TX_NO_WAIT) != TX_SUCCESS) return TX_POOL_ERROR;
  ret = nx_arp_enable(&IpInstance, (VOID *)pointer, ARP_MEMORY_SIZE);
  if (ret != NX_SUCCESS) return NX_NOT_ENABLED;

  /* Enable the ICMP */
  ret = nx_icmp_enable(&IpInstance);
  if (ret != NX_SUCCESS) return NX_NOT_ENABLED;

  /* Enable the UDP */
  ret = nx_udp_enable(&IpInstance);
  if (ret != NX_SUCCESS) return NX_NOT_ENABLED;

  /* Enable the TCP */
  ret = nx_tcp_enable(&IpInstance);
  if (ret != NX_SUCCESS) return NX_NOT_ENABLED;

  /* Create the main thread */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, THREAD_MEMORY_SIZE, TX_NO_WAIT) != TX_SUCCESS) return TX_POOL_ERROR;
  ret = tx_thread_create(&AppMainThread, "App Main thread", App_Main_Thread_Entry, 0, pointer, THREAD_MEMORY_SIZE, DEFAULT_MAIN_PRIORITY, DEFAULT_MAIN_PRIORITY, TX_NO_TIME_SLICE, TX_AUTO_START);
  if (ret != TX_SUCCESS) return NX_NOT_ENABLED;

  /* Create the Test thread */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, THREAD_MEMORY_SIZE, TX_NO_WAIT) != TX_SUCCESS) return TX_POOL_ERROR;
  ret = tx_thread_create(&AppTestThread, "App Test Thread", App_Test_Thread_Entry, 0, pointer, THREAD_MEMORY_SIZE, TEST_PRIORITY, TEST_PRIORITY, TX_NO_TIME_SLICE, TX_DONT_START);
  if (ret != TX_SUCCESS) return NX_NOT_ENABLED;

  /* set DHCP notification callback  */
  tx_semaphore_create(&Semaphore, "DHCP Semaphore", 0);
#endif

  return ret;
}

static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr)
{
  tx_semaphore_put(&Semaphore);
}

UINT dns_create(NX_DNS *dns_ptr)
{
  UINT ret = NX_SUCCESS;
  ret = nx_dns_create(dns_ptr, &IpInstance, (UCHAR *)"DNS Client");
  if (ret) Error_Handler();
  ret = nx_dns_server_add(dns_ptr, USER_DNS_ADDRESS);
  if (ret) Error_Handler();
  return ret;
}

static VOID App_Main_Thread_Entry(ULONG thread_input)
{
  UINT ret = NX_SUCCESS;
  extern MX_WIFIObject_t *wifi_obj_get(void);
  MX_WIFIObject_t *pWifi = wifi_obj_get();

  ret = dns_create(&DnsClient);
  if (ret != NX_SUCCESS) { printf("\r\nDNS ERROR: 0x%02X\r\n", ret); Error_Handler(); }

  ret = nx_ip_address_change_notify(&IpInstance, ip_address_change_notify_callback, NULL);
  if (ret != NX_SUCCESS) { printf("\r\nNOTIFY ERROR: 0x%02X\r\n", ret); Error_Handler(); }

  printf("Connecting to WiFi: %s...\r\n", WIFI_SSID);

  if (MX_WIFI_Connect(pWifi, WIFI_SSID, WIFI_PASSWORD, MX_WIFI_SEC_WPA2_AES) != MX_WIFI_STATUS_OK)
  {
      printf("DISASTER: Couldn't connect to the network!\r\n");
      Error_Handler();
  }

  // CRITICAL: Disable power save so the router doesn't kill our session randomly
  MX_WIFI_station_powersave(pWifi, 0);

  ret = nx_dhcp_start(&DhcpClient);
  if (ret != NX_SUCCESS) { printf("\r\nDHCP ERROR: 0x%02X\r\n", ret); Error_Handler(); }

  if(tx_semaphore_get(&Semaphore, TX_WAIT_FOREVER) != TX_SUCCESS) Error_Handler();

  ret = nx_ip_address_get(&IpInstance, &IpAddress, &NetMask);
  if (ret != TX_SUCCESS) Error_Handler();

  PRINT_IP_ADDRESS(IpAddress);
  
  printf("\r\n--- WIFI SETUP SUCCESSFUL. KICKING OFF TEST ---\r\n");

  // fire up the barebones test thread
  tx_thread_resume(&AppTestThread);
  tx_thread_relinquish();
  return;
}

static VOID App_Test_Thread_Entry(ULONG thread_input)
{
    printf("\r\n--- WIFI STABILITY TEST (No bloat) ---\r\n");

    uint32_t seconds = 0;

    while(1)
    {
        // sleep for 1 sec
        tx_thread_sleep(1 * NX_IP_PERIODIC_RATE);
        seconds++;

        // ping the TCP/IP stack every 30s to see if the link is still alive
        if (seconds % 30 == 0)
        {
            ULONG link_status;
            nx_ip_interface_status_check(&IpInstance, 0, NX_IP_LINK_ENABLED, &link_status, NX_NO_WAIT);

            if (link_status & NX_IP_LINK_ENABLED) 
            {
                printf("[%lus] Wi-Fi is ALIVE. Waiting...\r\n", seconds);
            } 
            else 
            {
                printf("[%lus] [!!!] Wi-Fi DIED! (No LINK_ENABLED flag)\r\n", seconds);
            }
        }
    }
}