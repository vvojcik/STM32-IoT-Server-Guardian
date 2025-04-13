/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_netxduo.h
  * @author  MCD Application Team
  * @brief   NetXDuo applicative header file
  ******************************************************************************
  */
/* USER CODE END Header */
#ifndef __APP_NETXDUO_H__
#define __APP_NETXDUO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "nx_api.h"
#include "main.h"
#include "nxd_dhcp_client.h"
#include "nxd_dns.h"
#include "nx_driver_emw3080.h"

/* Threads configuration */
#define PAYLOAD_SIZE                1544
#define NX_PACKET_POOL_SIZE         (( PAYLOAD_SIZE + sizeof(NX_PACKET)) * 20)
#define DEFAULT_MEMORY_SIZE         1024
#define ARP_MEMORY_SIZE             DEFAULT_MEMORY_SIZE
#define DEFAULT_MAIN_PRIORITY       10
#define TEST_PRIORITY               12
#define THREAD_MEMORY_SIZE          4 * DEFAULT_MEMORY_SIZE
#define APP_QUEUE_SIZE              10

#define NULL_ADDRESS                0
#define USER_DNS_ADDRESS            IP_ADDRESS(1, 1, 1, 1)

#define DEFAULT_TIMEOUT             10 * NX_IP_PERIODIC_RATE

/* Exported macro ------------------------------------------------------------*/
#define PRINT_IP_ADDRESS(addr)           do { \
                                              printf("STM32 %s: %lu.%lu.%lu.%lu \r\n", #addr, \
                                                (addr >> 24) & 0xff,                        \
                                                  (addr >> 16) & 0xff,                      \
                                                    (addr >> 8) & 0xff,                     \
                                                      (addr & 0xff));                       \
                                            } while(0)

/* Exported functions prototypes ---------------------------------------------*/
UINT MX_NetXDuo_Init(VOID *memory_ptr);

#ifdef __cplusplus
}
#endif
#endif /* __APP_NETXDUO_H__ */