// ArpCheater.h : main header file for the ARPCHEATER application
//

#if !defined(AFX_ARPCHEATER_H__C4E377E0_42D4_493F_AB47_1260D267553F__INCLUDED_)
#define AFX_ARPCHEATER_H__C4E377E0_42D4_493F_AB47_1260D267553F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CArpCheaterApp:
// See ArpCheater.cpp for the implementation of this class
//

class CArpCheaterApp : public CWinApp
{
public:
	CArpCheaterApp();
	
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CArpCheaterApp)
public:
	virtual BOOL InitInstance();

	
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CArpCheaterApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ARPCHEATER_H__C4E377E0_42D4_493F_AB47_1260D267553F__INCLUDED_)
