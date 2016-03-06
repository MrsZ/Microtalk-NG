#pragma once

#include "resource.h"
// Conference dialog

class Conference : public CDialog
{
	DECLARE_DYNAMIC(Conference)

public:
	Conference(CWnd* pParent = NULL);   // standard constructor
	virtual ~Conference();

// Dialog Data
	enum { IDD = IDD_CONF };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	//afx_msg void OnLvnDeleteitemConflist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedConfsubmit();
	void LoadList();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnJoin();
	afx_msg void OnDelete();
	afx_msg void OnCopy();
};
