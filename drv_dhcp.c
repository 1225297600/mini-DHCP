#include "drv_dhcp.h"
#include <string.h>
#include <stdlib.h>

typedef struct {
	uint8_t  op;            ///< @ref DHCP_BOOTREQUEST or @ref DHCP_BOOTREPLY
	uint8_t  htype;         ///< @ref DHCP_HTYPE10MB or @ref DHCP_HTYPE100MB
	uint8_t  hlen;          ///< @ref DHCP_HLENETHERNET
	uint8_t  hops;          ///< @ref DHCP_HOPS
	uint32_t xid;           ///< @ref DHCP_XID  This increase one every DHCP transaction.
	uint16_t secs;          ///< @ref DHCP_SECS
	uint16_t flags;         ///< @ref DHCP_FLAGSBROADCAST or @ref DHCP_FLAGSUNICAST
	uint8_t  ciaddr[4];     ///< @ref Request IP to DHCP sever
	uint8_t  yiaddr[4];     ///< @ref Offered IP from DHCP server
	uint8_t  siaddr[4];     ///< No use 
	uint8_t  giaddr[4];     ///< No use
	uint8_t  chaddr[16];    ///< DHCP client 6bytes MAC address. Others is filled to zero
	uint8_t  sname[64];     ///< No use
	uint8_t  file[128];     ///< No use
	uint8_t  OPT[312]; ///< Option
}RIP_MSG;

typedef enum{
	DHCP_DISCOVER            =1,        ///< DISCOVER message in OPT of @ref RIP_MSG
	DHCP_OFFER               =2,        ///< OFFER message in OPT of @ref RIP_MSG
	DHCP_REQUEST             =3,        ///< REQUEST message in OPT of @ref RIP_MSG
	DHCP_DECLINE             =4,        ///< DECLINE message in OPT of @ref RIP_MSG
	DHCP_ACK                 =5,        ///< ACK message in OPT of @ref RIP_MSG
	DHCP_NAK                 =6,        ///< NACK message in OPT of @ref RIP_MSG
	DHCP_RELEASE             =7,        ///< RELEASE message in OPT of @ref RIP_MSG. No use
	DHCP_INFORM              =8,        ///< INFORM message in OPT of @ref RIP_MSG. No use
}DHCP_TYPE;

/**********************************  **********************************/

static void DRV_DHCP_DealyMs(DRV_DHCP *dhcp, uint16_t ms){
	if(dhcp->hal.delay_ms != NULL){
		dhcp->hal.delay_ms(ms);
	}else{
		
	}
}

static void DRV_DHCP_UdpSocket(DRV_DHCP *dhcp, uint16_t port){
	if(dhcp->hal.udp_socket != NULL){
		dhcp->hal.udp_socket(port);
	}else{
		
	}
}

static void DRV_DHCP_UdpSend(DRV_DHCP *dhcp, uint8_t *ip, uint16_t port, uint8_t *data, uint16_t len){
	if(dhcp->hal.udp_send != NULL){
		dhcp->hal.udp_send(ip, port, data, len);
	}else{
		
	}
}

static void DRV_DHCP_Set(DRV_DHCP *dhcp, uint8_t *local_ip, uint8_t *gateway_ip, uint8_t *subnet_mask){
	if(dhcp->hal.set != NULL){
		dhcp->hal.set(local_ip, gateway_ip, subnet_mask);
	}else{
		
	}
}

static void DRV_DHCP_EventHandle(DRV_DHCP *dhcp){
	if(dhcp->hal.event_handle != NULL){
		dhcp->hal.event_handle(dhcp->status.step);
	}else{
		
	}
}

static void DRV_DHCP_TimerStart(DRV_DHCP *dhcp, void(*call)(void *body), bool cyc, uint16_t sec){
	if(dhcp->hal.timer_start != NULL){
		dhcp->hal.timer_start(dhcp, call, cyc, sec);
	}else{
		
	}
}
static void DRV_DHCP_TimerStop(DRV_DHCP *dhcp){
	if(dhcp->hal.timer_stop != NULL){
		dhcp->hal.timer_stop();
	}else{
		
	}
}
/**********************************  **********************************/

static void DRV_DHCP_Timeout(void *body);
static void DRV_DHCP_SendDiscover(DRV_DHCP *dhcp);
static void DRV_DHCP_SendRequest(DRV_DHCP *dhcp);

/**********************************  **********************************/

void DRV_DHCP_Init(DRV_DHCP *dhcp, uint8_t *mac, char *name){
	memset(&dhcp->status, 0, sizeof(dhcp->status));
	memmove(dhcp->init.mac, mac, 6);
	memset(dhcp->init.file_name, 0, 128);
	memmove(dhcp->init.file_name, name, strlen(name));
	dhcp->status.xid = rand();
	
	DRV_DHCP_TimerStop(dhcp);
}

void DRV_DHCP_Boot(DRV_DHCP *dhcp, uint16_t timeout_sec){
	DRV_DHCP_UdpSocket(dhcp, 68);
	
	dhcp->status.step = DHCP_STEP_DISCOVER;
	dhcp->status.event = true;
	dhcp->status.run = true;
	if(timeout_sec < 15){
		timeout_sec = 15;
	}
	dhcp->init.timeout_sec = timeout_sec;
	DRV_DHCP_TimerStart(dhcp, DRV_DHCP_Timeout, true, 2);//2秒后启动第一次
}

void DRV_DHCP_Shut(DRV_DHCP *dhcp){
	dhcp->status.run = false;
	DRV_DHCP_TimerStop(dhcp);
}

void DRV_DHCP_Poller(DRV_DHCP *dhcp){
	if(dhcp->status.run == false){
		return;//没有运行
	}
	if(dhcp->status.event){
		dhcp->status.event = false;
		switch(dhcp->status.step){
			case DHCP_STEP_DISCOVER:
				///<发送DISCOVER并等待OFFER
				DRV_DHCP_SendDiscover(dhcp);
				break;
			
			case DHCP_STEP_REQUEST:
				///＜发送REQUEST并等待ACK或NACK
				DRV_DHCP_SendRequest(dhcp);
				break;
			
			case DHCP_STEP_HIRE:
				//续租
				DRV_DHCP_SendRequest(dhcp);
				break;
		}
		DRV_DHCP_EventHandle(dhcp);
	}
}

void DRV_DHCP_UdpHandle(DRV_DHCP *dhcp, uint8_t *data,uint16_t len){
	if(dhcp->status.run == false){
		return;//没有运行
	}
	RIP_MSG msg;
	
	memset(&msg, 0, sizeof(RIP_MSG));
	memmove(&msg, data, len);
	
	switch(dhcp->status.step){
		case DHCP_STEP_DISCOVER:
			///<发送DISCOVER并等待OFFER
			if(msg.OPT[6] == DHCP_OFFER){
				dhcp->status.step = DHCP_STEP_REQUEST;
				dhcp->status.event = true;
				memmove(dhcp->status.local_ip, msg.yiaddr, 4);
				memmove(dhcp->status.gateway_ip, &msg.OPT[9], 4);
				memmove(dhcp->status.subnet_mask, &msg.OPT[21], 4);
			}
			break;
		
		case DHCP_STEP_REQUEST:
			///＜发送REQUEST并等待ACK或NACK
			if(msg.OPT[6] == DHCP_ACK){
				DRV_DHCP_Set(dhcp, dhcp->status.local_ip, dhcp->status.gateway_ip, dhcp->status.subnet_mask);
				dhcp->status.hire_timeout_sec = msg.OPT[17]<<8 | msg.OPT[18];
				DRV_DHCP_TimerStart(dhcp, DRV_DHCP_Timeout, true, dhcp->status.hire_timeout_sec/2);
				
				dhcp->status.step = DHCP_STEP_HIRE;
				dhcp->status.event = true;
			}
			if(msg.OPT[6] == DHCP_NAK){
				memset(dhcp->status.local_ip, 0, 4);
				memset(dhcp->status.gateway_ip, 0, 4);
				memset(dhcp->status.subnet_mask, 0, 4);
				DRV_DHCP_Set(dhcp, dhcp->status.local_ip, dhcp->status.gateway_ip, dhcp->status.subnet_mask);
				
				dhcp->status.step = DHCP_STEP_DISCOVER;
				dhcp->status.event = true;
			}
			break;
			
			case DHCP_STEP_HIRE:
				//续租
				if(msg.OPT[6] == DHCP_ACK){
					dhcp->status.hire_timeout_sec = msg.OPT[17]<<8 | msg.OPT[18];
					DRV_DHCP_TimerStart(dhcp, DRV_DHCP_Timeout, true, dhcp->status.hire_timeout_sec/2);
				}
				if(msg.OPT[6] == DHCP_NAK){
					memset(dhcp->status.local_ip, 0, 4);
					
					dhcp->status.step = DHCP_STEP_DISCOVER;
					dhcp->status.event = true;
				}
				break;
	}
}

/**********************************  **********************************/

static void DRV_DHCP_Timeout(void *body){
	DRV_DHCP *dhcp = body;
	if(dhcp->status.step == DHCP_STEP_HIRE){
		//继续租用
		dhcp->status.step = DHCP_STEP_HIRE;
		dhcp->status.event = true;
		DRV_DHCP_TimerStart(dhcp, DRV_DHCP_Timeout, true, dhcp->status.hire_timeout_sec);
	}else{
		dhcp->status.event = true;//超时定时
		DRV_DHCP_TimerStart(dhcp, DRV_DHCP_Timeout, true, dhcp->init.timeout_sec);
	}
}

static void DRV_DHCP_RipMsgInit(DRV_DHCP *dhcp, RIP_MSG *rip_msg, bool broadcast)
{
	uint8_t* ptmp;
	uint8_t  i;
	
	rip_msg->op      = 1;//1客户端发出；2服务器发出
	rip_msg->htype   = 1;//1以太网
	rip_msg->hlen    = 6;//mac的长度
	rip_msg->hops    = 0;//中继计数，不能大于4，内网0
	rip_msg->xid 		 = dhcp->status.xid;//XID DHCP REQUEST 时产生的数值，以作 DHCPREPLY 时的依据
	rip_msg->secs    = 0;//客户端启动长
	
	ptmp              = (uint8_t*)(&rip_msg->flags);//指向
	//FLAGS 1单播(0x0000)；2广播(0x8000)
	if(broadcast){
		*(ptmp+0)         = 0x80;
		*(ptmp+1)         = 0x00;
	}else{
		*(ptmp+0)         = 0x00;
		*(ptmp+1)         = 0x00;
	}
	//客户端IP，想要分配的地址
	memmove(rip_msg->ciaddr, dhcp->status.local_ip, 4);
	//you IP，分配的地址
	memset(&rip_msg->yiaddr, 0, 4);
	//serice IP，网络开机，程序地址
	memset(&rip_msg->siaddr, 0, 4);
	//gatway IP，跨网域时，网关地址
	memset(&rip_msg->giaddr, 0, 4);
	//MAC
	memmove(rip_msg->chaddr, dhcp->init.mac, 6);
	memset(&rip_msg->chaddr[6], 0, 10);
	//服务器返回名称
	memset(&rip_msg->sname, 0, 64);
	//NAME
	memmove(rip_msg->file, dhcp->init.file_name, 128);
	// MAGIC_COOKIE = 0x63825363 固定的
	rip_msg->OPT[0] = 0x63;
	rip_msg->OPT[1] = 0x82;
	rip_msg->OPT[2] = 0x53;
	rip_msg->OPT[3] = 0x63;
}




static void DRV_DHCP_SendDiscover(DRV_DHCP *dhcp)
{
	uint8_t udp_ip[4]={255,255,255,255};
	uint16_t k = 0;
  RIP_MSG rip_msg;
	
  DRV_DHCP_RipMsgInit(dhcp, &rip_msg, true);
	
	k = 4;
	
	// Option Request Param
	rip_msg.OPT[k++] = 53;//dhcpMessageType
	rip_msg.OPT[k++] = 0x01;
	rip_msg.OPT[k++] = 0x01;//DHCP_DISCOVER
	
	// Client identifier
	rip_msg.OPT[k++] = 61;//dhcpClientIdentifier
	rip_msg.OPT[k++] = 0x07;
	rip_msg.OPT[k++] = 0x01;//Ethernet
	memmove(&rip_msg.OPT[k], dhcp->init.mac, 6);
	k+=6;
	
	// host name
	rip_msg.OPT[k++] = 12;//hostName
	rip_msg.OPT[k++] = strlen(dhcp->init.file_name);          // fill zero length of hostname 
	memmove(&rip_msg.OPT[k], dhcp->init.file_name, strlen(dhcp->init.file_name));
	k += strlen(dhcp->init.file_name);
	
	rip_msg.OPT[k++] = 55;//dhcpParamRequest
	rip_msg.OPT[k++] = 0x06;	// length of request
	rip_msg.OPT[k++] = 1;//subnetMask
	rip_msg.OPT[k++] = 3;//routersOnSubnet
	rip_msg.OPT[k++] = 6;//dns
	rip_msg.OPT[k++] = 15;//domainName
	rip_msg.OPT[k++] = 58;//dhcpT1value
	rip_msg.OPT[k++] = 59;//dhcpT2value
	rip_msg.OPT[k++] = 255;//endOption
	
	memset(&rip_msg.OPT[k], 0, 312-k);
	
	memset(dhcp->status.local_ip, 0, 4);
	memset(dhcp->status.gateway_ip, 255, 4);
	DRV_DHCP_Set(dhcp, dhcp->status.local_ip, dhcp->status.gateway_ip, dhcp->status.subnet_mask);
	DRV_DHCP_UdpSend(dhcp, udp_ip, 67, (uint8_t*)&rip_msg, sizeof(RIP_MSG));
}

static void DRV_DHCP_SendRequest(DRV_DHCP *dhcp)
{
	uint8_t udp_ip[4]={255, 255, 255, 255};
	uint16_t k = 0;
  RIP_MSG rip_msg;
	
  DRV_DHCP_RipMsgInit(dhcp, &rip_msg, true);
	
	memmove(rip_msg.yiaddr, dhcp->status.local_ip, 4);//想要继续使用 dhcp->status.local_ip
	
  k = 4;      // beacaue MAGIC_COOKIE already made by makeDHCPMSG()
	
	// Option Request Param.
	rip_msg.OPT[k++] = 53;//dhcpMessageType
	rip_msg.OPT[k++] = 0x01;
	rip_msg.OPT[k++] = 3;//DHCP_REQUEST
	
	rip_msg.OPT[k++] = 61;//dhcpClientIdentifier
	rip_msg.OPT[k++] = 0x07;
	rip_msg.OPT[k++] = 0x01;//Ethernet
	memmove(&rip_msg.OPT[k], dhcp->init.mac, 6);
	k+=6;
	
	if(1)
	{
		rip_msg.OPT[k++] = 50;//dhcpRequestedIPaddr
		rip_msg.OPT[k++] = 0x04;
		memmove(&rip_msg.OPT[k], dhcp->status.local_ip, 4);
		k+=4;
		
		rip_msg.OPT[k++] = 54;//dhcpServerIdentifier
		rip_msg.OPT[k++] = 0x04;
		memmove(&rip_msg.OPT[k], dhcp->status.gateway_ip, 4);
		k+=4;
	}
	
	// host name
	rip_msg.OPT[k++] = 12;//hostName
	rip_msg.OPT[k++] = strlen(dhcp->init.file_name);          // fill zero length of hostname 
	memmove(&rip_msg.OPT[k], dhcp->init.file_name, strlen(dhcp->init.file_name));
	k += strlen(dhcp->init.file_name);
	
	rip_msg.OPT[k++] = 55;//dhcpParamRequest
	rip_msg.OPT[k++] = 0x08;
	rip_msg.OPT[k++] = 1;//subnetMask
	rip_msg.OPT[k++] = 3;//routersOnSubnet
	rip_msg.OPT[k++] = 6;//dns
	rip_msg.OPT[k++] = 15;//domainName
	rip_msg.OPT[k++] = 58;//dhcpT1value
	rip_msg.OPT[k++] = 59;//dhcpT2value
	rip_msg.OPT[k++] = 31;//performRouterDiscovery
	rip_msg.OPT[k++] = 33;//staticRoute
	rip_msg.OPT[k++] = 255;//endOption
	
	memset(&rip_msg.OPT[k], 0, 312-k);
	
	DRV_DHCP_UdpSend(dhcp, udp_ip, 67, (uint8_t*)&rip_msg, sizeof(RIP_MSG));
}


