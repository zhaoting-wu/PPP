#include "stdio.h"
#include "winsock2.h"//socket通信，系统头文件
#include "windows.h"
#include "iostream"
#pragma comment(lib,"ws2_32.lib")//静态加入一个lib文件也就是库文件ws2_32.lib文件
#pragma comment(lib,"netapi32.lib")   //连接Netapi32.lib库，MAC获取中用到了NetApi32.DLL的功能

using namespace std;

int id_num ;

int getMAC(char * mac , int id)     //用NetAPI来获取网卡MAC地址
{    
NCB ncb;     //定义一个NCB(网络控制块)类型的结构体变量ncb
typedef struct _ASTAT_     //自定义一个结构体_ASTAT_
{
  ADAPTER_STATUS   adapt;
  NAME_BUFFER   NameBuff   [30];    
}ASTAT, *PASTAT;
ASTAT Adapter; 

typedef struct _LANA_ENUM     //自定义一个结构体_ASTAT_
{
  UCHAR length;
  UCHAR lana[MAX_LANA];     //存放网卡MAC地址
}LANA_ENUM;    
LANA_ENUM lana_enum; 

//   取得网卡信息列表    
UCHAR uRetCode;    
memset(&ncb, 0, sizeof(ncb));     //将已开辟内存空间ncb 的值均设为值 0
memset(&lana_enum, 0, sizeof(lana_enum));     //清空一个结构类型的变量lana_enum，赋值为0
//对结构体变量ncb赋值
ncb.ncb_command = NCBENUM;     //统计系统中网卡的数量
ncb.ncb_buffer = (unsigned char *)&lana_enum; //ncb_buffer成员指向由LANA_ENUM结构填充的缓冲区
ncb.ncb_length = sizeof(LANA_ENUM);  
//向网卡发送NCBENUM命令，以获取当前机器的网卡信息，如有多少个网卡，每个网卡的编号（MAC地址）
uRetCode = Netbios(&ncb); //调用netbois(ncb)获取网卡序列号   
if(uRetCode != NRC_GOODRET)    
return uRetCode;   

//对每一个网卡，以其网卡编号为输入编号，获取其MAC地址  
id_num = 1;
for(int lana=0; lana<lana_enum.length; lana++)    
{
  ncb.ncb_command = NCBRESET;   //对网卡发送NCBRESET命令，进行初始化
  ncb.ncb_lana_num = lana_enum.lana[lana];
  uRetCode = Netbios(&ncb);  
}
if(uRetCode != NRC_GOODRET)
return uRetCode;    
//   准备取得接口卡的状态块取得MAC地址
memset(&ncb, 0, sizeof(ncb));
ncb.ncb_command = NCBASTAT;    //对网卡发送NCBSTAT命令，获取网卡信息
ncb.ncb_lana_num = lana_enum.lana[id];     //指定网卡号，这里仅仅指定第一块网卡，通常为有线网卡
strcpy((char*)ncb.ncb_callname, "*");     //远程系统名赋值为*
ncb.ncb_buffer = (unsigned char *)&Adapter; //指定返回的信息存放的变量
ncb.ncb_length = sizeof(Adapter);
//接着发送NCBASTAT命令以获取网卡的信息
uRetCode = Netbios(&ncb);
//   取得网卡的信息，并且如果网卡正常工作的话，返回标准的冒号分隔格式。  
if(uRetCode != NRC_GOODRET)  
return uRetCode;  
//把网卡MAC地址格式转化为常用的16进制形式,输出到字符串mac中
sprintf(mac,"%02X-%02X-%02X-%02X-%02X-%02X",    
Adapter.adapt.adapter_address[0],    
Adapter.adapt.adapter_address[1],    
Adapter.adapt.adapter_address[2],    
Adapter.adapt.adapter_address[3],    
Adapter.adapt.adapter_address[4],    
Adapter.adapt.adapter_address[5]
);
return 0;  
}


DWORD WINAPI RecevData(LPVOID lpParameter);// 双字，32位

SOCKET SerSocket;
SOCKADDR_IN ClientAddr;
int ClientLen =  sizeof(SOCKADDR);//自动转换
char host_name[225];
HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 

int main()
{
	  
	WSADATA wsadate;
	int Ret;
	if ((Ret =WSAStartup(MAKEWORD(2,1),&wsadate)) != 0 )//启用winsock服务
	{
		cout << "WSAStartup error " << Ret << endl;
	}
	SerSocket =  socket(AF_INET,SOCK_DGRAM,0);
	memset(&ClientAddr,0,sizeof(ClientAddr));//数组的初始化

	SOCKADDR_IN SverAddr;
	SverAddr.sin_family = AF_INET;
	SverAddr.sin_port =htons(8000);
	SverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	bind(SerSocket,(LPSOCKADDR)&SverAddr,sizeof(SOCKADDR));
    
    struct hostent *pHost = gethostbyname(host_name); // //////////// host name
    
	char mac[200];
	getMAC(mac, 0);
//	printf("id_num = %d\n", id_num) ;


	for(int id=0; id<id_num; id++) {
		cout << "本机ip:";
		cout << inet_ntoa(*((struct in_addr *)pHost->h_addr_list[id])) << endl;
	}

	{
 
		char   mac[200];  
		for(int id=0; id<id_num; id++)
		{
			getMAC(mac, id);   //调用getMAC()函数获得，输出MAC地址
			cout << "本机MAC:";
			cout << mac <<endl; 
		}

	}


	HANDLE handl = CreateThread(NULL,NULL,RecevData,NULL,NULL,0);
	 
	while (1)
	{
		//cout<< "Server:" ;
		char buffer[1024];
		memset(buffer,0,1024);
	    //cin >> buffer;
		SetConsoleTextAttribute(hStdout,FOREGROUND_RED|FOREGROUND_INTENSITY);
		cin.getline(buffer,1024);
		if(strlen(buffer) == 0)
		{
			continue;
		}
		if (buffer[0] == 'q')
		{
			cout << "你已终止聊天" <<endl;
			strcpy(buffer,"对方已终止聊天");
			int Ret;
			Ret = sendto(SerSocket,buffer,strlen(buffer),0,(LPSOCKADDR)&ClientAddr,ClientLen);
			if (Ret == SOCKET_ERROR)
			{
				cout << "sendto error " << GetLastError() << endl;
			}
			system("pause");
			break;
		}
		int Ret;
		Ret = sendto(SerSocket,buffer,strlen(buffer),0,(LPSOCKADDR)&ClientAddr,ClientLen);
		if (Ret == SOCKET_ERROR)
		{
			cout << "sendto error " << GetLastError() << endl;
			continue;
		}
		Sleep(10);
	}
	closesocket(SerSocket);//断开
	CloseHandle(handl);
	WSACleanup();
	return 0;
}


DWORD WINAPI RecevData(LPVOID lpParameter)
{
	while(1)
	{
		char buffer[1024];     
		memset(buffer,0,1024);
		int Ret;
		Ret = recvfrom(SerSocket,buffer,1024,0,(LPSOCKADDR)&ClientAddr,&ClientLen);
		if (Ret == SOCKET_ERROR)
		{
			cout << "recvfrom error" << GetLastError() << endl;
			continue;
		}
		SetConsoleTextAttribute(hStdout,FOREGROUND_GREEN|FOREGROUND_INTENSITY);
		cout << inet_ntoa(ClientAddr.sin_addr) << endl;
		cout << "对方说:" << buffer << endl;
		SetConsoleTextAttribute(hStdout,FOREGROUND_RED|FOREGROUND_INTENSITY);
		Sleep(10);
	}
	            system("pause");
	return 0;
}


