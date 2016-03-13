#pragma once
#include "resource.h"
#include "settings.h"
#include "microsipDlg.h"

// CLoginDialog dialog
class CLoginDialog : public CDialog
{
	DECLARE_DYNAMIC(CLoginDialog)

public:
	CLoginDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLoginDialog();
	afx_msg void OnBnClickedCancel();
// Dialog Data
	enum { IDD = IDD_LOGIN };
	CString caption;
	void Load(int id);
	CWnd* pParent;
//API funcs
	bool requestPassword(void);
	bool registration(void);
	bool pinReminder(void);


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP();
public:
	afx_msg void OnCbnSelchangeCountry();
	afx_msg void OnEnChangeCountrycode();
	afx_msg void OnBnClickedLoginbtn();
	afx_msg void OnBnClickedPinrem();
	afx_msg void OnBnClickedRegister();
	afx_msg void OnEnSetfocusPhone();
	afx_msg void OnEnSetfocusPassword();
	afx_msg void OnBnClickedBack();
private:
	int accountId;
	Account m_Account;
public:
	afx_msg void OnEnSetfocusName();
	afx_msg void OnEnSetfocusEmail();
	afx_msg void OnBnClickedSave();
	afx_msg void OnStnClickedLogtext();
	friend class CmicrosipDlg;
};
