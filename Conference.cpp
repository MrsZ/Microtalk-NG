// onference.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "Conference.h"
#include "microsipDlg.h"
#include <afxinet.h>
#include <sstream>
#include <iomanip>
#ifdef _DEBUG
#include <conio.h>
#endif
#include "addons\cJSON\cJSON.h"


// Conference dialog

IMPLEMENT_DYNAMIC(Conference, CDialog)

Conference::Conference(CWnd* pParent /*=NULL*/)
	: CDialog(Conference::IDD, pParent)
{

}

Conference::~Conference()
{
}

void Conference::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(Conference, CDialog)
	//ON_NOTIFY(LVN_DELETEITEM, IDC_CONFLIST, &Conference::OnLvnDeleteitemConflist)
	ON_BN_CLICKED(IDC_CONFSUBMIT, &Conference::OnBnClickedConfsubmit)
	//ON_NOTIFY(NM_RCLICK, IDC_CONFLIST, &Conference::OnNMRClickConflist)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_JOIN, &Conference::OnJoin)
	ON_COMMAND(ID__DELETE, &Conference::OnDelete)
	ON_COMMAND(ID__COPY, &Conference::OnCopy)
END_MESSAGE_MAP()


BOOL Conference::OnInitDialog(){
	CDialog::OnInitDialog();	
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONFLIST);


//CHeaderCtrl* m_npheader = list->GetHeaderCtrl();
//HDITEM pHeaderItem;
//pHeaderItem.mask = HDI_TEXT | HDI_HEIGHT | HDI_FORMAT | HDI_ORDER;
//m_npheader->GetItem(0, &pHeaderItem);
//pHeaderItem.cxy = 40;
//m_npheader->SetItem(0, &pHeaderItem);


	list->SetExtendedStyle( list->GetExtendedStyle() |  LVS_EX_FULLROWSELECT |LVS_EX_GRIDLINES );

	list->InsertColumn(0,Translate(_T("Conference number")),LVCFMT_LEFT, 150);
	list->InsertColumn(1,Translate(_T("Pin")),LVCFMT_LEFT,150);

	LoadList();
	return TRUE;
}

// Conference message handlers


void Conference::OnBnClickedConfsubmit()
{

	std::string header = "/oneworld/conf_create?api_token=";
	header+=((CmicrosipDlg*)GetParent())->getToken();
	header+="&pin=";
	CString pinNum;
	GetDlgItemText(IDC_CONFPIN,pinNum);
	header+=(CT2CA)pinNum;
	header+="&length=0";
	CInternetSession session;
	CHttpConnection *pConnection = session.GetHttpConnection(_T("89.163.142.253"));
	char result[500];
	CString request(header.c_str());
	CHttpFile *pFile = pConnection->OpenRequest(1,request);
	if(!pFile->SendRequest())
		return;
	pFile->Read((void*)result,500);
	char* status = strchr(result,']'); //checking if data is receive and is parseable
	char* eom = strchr(result,'}');
#ifdef _DEBUG
	_cprintf("Size: %p, %p, %d\n",result, status, (status-result));
#endif
	if(status==NULL)
		result[eom-result+1]='\0';
	else if(status - result < 498)
		result[status - result +2]='\0';
#ifdef _DEBUG
	_cprintf("Result: %s\n",result);
#endif

	cJSON *root = cJSON_Parse(result);
	cJSON *success = cJSON_GetObjectItem(root,"success");

	
#ifdef _DEBUG
	_cprintf("Success: %s\n",success->valuestring);
#endif
	LoadList();
}


void Conference::LoadList(){
	CListCtrl *conf= (CListCtrl*)GetDlgItem(IDC_CONFLIST);
	conf->DeleteAllItems();

	std::string header = "/oneworld/conf_list?api_token=";
	header+=((CmicrosipDlg*)GetParent())->getToken();
	CInternetSession session;
	CHttpConnection *pConnection = session.GetHttpConnection(_T("89.163.142.253"));
	char result[500];
	CString request(header.c_str());
	CHttpFile *pFile = pConnection->OpenRequest(1,request);
	if(!pFile->SendRequest())
		return;
	pFile->Read((void*)result,500);
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
		CString confNum(cJSON_GetObjectItem(item,"confno")->valuestring);
		CString pin(cJSON_GetObjectItem(item,"pin")->valuestring);
#ifdef _DEBUG
		_cprintf("Item: %s\n",(CT2CA)confNum);
		_cprintf("Pin: %s\n",(CT2CA)pin);
#endif
		int index = conf->InsertItem(i, confNum);
		conf->SetItemText(index, 1, pin);
		//conf->SetItemData(i, &confNum);
	}
}
void Conference::OnContextMenu(CWnd* pWnd, CPoint point){
	int CtrlID = pWnd->GetDlgCtrlID();
#ifdef _DEBUG
		_cprintf("CtrlID: %d\n",CtrlID);
#endif
	if(CtrlID == IDC_CONFLIST){
		CMenu popupMenu;
		//popupMenu.LoadMenu(IDR_CONFMENU);
		popupMenu.LoadMenu(IDR_CONFMENU);
		CMenu* tracker = popupMenu.GetSubMenu(0);
		TranslateMenu(tracker->m_hMenu);
		tracker->EnableMenuItem(ID_JOIN, MF_ENABLED);
		tracker->EnableMenuItem(ID_COPY, MF_ENABLED);
		tracker->EnableMenuItem(ID_DELETE, MF_ENABLED);
		tracker->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, point.x, point.y, this );
	}
}

void Conference::OnJoin()
{
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONFLIST);
	POSITION pos = list->GetFirstSelectedItemPosition();
	if (pos)
	{
		int i = list->GetNextSelectedItem(pos);
		//Call *pCall = (Call *) list->GetItemData(i);
		CString number = L"105"+list->GetItemText(i,0);
		microsipDlg->messagesDlg->AddTab(FormatNumber(number), L"Conference", TRUE, NULL, TRUE && accountSettings.singleMode);
		microsipDlg->messagesDlg->Call(0);
	}
	EndDialog(1);
}

void Conference::OnDelete()
{
	
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONFLIST);
	POSITION pos = list->GetFirstSelectedItemPosition();
	CString confNum;
	if (pos)
	{
		int i = list->GetNextSelectedItem(pos);
		//Call *pCall = (Call *) list->GetItemData(i);
		confNum=list->GetItemText(i,0);
	}
	if(confNum.GetLength()==0)
		return;

	std::string header = "/oneworld/conf_del?api_token=";
	header+=((CmicrosipDlg*)GetParent())->getToken();
	header+="&confno=";
	header+=(CT2CA)confNum;
	CInternetSession session;
	CHttpConnection *pConnection = session.GetHttpConnection(_T("89.163.142.253"));
	char result[500];
	CString request(header.c_str());
	CHttpFile *pFile = pConnection->OpenRequest(1,request);
	if(!pFile->SendRequest())
		return;
	pFile->Read((void*)result,500);
	char* status = strchr(result,']'); //checking if data is receive and is parseable
	char* eom = strchr(result,'}');
#ifdef _DEBUG
	_cprintf("Size: %p, %p, %d\n",result, status, (status-result));
#endif
	if(status==NULL)
		result[eom-result+1]='\0';
	else if(status - result < 498)
		result[status - result +2]='\0';
#ifdef _DEBUG
	_cprintf("Result: %s\n",result);
#endif
	LoadList();
}

void Conference::OnCopy()
{
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONFLIST);
	POSITION pos = list->GetFirstSelectedItemPosition();
	if (pos)
	{
		int i = list->GetNextSelectedItem(pos);
		//Call *pCall = (Call *) list->GetItemData(i);
		CString conf = L"ID# "+list->GetItemText(i,0)+L", Pin# "+list->GetItemText(i,1);
		if ( !OpenClipboard() ){
		  AfxMessageBox( _T("Cannot open the Clipboard") );
		  return;
	   }
	   // Remove the current Clipboard contents 
	   if( !EmptyClipboard() )
	   {
		  AfxMessageBox( _T("Cannot empty the Clipboard") );
		  return;
	   }
	   // Get the currently selected data
	   HGLOBAL hGlob = GlobalAlloc(GMEM_FIXED, 64);
	   strcpy_s((char*)hGlob, 64, (CT2A)conf);

	    if ( ::SetClipboardData( CF_TEXT, hGlob ) == NULL ){
		  CString msg;
		  msg.Format(_T("Unable to set Clipboard data, error: %d"), GetLastError());
		  AfxMessageBox( msg );
		  CloseClipboard();
		  GlobalFree(hGlob);
		  return;
	   }

	   CloseClipboard();
	}

}
