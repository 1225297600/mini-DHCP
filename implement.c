#include "drv_dhcp.h"

DRV_DHCP dhcp;

/*********************************** DHCP ***********************************/
void NET_DhcpDelayMs(uint16_t ms);
void NET_DhcpUdpSocket(uint16_t port);
void NET_DhcpUdpSend(uint8_t *ip, uint16_t port, uint8_t *data,uint16_t len);
void NET_DhcpSet(uint8_t *local_ip, uint8_t *gateway_ip, uint8_t *subnet_mask);
void NET_DhcpEventHandle(DHCP_STEP step);
void NET_DhcpTimerStart(void *body, void(*call)(void *body), bool cyc, uint16_t sec);
void NET_DhcpTimerStop(void);

/*********************************** 初始化 Init ***********************************/
void NET_DhcpInit(void){
	dhcp.hal.delay_ms = NET_DhcpDelayMs;
	dhcp.hal.udp_socket = NET_DhcpUdpSocket;
	dhcp.hal.udp_send = NET_DhcpUdpSend;
	dhcp.hal.set = NET_DhcpSet;
	dhcp.hal.event_handle = NET_DhcpEventHandle;
	dhcp.hal.timer_start = NET_DhcpTimerStart;
	dhcp.hal.timer_stop = NET_DhcpTimerStop;
	
	DRV_DHCP_Init(&dhcp, net_obj.w5500.local.mac, "NetworkModuleName");
}

/*********************************** 回调 CallBack ***********************************/
static void Handle(SOCKET_EVENT event, uint8_t socket_ch, uint8_t *data, uint16_t len){
	switch(event){
		case SOCKET_PHY:
			break;
		case SOCKET_CON:
			
			break;
		case SOCKET_DISCON:
			break;
		case SOCKET_RECIEVE:
			DRV_DHCP_UdpHandle(&dhcp, data, len);
			break;
		case SOCKET_TIMEOUT:
			break;
		case SOCKET_SEND_OK:
			break;
	}
}

/*********************************** 实现 implement ***********************************/
void NET_DhcpDelayMs(uint16_t ms){
  //毫秒延时
  //delay ms
}

void NET_DhcpUdpSocket(uint16_t port){
  //创建UDP的socket
  //Creat UDP socket
}

void NET_DhcpUdpSend(uint8_t *ip, uint16_t port, uint8_t *data,uint16_t len){
  //UDP 发生
  //UDP send
}

void NET_DhcpSet(uint8_t *local_ip, uint8_t *gateway_ip, uint8_t *subnet_mask){
  //设置网卡参数
  //set NetWork Module Config
}


void NET_DhcpEventHandle(DHCP_STEP step){
	switch(step){
		case DHCP_STEP_DISCOVER:
			//发送DISCOVER并等待OFFER
			//send DISCOVER，watting OFFER
			break;
		
		case DHCP_STEP_REQUEST:
			//发送REQUEST并等待ACK或NACK
			//send REQUES，watting ACK/NACK
			break;
			
		case DHCP_STEP_HIRE:
			if(net_obj.net.data.dhcp_ok){
				//Hire
        //续租
			}else{
				//First Hire
        //初次租用
			}
			break;
	}
}

void NET_DhcpTimerStart(void *body, void(*call)(void *body), bool cyc, uint16_t sec){
	//定时器启动
  //Timer Start
}

void NET_DhcpTimerStop(void){
  //定时器停止
	//Timer Stop
}



