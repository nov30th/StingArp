// RecordDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ArpCheater.h"
#include "RecordDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRecordDialog dialog


CRecordDialog::CRecordDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CRecordDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRecordDialog)
	m_recstring = _T("");
	//}}AFX_DATA_INIT
}


void CRecordDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRecordDialog)
	DDX_Text(pDX, IDC_EDITREC, m_recstring);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRecordDialog, CDialog)
	//{{AFX_MSG_MAP(CRecordDialog)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTONCLEAN, OnButtonclean)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRecordDialog message handlers

void CRecordDialog::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	ShowWindow(SW_HIDE);
	//CDialog::OnClose();
}

void CRecordDialog::OnButtonclean() 
{
	// TODO: Add your control notification handler code here
	m_recstring = "";
	UpdateData(false);
}



void CRecordDialog::LocalUpdate() 
{
	// TODO: Add your control notification handler code here
	UpdateData(false);
}
