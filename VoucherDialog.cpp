// VoucherDialog.cpp : implementation file
//

#include "stdafx.h"
#include "microsip.h"
#include "VoucherDialog.h"
#include "microsipDlg.h"
#ifdef _DEBUG
#include <conio.h>
#endif


// VoucherDialog dialog

IMPLEMENT_DYNAMIC(VoucherDialog, CDialog)

VoucherDialog::VoucherDialog(CWnd* pParent /*=NULL*/)
	: CDialog(VoucherDialog::IDD, pParent)
{

}

VoucherDialog::~VoucherDialog()
{
}

void VoucherDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(VoucherDialog, CDialog)
	ON_BN_CLICKED(IDOK, &VoucherDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &VoucherDialog::OnBnClickedCancel)
	ON_BN_CLICKED(ID_VOUCH_SUB, &VoucherDialog::OnBnClickedVouchSub)
END_MESSAGE_MAP()


// VoucherDialog message handlers

void VoucherDialog::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}

void VoucherDialog::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

BOOL VoucherDialog::OnInitDialog(){
	CFont *m_Font1 = new CFont;
	m_Font1->CreatePointFont(120, (CString)"Arial Bold");
	GetDlgItem(IDC_STATUS)->SetFont(m_Font1);
	GetDlgItem(IDC_STATUS)->SetWindowTextW(L"Enter Voucher #");
	GetDlgItem(IDC_VOUCH_ID)->SetFont(m_Font1);
	return true;
}
void VoucherDialog::OnBnClickedVouchSub()
{

	CString voucher, status;
	GetDlgItemText(IDC_VOUCH_ID, voucher);
	GetDlgItemText(IDC_STATUS, status);
#ifdef _DEBUG
	_cprintf("%s\n",(CT2CA)voucher);
	_cprintf("%s\n",(CT2CA)status);
#endif
	std::string strVouch=(CT2CA)voucher;
	status = ((CmicrosipDlg*)GetParent())->SubmitVoucher(strVouch);
	if(status.GetLength()==0)
		SetDlgItemText(IDC_STATUS, L"Voucher record not found.");
	else{
		SetDlgItemText(IDC_STATUS, L"Success");
		Sleep(2000);
		EndDialog(IDOK);
	}
}
