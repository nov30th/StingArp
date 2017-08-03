// DialogIP.cpp : implementation file
//

#include "stdafx.h"
#include "ArpCheater.h"
#include "DialogIP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDialogIP dialog


CDialogIP::CDialogIP(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogIP::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogIP)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDialogIP::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogIP)
	DDX_Control(pDX, IDC_IPADDRESSINPUT, m_ip);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogIP, CDialog)
	//{{AFX_MSG_MAP(CDialogIP)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogIP message handlers

void CDialogIP::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData(true);
	BYTE ipb[4];
	m_ip.GetAddress(ipb[0],ipb[1],ipb[2],ipb[3]);
	ipaddress = ipb[3]*256*256*256 + ipb[2]*256*256 + ipb[1]*256 + ipb[0];
	CDialog::OnOK();
}

//DEL void CDialogIP::Reset(BYTE ip[])
//DEL {
//DEL 	//m_ip.SetAddress(ip[0],ip[1],ip[2],ip[3]);
//DEL 	
//DEL }

//DEL int CDialogIP::DoModal() 
//DEL {
//DEL 	// TODO: Add your specialized code here and/or call the base class
//DEL 	m_ip.SetAddress(1);
//DEL 	return CDialog::DoModal();
//DEL }
