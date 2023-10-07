#include "drv_dhcp.h"

DRV_DHCP dhcp;

/*********************************** DHCP ***********************************/
void MOD_NET_DhcpDelayMs(uint16_t ms);
void MOD_NET_DhcpUdpSocket(uint16_t port);
void MOD_NET_DhcpUdpSend(uint8_t *ip, uint16_t port, uint8_t *data,uint16_t len);
void MOD_NET_DhcpSet(uint8_t *local_ip, uint8_t *gateway_ip, uint8_t *subnet_mask);
void MOD_NET_DhcpEventHandle(DHCP_STEP step);
void MOD_NET_DhcpTimerStart(void *body, void(*call)(void *body), bool cyc, uint16_t sec);
void MOD_NET_DhcpTimerStop(void);

/*********************************** ��ʼ�� Init ***********************************/
void MOD_NET_DhcpInit(void){
	dhcp.hal.delay_ms = MOD_NET_DhcpDelayMs;
	dhcp.hal.udp_socket = MOD_NET_DhcpUdpSocket;
	dhcp.hal.udp_send = MOD_NET_DhcpUdpSend;
	dhcp.hal.set = MOD_NET_DhcpSet;
	dhcp.hal.event_handle = MOD_NET_DhcpEventHandle;
	dhcp.hal.timer_start = MOD_NET_DhcpTimerStart;
	dhcp.hal.timer_stop = MOD_NET_DhcpTimerStop;
	
	DRV_DHCP_Init(&dhcp, net_obj.w5500.local.mac, "NetworkModuleName");
}

/*********************************** �ص� CallBack ***********************************/
static void MOD_Handle(SOCKET_EVENT event, uint8_t socket_ch, uint8_t *data, uint16_t len){
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

/*********************************** ʵ�� implement ***********************************/
void MOD_NET_DhcpDelayMs(uint16_t ms){
  //������ʱ
  //delay ms
}

void MOD_NET_DhcpUdpSocket(uint16_t port){
  //����UDP��socket
  //Creat UDP socket
}

void MOD_NET_DhcpUdpSend(uint8_t *ip, uint16_t port, uint8_t *data,uint16_t len){
  //UDP ����
  //UDP send
}

void MOD_NET_DhcpSet(uint8_t *local_ip, uint8_t *gateway_ip, uint8_t *subnet_mask){
  //������������
  //set NetWork Module Config
}


void MOD_NET_DhcpEventHandle(DHCP_STEP step){
	switch(step){
		case DHCP_STEP_DISCOVER:
			//����DISCOVER���ȴ�OFFER
			//send DISCOVER��watting OFFER
			break;
		
		case DHCP_STEP_REQUEST:
			//����REQUEST���ȴ�ACK��NACK
			//send REQUES��watting ACK/NACK
			break;
			
		case DHCP_STEP_HIRE:
			if(net_obj.net.data.dhcp_ok){
				//Hire
        //����
			}else{
				//First Hire
        //��������
			}
			break;
	}
}

void MOD_NET_DhcpTimerStart(void *body, void(*call)(void *body), bool cyc, uint16_t sec){
	//��ʱ������
  //Timer Start
}

void MOD_NET_DhcpTimerStop(void){
  //��ʱ��ֹͣ
	//Timer Stop
}



