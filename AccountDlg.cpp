#include "StdAfx.h"
#include "AccountDlg.h"
#include "microsipDlg.h"
#include <ws2tcpip.h>
#include "LoginDialog.h"
#include <string>
#include <afxinet.h>
#include <sstream>

#ifndef _GLOBAL_NO_ACCOUNT

#ifdef _GLOBAL_PROFILE
static CString profileItems[] = _GLOBAL_PROFILE;
static CString profileItemsValues[] = _GLOBAL_PROFILE_VALUES;
#endif

AccountDlg::AccountDlg(CWnd* pParent /*=NULL*/)
: CDialog(AccountDlg::IDD, pParent)
{
	accountId = 0;
	Create (IDD, pParent);

}

AccountDlg::~AccountDlg(void)
{
}


BOOL AccountDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	TranslateDialog(this->m_hWnd);

	GetDlgItem(IDC_SYSLINK_DELETE)->ShowWindow(SW_HIDE);

	CString str;

	str.Format(_T("<a>%s</a>"),Translate(_T("Remove account")));
	GetDlgItem(IDC_SYSLINK_DELETE)->SetWindowText(str);

#if !defined _GLOBAL_ACCOUNT_PASSWORD && !defined _GLOBAL_NO_DISPLAY_PASSWORD
#ifndef _GLOBAL_ACCOUNT_PIN
	str.Format(_T("<a>%s</a>"),Translate(_T("display pin")));
	GetDlgItem(IDC_SYSLINK_DISPLAY_PASSWORD)->SetWindowText(str);
#else
	str.Format(_T("<a>%s</a>"),Translate(_T("display Pin No")));
	GetDlgItem(IDC_SYSLINK_DISPLAY_PASSWORD)->SetWindowText(str);
#endif
#endif

#ifdef _GLOBAL_ACCOUNT_REG
	str.Format(_T("<a>%s</a>"),Translate(_T("Create Account")));
	GetDlgItem(IDC_SYSLINK_REG)->SetWindowText(str);
#endif

	CComboBox *combobox;

#ifndef _GLOBAL_ACCOUNT_TRANSPORT
	combobox= (CComboBox*)GetDlgItem(IDC_TRANSPORT);
	combobox->AddString(Translate(_T("Auto")));
	combobox->AddString(_T("UDP"));
	combobox->AddString(_T("TCP"));
	combobox->AddString(_T("TLS"));
	combobox->SetCurSel(0);
#endif

#ifndef _GLOBAL_ACCOUNT_MINI

	CEdit* edit;

	combobox= (CComboBox*)GetDlgItem(IDC_SRTP);
	combobox->AddString(Translate(_T("Disabled")));
	combobox->AddString(Translate(_T("Optional")));
	combobox->AddString(Translate(_T("Mandatory")));
	combobox->SetCurSel(0);

	combobox= (CComboBox*)GetDlgItem(IDC_PUBLIC_ADDR);
	combobox->AddString(Translate(_T("Auto")));
	char buf[256]={0};
	if ( gethostname(buf, 256) == 0) {
		struct addrinfo* l_addrInfo = NULL;
		struct addrinfo l_addrInfoHints;
		ZeroMemory(&l_addrInfoHints, sizeof(addrinfo));
		l_addrInfoHints.ai_socktype = SOCK_STREAM;
		l_addrInfoHints.ai_family = PF_INET;
		if ( getaddrinfo(buf,NULL, &l_addrInfoHints,&l_addrInfo) == 0 ) {
			if (l_addrInfo) {
				struct addrinfo* l_addrInfoCurrent = l_addrInfo;
				for (l_addrInfoCurrent = l_addrInfo; l_addrInfoCurrent; l_addrInfoCurrent=l_addrInfoCurrent->ai_next) {
					struct sockaddr_in *ipv4 = (struct sockaddr_in *)l_addrInfoCurrent->ai_addr;
					char * ip = inet_ntoa(ipv4->sin_addr);
					combobox->AddString(CString(ip));
				}
			}
		}
	}
	combobox->SetCurSel(0);

#endif

	return TRUE;
}

void AccountDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete this;
}

BEGIN_MESSAGE_MAP(AccountDlg, CDialog)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &AccountDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &AccountDlg::OnBnClickedOk)
#ifndef _GLOBAL_CUSTOM
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_SIP_SERVER, &AccountDlg::OnNMClickSyslinkSipServer)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_SIP_PROXY, &AccountDlg::OnNMClickSyslinkSipProxy)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_USERNAME, &AccountDlg::OnNMClickSyslinkUsername)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_DOMAIN, &AccountDlg::OnNMClickSyslinkDomain)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_AUTHID, &AccountDlg::OnNMClickSyslinkAuthID)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_PASSWORD, &AccountDlg::OnNMClickSyslinkPassword)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_NAME, &AccountDlg::OnNMClickSyslinkName)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_ENCRYPTION, &AccountDlg::OnNMClickSyslinkEncryption)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_TRANSPORT, &AccountDlg::OnNMClickSyslinkTransport)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_PUBLIC_ADDRESS, &AccountDlg::OnNMClickSyslinkPublicAddress)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_PUBLISH_PRESENCE, &AccountDlg::OnNMClickSyslinkPublishPresence)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_STUN_SERVER, &AccountDlg::OnNMClickSyslinkStunServer)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_ICE, &AccountDlg::OnNMClickSyslinkIce)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_REWRITE, &AccountDlg::OnNMClickSyslinkRewrite)
#endif
#if !defined _GLOBAL_ACCOUNT_PASSWORD && !defined _GLOBAL_NO_DISPLAY_PASSWORD
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_DISPLAY_PASSWORD, &AccountDlg::OnNMClickSyslinkDisplayPasswod)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_DISPLAY_PASSWORD, &AccountDlg::OnNMClickSyslinkDisplayPasswod)
#endif
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_DELETE, &AccountDlg::OnNMClickSyslinkDelete)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_DELETE, &AccountDlg::OnNMClickSyslinkDelete)
#ifdef _GLOBAL_ACCOUNT_REG
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_REG, &AccountDlg::OnNMClickSyslinkReg)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_REG, &AccountDlg::OnNMClickSyslinkReg)
#endif
	
END_MESSAGE_MAP()


void AccountDlg::OnClose() 
{
	DestroyWindow();
}

void AccountDlg::OnBnClickedCancel()
{
	OnClose();
}

void AccountDlg::Load(int id)
{
	CEdit* edit;
	CComboBox *combobox;
	accountId = id;
	m_Account=accountSettings.account;
	//if (accountSettings.AccountLoad(id,&m_Account)) {
	//	accountId = id;
	//} else {
	//	accountId = 0;
	//}

#ifndef _GLOBAL_ACCOUNT_SIP_SERVER
	edit = (CEdit*)GetDlgItem(IDC_EDIT_SERVER);
	edit->SetWindowText(m_Account.server);
#endif

#ifndef _GLOBAL_ACCOUNT_SIP_PROXY
	edit = (CEdit*)GetDlgItem(IDC_EDIT_PROXY);
	edit->SetWindowText(m_Account.proxy);
#endif

#ifndef _GLOBAL_ACCOUNT_DOMAIN
	edit = (CEdit*)GetDlgItem(IDC_EDIT_DOMAIN);
	edit->SetWindowText(m_Account.domain);
#endif


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

#ifndef _GLOBAL_ACCOUNT_LOGIN
	edit = (CEdit*)GetDlgItem(IDC_EDIT_AUTHID);
	edit->SetWindowText(m_Account.authID);
#endif


#ifndef _GLOBAL_ACCOUNT_PIN
	edit = (CEdit*)GetDlgItem(IDC_EDIT_USERNAME);
#ifdef _GLOBAL_ACCOUNT_API
	edit->SetWindowText(m_Account.apiLogin);
#else
	edit->SetWindowText(m_Account.username);
#endif
#endif

#ifndef _GLOBAL_ACCOUNT_PASSWORD
	edit = (CEdit*)GetDlgItem(IDC_EDIT_PASSWORD);
#ifdef _GLOBAL_ACCOUNT_API
	edit->SetWindowText(m_Account.apiPassword);
#else
	edit->SetWindowText(m_Account.password);
#endif
#endif

#ifdef _GLOBAL_ACCOUNT_REMEMBER_PASSWORD
	((CButton*)GetDlgItem(IDC_REMEMBER_PASSWORD))->SetCheck(m_Account.rememberPassword);
#endif

#ifdef _GLOBAL_ACCOUNT_API_ID
	edit = (CEdit*)GetDlgItem(IDC_EDIT_API_ID);
	edit->SetWindowText(m_Account.apiId);
#endif

#ifndef _GLOBAL_ACCOUNT_NAME
	edit = (CEdit*)GetDlgItem(IDC_EDIT_DISPLAYNAME);
	edit->SetWindowText(m_Account.displayName);
#endif

int i;

#ifndef _GLOBAL_ACCOUNT_TRANSPORT
	combobox= (CComboBox*)GetDlgItem(IDC_TRANSPORT);
	if (m_Account.transport==_T("udp")) {
		i=1;
	} else if (m_Account.transport==_T("tcp")) {
		i=2;
	} else if (m_Account.transport==_T("tls")) {
		i=3;
	} else {
		i=0;
	}
	if (i>0) {
		combobox->SetCurSel(i);
	}
#endif

#ifndef _GLOBAL_ACCOUNT_STUN
	edit = (CEdit*)GetDlgItem(IDC_STUN);
	edit->SetWindowText(m_Account.stun);
#endif

#ifndef _GLOBAL_ACCOUNT_MINI
	combobox= (CComboBox*)GetDlgItem(IDC_SRTP);
	if (m_Account.srtp==_T("optional")) {
		i=1;
	} else if (m_Account.srtp==_T("mandatory")) {
		i=2;
	} else {
		i=0;
	}
	if (i>0) {
		combobox->SetCurSel(i);
	}


	combobox= (CComboBox*)GetDlgItem(IDC_PUBLIC_ADDR);
	combobox->AddString(Translate(_T("Auto")));
	if (m_Account.publicAddr.GetLength()) {
		combobox->SetWindowText(m_Account.publicAddr);
	}

	((CButton*)GetDlgItem(IDC_PUBLISH))->SetCheck(m_Account.publish);

	((CButton*)GetDlgItem(IDC_ICE))->SetCheck(m_Account.ice);
	((CButton*)GetDlgItem(IDC_REWRITE))->SetCheck(m_Account.allowRewrite);
#endif
#ifdef _GLOBAL_ACCOUNT_API
	if (accountId>0 && !m_Account.apiLogin.IsEmpty()) {
#else
	if (accountId>0 && !m_Account.username.IsEmpty()) {
#endif
		GetDlgItem(IDC_SYSLINK_DELETE)->ShowWindow(SW_SHOW);
	}
}

void AccountDlg::OnBnClickedOk()
{
	CEdit* edit;
	CString str;
	CComboBox *combobox;
	int i;

#ifndef _GLOBAL_ACCOUNT_SIP_SERVER
	edit = (CEdit*)GetDlgItem(IDC_EDIT_SERVER);
	edit->GetWindowText(str);
	m_Account.server=str.Trim();
#endif

#ifndef _GLOBAL_ACCOUNT_SIP_PROXY
	edit = (CEdit*)GetDlgItem(IDC_EDIT_PROXY);
	edit->GetWindowText(str);
	m_Account.proxy=str.Trim();
#else
#ifdef _GLOBAL_ACCOUNT_PROXY_EQ_SERVER
	m_Account.proxy = m_Account.server;
#endif
#endif

#ifndef _GLOBAL_ACCOUNT_DOMAIN
	edit = (CEdit*)GetDlgItem(IDC_EDIT_DOMAIN);
	edit->GetWindowText(str);
	m_Account.domain=str.Trim();
#else
#ifdef _GLOBAL_ACCOUNT_DOMAIN_EQ_SERVER
	m_Account.domain = m_Account.server;
#endif
#endif

#ifdef _GLOBAL_PROFILE
	combobox= (CComboBox*)GetDlgItem(IDC_PROFILE);
	m_Account.profile = profileItems[combobox->GetCurSel()];
#endif

#ifdef _GLOBAL_ACCOUNT_PORT
	edit = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
	edit->GetWindowText(str);
	m_Account.port = atoi(CStringA(str.Trim()));
	if (m_Account.port<1 || m_Account.port>65535) {
		AfxMessageBox(_T("Port value is not valid."));
		return;
	}
#endif

#ifndef _GLOBAL_ACCOUNT_LOGIN
	edit = (CEdit*)GetDlgItem(IDC_EDIT_AUTHID);
	edit->GetWindowText(str);
	m_Account.authID=str.Trim();
#endif

#ifndef _GLOBAL_ACCOUNT_PIN
	edit = (CEdit*)GetDlgItem(IDC_EDIT_USERNAME);
#else
	edit = (CEdit*)GetDlgItem(IDC_EDIT_PASSWORD);
#endif
	edit->GetWindowText(str);
#ifndef _GLOBAL_ACCOUNT_API
	m_Account.username=str.Trim();
#else
	m_Account.apiLogin=str.Trim();
#endif

#ifndef _GLOBAL_ACCOUNT_PASSWORD
	edit = (CEdit*)GetDlgItem(IDC_EDIT_PASSWORD);
	edit->GetWindowText(str);
#ifndef _GLOBAL_ACCOUNT_API
	CString pin = m_Account.password=str.Trim();
#else
	m_Account.apiPassword=str.Trim();
#endif
#endif

requestPassword();

#ifdef _GLOBAL_ACCOUNT_API_ID
	edit = (CEdit*)GetDlgItem(IDC_EDIT_API_ID);
	edit->GetWindowText(str);
	m_Account.apiId=str.Trim();
#endif

#ifndef _GLOBAL_ACCOUNT_NAME
	edit = (CEdit*)GetDlgItem(IDC_EDIT_DISPLAYNAME);
	edit->GetWindowText(str);
	m_Account.displayName=str.Trim();
#endif

#ifndef _GLOBAL_ACCOUNT_TRANSPORT
	combobox= (CComboBox*)GetDlgItem(IDC_TRANSPORT);
	i = combobox->GetCurSel();
	switch (i) {
		case 1:
			m_Account.transport=_T("udp");
			break;
		case 2:
			m_Account.transport=_T("tcp");
			break;
		case 3:
			m_Account.transport=_T("tls");
			break;
		default:
			m_Account.transport=_T("");
	}
#endif

#ifndef _GLOBAL_ACCOUNT_STUN
	edit = (CEdit*)GetDlgItem(IDC_STUN);
	edit->GetWindowText(str);
	m_Account.stun=str.Trim();
#endif

#ifdef _GLOBAL_ACCOUNT_REMEMBER_PASSWORD
	m_Account.rememberPassword = ((CButton*)GetDlgItem(IDC_REMEMBER_PASSWORD))->GetCheck();
#else
#ifdef _GLOBAL_ACCOUNT_NO_REMEMBER_PASSWORD
	m_Account.rememberPassword = 0;
#else
	m_Account.rememberPassword = 1;
#endif
#endif

#ifndef _GLOBAL_ACCOUNT_MINI
	combobox= (CComboBox*)GetDlgItem(IDC_SRTP);
	i = combobox->GetCurSel();
	switch (i) {
		case 1:
			m_Account.srtp=_T("optional");
			break;
		case 2:
			m_Account.srtp=_T("mandatory");
			break;
		default:
			m_Account.srtp=_T("");
	}

	m_Account.publish = ((CButton*)GetDlgItem(IDC_PUBLISH))->GetCheck();

	m_Account.ice = ((CButton*)GetDlgItem(IDC_ICE))->GetCheck();

	m_Account.allowRewrite = ((CButton*)GetDlgItem(IDC_REWRITE))->GetCheck();

	combobox= (CComboBox*)GetDlgItem(IDC_PUBLIC_ADDR);
	i = combobox->GetCurSel();
	combobox->GetWindowText(m_Account.publicAddr);
	if (m_Account.publicAddr==Translate(_T("Auto")))
	{
		m_Account.publicAddr = _T("");
	}
#endif

#ifndef _GLOBAL_ACCOUNT_API
	if (m_Account.domain.IsEmpty() || m_Account.username.IsEmpty()) {
#else
	if (m_Account.apiLogin.IsEmpty()) {
#endif
		CString str;
#ifdef _GLOBAL_CUSTOM
		str.Append(Translate(_T("Please fill out all required fields.")));
#else
		str.Append(Translate(_T("Please fill out at least the required fields marked with *.")));
#endif
#ifndef _GLOBAL_ACCOUNT_MINI
		str.AppendFormat(_T(" %s"),Translate(_T("Ask your SIP provider how to configure the account correctly.")));
#endif
		AfxMessageBox(str);
		return;
	}

	this->ShowWindow(SW_HIDE);

	if (!accountId) {
		Account dummy;
		int i = 1;
		while (true) {
			if (!accountSettings.AccountLoad(i,&dummy)) {
				break;
			}
			i++;
		}
		accountId = i;
	}
	m_Account.password= pin;
	accountSettings.AccountSave(accountId,&m_Account);
	
	if (accountSettings.accountId) {
		microsipDlg->PJAccountDelete();
	}
	accountSettings.accountId = accountId;
	accountSettings.account = m_Account;
	accountSettings.SettingsSave();
	microsipDlg->PJAccountAdd();
	m_Account.password= pin;
	OnClose();
}

#ifndef _GLOBAL_CUSTOM

void AccountDlg::OnNMClickSyslinkSipServer(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("sipServer"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkSipProxy(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("sipProxy"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkUsername(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("username"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkDomain(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("domain"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkAuthID(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("login"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkPassword(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("password"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkName(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("name"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkEncryption(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("encryption"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkTransport(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("transport"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkPublicAddress(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("publicAddress"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkPublishPresence(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("publishPresence"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkStunServer(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("stunServer"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkIce(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("ice"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkRewrite(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("allowRewrite"));
	*pResult = 0;
}

#endif

#if !defined _GLOBAL_ACCOUNT_PASSWORD && !defined _GLOBAL_NO_DISPLAY_PASSWORD
void AccountDlg::OnNMClickSyslinkDisplayPasswod(NMHDR *pNMHDR, LRESULT *pResult)
{
	GetDlgItem(IDC_SYSLINK_DISPLAY_PASSWORD)->ShowWindow(SW_HIDE);
	CEdit* edit = (CEdit*)GetDlgItem(IDC_EDIT_PASSWORD);
	edit->SetPasswordChar(0);
	edit->Invalidate();
	edit->SetFocus();
	int nLength = edit->GetWindowTextLength();
	edit->SetSel(nLength,nLength);
	*pResult = 0;
}
#endif

void AccountDlg::OnNMClickSyslinkDelete(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (accountId>0 && AfxMessageBox(Translate(_T("Are you sure you want to remove?")), MB_YESNO)==IDYES) {
		this->ShowWindow(SW_HIDE);

		Account account;
		int i = accountId;
		while (true) {
			if (!accountSettings.AccountLoad(i+1,&account)) {
				break;
			}
			accountSettings.AccountSave(i,&account);
			if (accountSettings.accountId == i+1) {
				accountSettings.accountId = i;
				accountSettings.SettingsSave();
				accountId = 0;
			}
			i++;
		}
		accountSettings.AccountDelete(i);
		if (accountId && accountId == accountSettings.accountId) {
			microsipDlg->PJAccountDelete();
			if (i>1) {
				accountSettings.accountId = 1;
				accountSettings.AccountLoad(accountSettings.accountId,&accountSettings.account);
				microsipDlg->PJAccountAdd();
			} else {
				accountSettings.accountId = 0;
			}
			accountSettings.SettingsSave();
		}
		OnClose();
	}
	*pResult = 0;
}

#ifdef _GLOBAL_ACCOUNT_REG
void AccountDlg::OnNMClickSyslinkReg(NMHDR *pNMHDR, LRESULT *pResult)
{
		if (!microsipDlg->reg1Dlg)
		{
			microsipDlg->reg1Dlg = new Reg1(microsipDlg);
		} else {
			microsipDlg->reg1Dlg->SetForegroundWindow();
		}
	OnClose();
	*pResult = 0;
}
#endif

#endif


bool AccountDlg::requestPassword(){
	CString header = _T("Content-Type: application/x-www-form-urlencoded");
	CInternetSession session;
	CHttpConnection *pConnection = session.GetHttpConnection(_T("89.163.142.253"));
	CHttpFile *pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST,_T("/oneworld/login"));
	
	std::string req="submit=1&number=";
	req+=(CT2CA)m_Account.username;
	req+="&pincode=";
	req+=(CT2CA)m_Account.password;

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


	start=rest.Find(_T("fname\":\""));
	if(start<0)
		return false;
	start+=((CString)_T("fname\":\"")).GetLength();
	end=rest.Find(_T("\""),start);
	if(end<0)
		return false;
	CString name=rest.Mid(start, end-start);

	m_Account.password=password;
	m_Account.displayName=name;
	return true;
}