#ifndef SETTINGS_H
#define SETTINGS_H
#pragma once

#include "global.h"
struct Account {
#ifdef _GLOBAL_PROFILE
	CString profile;
#endif
	CString server;
	CString proxy;
	CString username;
	CString domain;
	int Country;
	int port;
	CString authID;
	CString password;
	int rememberPassword;
#ifdef _GLOBAL_ACCOUNT_API
	CString apiLogin;
	CString apiPassword;
#endif
#ifdef _GLOBAL_ACCOUNT_API_ID
	CString apiId;
#endif
	CString displayName;
	CString srtp;
	CString transport;
	CString publicAddr;
	int publish;
	CString stun;
	int ice;
	int allowRewrite;
};

struct AccountSettings {

	int accountId;
	Account account;

	CString userAgent;
	int autoAnswer;
	CString denyIncoming;
	CString usersDirectory;
	int singleMode;
	int disableLocalAccount;
	int enableLog;
	int randomAnswerBox;
	CString ringingSound;
	CString audioInputDevice;
	CString audioOutputDevice;
	CString audioRingDevice;
	CString audioCodecs;
	int vad;
	int ec;
	int forceCodec;
	CString videoCaptureDevice;
	CString videoCodec;
	int disableH264;
	CString bitrateH264;	
	int disableH263;
	CString bitrateH263;
	int localDTMF;

	CString updatesInterval;

	int activeTab;
	int alwaysOnTop;

	int mainX;
	int mainY;
	int mainW;
	int mainH;

	int messagesX;
	int messagesY;
	int messagesW;
	int messagesH;

	int callsWidth0;
	int callsWidth1;
	int callsWidth2;
	int callsWidth3;
	int callsWidth4;

	int volumeOutput;
	int volumeInput;
	
	CString iniFile;
	CString logFile;
	CString pathRoaming;
	int checkUpdatesTime;

	int hidden;
	CString cmdCallStart;
	CString cmdCallEnd;
	CString cmdIncomingCall;
	CString cmdCallAnswer;

	AccountSettings();
	void Init();
	bool AccountLoad(int id, Account *account);
	void AccountSave(int id, Account *account);
	void AccountDelete(int id);
	void SettingsSave();
};
extern AccountSettings accountSettings;
extern bool firstRun;
extern bool pj_ready;
#endif