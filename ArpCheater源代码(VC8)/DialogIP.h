#if !defined(AFX_DIALOGIP_H__FB9CF474_8040_4F34_90E2_4085C301EB3E__INCLUDED_)
#define AFX_DIALOGIP_H__FB9CF474_8040_4F34_90E2_4085C301EB3E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DialogIP.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDialogIP dialog

class CDialogIP : public CDialog
{
// Construction
public:
	CDialogIP(CWnd* pParent = NULL);   // standard constructor
	
// Dialog Data
	//{{AFX_DATA(CDialogIP)
	enum { IDD = IDD_DIALOGIPINPUT };
	CIPAddressCtrl	m_ip;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogIP)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogIP)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	unsigned long ipaddress;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIALOGIP_H__FB9CF474_8040_4F34_90E2_4085C301EB3E__INCLUDED_)
