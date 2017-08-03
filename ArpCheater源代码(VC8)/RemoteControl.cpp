#include "stdafx.h"
#include "RemoteControl.h"
#include <winsock.h>
#include "Iphlpapi.h"

#define THISVERSION "0001"
CRemoteControl::CRemoteControl(CString localip, CString localmac, CString gateip, CString gatemac, CString netmask)
{
    //初始化数据
    localIPAddress = localip=="0.0.0.0"?"127.0.0.1":localip;
    localMAC = localmac;
    gateIPAddress = gateip;
    gateMAC = gatemac;
    netMask = netmask;

    //读取配置
    CStdioFile file;
    if (LoadConfig())
    {
        NULL;
    }
    else
    {
        if (file.Open("RemoteConfig.cfg",CFile::modeWrite | CFile::modeCreate | CFile::typeText))
        {
            //Write The File
            /****************************************
            *************QZJ 远程操作配置
            *************版本0.1
            是否启用远程消息    0,1
            是否接受远程控制    0,1
            接送控制密钥        null
            发送消息警告级别    0,1,2,3,4
            发送消息方式        0:Windows,1:Program
            发送消息目的地      127.0.0.1
            发送消息端口        7069
            发送消息协议        UDP,TCP,ICMP,QZJ
            ********************************************/
            file.WriteString("0\n1\nnull\n1\n1\n127.0.0.1\n7069\nUDP\n");
            file.Close();
            LoadConfig();
        }
        else
        {
            //No file and NO WRITE ACCESS
            return;
        }
    }
    //如果启用，则自动发送数据
    if (enableControl)
    {
        CWinThread* m_pWinThread = AfxBeginThread(SendAvailableData,(LPVOID) this,THREAD_PRIORITY_NORMAL,0,0,0);
        m_pWinThread->m_bAutoDelete = true;
    }
}

CRemoteControl::~CRemoteControl(void)
{
}


//==================================================================
//函数名： SaveConfig
//作者： Nov30th
//日期： 2007.10.10
//功能： 保存远程消息配置文件
//输入参数：无
//返回值： 是否保存读取
//修改记录：
//==================================================================
bool CRemoteControl::SaveConfig(void)
{
    CStdioFile file;
    if (file.Open("RemoteConfig.cfg",CFile::modeWrite | CFile::modeCreate | CFile::typeText))
    {
        CString tempString;
        file.WriteString(enableControl?"1":"0");
        file.WriteString("\n");
        file.WriteString(acceptControl?"1":"0");
        file.WriteString("\n");
        file.WriteString(strPassword);
        file.WriteString("\n");
        tempString.Format("%d",warningLevel);
        file.WriteString(tempString);
        file.WriteString("\n");
        tempString.Format("%d",messageType);
        file.WriteString(tempString);
        file.WriteString("\n");
        file.WriteString(destAddress);
        file.WriteString("\n");
        tempString.Format("%d",destPort);
        file.WriteString(tempString);
        file.WriteString("\n");
        file.WriteString(protocolType);
        file.WriteString("\n");
        file.Close();
        return true;
    }
    else
    {
        return false;
    }
}

//==================================================================
//函数名： LoadConfig
//作者： Nov30th
//日期： 2007.10.10
//功能： 读取远程消息配置文件
//输入参数：无
//返回值： 是否成功读取
//修改记录：
//==================================================================
bool CRemoteControl::LoadConfig(void)
{
    CStdioFile file;
    if (file.Open("RemoteConfig.cfg",CFile::modeRead | CFile::typeText))
    {
        CString tempRead;
        file.ReadString(tempRead);
        tempRead.Replace("\n","");
        enableControl = (tempRead=="1"?true:false);
        file.ReadString(tempRead);
        tempRead.Replace("\n","");
        acceptControl= (tempRead=="1"?true:false);
        file.ReadString(tempRead);
        tempRead.Replace("\n","");
        strPassword= tempRead;
        file.ReadString(tempRead);
        tempRead.Replace("\n","");
        warningLevel=atoi( tempRead);
        file.ReadString(tempRead);
        tempRead.Replace("\n","");
        messageType =  atol(tempRead);
        file.ReadString(tempRead);
        tempRead.Replace("\n","");
        destAddress= tempRead;
        file.ReadString(tempRead);
        tempRead.Replace("\n","");
        destPort=atoi( tempRead);
        file.ReadString(tempRead);
        tempRead.Replace("\n","");
        protocolType=tempRead;
        file.Close();
        return true;
    }
    else
    {
        return false;
    }
}


//==================================================================
//函数名： SendUdpPacket
//作者： Nov30th
//日期： 2007.10.10
//功能： 发送UDP数据到远程控制机
//输入参数：无
//返回值： 发送的字节数
//修改记录：
//==================================================================
int CRemoteControl::SendUdpPacket(CString packetContext)
{
    packetContext.Format("HOHOARPTOOL\n[VERSION]\n%s\n[LOCALIP]\n%s\n[NETMASK]\n%s\n[GATEWAYIP]\n%s\n[GATEWAYMAC]\n%s\n[LOCALMAC]\n%s\n[DATA]\n%s\n",THISVERSION,localIPAddress,netMask,gateIPAddress,gateMAC,localMAC,packetContext);
    int   cli   =   socket(AF_INET,   SOCK_DGRAM,   0);
    struct   sockaddr_in servaddr,cliaddr;
    memset(&cliaddr,   0,   sizeof(cliaddr));
    cliaddr.sin_family             =   AF_INET;   
    cliaddr.sin_addr.S_un.S_addr   =   inet_addr(localIPAddress);
    cliaddr.sin_port =  htons(8512);
    memset(&servaddr,   0,   sizeof(servaddr));
    servaddr.sin_family             =   AF_INET;
    servaddr.sin_addr.S_un.S_addr   =   inet_addr(destAddress);   
    servaddr.sin_port                 =   htons(destPort);   
    bind(cli,   (struct   sockaddr   *)   &cliaddr,   sizeof(cliaddr));   //绑定发送端口   
    char   *send_buffer   =  packetContext.GetBuffer(0);   
    int   length   =  strlen(send_buffer);
    int   len   =   sizeof(servaddr);
    int   flag   =   sendto(cli,send_buffer,length,0,(struct   sockaddr   *)&servaddr,len);
    packetContext.ReleaseBuffer();
    closesocket(cli);
    return flag;
}


//==================================================================
//函数名： SendAvailableData
//作者： Nov30th
//日期： 2007.10.11
//功能： 静态函数，定时发送在线数据包
//输入参数：this
//返回值： 无
//修改记录：
//==================================================================
static UINT SendAvailableData(LPVOID lpParam)
{
    CTime theTime;
    CString tempString;
    CRemoteControl *pDlg = (CRemoteControl*) lpParam;
    while(1)
    {
        theTime = CTime::GetCurrentTime();
        pDlg->SendData("ONLINE," + theTime.Format("%Y-%b-%d %H:%M:%S") + ",");
        Sleep(10000);
    }
}

//==================================================================
//函数名： SendData
//作者： Nov30th
//日期： 2007.10.11
//功能： 根据配置发送数据
//输入参数：需要发送的数据
//返回值： 发送字节数
//修改记录：
//==================================================================
int CRemoteControl::SendData(CString packetContext)
{
    if (messageType == 1 && enableControl)
    {
        //Program Message
        if (protocolType == "UDP")
        {
            return SendUdpPacket(packetContext);
        }
    }
    return -1;
}

//==================================================================
//函数名： FullScanFind
//作者： Nov30th
//日期： 2007.10.10
//功能： 发现全网段扫描提交函数
//输入参数：扫描者ip,扫描者mac,目标ip,扫描时间
//返回值： 操作结果
//修改记录：
//==================================================================
int CRemoteControl::FullScanFind(CString ipAddress, CString macAddress, CString scanDestination, CString scanTime)
{
    CString tempString;
    tempString.Format("FULLSCAN,%s,%s,%s,%s,",ipAddress,macAddress,scanDestination,scanTime);
    if (SendData(tempString)>0)
        return 1;
    else
        return -1;
}

//==================================================================
//函数名： CheatGateFound
//作者： Nov30th
//日期： 2007.10.11
//功能： 发现网关欺骗
//输入参数：欺骗者ip,欺骗者mac,欺骗MAC,欺骗时间
//返回值： 操作结果
//修改记录：
//==================================================================
int CRemoteControl::CheatGateFound(CString ipAddress, CString macAddress, CString scanTime)
{
    CString tempString;
    tempString.Format("CHEATMAC,%s,%s,%s,",ipAddress,macAddress,scanTime);
    if (SendData(tempString)>0)
        return 1;
    else
        return -1;
}

//==================================================================
//函数名： FindIPFromMAC
//作者： Nov30th
//日期： 2007.10.11
//功能： 从ARP缓存中，用MAC找到IP
//输入参数：MAC地址,排除的IP地址
//返回值： 操作结果
//修改记录：
//==================================================================
CString CRemoteControl::FindIPFromMAC(CString macString,CString ipAddress)
{
    char tempChar;
    char tempIP[20];
    char tempMAC[40];
    CString retMAC;
    CString retIP = "0.0.0.0";
    DWORD dwListSize = 1;
    DWORD dwRet;
    in_addr inaddr;
    dwRet = GetIpNetTable((PMIB_IPNETTABLE)&tempChar, &dwListSize, TRUE); // 关键函数
    if (dwRet == ERROR_INSUFFICIENT_BUFFER)
    {
        PMIB_IPNETTABLE pIpNetTable = (PMIB_IPNETTABLE)new(char[dwListSize]);
        dwRet = GetIpNetTable(pIpNetTable, &dwListSize, TRUE);
        if (dwRet == ERROR_SUCCESS)
        {
            for (int i=0; i<(int)pIpNetTable->dwNumEntries; i++)
            {
                retMAC.Format("%02x-%02x-%02x-%02x-%02x-%02x",pIpNetTable->table[i].bPhysAddr[0],pIpNetTable->table[i].bPhysAddr[1],pIpNetTable->table[i].bPhysAddr[2],pIpNetTable->table[i].bPhysAddr[3],pIpNetTable->table[i].bPhysAddr[4],pIpNetTable->table[i].bPhysAddr[5]);
                if (retMAC==macString)
                {
                    //找到同样的MAC
                    inaddr.S_un.S_addr = pIpNetTable->table[i].dwAddr;
                    if (inet_ntoa(inaddr) != ipAddress)
                    {
                        //找到欺骗者的IP
                        retIP = inet_ntoa(inaddr);
                    }
                }
            }
        }
        delete pIpNetTable;
    }
    return retIP;
}