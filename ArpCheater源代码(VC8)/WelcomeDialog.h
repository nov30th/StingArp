#if !defined(AFX_WELCOMEDIALOG_H__99A772FD_9068_48EC_AA77_C4928504A3EA__INCLUDED_)
#define AFX_WELCOMEDIALOG_H__99A772FD_9068_48EC_AA77_C4928504A3EA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WelcomeDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWelcomeDialog dialog

class CWelcomeDialog : public CDialog
{
// Construction
public:
	CWelcomeDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CWelcomeDialog)
	enum { IDD = IDD_WELCOME_DIALOG };
	CString	m_information;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWelcomeDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWelcomeDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WELCOMEDIALOG_H__99A772FD_9068_48EC_AA77_C4928504A3EA__INCLUDED_)
