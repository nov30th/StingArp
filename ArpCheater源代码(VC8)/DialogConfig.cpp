// DialogConfig.cpp : implementation file
//

#include "stdafx.h"
#include "ArpCheater.h"
#include "DialogConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDialogConfig dialog


CDialogConfig::CDialogConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogConfig::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogConfig)
	m_autoprotect = FALSE;//saveoption[0];
	m_autorun = FALSE;//saveoption[1];
	m_startmini = FALSE;//saveoption[2];
	m_path = _T("");
	//}}AFX_DATA_INIT
}


void CDialogConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogConfig)
	DDX_Check(pDX, IDC_CHECKAUTOPROTECT, m_autoprotect);
	DDX_Check(pDX, IDC_CHECKAUTORUN, m_autorun);
	DDX_Check(pDX, IDC_CHECKSTARTMINI, m_startmini);
	DDX_Text(pDX, IDC_EDITPATH, m_path);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogConfig, CDialog)
	//{{AFX_MSG_MAP(CDialogConfig)
	ON_BN_CLICKED(IDC_CHECKAUTORUN, OnCheckautorun)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogConfig message handlers

void CDialogConfig::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData(true);
	saveoption[0] = m_autorun == 0 ? false : true ;
	saveoption[1] = m_autoprotect == 0 ? false : true ;
	saveoption[2] = m_startmini == 0 ? false : true ;
	configfilePath = m_path;
	CDialog::OnOK();
}

void CDialogConfig::Reset() 
{
	m_autoprotect = saveoption[1];
	m_autorun = saveoption[0];
	m_startmini = saveoption[2];
}
void CDialogConfig::OnCheckautorun() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	if (m_autorun==0)
		((CButton*)GetDlgItem(IDC_CHECKSTARTMINI))->EnableWindow(false);
	else
		((CButton*)GetDlgItem(IDC_CHECKSTARTMINI))->EnableWindow(true);
}
