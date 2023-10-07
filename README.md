# mini-DHCP



关于DCHP的解析，见：
About DHCP protocol，See this page：

https://oshwhub.com/article/DHCPxie-yi-fen-xi-he-dan-pian-ji-Cyu-yan-qu-dong-bian-xie



如何使用：
How to use：

	/*********************************** DHCP ***********************************/
	void MOD_NET_DhcpDelayMs(uint16_t ms);
	void MOD_NET_DhcpUdpSocket(uint16_t port);
	void MOD_NET_DhcpUdpSend(uint8_t *ip, uint16_t port, uint8_t *data,uint16_t len);
	void MOD_NET_DhcpSet(uint8_t *local_ip, uint8_t *gateway_ip, uint8_t *subnet_mask);
	void MOD_NET_DhcpEventHandle(DHCP_STEP step);
	void MOD_NET_DhcpTimerStart(void *body, void(*call)(void *body), bool cyc, uint16_t sec);
	void MOD_NET_DhcpTimerStop(void);
	
	/*********************************** 初始化 Init ***********************************/
	void MOD_NET_DhcpInit(void){
		net_obj.dhcp.hal.delay_ms = MOD_NET_DhcpDelayMs;
		net_obj.dhcp.hal.udp_socket = MOD_NET_DhcpUdpSocket;
		net_obj.dhcp.hal.udp_send = MOD_NET_DhcpUdpSend;
		net_obj.dhcp.hal.set = MOD_NET_DhcpSet;
		net_obj.dhcp.hal.event_handle = MOD_NET_DhcpEventHandle;
		net_obj.dhcp.hal.timer_start = MOD_NET_DhcpTimerStart;
		net_obj.dhcp.hal.timer_stop = MOD_NET_DhcpTimerStop;
		
		DRV_DHCP_Init(&net_obj.dhcp, net_obj.w5500.local.mac, "LightLine-2-TypeE[1]");
	}
	
	/*********************************** 回调 CallBack ***********************************/
	static void MOD_Handle(SOCKET_EVENT event, uint8_t socket_ch, uint8_t *data, uint16_t len){
		switch(event){
			case SOCKET_PHY:
				break;
			case SOCKET_CON:
				
				break;
			case SOCKET_DISCON:
				break;
			case SOCKET_RECIEVE:
				DRV_DHCP_UdpHandle(&net_obj.dhcp, data, len);
				break;
			case SOCKET_TIMEOUT:
				break;
			case SOCKET_SEND_OK:
				break;
		}
	}
	
	/*********************************** 实现 implement ***********************************/
	void MOD_NET_DhcpDelayMs(uint16_t ms){
	  //毫秒延时
	  //delay ms
	}
	
	void MOD_NET_DhcpUdpSocket(uint16_t port){
	  //创建UDP的socket
	  //Creat UDP socket
	}
	
	void MOD_NET_DhcpUdpSend(uint8_t *ip, uint16_t port, uint8_t *data,uint16_t len){
	  //UDP 发生
	  //UDP send
	}
	
	void MOD_NET_DhcpSet(uint8_t *local_ip, uint8_t *gateway_ip, uint8_t *subnet_mask){
	  //设置网卡参数
	  //set NetWork Module Config
	}
	
	
	void MOD_NET_DhcpEventHandle(DHCP_STEP step){
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
	
	void MOD_NET_DhcpTimerStart(void *body, void(*call)(void *body), bool cyc, uint16_t sec){
		//定时器启动
	  //Timer Start
	}
	
	void MOD_NET_DhcpTimerStop(void){
	  //定时器停止
		//Timer Stop
	}
	


