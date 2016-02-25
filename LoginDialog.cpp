// LoginDialog.cpp : implementation file
//

#include "stdafx.h"
#include "LoginDialog.h"
#include "CountryCodes.h"
#include "microsipDlg.h"
#include <algorithm>
#include <string>
#include <afxinet.h>
#ifdef DEBUG
#include <conio.h>
#endif

#include <sstream>
#include <iomanip>

// CLoginDialog dialog

IMPLEMENT_DYNAMIC(CLoginDialog, CDialog)

CLoginDialog::CLoginDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CLoginDialog::IDD, pParent)
{
	caption="Welcome";
	//m_Account=((CmicrosipDlg*)GetParent())->m_Account;
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
	ON_EN_SETFOCUS(IDC_PHONE, &CLoginDialog::OnEnSetfocusPhone)
	ON_EN_SETFOCUS(IDC_PASSWORD, &CLoginDialog::OnEnSetfocusPassword)
	ON_BN_CLICKED(IDC_BACK, &CLoginDialog::OnBnClickedBack)
END_MESSAGE_MAP()


// CLoginDialog message handlers

void CLoginDialog::OnStnClickedLogtext()
{
	// TODO: Add your control notification handler code here
}

void CLoginDialog::OnCbnSelchangeCountry()
{
	CComboBox* countryBox = ((CComboBox*) GetDlgItem(IDC_COUNTRY));
	SetDlgItemText(IDC_CountryCode, (CString) countries[countryBox->GetCurSel()][1].c_str());
	
}

void CLoginDialog::OnEnChangeCountrycode()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}


void CLoginDialog::OnEnSetfocusPhone()
{
	CString test;
	GetDlgItemText(IDC_PHONE,test);
	if(test.Compare(L"Phone")==0){
		SetDlgItemText(IDC_PHONE,L"");
	}

	// TODO: Add your control notification handler code here
	//IDC_PASSWORD
}

void CLoginDialog::OnEnChangePhone()
{

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
	GetDlgItem(IDC_BACK)->ShowWindow(SW_NORMAL);
	CString text;
	GetDlgItem(IDC_LoginBTN)->GetWindowText(text);
	if(0==text.Compare(L"Submit")){
		accountSettings.SettingsSave();
		m_Account.Country=((CComboBox*) GetDlgItem(IDC_COUNTRY))->GetCurSel();
		std::string userPhone=countries[m_Account.Country][1];
		userPhone.erase(std::find(userPhone.begin(), userPhone.end(), '+'));
		userPhone.erase(std::find(userPhone.begin(), userPhone.end(), '-'));
		CString user(m_Account.username);
		user.Replace((CString)userPhone.c_str(),NULL);
		GetDlgItemText(IDC_PHONE,m_Account.username);
		CT2CA ansistring (m_Account.username);
		userPhone.append(ansistring);
		m_Account.username=(CString)userPhone.c_str();
		GetDlgItemText(IDC_PASSWORD,m_Account.password);
		accountSettings.accountId = accountId;
		accountSettings.account = m_Account;
		m_Account.rememberPassword=1;
		accountSettings.AccountSave(accountId, &m_Account);
		//accountSettings.SettingsSave();
		if(requestPassword())
			EndDialog(IDOK);
		else
			SetWindowText(L"Wrong username or password");
	}
	else{
		caption="Log In";
		SetWindowText(caption);
		SetDlgItemText(IDC_LoginBTN,L"Submit");
		GetDlgItem(IDC_LOGTEXT)->ShowWindow(SW_NORMAL);
		GetDlgItem(IDC_LOGO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COUNTRY)->ShowWindow(SW_NORMAL);
		GetDlgItem(IDC_PHONE)->ShowWindow(SW_NORMAL);
		GetDlgItem(IDC_CountryCode)->ShowWindow(SW_NORMAL);
		GetDlgItem(IDC_PASSWORD)->ShowWindow(SW_NORMAL);
		GetDlgItem(IDC_Register)->ShowWindow(SW_HIDE);
	}
	
}

void CLoginDialog::OnBnClickedPinrem()
{
	caption="Pin Reminder";
	SetWindowText(caption);
	CString text;
	GetDlgItem(IDC_PinRem)->GetWindowText(text);

	CStatic *m_Label;
	CFont *m_Font1 = new CFont;
	m_Font1->CreatePointFont(160, (CString)"Arial Bold");
	m_Label = (CStatic *)GetDlgItem(IDC_LOGTEXT);
	
	m_Label->SetWindowText(caption);
	m_Label->SetFont(m_Font1);

	if(0==text.Compare(L"Submit")){
		if(pinReminder()){
			SetDlgItemText(IDC_LOGTEXT,L"You will receive a PIN on your email soon");
			Sleep(3000);
			SetDlgItemText(IDC_LoginBTN,L"Log In");
			OnBnClickedLoginbtn();
		}else{
			SetDlgItemText(IDC_LOGTEXT,L"Something went wrong");
		}

	}else{

		GetDlgItem(IDC_LOGTEXT)->ShowWindow(SW_NORMAL);
		GetDlgItem(IDC_LOGO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_Register)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LoginBTN)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COUNTRY)->ShowWindow(SW_NORMAL);
		GetDlgItem(IDC_PHONE)->ShowWindow(SW_NORMAL);
		GetDlgItem(IDC_CountryCode)->ShowWindow(SW_NORMAL);
		GetDlgItem(IDC_EMAIL)->ShowWindow(SW_NORMAL);
		((CEdit*)GetDlgItem(IDC_EMAIL))->SetWindowText(L"Please enter your email");
		GetDlgItem(IDC_BACK)->ShowWindow(SW_NORMAL);
		SetDlgItemText(IDC_PinRem,L"Submit");
	}
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
	GetDlgItem(IDC_BACK)->ShowWindow(SW_NORMAL);
	SetDlgItemText(IDC_Register,L"Submit");
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

   	GetDlgItem(IDC_LOGO)->ShowWindow(SW_NORMAL);
	GetDlgItem(IDC_Register)->ShowWindow(SW_NORMAL);
	GetDlgItem(IDC_LoginBTN)->ShowWindow(SW_NORMAL);
	GetDlgItem(IDC_PinRem)->ShowWindow(SW_NORMAL);

   	CString number= _T("Phone Number");
	CStatic *m_Label;
	CFont *m_Font1 = new CFont;
	m_Font1->CreatePointFont(160, (CString)"Arial Bold");
	m_Label = (CStatic *)GetDlgItem(IDC_LOGTEXT);
	m_Label->SetFont(m_Font1);
	((CEdit*)GetDlgItem(IDC_PHONE))->SetWindowText(number);


	((CEdit*)GetDlgItem(IDC_PASSWORD))->SetWindowText(L"Pin");
   CComboBox* countryBox = ((CComboBox*) GetDlgItem(IDC_COUNTRY));
   if(countryBox->GetCount()<2)
   for(int i=0;i<sizeof(countries)/(2*sizeof(std::string));i++){
	   countryBox->AddString( (CString) countries[i][0].c_str() );
	}
   accountSettings.Init();
   Load(accountSettings.accountId);
   return true;
}


void CLoginDialog::OnEnSetfocusPassword()
{
	SetDlgItemText(IDC_PASSWORD,L"");
}

void CLoginDialog::Load(int id)
{
	CEdit* edit;
	CComboBox *combobox;
	accountId = id;
	if (accountSettings.AccountLoad(id,&m_Account)) {
		accountId = id;
	} else {
		accountId = 0;
	}
	if(accountId==0){
		((CmicrosipDlg*)GetParent())->PJDestroy();
		accountSettings.Init();
		((CmicrosipDlg*)GetParent())->pageContacts=new Contacts;
		//((CmicrosipDlg*)GetParent())->PJCreate();
		//((CmicrosipDlg*)GetParent())->PJAccountAdd();
		accountId++;
		accountSettings.accountId++;
		m_Account.domain=m_Account.server="89.163.142.253";
		m_Account.Country=155;
		m_Account.username="Phone";
		m_Account.password="Pin";
		m_Account.ice=0;
		m_Account.publish=0;
		m_Account.allowRewrite=0;
		
		
	}
	SetDlgItemText(IDC_PASSWORD,m_Account.password);

	CComboBox* countryBox = ((CComboBox*) GetDlgItem(IDC_COUNTRY));
	countryBox->SetCurSel(m_Account.Country);
	SetDlgItemText(IDC_CountryCode, (CString) countries[countryBox->GetCurSel()][1].c_str());
	SetDlgItemText(IDC_CountryCode, (CString) countries[m_Account.Country][1].c_str());

	std::string userPhone=countries[m_Account.Country][1];
	userPhone.erase(std::find(userPhone.begin(), userPhone.end(), '+'));
	userPhone.erase(std::find(userPhone.begin(), userPhone.end(), '-'));
	CString user(m_Account.username);
	user.Replace((CString)userPhone.c_str(),NULL);
	SetDlgItemText(IDC_PHONE,user);

#ifdef _GLOBAL_PROFILE
	combobox= (CComboBox*)GetDlgItem(IDC_PROFILE);
	int n = sizeof(profileItems)/sizeof(profileItems[0]);
	bool found = false;
	for (int i=0;i<n;i++) {
		combobox->AddString(profileItemsValues[i]);
		if (m_Account.profile==profileItems[i]) {
			combobox->SetCurSel(i);
			found = true;
		}
	}
	if (!found)  {
		combobox->SetCurSel(0);
	}
#endif

#ifdef _GLOBAL_ACCOUNT_PORT
	edit = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
	CString str;
	str.Format(_T("%d"),m_Account.port);
	edit->SetWindowText(str);
#endif


#ifdef _GLOBAL_ACCOUNT_REMEMBER_PASSWORD
	((CButton*)GetDlgItem(IDC_REMEMBER_PASSWORD))->SetCheck(m_Account.rememberPassword);
#endif

#ifdef _GLOBAL_ACCOUNT_API_ID
	edit = (CEdit*)GetDlgItem(IDC_EDIT_API_ID);
	edit->SetWindowText(m_Account.apiId);
#endif

//((CmicrosipDlg*)GetParent())->PJDestroy();

}


void CLoginDialog::OnBnClickedBack()
{
	SetDlgItemText(IDC_LoginBTN,L"Log In");
	SetDlgItemText(IDC_Register,L"Register");
	SetDlgItemText(IDC_PinRem,L"Pin Reminder");
	GetDlgItem(IDC_Register)->ShowWindow(SW_NORMAL);
	GetDlgItem(IDC_PinRem)->ShowWindow(SW_NORMAL);
	OnInitDialog();
}

bool CLoginDialog::requestPassword(){
#ifdef DEBUG
	if (!AllocConsole())
		AfxMessageBox(L"Failed to create the console!");
#endif
	CString header = _T("Content-Type: application/x-www-form-urlencoded");
	CInternetSession session;
	CHttpConnection *pConnection = session.GetHttpConnection(_T("89.163.142.253"));
	CHttpFile *pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST,_T("/oneworld/login"));
	
	std::string req="submit=1&number=";
	req+=(CT2CA)accountSettings.account.username;
	req+="&pincode=";
	req+=(CT2CA)accountSettings.account.password;

	long len = req.length();
	BOOL res = pFile->SendRequest(header,lstrlen(header), (LPVOID)req.c_str(),len);
	if(!res)
		return !res;
	//pFile
	char result[500];
	pFile->Read(result,500);
	CString rest(result);
	int start=rest.Find(_T("password\":\""));
	if(start<0)
		return false;
	start+=((CString)_T("password\":\"")).GetLength();
	int end=rest.Find(_T("\""),start);
	if(end<0)
		return false;
	CString password=rest.Mid(start, end-start);
#ifdef DEBUG
	_cprintf("%s",password.GetString());
#endif
	accountSettings.account.password=password;
	return true;
}

bool CLoginDialog::registration(){
#ifdef DEBUG
	if (!AllocConsole())
		AfxMessageBox(L"Failed to create the console!");
#endif
	CInternetSession session;
	CHttpConnection *pConnection = session.GetHttpConnection(_T("89.163.142.253"));
	CHttpFile *pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST,_T("/oneworld/reg"));


return true;
}

bool CLoginDialog::pinReminder(){
#ifdef DEBUG
	if (!AllocConsole())
		AfxMessageBox(L"Failed to create the console!");
#endif
	CString username, email;
	int country=((CComboBox*) GetDlgItem(IDC_COUNTRY))->GetCurSel();
	std::string userPhone=countries[country][1];
	userPhone.erase(std::find(userPhone.begin(), userPhone.end(), '+'));
	userPhone.erase(std::find(userPhone.begin(), userPhone.end(), '-'));
	GetDlgItemText(IDC_PHONE,username);
	username=(CString)userPhone.c_str()+username;
	GetDlgItemText(IDC_EMAIL,email);


	std::string header = "/oneworld/forgotpin?number=";
		header+=(CT2CA)username;
		header+="&email=";
		header+=(CT2CA)email;

	CInternetSession session;
	CHttpConnection *pConnection = session.GetHttpConnection(_T("89.163.142.253"));
	char result[500];
	CString request(header.c_str());
	CHttpFile *pFile = pConnection->OpenRequest(1,request);
	if(!pFile->SendRequest())
		return false;
	pFile->Write(result,500);
#ifdef DEBUG
	_cprintf("%s",result);
#endif

	return true;

}

