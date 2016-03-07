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
END_MESSAGE_MAP()

BOOL Charges::OnInitDialog(){
	CDialog::OnInitDialog();	
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CHARGELIST);
	list->SetExtendedStyle( list->GetExtendedStyle() |  LVS_EX_FULLROWSELECT |LVS_EX_GRIDLINES );

	SetDlgItemText(IDC_HRS, L"24");
	list->InsertColumn(0,Translate(_T("Destination")),LVCFMT_LEFT, 150);
	list->InsertColumn(1,Translate(_T("Call time")),LVCFMT_LEFT,150);
	list->InsertColumn(2,Translate(_T("Duration")),LVCFMT_LEFT,50);
	list->InsertColumn(3,Translate(_T("Charge")),LVCFMT_LEFT,50);
	OnBnClickedSubmit();
	return TRUE;
}

void Charges::OnBnClickedSubmit(){

	CString period;
	GetDlgItemText(IDC_HRS, period);
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
	char result[5000];
	CString request(header.c_str());
	CHttpFile *pFile = pConnection->OpenRequest(1,request);
	if(!pFile->SendRequest())
		return;
	pFile->Read((void*)result,5000);
	char* status = strchr(result,']'); //checking if data is receive and is parseable
	char* eom = strchr(result,'}');
#ifdef _DEBUG
	_cprintf("Size: %p, %p, %d\n",result, status, (status-result));
#endif
	if(status==NULL)
		result[eom-result+1]='\0';
	else if(status - result < 4998)
		result[status - result +2]='\0';
#ifdef _DEBUG
	_cprintf("Result: %s\n",result);
#endif

	cJSON *root = cJSON_Parse(result);
	cJSON *msg = cJSON_GetObjectItem(root,"msg");
	if((status==NULL && msg == NULL) || status-result >= 4999 )
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
}
