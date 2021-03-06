#ifndef __CIRCULAR_BUFFER_H__
#define __CIRCULAR_BUFFER_H__

#include <stdlib.h>
#include "types.h"

typedef enum
{
	//to biz_device
	EM_MQ_MSG_DEV_NET_REQ=0x10, 	//网络命令请求
	EM_MQ_MSG_DEV_STREAM_STATUS,		//流状态通知
	
	//to stream
	EM_MQ_MSG_STREAM_STATUS=0x20,		//流状态通知
} EM_MQ_MSG_TYPE;

typedef struct
{
	u32 msg_type;
	u32 msg_len;	//不包含头
} SMQHdr_t;	//消息队列头


class CcircularBuffer
{
public:
	explicit CcircularBuffer(uint buf_size = 0);
	virtual ~CcircularBuffer();	
	virtual int CreateBuffer(); //鐢宠鐜舰缂撳啿绌洪棿	
	virtual void Reset();	
	virtual void PutRst();
	virtual int Put(u8 *pbuf, u32 len);
	virtual void GetRst();
	virtual int Get(u8 *pbuf, u32 len);
	
private:
	CcircularBuffer(const CcircularBuffer &)
	{
		
	}
	
protected:
	uchar *m_pbuf;
	uint m_buf_size;//蹇呴』鏄�2鐨凬娆℃柟锛�
	uint m_read_p;//鍐欑储寮�,璇荤储寮�
	uint m_pre_read_p;
	uint m_write_p;
	uint m_pre_write_p;
};

#endif
	
	