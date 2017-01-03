#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <signal.h>

#include "bond.h"

#include "ctimer.h"
#include "cthread.h"

#include "biz.h"
#include "biz_types.h"
#include "biz_config.h"
#include "biz_net.h"
#include "biz_device.h"
#include "biz_remote_stream.h"
#include "biz_preview.h"
#include "biz_playback.h"

#include "biz_system_complex.h"
#include "hisi_sys.h"

#include "ctrlprotocol.h"
#include "net.h"


static VD_BOOL biz_inited = 0;




/*******************************************
bond 层接口声明，用以改变gui 控件
*******************************************/
#include "bond.h"

/*
gui起来之后要设置显示窗口信息

typedef struct hiRECT_S 
{ 
	 HI_S32 s32X; 
	 HI_S32 s32Y; 
	 HI_U32 u32Width; 
	 HI_U32 u32Height; 
}RECT_S;
*/

void term_exit(int signo)
{
	time_t cur;
	cur = time(NULL);
	cur += 3600*8;
	
	DBG_PRINT("!!!!!!recv signal(%d),SIGBUS=%d,SIGPIPE=%d,at %s", signo, SIGBUS, SIGPIPE, ctime(&cur));
	if(signo != 17)//子进程结束//SIGCHLD
	{
		//SIGINT=2;//SIGTSTP=20
		if(signo != SIGPIPE && signo != SIGINT && signo != SIGTSTP && signo != 21 && signo != SIGQUIT && signo != SIGWINCH)
		{
			//sleep(10);
			printf("process quit!!!\n");
			exit(-1);
		}
		else
		{
			//ignore "CTRL+C" "CTRL+Z"
			printf("???\n");
		}
	}
}

int BizFirstInit(void)
{
	biz_inited = 0;

	if(signal(SIGPIPE, term_exit) == SIG_ERR)
	{
		DBG_PRINT("Register SIGPIPE handler failed, %s\n", strerror(errno));
	}

	//hisi_3535
	if (HisiSysInit())
    {
        ERR_PRINT("HisiSysInit failed\n");
        return -FAILURE;
    }
	DBG_PRINT("HisiSysInit success\n");
	
	g_ThreadManager.RegisterMainThread(ThreadGetID());
	g_TimerManager.Start();
	
	if (BizConfigInit())
	{
		ERR_PRINT("BizConfigInit failed\n");
		return -FAILURE;
	}
	DBG_PRINT("BizConfigInit success\n");

	if (BizNetInit())
	{
		ERR_PRINT("BizNetInit failed\n");
		return -FAILURE;
	}
	DBG_PRINT("BizNetInit success\n");
	
	if (BizDeviceFirstInit())
	{
		ERR_PRINT("BizDeviceFirstInit failed\n");
		return -FAILURE;
	}
	DBG_PRINT("BizDeviceFirstInit success\n");
	
	return SUCCESS;
}

int BizSecondInit(void)
{
	if (BizDeviceSecondInit())
	{
		ERR_PRINT("BizDeviceSecondInit failed\n");
		return -FAILURE;
	}
	DBG_PRINT("BizDeviceSecondInit success\n");

	if (BizStreamInit())
	{
		ERR_PRINT("BizStreamInit failed\n");
		return -FAILURE;
	}
	DBG_PRINT("BizStreamInit success\n");
	
	if (BizPreviewInit())
	{
		ERR_PRINT("BizPreviewInit failed\n");
		return -FAILURE;
	}
	DBG_PRINT("BizPreviewInit success\n");

	if (BizPlaybackInit())
	{
		ERR_PRINT("BizPlaybackInit failed\n");
		return -FAILURE;
	}
	DBG_PRINT("BizPlaybackInit success\n");
	
	if (BizSystemComplexInit())
	{
		ERR_PRINT("BizSystemComplexInit failed\n");
		return -FAILURE;
	}
	DBG_PRINT("BizSystemComplexInit success\n");

	biz_inited = 1;
	
	return SUCCESS;
}


//设置参数
int BizSetNetParam(SConfigNetParam &snet_param)
{
	return BizNetSetNetParam(snet_param);
}

//查询gui 是否准备好接收通知消息
VD_BOOL BizGuiIsReady()
{
	return notifyIsReady();
}


int BizEventCB(SBizEventPara* pSBizEventPara)
{
	EMBIZEVENT emType = pSBizEventPara->emType;

	DBG_PRINT("emType: %d\n", emType);

	switch (emType)
	{
		case EM_BIZ_EVENT_PLAYBACK_START: //回放开始
		{
			//hisi process
			hisi_chn_start(0);
			
		} break;
		
		case EM_BIZ_EVENT_PLAYBACK_DONE: //回放结束
		{
			//hisi process
			hisi_chn_stop(0);

			//notify gui
			SPlaybackNotify_t para;
			para.msg_type = 0;
			para.dev_ip = pSBizEventPara->playback_para.dev_ip;
			notifyPlaybackInfo(&para);
		} break;
		
		case EM_BIZ_EVENT_PLAYBACK_NETWORK_ERR: //回放时发生网络错误
		{
			//hisi process
			hisi_chn_stop(0);

			//notify gui
			SPlaybackNotify_t para;
			para.msg_type = 1;
			para.dev_ip = pSBizEventPara->playback_para.dev_ip;
			notifyPlaybackInfo(&para);
		} break;	

		default:
			break;
	}
	
	return 0;
}

//处理网络客户机命令(回应客户机命令)
u16 BizDealClientCmd(
	CPHandle 	cph,
	u16 		event,
	char*		pbyMsgBuf,
	int 		msgLen,
	char*		pbyAckBuf,
	int*		pAckLen )
{
	DBG_PRINT("client cmd: %d\n", event);

	if(pAckLen)
	{
		*pAckLen = 0;
	}
	
	switch (event)
	{
		case CTRL_CMD_GETDEVICEINFO:
			ifly_DeviceInfo_t dev_info;
					
			if (BizConfigGetDevInfo(&dev_info))
			{
				ERR_PRINT("BizConfigGetDevInfo failed\n");
				return CTRL_FAILED_RESOURCE;
			}
			else
			{
				if(pAckLen)
				{
					*pAckLen = sizeof(ifly_DeviceInfo_t);
				}

				dev_info.devicePort = htons(dev_info.devicePort);
				memcpy(pbyAckBuf, &dev_info, sizeof(ifly_DeviceInfo_t));
			}
			
			break;
		default:
			ERR_PRINT("client cmd: %d nonsupport\n", event);
			return CTRL_FAILED_COMMAND;
			
	}
	
	
	return CTRL_SUCCESS;
}

//处理网络客户机数据连接
int BizDealClientDataLink(
	int sock, 
	ifly_TCP_Stream_Req *preq_hdr, 
	struct sockaddr_in *pcli_addr_in)
{
	
	return SUCCESS;
}

//处理网络服务器事件通知
void BizDealSvrNotify(u32 dev_ip, u16 event, s8 *pbyMsgBuf, int msgLen)
{
	struct in_addr in;
	in.s_addr = dev_ip;
		
	DBG_PRINT("svr IP: %s, event: %d\n", inet_ntoa(in), event);
}



