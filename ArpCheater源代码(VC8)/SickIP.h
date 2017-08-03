#if !defined(AFX_SICKIP_H__0C7DEDEB_550C_4D06_BF89_B17E590BE3D3__INCLUDED_)
#define AFX_SICKIP_H__0C7DEDEB_550C_4D06_BF89_B17E590BE3D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SickIP.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSickIP window

class CSickIP : public CWnd
{
// Construction
public:
	CSickIP();
	int AddIP(unsigned long ip);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSickIP)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSickIP();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSickIP)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SICKIP_H__0C7DEDEB_550C_4D06_BF89_B17E590BE3D3__INCLUDED_)
