#include "stdafx.h"
#include "RemoteControl.h"
#include <winsock.h>
#include "Iphlpapi.h"

#define THISVERSION "0001"
CRemoteControl::CRemoteControl(CString localip, CString localmac, CString gateip, CString gatemac, CString netmask)
{
    //��ʼ������
    localIPAddress = localip=="0.0.0.0"?"127.0.0.1":localip;
    localMAC = localmac;
    gateIPAddress = gateip;
    gateMAC = gatemac;
    netMask = netmask;

    //��ȡ����
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
            *************QZJ Զ�̲�������
            *************�汾0.1
            �Ƿ�����Զ����Ϣ    0,1
            �Ƿ����Զ�̿���    0,1
            ���Ϳ�����Կ        null
            ������Ϣ���漶��    0,1,2,3,4
            ������Ϣ��ʽ        0:Windows,1:Program
            ������ϢĿ�ĵ�      127.0.0.1
            ������Ϣ�˿�        7069
            ������ϢЭ��        UDP,TCP,ICMP,QZJ
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
    //������ã����Զ���������
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
//�������� SaveConfig
//���ߣ� Nov30th
//���ڣ� 2007.10.10
//���ܣ� ����Զ����Ϣ�����ļ�
//�����������
//����ֵ�� �Ƿ񱣴��ȡ
//�޸ļ�¼��
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
//�������� LoadConfig
//���ߣ� Nov30th
//���ڣ� 2007.10.10
//���ܣ� ��ȡԶ����Ϣ�����ļ�
//�����������
//����ֵ�� �Ƿ�ɹ���ȡ
//�޸ļ�¼��
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
//�������� SendUdpPacket
//���ߣ� Nov30th
//���ڣ� 2007.10.10
//���ܣ� ����UDP���ݵ�Զ�̿��ƻ�
//�����������
//����ֵ�� ���͵��ֽ���
//�޸ļ�¼��
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
    bind(cli,   (struct   sockaddr   *)   &cliaddr,   sizeof(cliaddr));   //�󶨷��Ͷ˿�   
    char   *send_buffer   =  packetContext.GetBuffer(0);   
    int   length   =  strlen(send_buffer);
    int   len   =   sizeof(servaddr);
    int   flag   =   sendto(cli,send_buffer,length,0,(struct   sockaddr   *)&servaddr,len);
    packetContext.ReleaseBuffer();
    closesocket(cli);
    return flag;
}


//==================================================================
//�������� SendAvailableData
//���ߣ� Nov30th
//���ڣ� 2007.10.11
//���ܣ� ��̬��������ʱ�����������ݰ�
//���������this
//����ֵ�� ��
//�޸ļ�¼��
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
//�������� SendData
//���ߣ� Nov30th
//���ڣ� 2007.10.11
//���ܣ� �������÷�������
//�����������Ҫ���͵�����
//����ֵ�� �����ֽ���
//�޸ļ�¼��
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
//�������� FullScanFind
//���ߣ� Nov30th
//���ڣ� 2007.10.10
//���ܣ� ����ȫ����ɨ���ύ����
//���������ɨ����ip,ɨ����mac,Ŀ��ip,ɨ��ʱ��
//����ֵ�� �������
//�޸ļ�¼��
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
//�������� CheatGateFound
//���ߣ� Nov30th
//���ڣ� 2007.10.11
//���ܣ� ����������ƭ
//�����������ƭ��ip,��ƭ��mac,��ƭMAC,��ƭʱ��
//����ֵ�� �������
//�޸ļ�¼��
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
//�������� FindIPFromMAC
//���ߣ� Nov30th
//���ڣ� 2007.10.11
//���ܣ� ��ARP�����У���MAC�ҵ�IP
//���������MAC��ַ,�ų���IP��ַ
//����ֵ�� �������
//�޸ļ�¼��
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
    dwRet = GetIpNetTable((PMIB_IPNETTABLE)&tempChar, &dwListSize, TRUE); // �ؼ�����
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
                    //�ҵ�ͬ����MAC
                    inaddr.S_un.S_addr = pIpNetTable->table[i].dwAddr;
                    if (inet_ntoa(inaddr) != ipAddress)
                    {
                        //�ҵ���ƭ�ߵ�IP
                        retIP = inet_ntoa(inaddr);
                    }
                }
            }
        }
        delete pIpNetTable;
    }
    return retIP;
}