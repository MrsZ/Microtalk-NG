#include "StdAfx.h"
#include "Dialer.h"
#include "global.h"
#include "settings.h"
#include "microsipDlg.h"
#include "microsip.h"
#include "Strsafe.h"

Dialer::Dialer(CWnd* pParent /*=NULL*/)
: CBaseDialog(Dialer::IDD, pParent)
{
	Create (IDD, pParent);
}

Dialer::~Dialer(void)
{
}

void Dialer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
#ifndef _GLOBAL_NO_DIALPAD	
	DDX_Control(pDX, IDC_KEY_1, m_ButtonDialer1);
	DDX_Control(pDX, IDC_KEY_2, m_ButtonDialer2);
	DDX_Control(pDX, IDC_KEY_3, m_ButtonDialer3);
	DDX_Control(pDX, IDC_KEY_4, m_ButtonDialer4);
	DDX_Control(pDX, IDC_KEY_5, m_ButtonDialer5);
	DDX_Control(pDX, IDC_KEY_6, m_ButtonDialer6);
	DDX_Control(pDX, IDC_KEY_7, m_ButtonDialer7);
	DDX_Control(pDX, IDC_KEY_8, m_ButtonDialer8);
	DDX_Control(pDX, IDC_KEY_9, m_ButtonDialer9);
	DDX_Control(pDX, IDC_KEY_0, m_ButtonDialer0);
	DDX_Control(pDX, IDC_KEY_STAR, m_ButtonDialerStar);
    DDX_Control(pDX, IDC_KEY_GRATE, m_ButtonDialerGrate);
    DDX_Control(pDX, IDC_DELETE, m_ButtonDialerDelete);
    DDX_Control(pDX, IDC_KEY_PLUS, m_ButtonDialerPlus);
    DDX_Control(pDX, IDC_CLEAR, m_ButtonDialerClear);
#endif
}

BOOL Dialer::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	TranslateDialog(this->m_hWnd);

	DialedLoad();
	
	CComboBox *combobox= (CComboBox*)GetDlgItem(IDC_NUMBER);
	combobox->SetWindowPos(NULL,0,0,combobox->GetDroppedWidth(),400,SWP_NOZORDER|SWP_NOMOVE);

	CFont* font = this->GetFont();
	LOGFONT lf;
	font->GetLogFont(&lf);
	lf.lfHeight = 22;
	StringCchCopy(lf.lfFaceName,LF_FACESIZE,_T("Franklin Gothic Medium"));
	m_font.CreateFontIndirect(&lf);
	combobox->SetFont(&m_font);

#ifdef _GLOBAL_BALANCE_IN_DIALER
	font->GetLogFont(&lf);
	lf.lfHeight = 16;
	m_font_balance.CreateFontIndirect(&lf);
	GetDlgItem(IDC_BALANCE)->SetFont(&m_font_balance);
#endif


#ifdef _GLOBAL_LOGO_DIALER
	m_hIcon = (HICON)LoadImage(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(IDI_LOGO),
		IMAGE_ICON, 0, 0, LR_SHARED );
	CRect rect;
	GetWindowRect(&rect);
	ICONINFO info;
	GetIconInfo(m_hIcon,&info);
	GetObject(info.hbmColor,sizeof(bm),&bm);
	logoRect.left = (rect.Width()-bm.bmWidth)/2;
	CButton* but = (CButton*)GetDlgItem(IDC_CALL);
	but->GetWindowRect(&rect);
	ScreenToClient(rect);
#ifdef _GLOBAL_LOGO_TOP
	logoRect.top = _GLOBAL_LOGO_TOP;
#else
	logoRect.top = rect.bottom + 15;
#endif
#endif

#ifndef _GLOBAL_NO_DIALPAD	
	GetDlgItem(IDC_KEY_1)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_2)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_3)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_4)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_5)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_6)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_7)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_8)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_9)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_0)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_STAR)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_GRATE)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_PLUS)->SetFont(&m_font);
	GetDlgItem(IDC_CLEAR)->SetFont(&m_font);
	GetDlgItem(IDC_DELETE)->SetFont(&m_font);
#endif

	muteOutput = FALSE;
	muteInput = FALSE;

	CSliderCtrl *sliderCtrl;
	sliderCtrl = (CSliderCtrl *)GetDlgItem(IDC_VOLUME_OUTPUT);
	sliderCtrl->SetRange(0,200);
	sliderCtrl->SetPos(200-accountSettings.volumeOutput);
	sliderCtrl = (CSliderCtrl *)GetDlgItem(IDC_VOLUME_INPUT);
	sliderCtrl->SetRange(0,200);
	sliderCtrl->SetPos(200-accountSettings.volumeInput);

#ifdef _GLOBAL_NO_MUTE_SPEAKER
	GetDlgItem(IDC_BUTTON_MUTE_OUTPUT)->ShowWindow(SW_HIDE);
#endif
#ifdef _GLOBAL_NO_MUTE_MICROPHONE
	GetDlgItem(IDC_BUTTON_MUTE_INPUT)->ShowWindow(SW_HIDE);
#endif

	m_hIconMuteOutput = (HICON)LoadImage(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(IDI_MUTE_OUTPUT),
		IMAGE_ICON, 0, 0, LR_SHARED );
	((CButton*)GetDlgItem(IDC_BUTTON_MUTE_OUTPUT))->SetIcon(m_hIconMuteOutput);
	m_hIconMutedOutput = (HICON)LoadImage(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(IDI_MUTED_OUTPUT),
		IMAGE_ICON, 0, 0, LR_SHARED );
	
	m_hIconMuteInput = (HICON)LoadImage(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(IDI_MUTE_INPUT),
		IMAGE_ICON, 0, 0, LR_SHARED );
	((CButton*)GetDlgItem(IDC_BUTTON_MUTE_INPUT))->SetIcon(m_hIconMuteInput);
	m_hIconMutedInput = (HICON)LoadImage(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(IDI_MUTED_INPUT),
		IMAGE_ICON, 0, 0, LR_SHARED );
#ifndef _GLOBAL_NO_HOLD_TRANSFER
	m_hIconHold = (HICON)LoadImage(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(IDI_HOLD),
		IMAGE_ICON, 0, 0, LR_SHARED );
	((CButton*)GetDlgItem(IDC_HOLD))->SetIcon(m_hIconHold);
	m_hIconTransfer = (HICON)LoadImage(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(IDI_TRANSFER),
		IMAGE_ICON, 0, 0, LR_SHARED );
	((CButton*)GetDlgItem(IDC_TRANSFER))->SetIcon(m_hIconTransfer);
#endif
#ifdef _GLOBAL_VIDEO
	m_hIconVideo = (HICON)LoadImage(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(IDI_VIDEO),
		IMAGE_ICON, 0, 0, LR_SHARED );
	((CButton*)GetDlgItem(IDC_VIDEO_CALL))->SetIcon(m_hIconVideo);
#endif
#ifndef _GLOBAL_NO_MESSAGING
	m_hIconMessage = (HICON)LoadImage(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(IDI_MESSAGE),
		IMAGE_ICON, 0, 0, LR_SHARED );
	((CButton*)GetDlgItem(IDC_MESSAGE))->SetIcon(m_hIconMessage);
#endif
	
	return TRUE;
}

void Dialer::PostNcDestroy()
{
	CBaseDialog::PostNcDestroy();
	delete this;
}

BEGIN_MESSAGE_MAP(Dialer, CBaseDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDC_CALL, OnBnClickedCall)
#ifdef _GLOBAL_VIDEO
	ON_BN_CLICKED(IDC_VIDEO_CALL, OnBnClickedVideoCall)
#endif
#ifndef _GLOBAL_NO_MESSAGING
	ON_BN_CLICKED(IDC_MESSAGE, OnBnClickedMessage)
#endif
#ifndef _GLOBAL_NO_HOLD_TRANSFER
	ON_BN_CLICKED(IDC_HOLD, OnBnClickedHold)
	ON_BN_CLICKED(IDC_TRANSFER, OnBnClickedTransfer)
#endif
	ON_BN_CLICKED(IDC_END, OnBnClickedEnd)
#ifdef _GLOBAL_CALL_TO_SKYPE
	ON_BN_CLICKED(IDC_CALL_SKYPE, OnBnClickedCallSkype)
#endif
	ON_BN_CLICKED(IDC_BUTTON_MUTE_OUTPUT, &Dialer::OnBnClickedMuteOutput)
	ON_BN_CLICKED(IDC_BUTTON_MUTE_INPUT, &Dialer::OnBnClickedMuteInput)
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_CBN_EDITCHANGE(IDC_NUMBER, &Dialer::OnCbnEditchangeComboAddr)
	ON_CBN_SELCHANGE(IDC_NUMBER, &Dialer::OnCbnSelchangeComboAddr)
#ifndef _GLOBAL_NO_DIALPAD	
	ON_BN_CLICKED(IDC_KEY_1, &Dialer::OnBnClickedKey1)
	ON_BN_CLICKED(IDC_KEY_2, &Dialer::OnBnClickedKey2)
	ON_BN_CLICKED(IDC_KEY_3, &Dialer::OnBnClickedKey3)
	ON_BN_CLICKED(IDC_KEY_4, &Dialer::OnBnClickedKey4)
	ON_BN_CLICKED(IDC_KEY_5, &Dialer::OnBnClickedKey5)
	ON_BN_CLICKED(IDC_KEY_6, &Dialer::OnBnClickedKey6)
	ON_BN_CLICKED(IDC_KEY_7, &Dialer::OnBnClickedKey7)
	ON_BN_CLICKED(IDC_KEY_8, &Dialer::OnBnClickedKey8)
	ON_BN_CLICKED(IDC_KEY_9, &Dialer::OnBnClickedKey9)
	ON_BN_CLICKED(IDC_KEY_STAR, &Dialer::OnBnClickedKeyStar)
	ON_BN_CLICKED(IDC_KEY_0, &Dialer::OnBnClickedKey0)
	ON_BN_CLICKED(IDC_KEY_GRATE, &Dialer::OnBnClickedKeyGrate)
	ON_BN_CLICKED(IDC_KEY_PLUS, &Dialer::OnBnClickedKeyPlus)
	ON_BN_CLICKED(IDC_CLEAR, &Dialer::OnBnClickedClear)
	ON_BN_CLICKED(IDC_DELETE, &Dialer::OnBnClickedDelete)
#endif
	ON_WM_VSCROLL()
#ifdef _GLOBAL_LOGO_DIALER
	ON_WM_PAINT()
#endif
END_MESSAGE_MAP()

BOOL Dialer::PreTranslateMessage(MSG* pMsg)
{
	BOOL catched = FALSE;
	BOOL isEdit = FALSE;
	CEdit* edit = NULL;
	if (pMsg->message == WM_CHAR || (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)) {
		CComboBox *combobox= (CComboBox*)GetDlgItem(IDC_NUMBER);
		edit = (CEdit*)FindWindowEx(combobox->m_hWnd,NULL,_T("EDIT"),NULL);
		isEdit = !edit || edit == GetFocus();
	}
	if (pMsg->message == WM_CHAR)
	{
		if (pMsg->wParam == 48)
		{
			if (!isEdit) {
#ifndef _GLOBAL_NO_DIALPAD	
				GotoDlgCtrl(GetDlgItem(IDC_KEY_0));
#endif
				OnBnClickedKey0();
				catched = TRUE;
			} else {
				DTMF(_T("0"));
			}
		} else if (pMsg->wParam == 49)
		{
			if (!isEdit) {
#ifndef _GLOBAL_NO_DIALPAD	
				GotoDlgCtrl(GetDlgItem(IDC_KEY_1));
#endif
				OnBnClickedKey1();
				catched = TRUE;
			} else {
				DTMF(_T("1"));
			}
		} else if (pMsg->wParam == 50)
		{
			if (!isEdit) {
#ifndef _GLOBAL_NO_DIALPAD	
				GotoDlgCtrl(GetDlgItem(IDC_KEY_2));
#endif
				OnBnClickedKey2();
				catched = TRUE;
			} else {
				DTMF(_T("2"));
			}
		} else if (pMsg->wParam == 51)
		{
			if (!isEdit) {
#ifndef _GLOBAL_NO_DIALPAD	
				GotoDlgCtrl(GetDlgItem(IDC_KEY_3));
#endif
				OnBnClickedKey3();
				catched = TRUE;
			} else {
				DTMF(_T("3"));
			}
		} else if (pMsg->wParam == 52)
		{
			if (!isEdit) {
#ifndef _GLOBAL_NO_DIALPAD	
				GotoDlgCtrl(GetDlgItem(IDC_KEY_4));
#endif
				OnBnClickedKey4();
				catched = TRUE;
			} else {
				DTMF(_T("4"));
			}
		} else if (pMsg->wParam == 53)
		{
			if (!isEdit) {
#ifndef _GLOBAL_NO_DIALPAD	
				GotoDlgCtrl(GetDlgItem(IDC_KEY_5));
#endif
				OnBnClickedKey5();
				catched = TRUE;
			} else {
				DTMF(_T("5"));
			}
		} else if (pMsg->wParam == 54)
		{
			if (!isEdit) {
#ifndef _GLOBAL_NO_DIALPAD	
				GotoDlgCtrl(GetDlgItem(IDC_KEY_6));
#endif
				OnBnClickedKey6();
				catched = TRUE;
			} else {
				DTMF(_T("6"));
			}
		} else if (pMsg->wParam == 55)
		{
			if (!isEdit) {
#ifndef _GLOBAL_NO_DIALPAD	
				GotoDlgCtrl(GetDlgItem(IDC_KEY_7));
#endif
				OnBnClickedKey7();
				catched = TRUE;
			} else {
				DTMF(_T("7"));
			}
		} else if (pMsg->wParam == 56)
		{
			if (!isEdit) {
#ifndef _GLOBAL_NO_DIALPAD	
				GotoDlgCtrl(GetDlgItem(IDC_KEY_8));
#endif
				OnBnClickedKey8();
				catched = TRUE;
			} else {
				DTMF(_T("8"));
			}
		} else if (pMsg->wParam == 57)
		{
			if (!isEdit) {
#ifndef _GLOBAL_NO_DIALPAD	
				GotoDlgCtrl(GetDlgItem(IDC_KEY_9));
#endif
				OnBnClickedKey9();
				catched = TRUE;
			} else {
				DTMF(_T("9"));
			}
		} else if (pMsg->wParam == 35 || pMsg->wParam == 47 )
		{
			if (!isEdit) {
#ifndef _GLOBAL_NO_DIALPAD	
				GotoDlgCtrl(GetDlgItem(IDC_KEY_GRATE));
#endif
				OnBnClickedKeyGrate();
				catched = TRUE;
			} else {
				DTMF(_T("#"));
			}
		} else if (pMsg->wParam == 42 )
		{
			if (!isEdit) {
#ifndef _GLOBAL_NO_DIALPAD	
				GotoDlgCtrl(GetDlgItem(IDC_KEY_STAR));
#endif
				OnBnClickedKeyStar();
				catched = TRUE;
			} else {
				DTMF(_T("*"));
			}
		} else if (pMsg->wParam == 43 )
		{
			if (!isEdit) {
#ifndef _GLOBAL_NO_DIALPAD	
				GotoDlgCtrl(GetDlgItem(IDC_KEY_PLUS));
#endif
				OnBnClickedKeyPlus();
				catched = TRUE;
			}
		} else if (pMsg->wParam == 8 || pMsg->wParam == 45 )
		{
			if (!isEdit)
			{
				GotoDlgCtrl(GetDlgItem(IDC_DELETE));
				OnBnClickedDelete();
				catched = TRUE;
			}
		} else if (pMsg->wParam == 46 )
		{
			if (!isEdit)
			{
				Input(_T("."), TRUE);
				catched = TRUE;
			}
		}
	} else if (pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_ESCAPE) {
			if (!isEdit) {
				GotoDlgCtrl(GetDlgItem(IDC_NUMBER)); 
				catched = TRUE;
			}
			CString str;
			edit->GetWindowText(str);
			if (!str.IsEmpty()) {
				Clear();
				catched = TRUE;
			}
		}
	}
	if (!catched)
	{
		return CBaseDialog::PreTranslateMessage(pMsg);
	} else {
		return TRUE;
	}
}

void Dialer::OnBnClickedOk()
{
#ifndef _GLOBAL_TWO_BUTTONS
	if (accountSettings.singleMode && GetDlgItem(IDC_END)->IsWindowVisible()) {
#else
	if (accountSettings.singleMode && GetDlgItem(IDC_END)->IsWindowEnabled()) {
#endif
		OnBnClickedEnd();
	} else {
		OnBnClickedCall();
	}
}

void Dialer::OnBnClickedCancel()
{
	microsipDlg->ShowWindow(SW_HIDE);
}

void Dialer::Action(DialerActions action)
{
	CString number;
	CComboBox *combobox= (CComboBox*)GetDlgItem(IDC_NUMBER);
	combobox->GetWindowText(number);
	if (!number.IsEmpty()) {
		number.Trim();
#ifdef _GLOBAL_CONFERENCE_DIALOG
		microsipDlg->lastCallNumber = number;
#endif
		CString numberFormated = FormatNumber(number);
		pj_status_t pj_status = pjsua_verify_sip_url(StrToPj(numberFormated));
		if (pj_status==PJ_SUCCESS) {
			int pos = combobox->FindStringExact(-1,number);
			if (pos==CB_ERR || pos>0) {
				if (pos>0) {
					combobox->DeleteString(pos);
				} else if (combobox->GetCount()>=10)
				{
					combobox->DeleteString(combobox->GetCount()-1);
				}
				combobox->InsertString(0,number);
				combobox->SetCurSel(0);
			}
			DialedSave(combobox);
			if (!accountSettings.singleMode) {
				Clear();
			}
			microsipDlg->messagesDlg->AddTab(numberFormated, _T(""), TRUE, NULL, accountSettings.singleMode && action != ACTION_MESSAGE);
			if (action!=ACTION_MESSAGE) {
				microsipDlg->messagesDlg->Call(action==ACTION_VIDEO_CALL);
			}
		} else {
			ShowErrorMessage(pj_status);
		}
	}
}

void Dialer::OnBnClickedCall()
{
	Action(ACTION_CALL);
}

#ifdef _GLOBAL_VIDEO
void Dialer::OnBnClickedVideoCall()
{
	Action(ACTION_VIDEO_CALL);
}
#endif

#ifndef _GLOBAL_NO_MESSAGING
void Dialer::OnBnClickedMessage()
{
	Action(ACTION_MESSAGE);
}
#endif

void Dialer::OnBnClickedHold()
{
	microsipDlg->messagesDlg->OnBnClickedHold();
}

void Dialer::OnBnClickedTransfer()
{
	if (!microsipDlg->transferDlg) {
		microsipDlg->transferDlg = new Transfer(this);
	}
	microsipDlg->transferDlg->SetForegroundWindow();
}

void Dialer::OnBnClickedEnd()
{
	MessagesContact*  messagesContact = microsipDlg->messagesDlg->GetMessageContact();
	if (messagesContact && messagesContact->callId != -1 ) {
		call_hangup_fast(messagesContact->callId);
	} else {
		call_hangup_all_noincoming();
	}
}

void Dialer::DTMF(CString digits, BOOL noLocalDTMF)
{
	BOOL simulate = TRUE;
	MessagesContact*  messagesContact = microsipDlg->messagesDlg->GetMessageContact();
	if (messagesContact && messagesContact->callId != -1 )
	{
		pjsua_call_info call_info;
		pjsua_call_get_info(messagesContact->callId, &call_info);
		if (call_info.media_status == PJSUA_CALL_MEDIA_ACTIVE)
		{
			pj_str_t pj_digits = StrToPjStr ( digits );
			if (pjsua_call_dial_dtmf(messagesContact->callId, &pj_digits) != PJ_SUCCESS) {
				simulate = !call_play_digit(messagesContact->callId, StrToPj(digits));
			}
		}
	}
	if (simulate && accountSettings.localDTMF && !noLocalDTMF) {
		microsipDlg->SetSoundDevice(microsipDlg->audio_output);
		call_play_digit(-1, StrToPj(digits));
	}
}

void Dialer::Input(CString digits, BOOL disableDTMF)
{
	if (!disableDTMF) {
		DTMF(digits);
	}
	CComboBox *combobox= (CComboBox*)GetDlgItem(IDC_NUMBER);
	CEdit* edit = (CEdit*)FindWindowEx(combobox->m_hWnd,NULL,_T("EDIT"),NULL);
	if (edit) {
		int nLength = edit->GetWindowTextLength();
		edit->SetSel(nLength,nLength);
		edit->ReplaceSel(digits);
	}
}

void Dialer::DialedClear()
{
	CComboBox *combobox= (CComboBox*)GetDlgItem(IDC_NUMBER);
	combobox->ResetContent();
	combobox->Clear();
}
void Dialer::DialedLoad()
{
	CComboBox *combobox= (CComboBox*)GetDlgItem(IDC_NUMBER);
	CString key;
	CString val;
	LPTSTR ptr = val.GetBuffer(255);
	int i=0;
	while (TRUE) {
		key.Format(_T("%d"),i);
		if (GetPrivateProfileString(_T("Dialed"), key, NULL, ptr, 256, accountSettings.iniFile)) {
			combobox->AddString(ptr);
		} else {
			break;
		}
		i++;
	}
}

void Dialer::DialedSave(CComboBox *combobox)
{
	CString key;
	CString val;
	WritePrivateProfileString(_T("Dialed"), NULL, NULL, accountSettings.iniFile);
	for (int i=0;i < combobox->GetCount();i++)
	{
		int n = combobox->GetLBTextLen( i );
		combobox->GetLBText( i, val.GetBuffer(n) );
		val.ReleaseBuffer();

		key.Format(_T("%d"),i);
		WritePrivateProfileString(_T("Dialed"), key, val, accountSettings.iniFile);
	}
}


void Dialer::OnBnClickedKey1()
{
	Input(_T("1"));
}

void Dialer::OnBnClickedKey2()
{
	Input(_T("2"));
}

void Dialer::OnBnClickedKey3()
{
	Input(_T("3"));
}

void Dialer::OnBnClickedKey4()
{
	Input(_T("4"));
}

void Dialer::OnBnClickedKey5()
{
	Input(_T("5"));
}

void Dialer::OnBnClickedKey6()
{
	Input(_T("6"));
}

void Dialer::OnBnClickedKey7()
{
	Input(_T("7"));
}

void Dialer::OnBnClickedKey8()
{
	Input(_T("8"));
}

void Dialer::OnBnClickedKey9()
{
	Input(_T("9"));
}

void Dialer::OnBnClickedKeyStar()
{
	Input(_T("*"));
}

void Dialer::OnBnClickedKey0()
{
	Input(_T("0"));
}

void Dialer::OnBnClickedKeyGrate()
{
	Input(_T("#"));
}

void Dialer::OnBnClickedKeyPlus()
{
	Input(_T("+"), TRUE);
}

void Dialer::OnBnClickedClear()
{
	Clear();
}

void Dialer::Clear(bool update)
{
	CComboBox *combobox= (CComboBox*)GetDlgItem(IDC_NUMBER);
	combobox->SetCurSel(-1);
	if (update) {
		UpdateCallButton();
	}
}

void Dialer::OnBnClickedDelete()
{
	CComboBox *combobox= (CComboBox*)GetDlgItem(IDC_NUMBER);
	CEdit* edit = (CEdit*)FindWindowEx(combobox->m_hWnd,NULL,_T("EDIT"),NULL);
	if (edit) {
		int nLength = edit->GetWindowTextLength();
		edit->SetSel(nLength-1,nLength);
		edit->ReplaceSel(_T(""));
	}
}

void Dialer::UpdateCallButton(BOOL forse, int callsCount)
{
#ifdef _GLOBAL_TWO_BUTTONS
	bool buttonDisabled = false;
#endif
	int len;
	if (!forse)	{
		CComboBox *combobox= (CComboBox*)GetDlgItem(IDC_NUMBER);
		len = combobox->GetWindowTextLength();
	} else {
		len = 1;
	}
	CButton *button = (CButton *)GetDlgItem(IDC_CALL);
	bool state = false;
	if (accountSettings.singleMode)	{
		if (callsCount == -1) {
			callsCount = call_get_count_noincoming();
		}
#ifndef _GLOBAL_TWO_BUTTONS
		if (callsCount) {
			if (!GetDlgItem(IDC_END)->IsWindowVisible()) {
				button->ShowWindow(SW_HIDE);
#ifdef _GLOBAL_VIDEO
				GetDlgItem(IDC_VIDEO_CALL)->ShowWindow(SW_HIDE);
#endif
#ifndef _GLOBAL_NO_MESSAGING
				GetDlgItem(IDC_MESSAGE)->ShowWindow(SW_HIDE);
#endif
#ifndef	_GLOBAL_NO_HOLD_TRANSFER
				GetDlgItem(IDC_HOLD)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_TRANSFER)->ShowWindow(SW_SHOW);
#endif
				GetDlgItem(IDC_END)->ShowWindow(SW_SHOW);
				GotoDlgCtrl(GetDlgItem(IDC_END));
			}
		} else {
			if (GetDlgItem(IDC_END)->IsWindowVisible()) {
#ifndef	_GLOBAL_NO_HOLD_TRANSFER
				GetDlgItem(IDC_HOLD)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_TRANSFER)->ShowWindow(SW_HIDE);
#endif
				GetDlgItem(IDC_END)->ShowWindow(SW_HIDE);
				button->ShowWindow(SW_SHOW);
#ifdef _GLOBAL_VIDEO
				GetDlgItem(IDC_VIDEO_CALL)->ShowWindow(SW_SHOW);
#endif
#ifndef _GLOBAL_NO_MESSAGING
				GetDlgItem(IDC_MESSAGE)->ShowWindow(SW_SHOW);
#endif
			}
		}
#else
		if (callsCount) {
			buttonDisabled = true;
			if (!GetDlgItem(IDC_END)->IsWindowEnabled()) {
				button->EnableWindow(FALSE);
				GetDlgItem(IDC_END)->EnableWindow(TRUE);
				GotoDlgCtrl(GetDlgItem(IDC_END));
			}
		} else {
			if (GetDlgItem(IDC_END)->IsWindowEnabled()) {
				GetDlgItem(IDC_END)->EnableWindow(FALSE);
				button->EnableWindow(TRUE);
			}
		}
#endif
		state = callsCount||len?true:false;
#ifdef _GLOBAL_PAGE_BUTTONS
		microsipDlg->pageButtons->GetDlgItem(IDC_CALL)->SetWindowText(callsCount?Translate(_T("End call")):callCaption);
		//		((CButton*)microsipDlg->pageButtons->GetDlgItem(IDC_CALL))->EnableWindow(callsCount||len?TRUE:FALSE);
#endif

	} else {
		state = len?true:false;
#ifdef _GLOBAL_PAGE_BUTTONS
		//		((CButton*)microsipDlg->pageButtons->GetDlgItem(IDC_CALL))->EnableWindow(len?TRUE:FALSE);
#endif
	}
	if (state==false && !GetFocus()) {
		GotoDlgCtrl(GetDlgItem(IDC_NUMBER));
	}
#ifndef _GLOBAL_TWO_BUTTONS
	button->EnableWindow(state);
#else
	button->EnableWindow(!buttonDisabled && state);
#endif

#ifdef _GLOBAL_VIDEO
				GetDlgItem(IDC_VIDEO_CALL)->EnableWindow(state);
#endif
#ifndef _GLOBAL_NO_MESSAGING
				GetDlgItem(IDC_MESSAGE)->EnableWindow(state);
#endif
}

void Dialer::SetNumber(CString  number, int callsCount)
{
	CComboBox *combobox= (CComboBox*)GetDlgItem(IDC_NUMBER);
	CString old;
	combobox->GetWindowText(old);
	if (old.IsEmpty() || number.Find(old)!=0) {
		combobox->SetWindowText(number);
	}
	UpdateCallButton(0, callsCount);
}

void Dialer::OnCbnEditchangeComboAddr()
{
	UpdateCallButton();
}

void Dialer::OnCbnSelchangeComboAddr()
{	
	UpdateCallButton(TRUE);
}

#ifdef _GLOBAL_LOGO_DIALER
void Dialer::OnPaint() 
{
	if (!IsIconic())
	{
		CPaintDC dc(this); // device context for painting
		DrawIconEx(dc, logoRect.left, logoRect.top, m_hIcon, 0, 0, NULL, NULL, DI_MASK | DI_IMAGE);
	}
	else
	{
		CBaseDialog::OnPaint();
	}
}
#endif

void Dialer::OnLButtonUp( UINT nFlags, CPoint pt ) 
{
#ifndef _GLOBAL_LOGO_WEBSITE
	OnRButtonUp( nFlags, pt );
#else
	OnRButtonUp( 1, pt );
#endif
}

void Dialer::OnRButtonUp( UINT nFlags, CPoint pt )
{
#if defined _GLOBAL_LOGO_DIALER && defined _GLOBAL_MENU_WEBSITE
	if (pt.y>=logoRect.top && pt.y<=logoRect.top+bm.bmHeight
		&& pt.x>=logoRect.left && pt.x<=logoRect.left+bm.bmWidth
		) {
			if (!nFlags) {
				microsipDlg->MainPopupMenu();
			} else {
#ifdef  _GLOBAL_MENU_WEBSITE_LOGO_EQ_PROFILE
				if (accountSettings.accountId) {
					CString url;
					url.Format(_T("http://%s"),accountSettings.account.profile);
					OpenURL(url);
				} else {
					microsipDlg->OnMenuWebsite();
				}
#else
#ifndef _GLOBAL_MENU_WEBSITE_LOGO
				microsipDlg->OnMenuWebsite();
#else
				CString url;
				url.Format(_T(_GLOBAL_MENU_WEBSITE_LOGO),accountSettings.account.username);
				OpenURL(url);
#endif
#endif
			}
	}
#endif
}

void Dialer::OnMouseMove(UINT nFlags, CPoint pt )
{
#if defined _GLOBAL_LOGO_DIALER && defined _GLOBAL_MENU_WEBSITE && defined _GLOBAL_LOGO_WEBSITE
	if (pt.y>=logoRect.top && pt.y<=logoRect.top+bm.bmHeight
		&& pt.x>=logoRect.left && pt.x<=logoRect.left+bm.bmWidth
		) {
			::SetCursor(AfxGetApp()->LoadStandardCursor(MAKEINTRESOURCE(32649))); 
	}
#endif
}

void Dialer::OnVScroll( UINT, UINT, CScrollBar* sender)
{
	if (pj_ready) {
		CSliderCtrl *sliderCtrl;
		int pos;
		int val;
		sliderCtrl = (CSliderCtrl *)GetDlgItem(IDC_VOLUME_OUTPUT);
		if (!sender || sender == (CScrollBar*)sliderCtrl)  {
			if (sender && muteOutput) {
				CButton *button = (CButton*)GetDlgItem(IDC_BUTTON_MUTE_OUTPUT);
				button->SetCheck(BST_UNCHECKED);
				OnBnClickedMuteOutput();
				return;
			}
			pos = muteOutput?0:200-sliderCtrl->GetPos();
			if (pos>100 && pos<110 || pos<100 && pos>90) {
				pos = 100;
				sliderCtrl->SetPos(pos);
			}
			val = pos<=100 ? pos : 100;
			pj_status_t status = 
				pjsua_snd_set_setting(
				PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING,
				&val, PJ_TRUE);
			if (status != PJ_SUCCESS) {
				pjsua_conf_adjust_tx_level(0, (float)val/100);
			}
			if (!muteOutput) {
				val = pos>100 ? 100+(pos-100)*3 : 100;
				pjsua_call_id call_ids[PJSUA_MAX_CALLS];
				unsigned calls_count = PJSUA_MAX_CALLS;
				if (pjsua_enum_calls ( call_ids, &calls_count)==PJ_SUCCESS)  {
					for (unsigned i = 0; i < calls_count; ++i) {
						pjsua_call_info call_info;
						pjsua_call_get_info(call_ids[i], &call_info);
						if (call_info.media_status == PJSUA_CALL_MEDIA_ACTIVE
							|| call_info.media_status == PJSUA_CALL_MEDIA_REMOTE_HOLD
							) {
								pjsua_conf_adjust_rx_level(call_info.conf_slot, (float)val/100);
						}
					}
				}
				accountSettings.volumeOutput = pos;
				microsipDlg->AccountSettingsPendingSave();
			}
		}
		sliderCtrl = (CSliderCtrl *)GetDlgItem(IDC_VOLUME_INPUT);
		if (!sender || sender == (CScrollBar*)sliderCtrl)  {
			if (sender && muteInput) {
				CButton *button = (CButton*)GetDlgItem(IDC_BUTTON_MUTE_INPUT);
				button->SetCheck(BST_UNCHECKED);
				OnBnClickedMuteInput();
				return;
			}
			pos = muteInput?0:200-sliderCtrl->GetPos();
			if (pos>100 && pos<110 || pos<100 && pos>90) {
				pos = 100;
				sliderCtrl->SetPos(pos);
			}
			pjsua_conf_adjust_rx_level(0, (pos>100?(100+pow((float)pos-100,1.4f)):(float)pos)/100);
			if (!muteInput) {
				accountSettings.volumeInput = pos;
				microsipDlg->AccountSettingsPendingSave();
			}
		}
	}
}

void Dialer::OnBnClickedMuteOutput()
{
	CButton *button = (CButton*)GetDlgItem(IDC_BUTTON_MUTE_OUTPUT);
	if (button->GetCheck() == BST_CHECKED) {
		button->SetIcon(m_hIconMutedOutput);
		muteOutput = TRUE;
		OnVScroll( 0, 0, NULL);
	} else {
		button->SetIcon(m_hIconMuteOutput);
		muteOutput = FALSE;
		OnVScroll( 0, 0, NULL);
	}
}

void Dialer::OnBnClickedMuteInput()
{
	CButton *button = (CButton*)GetDlgItem(IDC_BUTTON_MUTE_INPUT);
	if (button->GetCheck() == BST_CHECKED) {
		button->SetIcon(m_hIconMutedInput);
		muteInput = TRUE;
		OnVScroll( 0, 0, NULL);
	} else {
		button->SetIcon(m_hIconMuteInput);
		muteInput = FALSE;
		OnVScroll( 0, 0, NULL);
	}
}

#ifdef _GLOBAL_CALL_TO_SKYPE
#include _GLOBAL_CALL_TO_SKYPE
void Dialer::OnBnClickedCallSkype()
{
	CallToSkype();
}
#endif
