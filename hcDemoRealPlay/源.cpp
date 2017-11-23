#include <stdio.h>
#include <iostream>
#include "Windows.h"
#include "HCNetSDK.h"
using namespace std;
//typedef HWND(WINAPI *PROCGETCONSOLEWINDOW)();
//PROCGETCONSOLEWINDOW GetConsoleWindow;
void main() {
	//---------------------------------------
	// ��ʼ��
	NET_DVR_Init();
	//��������ʱ��������ʱ��
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);
	//---------------------------------------
	// ��ȡ����̨���ھ��
	HMODULE hKernel32 = GetModuleHandle("kernel32");
	//GetConsoleWindow = (PROCGETCONSOLEWINDOW)GetProcAddress(hKernel32, "GetConsoleWindow");
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
	HWND hWnd = GetConsoleWindow(); //��ȡ���ھ��
	NET_DVR_VOD_PARA struVodPara = { 0 };
	struVodPara.dwSize = sizeof(struVodPara);
	struVodPara.struIDInfo.dwChannel = 33; //ͨ����
	struVodPara.hWnd = hWnd; //�طŴ���
	//struVodPara.struBeginTime.dwYear = 2017; //��ʼʱ��
	//struVodPara.struBeginTime.dwMonth = 11;
	//struVodPara.struBeginTime.dwDay =21;
	//struVodPara.struBeginTime.dwHour = 9;
	//struVodPara.struBeginTime.dwMinute = 0;
	//struVodPara.struBeginTime.dwSecond = 0;
	//struVodPara.struEndTime.dwYear = 2017; //����ʱ��
	//struVodPara.struEndTime.dwMonth = 11;
	//struVodPara.struEndTime.dwDay = 21;
	//struVodPara.struEndTime.dwHour = 10;
	//struVodPara.struEndTime.dwMinute = 7;
	//struVodPara.struEndTime.dwSecond = 0;
	//---------------------------------------
	string startTime="2017-11-21 09:00:08", stopTime="2017-11-21 10:00:01";
	sscanf(startTime.c_str(), "%d-%d-%d   %d:%d:%d", &struVodPara.struBeginTime.dwYear, &struVodPara.struBeginTime.dwMonth, &struVodPara.struBeginTime.dwDay, &struVodPara.struBeginTime.dwHour, &struVodPara.struBeginTime.dwMinute, &struVodPara.struBeginTime.dwSecond) > 0
		&& sscanf(stopTime.c_str(), "%d-%d-%d   %d:%d:%d", &struVodPara.struEndTime.dwYear, &struVodPara.struEndTime.dwMonth, &struVodPara.struEndTime.dwDay, &struVodPara.struEndTime.dwHour, &struVodPara.struEndTime.dwMinute, &struVodPara.struEndTime.dwSecond) > 0
		;
	//��ʱ��ط�
	int hPlayback;
	hPlayback = NET_DVR_PlayBackByTime_V40(lUserID, &struVodPara);
	if (hPlayback < 0)
	{
		printf("NET_DVR_PlayBackByTime_V40 fail,last error %d\n", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return;
	}
	//---------------------------------------
	//��ʼ
	if (!NET_DVR_PlayBackControl_V40(hPlayback, NET_DVR_PLAYSTART, NULL, 0, NULL, NULL))
	{
		printf("play back control failed [%d]\n", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return;
	}
	Sleep(1500000); //millisecond
	if (!NET_DVR_StopPlayBack(hPlayback))
	{
		printf("failed to stop file [%d]\n", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return;
	}
	//ע���û�
	NET_DVR_Logout(lUserID);
	//�ͷ� SDK ��Դ
	NET_DVR_Cleanup(); return;
}