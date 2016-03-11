// Charges.cpp : implementation file
//

#include "stdafx.h"
#include "Charges.h"
#include "microsipDlg.h"
#include <afxinet.h>
#include <sstream>
#include <iomanip>
#ifdef _DEBUG
#include <conio.h>
#endif
#include "addons\cJSON\cJSON.h"


// Charges dialog

IMPLEMENT_DYNAMIC(Charges, CDialog)

Charges::Charges(CWnd* pParent /*=NULL*/)
	: CDialog(Charges::IDD, pParent)
{
}

Charges::~Charges()
{
}

void Charges::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(Charges, CDialog)
	ON_BN_CLICKED(IDC_SUBMIT, &Charges::OnBnClickedSubmit)
	ON_CBN_SELCHANGE(IDC_HRS, &Charges::OnCbnSelchangeHrs)
END_MESSAGE_MAP()

BOOL Charges::OnInitDialog(){
	CDialog::OnInitDialog();	
	//TranslateDialog(this->m_hWnd);
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CHARGELIST);
	list->SetExtendedStyle( list->GetExtendedStyle() |  LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

	//SetDlgItemText(IDC_HRS, L"24");
	((CComboBox*)GetDlgItem(IDC_HRS))->SetCurSel(0);
	list->InsertColumn(0,_T("Destination"),LVCFMT_LEFT, 150);
	list->InsertColumn(1,_T("Call time"),LVCFMT_LEFT,150);
	list->InsertColumn(2,_T("Duration"),LVCFMT_LEFT,50);
	list->InsertColumn(3,_T("Charge"),LVCFMT_LEFT,50);
	ShowWindow(SW_SHOW);
	list->InsertItem(0, L"");
	list->SetItemText(0, 1, L"");
	list->SetItemText(0, 2, L"");
	list->SetItemText(0, 3, L"");
	OnBnClickedSubmit();
	return TRUE;
}


void Charges::OnBnClickedSubmit(){

	CString period;
	CComboBox* hours = (CComboBox*)GetDlgItem(IDC_HRS);
	switch(hours->GetCurSel()){
		case 0:
			period=_T("24");//1 day
			break;
		case 1:
			period=_T("168");//1 week
			break;
		default:
			period=_T("720");//1 month
	}
#ifdef _DEBUG
	_cprintf("%s\n",(CT2CA)period);
#endif

	CListCtrl *chargesList= (CListCtrl*)GetDlgItem(IDC_CHARGELIST);
	chargesList->DeleteAllItems();
	std::string header = "/oneworld/call_history?api_token=";
	header+=((CmicrosipDlg*)GetParent())->getToken();
	header+="&period_hours=";
	header+=(CT2CA)period;

	CInternetSession session;
	CHttpConnection *pConnection = session.GetHttpConnection(_T("89.163.142.253"));
	CString request(header.c_str());
	CHttpFile *pFile = pConnection->OpenRequest(1,request);
	if(!pFile->SendRequest())
		return;
	CString contentLength;
	pFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH,contentLength);
	int intLength = _wtoi(contentLength);
	char* result=(char*)malloc(intLength*sizeof(char));
	pFile->Read((void*)result,intLength);
	char* status = strchr(result,']'); //checking if data is receive and is parseable
	char* eom = strchr(result,'}');
#ifdef _DEBUG
	_cprintf("Size: %p, %p, %d\n",result, status, (status-result));
	_cprintf("Content length: %d\n",intLength);
	_cprintf("Result: %s\n",result);
#endif

	cJSON *root = cJSON_Parse(result);
	cJSON *msg = cJSON_GetObjectItem(root,"msg");
	if((status==NULL && msg == NULL) )
		return;
	else if(status==NULL)
		return;
	cJSON *data = cJSON_GetObjectItem(root,"data");
	int size=cJSON_GetArraySize(root); 
	
#ifdef _DEBUG
	_cprintf("Size: %d\n",size);
#endif
	size=cJSON_GetArraySize(data); 
	
#ifdef _DEBUG
	_cprintf("Size: %d\n",size);
#endif
	for(int i=0;i<size;i++){
		cJSON* item= cJSON_GetArrayItem(data,i);
		CString destination(cJSON_GetObjectItem(item,"calledstation")->valuestring);
		CString startTime(cJSON_GetObjectItem(item,"starttime")->valuestring);
		CString duration(cJSON_GetObjectItem(item,"duration")->valuestring);
		CString charge(cJSON_GetObjectItem(item,"charge")->valuestring);
#ifdef _DEBUG
		_cprintf("Destination: %s\n",(CT2CA)destination);
		_cprintf("Time: %s\n",(CT2CA)startTime);
		_cprintf("Duration: %s\n",(CT2CA)duration);
		_cprintf("Charge: %s\n",(CT2CA)charge);
#endif
		int index = chargesList->InsertItem(i, destination);
		chargesList->SetItemText(index, 1, startTime);
		chargesList->SetItemText(index, 2, duration);
		chargesList->SetItemText(index, 3, charge);
		//chargesList->SetItemData(i, &chargesListNum);
	}
	free(result);
}

void Charges::OnCbnSelchangeHrs()
{
	OnBnClickedSubmit();
}
