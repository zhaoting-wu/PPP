// Client.cpp : 定义控制台应用程序的入口点。
//

#include "stdio.h"
#include "winsock2.h"
#include "windows.h"
#include "iostream"
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"netapi32.lib")   //连接Netapi32.lib库，MAC获取中用到了NetApi32.DLL的功能

using namespace std;
	//通过WindowsNT/Win2000中内置的NetApi32.DLL的功能来实现的。首先通过发送NCBENUM命令,获取网卡的
   //数目和每张网卡的内部编号,然后对每个网卡标号发送NCBASTAT命令获取其MAC地址。


int id_num =1; 
int getMAC(char * mac, int id)     //用NetAPI来获取网卡MAC地址
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
//id_num = lana_enum.length; 
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
ncb.ncb_command = NCBASTAT;   
//for (int i=0;i<lana_enum.length;++i) 
 //对网卡发送NCBSTAT命令，获取网卡信息
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



DWORD WINAPI RecevData(LPVOID lpParameter);

SOCKET ClientSocket;
SOCKADDR_IN SverAddr;
int SverLen = sizeof(SverAddr);
char host_name[225];

HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);//系统服务你的凭证 

int main( )
{
	
SOCKET clientSocket;
SOCKADDR_IN clientsock_in;
     char ip_addr[16]=
{"127.0.0.1"};
	WSADATA wsadate;
	int Ret;
	if ((Ret =WSAStartup(MAKEWORD(2,1),&wsadate)) != 0 )
	{
		cout << "WSAStartup error " << Ret << endl;
	}
	ClientSocket =  socket(AF_INET,SOCK_DGRAM,0);//ipv4网络协议和使用不连续不可靠的数据包连接

	SverAddr.sin_family = AF_INET;//TCP/ip协议族
	SverAddr.sin_port = htons(8000);//服务端端口
	 clientSocket=socket(AF_INET,SOCK_STREAM,0);
cout << "请输入主机ip:" << endl;
cin >> ip_addr;
//连接服务器
SverAddr.sin_addr.S_un.S_addr=inet_addr(ip_addr);      //服务端地址

	SOCKADDR_IN ClientAddr;//指针并初始化
	ClientAddr.sin_family = AF_INET;
	ClientAddr.sin_port = htons(7000);//端口号
	ClientAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//占用所有的ip进行监听

	bind(ClientSocket,(LPSOCKADDR)&ClientAddr,sizeof(ClientAddr));//监听
 
	HANDLE handl =CreateThread(NULL,NULL,RecevData,NULL,NULL,0);

    struct hostent *pHost = gethostbyname(host_name); // //////////// host name
	char mac[200];
	getMAC(mac, 0) ;
	for(int id=0;id<id_num ;id++){
		cout << "本机ip:";
		cout << inet_ntoa(*((struct in_addr *)pHost->h_addr_list[id])) << endl;

	}

	{
 
		char   mac[200];  
		for(int id=0; id<id_num; id++) {
			getMAC(mac, id);   //调用getMAC()函数获得，输出MAC地址
			cout <<"本机MAC:";
			cout << mac <<endl;  
		}

	}

	 
	while(1)
	{
		//cout<< "Client:" ;
		char buffer[1024];
		memset(buffer,0,1024);//在一段内存中填充给定的值
		SetConsoleTextAttribute(hStdout,FOREGROUND_RED|FOREGROUND_INTENSITY);//控制台颜色
		cin.getline(buffer,1024);//cin >> buffer;
		if(strlen(buffer) == 0)
		{
			continue;
		}
		if (buffer[0] == 'q')
		{
			cout << "你已终止聊天" <<endl;
			strcpy(buffer,"对方已终止聊天");
			int Ret;
			Ret = sendto(ClientSocket,buffer,strlen(buffer),0,(LPSOCKADDR)&SverAddr,sizeof(SverAddr));
			if (Ret == SOCKET_ERROR)
			{
				cout << "sendto error " << GetLastError() <<endl;
			}
			system("pause");
			break;
		}
		int Ret;
		Ret = sendto(ClientSocket,buffer,strlen(buffer),0,(LPSOCKADDR)&SverAddr,sizeof(SverAddr));//发送
		if (Ret == SOCKET_ERROR)
		{
			cout << "sendto error " << GetLastError() <<endl;
			continue;
		}
		 Sleep(10);
	}
	closesocket(ClientSocket);
	CloseHandle(handl);
	WSACleanup();//调用函数清除参数
	return 0;
}

DWORD WINAPI RecevData(LPVOID lpParameter)
{
	while(1)
	{
		char buffer[1024];
		memset(buffer,0,1024);
		int Ret;
		Ret = recvfrom(ClientSocket,buffer,1024,0,(LPSOCKADDR)&SverAddr,&SverLen);//接收
		if (Ret == SOCKET_ERROR)
		{
			cout << "recvfrom error " << GetLastError() <<endl;
			continue;
		}
		SetConsoleTextAttribute(hStdout,FOREGROUND_GREEN|FOREGROUND_INTENSITY);
		cout << inet_ntoa(SverAddr.sin_addr) << endl;
		cout << "对方说:" << buffer << endl;
		SetConsoleTextAttribute(hStdout,FOREGROUND_RED|FOREGROUND_INTENSITY);
		Sleep(10);
	}
               system("pause");
	return 0;
}
  
