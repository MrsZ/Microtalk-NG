// LoginForm.cpp : implementation file
//

#include "stdafx.h"
#include "LoginForm.h"
#include "microsip.h"



LoginForm::LoginForm(CWnd* pParent /*=NULL*/)
	: CBaseDialog(LoginForm::IDD, pParent)
{

}

LoginForm::~LoginForm()
{
}

void LoginForm::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(LoginForm, CBaseDialog)
END_MESSAGE_MAP()

BOOL LoginForm::OnInitDialog()
{
	CBaseDialog::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}