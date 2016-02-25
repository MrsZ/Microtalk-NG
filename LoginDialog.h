#pragma once
#include "resource.h"

// CLoginDialog dialog
class CLoginDialog : public CDialog
{
	DECLARE_DYNAMIC(CLoginDialog)

public:
	CLoginDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLoginDialog();

// Dialog Data
	enum { IDD = IDD_LOGIN };
	CString caption;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL CLoginDialog::OnInitDialog();
	DECLARE_MESSAGE_MAP();
public:
	afx_msg void OnStnClickedLogtext();
	afx_msg void OnCbnSelchangeCountry();
	afx_msg void OnEnChangeCountrycode();
	afx_msg void OnEnChangePhone();
	afx_msg void OnEnChangePassword();
	afx_msg void OnBnClickedLoginbtn();
	afx_msg void OnBnClickedPinrem();
	afx_msg void OnBnClickedRegister();
};
