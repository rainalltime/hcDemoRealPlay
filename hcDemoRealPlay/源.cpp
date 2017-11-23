#include <stdio.h>
#include <iostream>
#include "Windows.h"
#include "HCNetSDK.h"
using namespace std;
void main() {
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
	lUserID = NET_DVR_Login_V30("192.168.2.218", 8000, "admin", "Lb168888", &struDeviceInfo);
	if (lUserID < 0)
	{
		printf("Login error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Cleanup();
		return;
	}
	NET_DVR_PLAYCOND struDownloadCond = { 0 };
	struDownloadCond.dwChannel = 1; //ͨ����
	struDownloadCond.struStartTime.dwYear = 2017; //��ʼʱ��
	struDownloadCond.struStartTime.dwMonth = 11;
	struDownloadCond.struStartTime.dwDay = 22;
	struDownloadCond.struStartTime.dwHour = 9;
	struDownloadCond.struStartTime.dwMinute = 50;
	struDownloadCond.struStartTime.dwSecond = 0;
	struDownloadCond.struStopTime.dwYear = 2017; //����ʱ��
	struDownloadCond.struStopTime.dwMonth = 11;
	struDownloadCond.struStopTime.dwDay = 22;
	struDownloadCond.struStopTime.dwHour = 10;
	struDownloadCond.struStopTime.dwMinute = 7; 
	struDownloadCond.struStopTime.dwSecond = 0;
	//---------------------------------------
	//��ʱ������
	int hPlayback;
	hPlayback = NET_DVR_GetFileByTime_V40(lUserID, "./test.mp4", &struDownloadCond);
	if (hPlayback < 0)
	{
		printf("NET_DVR_GetFileByTime_V40 fail,last error %d\n", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return;
	}
	//---------------------------------------
	//��ʼ����
	if (!NET_DVR_PlayBackControl_V40(hPlayback, NET_DVR_PLAYSTART, NULL, 0, NULL, NULL))
	{
		printf("Play back control failed [%d]\n", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return;
	}
	int nPos = 0;
	for (nPos = 0; nPos < 100 && nPos >= 0; nPos = NET_DVR_GetDownloadPos(hPlayback))
	{
		printf("Be downloading... %d %%\n", nPos); //���ؽ���
		Sleep(5000); //millisecond
	}
	if (!NET_DVR_StopGetFile(hPlayback))
	{
		printf("failed to stop get file [%d]\n", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return;
	}
	if (nPos<0 || nPos>100)
	{
		printf("download err [%d]\n", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return;
	}printf("Be downloading... %d %%\n", nPos);
	//ע���û�
	NET_DVR_Logout(lUserID);
	//�ͷ� SDK ��Դ
	NET_DVR_Cleanup();
	return;
}