
// Editor.h : main header file for the Editor application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CEditor:
// See Editor.cpp for the implementation of this class
//

class CEditor : public CWinApp
{
public:
	CEditor();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CEditor theApp;
