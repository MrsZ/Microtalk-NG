// LoginDialog.cpp : implementation file
//

#include "stdafx.h"
#include "LoginDialog.h"
#include "CountryCodes.h"
#include "microsipDlg.h"
#include <algorithm>
#include <string>
#include <afxinet.h>
#include <sstream>
#include <iomanip>
#ifdef _DEBUG
#include <conio.h>
#endif


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

	ON_CBN_SELCHANGE(IDC_COUNTRY, &CLoginDialog::OnCbnSelchangeCountry)
	ON_BN_CLICKED(IDC_LoginBTN, &CLoginDialog::OnBnClickedLoginbtn)
	ON_BN_CLICKED(IDC_PinRem, &CLoginDialog::OnBnClickedPinrem)
	ON_BN_CLICKED(IDC_Register, &CLoginDialog::OnBnClickedRegister)
	ON_EN_SETFOCUS(IDC_PHONE, &CLoginDialog::OnEnSetfocusPhone)
	ON_EN_SETFOCUS(IDC_PASSWORD, &CLoginDialog::OnEnSetfocusPassword)
	ON_BN_CLICKED(IDC_BACK, &CLoginDialog::OnBnClickedBack)
	ON_EN_SETFOCUS(IDC_NAME, &CLoginDialog::OnEnSetfocusName)
	ON_EN_SETFOCUS(IDC_EMAIL, &CLoginDialog::OnEnSetfocusEmail)
	ON_BN_CLICKED(IDC_SAVE, &CLoginDialog::OnBnClickedSave)
	ON_STN_CLICKED(IDC_LOGTEXT, &CLoginDialog::OnStnClickedLogtext)
END_MESSAGE_MAP()


// CLoginDialog message handlers


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
	if(test.Compare(L"Phone")==0 || test.Compare(L"Phone Number")==0){
		SetDlgItemText(IDC_PHONE,L"");
	}

	// TODO: Add your control notification handler code here
	//IDC_PASSWORD
}

void CLoginDialog::OnBnClickedLoginbtn()
{
	GetDlgItem(IDC_SAVE)->ShowWindow(SW_NORMAL);
	GetDlgItem(IDC_BACK)->ShowWindow(SW_NORMAL);
	CString text;
	GetDlgItem(IDC_LoginBTN)->GetWindowText(text);
	SetDlgItemText(IDC_LOGTEXT,L"Log In");
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
		
		//m_Account.rememberPassword=1;
		accountSettings.AccountSave(accountId, &m_Account);
		//accountSettings.SettingsSave();
		if(requestPassword()){
			accountSettings.account = m_Account;
			EndDialog(IDOK);
		}
		else
			SetDlgItemText(IDC_LOGTEXT,L"Wrong username or password");
	}
	else{
		caption="Log In";
		SetWindowText(caption);
		SetDlgItemText(IDC_LoginBTN,L"Submit");
		GetDlgItem(IDC_EMAIL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LOGTEXT)->ShowWindow(SW_NORMAL);
		GetDlgItem(IDC_LOGO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COUNTRY)->ShowWindow(SW_NORMAL);
		GetDlgItem(IDC_PHONE)->ShowWindow(SW_NORMAL);
		GetDlgItem(IDC_CountryCode)->ShowWindow(SW_NORMAL);
		GetDlgItem(IDC_PASSWORD)->ShowWindow(SW_NORMAL);
		GetDlgItem(IDC_Register)->ShowWindow(SW_HIDE);
		((CButton*) GetDlgItem(IDC_SAVE))->SetCheck(BST_CHECKED);
		m_Account.rememberPassword=BST_CHECKED;
	}
	
}

void CLoginDialog::OnBnClickedPinrem(){
	GetDlgItem(IDC_SAVE)->ShowWindow(SW_HIDE);
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
			SetDlgItemText(IDC_LOGTEXT,L"You will receive a PIN soon");
			//Sleep(1000);
			//SetDlgItemText(IDC_PASSWORD,L"");
			GetDlgItem(IDC_PinRem)->ShowWindow(SW_HIDE);
			OnBnClickedLoginbtn();
			GetDlgItem(IDC_LoginBTN)->ShowWindow(SW_NORMAL);
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
	GetDlgItem(IDC_SAVE)->ShowWindow(SW_HIDE);
	caption="Register";
	SetWindowText(caption);

	CStatic *m_Label;
	CFont *m_Font1 = new CFont;
	m_Font1->CreatePointFont(160, (CString)"Arial Bold");
	m_Label = (CStatic *)GetDlgItem(IDC_LOGTEXT);
	
	m_Label->SetWindowText(caption);
	m_Label->SetFont(m_Font1);

	CString text;
	GetDlgItemText(IDC_Register,text);

	if(0==text.Compare(_T("Submit"))){
		if(registration()){
			GetDlgItem(IDC_PinRem)->ShowWindow(SW_NORMAL);
			SetDlgItemText(IDC_PinRem,L"Submit");
			OnBnClickedPinrem();
		}
	}else{
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
   for(int i=0;i<sizeof(countries)/(4*sizeof(std::string));i++){
	   countryBox->AddString( (CString) countries[i][0].c_str() );
	}
   accountSettings.Init();
   Load(accountSettings.accountId);
   return true;
}


void CLoginDialog::OnEnSetfocusPassword()
{
	CString text;
	GetDlgItemText(IDC_PASSWORD,text);
	if(text.Compare(_T("Pin"))==0)
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
		m_Account.Country=161;
		m_Account.username="Phone";
		m_Account.password="Pin";
		m_Account.ice=0;
		m_Account.publish=0;
		m_Account.allowRewrite=0;
	}else{
		((CmicrosipDlg*)GetParent())->PJDestroy();
		accountSettings.Init();
		m_Account.domain=m_Account.server="89.163.142.253";
		m_Account.Country=accountSettings.account.Country;
		m_Account.username=accountSettings.account.username;
		m_Account.password=accountSettings.account.password;
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
	GetDlgItem(IDC_BACK)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SAVE)->ShowWindow(SW_HIDE);
	OnInitDialog();
}

bool CLoginDialog::requestPassword(){
	CString header = _T("Content-Type: application/x-www-form-urlencoded");
	CInternetSession session;
	CHttpConnection *pConnection = session.GetHttpConnection(_T("89.163.142.253"));
	CHttpFile *pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST,_T("/oneworld/login"));
	
	std::string req="submit=1&number=";
	req+=(CT2CA)m_Account.username;
	req+="&pincode=";
	req+=(CT2CA)m_Account.password;

#ifdef _DEBUG
	_cprintf("Address: 89.163.142.253\n");
	_cprintf("Request: %s\n",req.c_str());
#endif

	long len = req.length();
	BOOL res = pFile->SendRequest(header,lstrlen(header), (LPVOID)req.c_str(),len);

	if(!res){
#ifdef _DEBUG
	_cprintf("Request failed!!!!\n");
#endif
		return res;
	}
	//pFile
	char result[500];
	pFile->Read(result,500);
#ifdef _DEBUG
	_cprintf("***********************************\n");
	_cprintf("%s\n",result);
	_cprintf("***********************************\n");
#endif
	CString rest(result);
	int start=rest.Find(_T("password\":\""));
	if(start<0)
		return false;
	start+=((CString)_T("password\":\"")).GetLength();
	int end=rest.Find(_T("\""),start);
	if(end<0)
		return false;
	CString password=rest.Mid(start, end-start);
#ifdef _DEBUG
	_cprintf("%s\n",(CT2CA)password);
	_cprintf("%s\n",result);
#endif

	start=rest.Find(_T("fname\":\""));
	if(start<0)
		return false;
	start+=((CString)_T("fname\":\"")).GetLength();
	end=rest.Find(_T("\""),start);
	if(end<0)
		return false;
	CString name=rest.Mid(start, end-start);
#ifdef _DEBUG
	_cprintf("%s\n",(CT2CA)name);
	_cprintf("%s\n",result);
#endif


	start=rest.Find(_T("api_token\":\""));
	if(start<0)
		return false;
	start+=((CString)_T("api_token\":\"")).GetLength();
	end=rest.Find(_T("\""),start);
	if(end<0)
		return false;
	CString token=rest.Mid(start, end-start);
	std::string strToken=(CT2CA)token;
	((CmicrosipDlg*)GetParent())->setToken(strToken);
	std::string pin=(CT2CA)m_Account.password;
	((CmicrosipDlg*)GetParent())->setPin(pin);
	//GetParent();
#ifdef _DEBUG
	_cprintf("%s\n",(CT2CA)token);
	_cprintf("%s\n",result);
#endif

	m_Account.password=password;
	m_Account.displayName=name;
	return true;
}

bool CLoginDialog::registration(){

	CString phone, name, email;
	int country=((CComboBox*) GetDlgItem(IDC_COUNTRY))->GetCurSel();
	GetDlgItemText(IDC_PHONE,phone);
	GetDlgItemText(IDC_NAME,name);
	GetDlgItemText(IDC_EMAIL,email);

	std::string header = "/oneworld/webreg?country=";
	header+=countries[country][2];
	header+="&number=";
	header+=(CT2CA)phone;
	header+="&name=";
	header+=(CT2CA)name;
	header+="&email=";
	header+=(CT2CA)email;
	header+="&countrycode=";
	header+=countries[country][2];

	CInternetSession session;
	CHttpConnection *pConnection = session.GetHttpConnection(_T("89.163.142.253"));
	char result[500];
	CString request(header.c_str());
	CHttpFile *pFile = pConnection->OpenRequest(1,request);
	if(!pFile->SendRequest())
		return false;
	//pFile->QueryInfo(HTTP_QUERY_FLAG_REQUEST_HEADERS,result,(LPDWORD)500);
	
#ifdef _DEBUG
	pFile->Read((void*)result,500);
	_cprintf("%s",result);
#endif

	CString rest(result);
	int start=rest.Find(_T("success\":\""));
	if(start<0)
		return false;
	start+=((CString)_T("success\":\"")).GetLength();
	int end=rest.Find(_T("\""),start);
	if(end<0)
		return false;
	CString success=rest.Mid(start, end-start);
#ifdef _DEBUG
	_cprintf("%s",(CT2CA)success);
	_cprintf("%s",result);
#endif
	start=rest.Find(_T("msg\":\""));
	start+=((CString)_T("msg\":\"")).GetLength();
	end=rest.Find(_T("\""),start);
	CString msg=rest.Mid(start, end-start);
	SetDlgItemText(IDC_LOGTEXT,msg);
	Sleep(2000);

	return true;
}

bool CLoginDialog::pinReminder(){
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
	//pFile->QueryInfo(HTTP_QUERY_FLAG_REQUEST_HEADERS,result,(LPDWORD)500);
	
#ifdef _DEBUG
	pFile->Read((void*)result,500);
	_cprintf("%s",result);
#endif

	return true;

}

void CLoginDialog::OnEnSetfocusName()
{
	SetDlgItemText(IDC_NAME,L"");
}

void CLoginDialog::OnEnSetfocusEmail()
{
	SetDlgItemText(IDC_EMAIL,L"");
}

void CLoginDialog::OnBnClickedSave()
{
	m_Account.rememberPassword=((CButton*) GetDlgItem(IDC_SAVE))->GetCheck();
#ifdef _DEBUG
	_cprintf("Clicked!\n%d\n",m_Account.rememberPassword);
#endif
}

void CLoginDialog::OnStnClickedLogtext()
{
	// TODO: Add your control notification handler code here
}
