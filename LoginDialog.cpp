// LoginDialog.cpp : implementation file
//

#include "stdafx.h"
#include "LoginDialog.h"


// CLoginDialog dialog

IMPLEMENT_DYNAMIC(CLoginDialog, CDialog)

CLoginDialog::CLoginDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CLoginDialog::IDD, pParent)
{
	caption="Welcome";
	
}

CLoginDialog::~CLoginDialog()
{
}

void CLoginDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLoginDialog, CDialog)
	ON_STN_CLICKED(IDC_LOGTEXT, &CLoginDialog::OnStnClickedLogtext)
	ON_CBN_SELCHANGE(IDC_COUNTRY, &CLoginDialog::OnCbnSelchangeCountry)
	ON_EN_CHANGE(IDC_CountryCode, &CLoginDialog::OnEnChangeCountrycode)
	ON_EN_CHANGE(IDC_PHONE, &CLoginDialog::OnEnChangePhone)
	ON_EN_CHANGE(IDC_PASSWORD, &CLoginDialog::OnEnChangePassword)
	ON_BN_CLICKED(IDC_LoginBTN, &CLoginDialog::OnBnClickedLoginbtn)
	ON_BN_CLICKED(IDC_PinRem, &CLoginDialog::OnBnClickedPinrem)
	ON_BN_CLICKED(IDC_Register, &CLoginDialog::OnBnClickedRegister)
END_MESSAGE_MAP()


// CLoginDialog message handlers

void CLoginDialog::OnStnClickedLogtext()
{
	// TODO: Add your control notification handler code here
}

void CLoginDialog::OnCbnSelchangeCountry()
{
	// TODO: Add your control notification handler code here
}

void CLoginDialog::OnEnChangeCountrycode()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CLoginDialog::OnEnChangePhone()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CLoginDialog::OnEnChangePassword()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CLoginDialog::OnBnClickedLoginbtn()
{
	caption="Log In";
	SetWindowText(caption);
	CString number= _T("Phone Number");
	CStatic *m_Label;
	CFont *m_Font1 = new CFont;
	m_Font1->CreatePointFont(160, (CString)"Arial Bold");
	m_Label = (CStatic *)GetDlgItem(IDC_LOGTEXT);
	m_Label->SetFont(m_Font1);

	GetDlgItem(IDC_LOGTEXT)->ShowWindow(SW_NORMAL);
	GetDlgItem(IDC_LOGO)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_COUNTRY)->ShowWindow(SW_NORMAL);
	GetDlgItem(IDC_PHONE)->ShowWindow(SW_NORMAL);
	((CEdit*)GetDlgItem(IDC_PHONE))->SetWindowText(number);
	GetDlgItem(IDC_CountryCode)->ShowWindow(SW_NORMAL);
	number= _T("Pin (or password maybe?)");
	GetDlgItem(IDC_PASSWORD)->ShowWindow(SW_NORMAL);
	((CEdit*)GetDlgItem(IDC_PASSWORD))->SetWindowText(number);
	GetDlgItem(IDC_Register)->ShowWindow(SW_HIDE);
	
}

void CLoginDialog::OnBnClickedPinrem()
{
	caption="Pin Reminder";
	SetWindowText(caption);

	CStatic *m_Label;
	CFont *m_Font1 = new CFont;
	m_Font1->CreatePointFont(160, (CString)"Arial Bold");
	m_Label = (CStatic *)GetDlgItem(IDC_LOGTEXT);
	
	m_Label->SetWindowText(caption);
	m_Label->SetFont(m_Font1);

	CString fieldText= _T("Phone Number");
	GetDlgItem(IDC_LOGTEXT)->ShowWindow(SW_NORMAL);
	GetDlgItem(IDC_LOGO)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_COUNTRY)->ShowWindow(SW_NORMAL);
	GetDlgItem(IDC_PHONE)->ShowWindow(SW_NORMAL);
	((CEdit*)GetDlgItem(IDC_PHONE))->SetWindowText(fieldText);
	GetDlgItem(IDC_CountryCode)->ShowWindow(SW_NORMAL);
	GetDlgItem(IDC_Register)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_LoginBTN)->ShowWindow(SW_HIDE);
	fieldText= _T("Please enter your email");
	GetDlgItem(IDC_EMAIL)->ShowWindow(SW_NORMAL);
	((CEdit*)GetDlgItem(IDC_EMAIL))->SetWindowText(fieldText);
	

}

void CLoginDialog::OnBnClickedRegister()
{
	caption="Register";
	SetWindowText(caption);

	CStatic *m_Label;
	CFont *m_Font1 = new CFont;
	m_Font1->CreatePointFont(160, (CString)"Arial Bold");
	m_Label = (CStatic *)GetDlgItem(IDC_LOGTEXT);
	
	m_Label->SetWindowText(caption);
	m_Label->SetFont(m_Font1);


	CString fieldText= _T("Phone Number");
	GetDlgItem(IDC_LOGTEXT)->ShowWindow(SW_NORMAL);
	GetDlgItem(IDC_LOGO)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_COUNTRY)->ShowWindow(SW_NORMAL);
	GetDlgItem(IDC_PHONE)->ShowWindow(SW_NORMAL);
	((CEdit*)GetDlgItem(IDC_PHONE))->SetWindowText(fieldText);
	GetDlgItem(IDC_CountryCode)->ShowWindow(SW_NORMAL);
	GetDlgItem(IDC_Register)->ShowWindow(SW_NORMAL);
	GetDlgItem(IDC_LoginBTN)->ShowWindow(SW_HIDE);
	fieldText= _T("Please enter your email");
	GetDlgItem(IDC_EMAIL)->ShowWindow(SW_NORMAL);
	((CEdit*)GetDlgItem(IDC_EMAIL))->SetWindowText(fieldText);
	GetDlgItem(IDC_NAME)->ShowWindow(SW_NORMAL);
	fieldText= _T("Please enter your name");
	((CEdit*)GetDlgItem(IDC_NAME))->SetWindowText(fieldText);
	GetDlgItem(IDC_PinRem)->ShowWindow(SW_HIDE);
}

BOOL CLoginDialog::OnInitDialog()
{
   // ...
   SetWindowText(caption);
   GetDlgItem(IDC_LOGTEXT)->ShowWindow(SW_HIDE);
   GetDlgItem(IDC_COUNTRY)->ShowWindow(SW_HIDE);
   GetDlgItem(IDC_PHONE)->ShowWindow(SW_HIDE);
   GetDlgItem(IDC_CountryCode)->ShowWindow(SW_HIDE);
   GetDlgItem(IDC_PASSWORD)->ShowWindow(SW_HIDE);
   GetDlgItem(IDC_EMAIL)->ShowWindow(SW_HIDE);
   GetDlgItem(IDC_NAME)->ShowWindow(SW_HIDE);
   return true;
}