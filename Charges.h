<<<<<<< HEAD
#pragma once

#include "resource.h"

// Charges dialog

class Charges : public CDialog
{
	DECLARE_DYNAMIC(Charges)

public:
	Charges(CWnd* pParent = NULL);   // standard constructor
	virtual ~Charges();

// Dialog Data
	enum { IDD = IDD_CHARGES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSubmit();
	BOOL OnInitDialog();
};
=======
#pragma once

#include "resource.h"

// Charges dialog

class Charges : public CDialog
{
	DECLARE_DYNAMIC(Charges)

public:
	Charges(CWnd* pParent = NULL);   // standard constructor
	virtual ~Charges();

// Dialog Data
	enum { IDD = IDD_CHARGES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSubmit();
	BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeHrs();
};
>>>>>>> master
