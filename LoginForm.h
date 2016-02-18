#pragma once

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

#include "resource.h"
#include "BaseDialog.h"

// LoginForm dialog

class LoginForm : public CBaseDialog
{

public:
	LoginForm(CWnd* pParent = NULL);   // standard constructor
	virtual ~LoginForm();
// Overrides


// Dialog Data
	enum { IDD = IDD_LOGIN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
