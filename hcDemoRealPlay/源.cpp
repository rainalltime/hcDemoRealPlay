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
	// ��ʼ��
	NET_DVR_Init();
	//��������ʱ��������ʱ��
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);
	//---------------------------------------
	// ע���豸
	LONG lUserID;
	NET_DVR_DEVICEINFO_V30 struDeviceInfo;
	lUserID = NET_DVR_Login_V30("192.168.2.218", 8000, "admin", "Ll168888", &struDeviceInfo);
	if (lUserID < 0)
	{
		printf("Login error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Cleanup();
		return;
	}
	printf("The max number of analog channels: %d\n", struDeviceInfo.byChanNum); //ģ��ͨ������
	printf("The max number of IP channels: %d\n", struDeviceInfo.byIPChanNum);//IP ͨ������
																			  //��ȡ IP ͨ��������Ϣ
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
			case 0: //ֱ�Ӵ��豸ȡ��
				if (IPAccessCfgV40.struStreamMode[i].uGetStream.struChanInfo.byEnable)
				{
					byIPID = IPAccessCfgV40.struStreamMode[i].uGetStream.struChanInfo.byIPID;
					byIPIDHigh = IPAccessCfgV40.struStreamMode[i].uGetStream.struChanInfo.byIPIDHigh;
					iDevInfoIndex = byIPIDHigh * 256 + byIPID - 1 - iGroupNO * 64;
					printf("IP channel no.%d is online, IP: %s\n", i + 1, \
						IPAccessCfgV40.struIPDevInfo[iDevInfoIndex].struIP.sIpV4);
				}
				break;
			case 1: //����ý��ȡ��
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
	//��ȡ IP ͨ����ز���(�Ի�ȡ IP ͨ��ͼ�����Ϊ��)
	DWORD iIPChanIndex = IPAccessCfgV40.dwStartDChan; //��ʼͨ������ IP ͨ�� 01
	NET_DVR_PICCFG_V40 m_struPicCfg; //���ͨ��ͼ������Ľṹ��
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
	//֧���Զ���Э��
	NET_DVR_CUSTOM_PROTOCAL struCustomPro;
	//��ȡ�Զ���Э�� 1
	if (!NET_DVR_GetDVRConfig(lUserID, NET_DVR_GET_CUSTOM_PRO_CFG, 1, &struCustomPro,
		sizeof(NET_DVR_CUSTOM_PROTOCAL), &dwReturned))
	{
		printf("NET_DVR_GET_CUSTOM_PRO_CFG error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return;
	}
	struCustomPro.dwEnabled = 1; //����������
	struCustomPro.dwEnableSubStream = 1; //����������
	strcpy((char *)struCustomPro.sProtocalName, "Protocal_RTSP"); //�Զ���Э������:Protocal_RTSP,��� 16 �ֽ�
	struCustomPro.byMainProType = 1; //������Э������: 1- RTSP
	struCustomPro.byMainTransType = 2; //����������Э��: 0- Auto, 1- udp, 2- rtp over rtsp
	struCustomPro.wMainPort = 554; //������ȡ���˿�
	strcpy((char *)struCustomPro.sMainPath, "rtsp://192.168.1.65/h264/ch1/main/av_stream");//������ȡ�� URL
	struCustomPro.bySubProType = 1; //������Э������: 1- RTSP
	struCustomPro.bySubTransType = 2; //����������Э��: 0- Auto, 1- udp, 2- rtp over rtsp
	struCustomPro.wSubPort = 554; //������ȡ���˿�
	strcpy((char *)struCustomPro.sSubPath, "rtsp://192.168.1.65/h264/ch1/sub/av_stream");//������ȡ�� URL
																						 //�����Զ���Э�� 1
	if (!NET_DVR_SetDVRConfig(lUserID, NET_DVR_SET_CUSTOM_PRO_CFG, 1, &struCustomPro,
		sizeof(NET_DVR_CUSTOM_PROTOCAL)))
	{
		printf("NET_DVR_SET_CUSTOM_PRO_CFG error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return;
	}
	printf("Set the custom protocol: %s\n", "Protocal_RTSP");
	NET_DVR_IPC_PROTO_LIST m_struProtoList; if (!NET_DVR_GetIPCProtoList(lUserID, &m_struProtoList)) //��ȡ�豸֧�ֵ�ǰ��Э��
	{
		printf("NET_DVR_GetIPCProtoList error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return;
	}
	//---------------------------------
	//��� IP �豸
	for (i = 0; i<MAX_IP_DEVICE_V40; i++)
	{
		if (IPAccessCfgV40.struIPDevInfo[i].byEnable == 0) //ѡ����е��豸 ID
		{
			iDevID = i + 1;
			break;
		}
	}
	//��� IP ͨ�� 5
	iIPCh = 5;
	IPAccessCfgV40.struIPDevInfo[iDevID - 1].byEnable = 1; //����
	for (i = 0; i<m_struProtoList.dwProtoNum; i++)
	{
		if (strcmp((char *)struCustomPro.sProtocalName, (char *)m_struProtoList.struProto[i].byDescribe) == 0)
		{
			IPAccessCfgV40.struIPDevInfo[iDevID - 1].byProType = m_struProtoList.struProto[i].dwType; //ѡ���Զ���Э��
			break;
		}
	}
	//IPAccessCfgV40.struIPDevInfo[iIPCh].byProType=0; //����˽��Э��
	strcpy((char *)IPAccessCfgV40.struIPDevInfo[iDevID - 1].struIP.sIpV4, "172.6.22.225"); //ǰ�� IP �豸�� IP ��ַ
	IPAccessCfgV40.struIPDevInfo[iDevID - 1].wDVRPort = 8000; //ǰ�� IP �豸����˿�
	strcpy((char *)IPAccessCfgV40.struIPDevInfo[iDevID - 1].sUserName, "admin"); //ǰ�� IP �豸��¼�û���
	strcpy((char *)IPAccessCfgV40.struIPDevInfo[iDevID - 1].sPassword, "12345"); //ǰ�� IP �豸��¼����
	IPAccessCfgV40.struStreamMode[iIPCh - 1].byGetStreamType = 0; //ֱ�Ӵ��豸ȡ��
	IPAccessCfgV40.struStreamMode[iIPCh - 1].uGetStream.struChanInfo.byChannel = 1; //ǰ�� IP �豸��ͨ����
	IPAccessCfgV40.struStreamMode[iIPCh - 1].uGetStream.struChanInfo.byIPID = iDevID % 256; //ͨ����Ӧǰ�� IP �豸 ID �ĵ� 8 λ
	IPAccessCfgV40.struStreamMode[iIPCh - 1].uGetStream.struChanInfo.byIPIDHigh = iDevID / 256;
	//ͨ����Ӧǰ�� IP �豸 ID �ĸ� 8 λ
	//IP ͨ�����ã�������ӡ�ɾ�����޸� IP ͨ����
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
	//ע���û�
	NET_DVR_Logout(lUserID);
	//�ͷ� SDK ��Դ
	NET_DVR_Cleanup();
	return;
}