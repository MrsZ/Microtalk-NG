#pragma once


// VoucherDialog dialog

class VoucherDialog : public CDialog
{
	DECLARE_DYNAMIC(VoucherDialog)

public:
	VoucherDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~VoucherDialog();

// Dialog Data
	enum { IDD = IDD_VOUCHER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL VoucherDialog::OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedVouchSub();
};
