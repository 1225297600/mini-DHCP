#ifndef __DRV_DHCP_H
#define __DRV_DHCP_H
#include <stdint.h>
#include <stdbool.h>

typedef enum{
	DHCP_STEP_DISCOVER,///<发送DISCOVER并等待OFFER
	DHCP_STEP_REQUEST,///＜发送REQUEST并等待ACK或NACK
	DHCP_STEP_HIRE,
}DHCP_STEP;

typedef struct{
	struct
	{
			void (*delay_ms)(uint16_t ms);
			void (*udp_socket)(uint16_t port);
			void (*udp_send)(uint8_t *ip, uint16_t port, uint8_t *data,uint16_t len);
			void (*set)(uint8_t *local_ip, uint8_t *gateway_ip, uint8_t *subnet_mask);
			void (*event_handle)(DHCP_STEP step);
			void (*timer_start)(void *body, void(*call)(void *body), bool cyc, uint16_t sec);
			void (*timer_stop)(void);
	}hal;
	
  struct
	{
		uint8_t mac[6];
		char file_name[128];
		uint16_t timeout_sec;
	}init;
	
  struct
	{
		bool run;
		DHCP_STEP step;
		bool event;
		uint8_t gateway_ip[4];
		uint8_t local_ip[4];
		uint8_t subnet_mask[4];
		uint16_t hire_timeout_sec;
		uint32_t xid;
	}status;
	
}DRV_DHCP;

void DRV_DHCP_Init(DRV_DHCP *dhcp, uint8_t *mac, char *name);
void DRV_DHCP_Poller(DRV_DHCP *dhcp);
void DRV_DHCP_UdpHandle(DRV_DHCP *dhcp, uint8_t *data,uint16_t len);

void DRV_DHCP_Boot(DRV_DHCP *dhcp, uint16_t timeout_sec);
void DRV_DHCP_Shut(DRV_DHCP *dhcp);

#endif
