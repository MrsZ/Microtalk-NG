#pragma once

#include "define.h"
#include <pjsua-lib/pjsua.h>
#include <pjsua-lib/pjsua_internal.h>
#include <string>
#include "LoginDialog.h"

#ifdef _GLOBAL_LIB_FILENAME
#pragma comment(lib, _GLOBAL_LIB_FILENAME)
#else

#ifdef NDEBUG
#ifdef _GLOBAL_VIDEO
#pragma comment(lib, "libpjproject-i386-Win32-vc8-Release.lib")
#else
#pragma comment(lib, "libpjproject-i386-Win32-vc8-Release-Static-NoVideo.lib")
#endif
#else
#ifdef _GLOBAL_VIDEO
#pragma comment(lib, "libpjproject-i386-Win32-vc8-Debug.lib")
#else
#pragma comment(lib, "libpjproject-i386-Win32-vc8-Debug-Static-NoVideo.lib")
#endif
#endif

#endif

#include "langpack.h"

#include "BaseDialog.h"
#include "RinginDlg.h"
#ifndef _GLOBAL_NO_ACCOUNT
#include "AccountDlg.h"
#endif
#ifndef _GLOBAL_NO_SETTINGS
#include "SettingsDlg.h"
#endif
#include "MessagesDlg.h"

#include "Dialer.h"
#ifndef _GLOBAL_NO_CONTACTS
#include "Contacts.h"
#endif
#include "Calls.h"
#include "Preview.h"
#include "Transfer.h"
#include "addons.h"

// CmicrosipDlg dialog
class CmicrosipDlg : public CBaseDialog
{
	// Construction
public:
	CmicrosipDlg(CWnd* pParent = NULL);	// standard constructor
	~CmicrosipDlg();
	void onBalance();


	// Dialog Data
	enum { IDD = IDD_MICROSIP };

	bool m_startMinimized;
#ifndef _GLOBAL_NO_SETTINGS
	SettingsDlg* settingsDlg;
#endif
MessagesDlg* messagesDlg;
	Transfer* transferDlg;

#ifdef _GLOBAL_ACCOUNT_REG
	Reg1* reg1Dlg;
	Reg2* reg2Dlg;
	Reg3* reg3Dlg;
#endif
#ifdef _GLOBAL_CONFERENCE_DIALOG
	ConferenceDlg* conferenceDlg;
	CString lastCallNumber;
#endif

	Dialer* pageDialer;
#ifndef _GLOBAL_NO_CONTACTS
	Contacts* pageContacts;
#endif
#ifndef _GLOBAL_NO_CALLS
	Calls* pageCalls;
#endif
#ifdef _GLOBAL_PAGE_BUTTONS
	Buttons* pageButtons;
#endif

	BOOL notStopRinging;
	CArray <RinginDlg*> ringinDlgs;
	CString dialNumberDelayed;
#ifdef _GLOBAL_BALANCE_IN_STATUSBAR
	CString balance;
#endif
	UINT callTimer;
	pjsua_acc_config acc_cfg;

	pjsua_transport_id transport_udp_local;
	pjsua_transport_id transport_udp;
	pjsua_transport_id transport_tcp;
	pjsua_transport_id transport_tls;
	pjsua_player_id player_id;
	int audio_input;
	int audio_output;
	int audio_ring;

	BOOL disableAutoRegister;
	CString callIdIncomingIgnore;
	CList<int,int> toneCalls;
	CList<int,int> attendedCalls;
	
	void PJCreate();
	void PJDestroy();
	void PJAccountAdd();
	void PJAccountAddLocal();
	void PJAccountDelete();
	void PJAccountDeleteLocal();
	void PJAccountConfig(pjsua_acc_config *acc_cfg);

	void UpdateWindowText(CString = CString(), int icon = IDI_DEFAULT, bool afterRegister = false);
	void PublishStatus(bool online = true, bool init=false);
	void BaloonPopup(CString title, CString message, DWORD flags = NIIF_WARNING);
	void GotoTab(int i, CTabCtrl* tab = NULL);
	void DialNumberFromCommandLine(CString number);
	void DialNumber(CString params);
	void MakeCall(CString number);
	void PlayerPlay(CString filename, BOOL noLoop = FALSE, BOOL inCall = FALSE);
	void SetSoundDevice(int outDev);
	BOOL CopyStringToClipboard( IN const CString & str );
	void OnTimerCall (bool manual = false);

#ifndef _GLOBAL_NO_CONTACTS
	void UsersDirectoryLoad();
	void OnTimerContactBlink();
	afx_msg LRESULT onUsersDirectoryLoaded(WPARAM wParam,LPARAM lParam);
#endif
	void SetupJumpList();
	void RemoveJumpList();
	void MainPopupMenu();
	void AccountSettingsPendingSave();
	void UpdateSoundDevicesIds();
	void PlayerStop();

#ifdef _GLOBAL_VIDEO
	Preview* previewWin;
	int VideoCaptureDeviceId(CString name=_T(""));
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Implementation
protected:
	HICON m_hIcon;
	HICON iconSmall;
	HICON iconInactive;
	NOTIFYICONDATA tnd;
	CStatusBar m_bar;

	unsigned char m_tabPrev;

	POINT m_mousePos;
	int m_idleCounter;
	BOOL m_isAway;
		
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();

	// Generated message map functions
	afx_msg LRESULT onTrayNotify(WPARAM, LPARAM);
	afx_msg LRESULT onCreateRingingDlg(WPARAM, LPARAM);
	afx_msg LRESULT onRegState2(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT onCallState(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT onCallMediaState(WPARAM, LPARAM);
	afx_msg LRESULT onCallTransferStatus(WPARAM, LPARAM);
#ifndef _GLOBAL_NO_MESSAGING
	afx_msg LRESULT onPager(WPARAM, LPARAM);
	afx_msg LRESULT onPagerStatus(WPARAM, LPARAM);
#endif
#ifndef _GLOBAL_NO_CONTACTS
	afx_msg LRESULT onBuddyState(WPARAM, LPARAM);
#endif
	afx_msg LRESULT onShellExecute(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT onDialNumber(WPARAM, LPARAM);
	afx_msg LRESULT CreationComplete(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT OnAccount(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT onPowerBroadcast(WPARAM, LPARAM);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnBnClickedOk();
	afx_msg void OnClose();
	afx_msg void OnContextMenu(CWnd *pWnd, CPoint point );
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);

	afx_msg void OnMove(int x, int y);
	afx_msg void OnSize(UINT type, int w, int h);
	afx_msg LRESULT onCallAnswer(WPARAM,LPARAM);
	afx_msg LRESULT onCallHangup(WPARAM,LPARAM);
	afx_msg LRESULT onSetPaneText(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT onPlayerPlay(WPARAM wParam,LPARAM lParam);
#ifndef _GLOBAL_NO_ACCOUNT
	afx_msg void OnMenuAccountAdd();
	afx_msg void OnMenuAccountChange(UINT nID);
	afx_msg void OnMenuAccountEdit(UINT nID);
#endif
	afx_msg void OnMenuSettings();
	afx_msg void OnMenuAlwaysOnTop();
#ifndef _GLOBAL_NO_LOG	
	afx_msg void OnMenuLog();
#endif	
	afx_msg void OnMenuExit();
	afx_msg void OnTimer (UINT TimerVal);
	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTcnSelchangingTab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenuWebsite();
#if !defined _GLOBAL_CUSTOM || defined _GLOBAL_UPDATES
	afx_msg void CheckUpdates();
#endif
#ifdef _GLOBAL_VIDEO
	afx_msg void createPreviewWin();
#endif
#ifdef _GLOBAL_BALANCE_PLAIN
	void BalancePlain();
	afx_msg LRESULT onBalancePlainLoaded(WPARAM wParam,LPARAM lParam);
#endif
#ifdef _GLOBAL_BALANCE_OPTIONS
	void BalanceOptions();
	afx_msg LRESULT onBalanceOptionsLoaded(WPARAM wParam,LPARAM lParam);
#endif
#ifdef _GLOBAL_BALANCE_IN_DIALER
	void BalanceDialer(CString balanceValue);
#endif
	void setToken(std::string);
	void setPin(std::string);
	std::string getToken();
	afx_msg void OnVoucher();
	CString SubmitVoucher(std::string);
	afx_msg void OnCredit();
	void OnSMS(CString, CString);
private:
	std::string token;
	std::string pin;
public:
	afx_msg void OnConf();
	afx_msg void OnCharges();
};

extern CmicrosipDlg *microsipDlg;
extern std::string countries[][4];