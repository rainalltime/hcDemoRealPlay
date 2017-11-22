#include <stdio.h>
#include <iostream>
#include "Windows.h"
#include "string.h"
#include "HCNetSDK.h"
using namespace std;
void main() {
	int i = 0;
	BYTE byIPID, byIPIDHigh;
	int iDevInfoIndex, iGroupNO, iIPCh, iDevID;
	DWORD dwReturned = 0;
	//---------------------------------------
	// 初始化
	NET_DVR_Init();
	//设置连接时间与重连时间
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);
	//---------------------------------------
	// 注册设备
	LONG lUserID;
	NET_DVR_DEVICEINFO_V30 struDeviceInfo;
	lUserID = NET_DVR_Login_V30("192.168.2.218", 8000, "admin", "Ll168888", &struDeviceInfo);
	if (lUserID < 0)
	{
		printf("Login error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Cleanup();
		return;
	}
	printf("The max number of analog channels: %d\n", struDeviceInfo.byChanNum); //模拟通道个数
	printf("The max number of IP channels: %d\n", struDeviceInfo.byIPChanNum);//IP 通道个数
																			  //获取 IP 通道参数信息
	NET_DVR_IPPARACFG_V40 IPAccessCfgV40;
	memset(&IPAccessCfgV40, 0, sizeof(NET_DVR_IPPARACFG));
	iGroupNO = 0; if (!NET_DVR_GetDVRConfig(lUserID, NET_DVR_GET_IPPARACFG_V40, iGroupNO, &IPAccessCfgV40,
		sizeof(NET_DVR_IPPARACFG_V40), &dwReturned))
	{
		printf("NET_DVR_GET_IPPARACFG_V40 error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return;
	}
	else
	{
		for (i = 0; i<IPAccessCfgV40.dwDChanNum; i++)
		{
			switch (IPAccessCfgV40.struStreamMode[i].byGetStreamType)
			{
			case 0: //直接从设备取流
				if (IPAccessCfgV40.struStreamMode[i].uGetStream.struChanInfo.byEnable)
				{
					byIPID = IPAccessCfgV40.struStreamMode[i].uGetStream.struChanInfo.byIPID;
					byIPIDHigh = IPAccessCfgV40.struStreamMode[i].uGetStream.struChanInfo.byIPIDHigh;
					iDevInfoIndex = byIPIDHigh * 256 + byIPID - 1 - iGroupNO * 64;
					printf("IP channel no.%d is online, IP: %s\n", i + 1, \
						IPAccessCfgV40.struIPDevInfo[iDevInfoIndex].struIP.sIpV4);
				}
				break;
			case 1: //从流媒体取流
				if (IPAccessCfgV40.struStreamMode[i].uGetStream.struPUStream.struStreamMediaSvrCfg.byValid)
				{
					printf("IP channel %d connected with the IP device by stream server.\n", i + 1);
					printf("IP of stream server: %s, IP of IP device: %s\n", \
						IPAccessCfgV40.struStreamMode[i].uGetStream.struPUStream.struStreamMediaSvrCfg.struDevIP.sIpV4, \
						IPAccessCfgV40.struStreamMode[i].uGetStream.struPUStream.struDevChanInfo.struIP.sIpV4);
				}
				break;
			default:
				break;
			}
		}
	}
	//获取 IP 通道相关参数(以获取 IP 通道图像参数为例)
	DWORD iIPChanIndex = IPAccessCfgV40.dwStartDChan; //起始通道，即 IP 通道 01
	NET_DVR_PICCFG_V40 m_struPicCfg; //存放通道图像参数的结构体
	memset(&m_struPicCfg, 0, sizeof(NET_DVR_PICCFG_V40));
	if (!NET_DVR_GetDVRConfig(lUserID, NET_DVR_GET_PICCFG_V40, iIPChanIndex, &m_struPicCfg, sizeof(NET_DVR_PICCFG_V40), &dwReturned))
	{
		printf("NET_DVR_GET_PICCFG_V40 error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return;
	}
	printf("The channel name of IP channel01: %s\n", m_struPicCfg.sChanName);
	//-----------------------
	//支持自定义协议
	NET_DVR_CUSTOM_PROTOCAL struCustomPro;
	//获取自定义协议 1
	if (!NET_DVR_GetDVRConfig(lUserID, NET_DVR_GET_CUSTOM_PRO_CFG, 1, &struCustomPro,
		sizeof(NET_DVR_CUSTOM_PROTOCAL), &dwReturned))
	{
		printf("NET_DVR_GET_CUSTOM_PRO_CFG error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return;
	}
	struCustomPro.dwEnabled = 1; //启用主码流
	struCustomPro.dwEnableSubStream = 1; //启用子码流
	strcpy((char *)struCustomPro.sProtocalName, "Protocal_RTSP"); //自定义协议名称:Protocal_RTSP,最大 16 字节
	struCustomPro.byMainProType = 1; //主码流协议类型: 1- RTSP
	struCustomPro.byMainTransType = 2; //主码流传输协议: 0- Auto, 1- udp, 2- rtp over rtsp
	struCustomPro.wMainPort = 554; //主码流取流端口
	strcpy((char *)struCustomPro.sMainPath, "rtsp://192.168.1.65/h264/ch1/main/av_stream");//主码流取流 URL
	struCustomPro.bySubProType = 1; //子码流协议类型: 1- RTSP
	struCustomPro.bySubTransType = 2; //子码流传输协议: 0- Auto, 1- udp, 2- rtp over rtsp
	struCustomPro.wSubPort = 554; //子码流取流端口
	strcpy((char *)struCustomPro.sSubPath, "rtsp://192.168.1.65/h264/ch1/sub/av_stream");//子码流取流 URL
																						 //设置自定义协议 1
	if (!NET_DVR_SetDVRConfig(lUserID, NET_DVR_SET_CUSTOM_PRO_CFG, 1, &struCustomPro,
		sizeof(NET_DVR_CUSTOM_PROTOCAL)))
	{
		printf("NET_DVR_SET_CUSTOM_PRO_CFG error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return;
	}
	printf("Set the custom protocol: %s\n", "Protocal_RTSP");
	NET_DVR_IPC_PROTO_LIST m_struProtoList; if (!NET_DVR_GetIPCProtoList(lUserID, &m_struProtoList)) //获取设备支持的前端协议
	{
		printf("NET_DVR_GetIPCProtoList error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return;
	}
	//---------------------------------
	//添加 IP 设备
	for (i = 0; i<MAX_IP_DEVICE_V40; i++)
	{
		if (IPAccessCfgV40.struIPDevInfo[i].byEnable == 0) //选择空闲的设备 ID
		{
			iDevID = i + 1;
			break;
		}
	}
	//添加 IP 通道 5
	iIPCh = 5;
	IPAccessCfgV40.struIPDevInfo[iDevID - 1].byEnable = 1; //启用
	for (i = 0; i<m_struProtoList.dwProtoNum; i++)
	{
		if (strcmp((char *)struCustomPro.sProtocalName, (char *)m_struProtoList.struProto[i].byDescribe) == 0)
		{
			IPAccessCfgV40.struIPDevInfo[iDevID - 1].byProType = m_struProtoList.struProto[i].dwType; //选择自定义协议
			break;
		}
	}
	//IPAccessCfgV40.struIPDevInfo[iIPCh].byProType=0; //厂家私有协议
	strcpy((char *)IPAccessCfgV40.struIPDevInfo[iDevID - 1].struIP.sIpV4, "172.6.22.225"); //前端 IP 设备的 IP 地址
	IPAccessCfgV40.struIPDevInfo[iDevID - 1].wDVRPort = 8000; //前端 IP 设备服务端口
	strcpy((char *)IPAccessCfgV40.struIPDevInfo[iDevID - 1].sUserName, "admin"); //前端 IP 设备登录用户名
	strcpy((char *)IPAccessCfgV40.struIPDevInfo[iDevID - 1].sPassword, "12345"); //前端 IP 设备登录密码
	IPAccessCfgV40.struStreamMode[iIPCh - 1].byGetStreamType = 0; //直接从设备取流
	IPAccessCfgV40.struStreamMode[iIPCh - 1].uGetStream.struChanInfo.byChannel = 1; //前端 IP 设备的通道号
	IPAccessCfgV40.struStreamMode[iIPCh - 1].uGetStream.struChanInfo.byIPID = iDevID % 256; //通道对应前端 IP 设备 ID 的低 8 位
	IPAccessCfgV40.struStreamMode[iIPCh - 1].uGetStream.struChanInfo.byIPIDHigh = iDevID / 256;
	//通道对应前端 IP 设备 ID 的高 8 位
	//IP 通道配置，包括添加、删除、修改 IP 通道等
	if (!NET_DVR_SetDVRConfig(lUserID, NET_DVR_SET_IPPARACFG_V40, iGroupNO, &IPAccessCfgV40,
		sizeof(NET_DVR_IPPARACFG_V40)))
	{
		printf("NET_DVR_SET_IPPARACFG_V40 error, %d\n", NET_DVR_GetLastError()); NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return;
	}
	else
	{
		printf("Set IP channel no.%d, IP: %s\n", iIPCh, IPAccessCfgV40.struIPDevInfo[iDevID - 1].struIP.sIpV4);
	}
	//注销用户
	NET_DVR_Logout(lUserID);
	//释放 SDK 资源
	NET_DVR_Cleanup();
	return;
}