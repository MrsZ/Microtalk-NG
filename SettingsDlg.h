#pragma once

#include "resource.h"
#include "global.h"

#ifndef _GLOBAL_NO_SETTINGS

class SettingsDlg :
	public CDialog
{
public:
	SettingsDlg(CWnd* pParent = NULL);	// standard constructor
	~SettingsDlg();
	enum { IDD = IDD_SETTINGS };

protected:
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();

	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
#ifndef _GLOBAL_CODECS_HARDCODED
	afx_msg void OnDeltaposSpinModify(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinOrder(NMHDR *pNMHDR, LRESULT *pResult);
#endif
#ifdef _GLOBAL_HELP_WEBSITE
	afx_msg void OnNMClickSyslinkRingingSound(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkAutoAnswer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkDenyIncoming(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkDirectory(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkLocalDTMF(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkSingleMode(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkVAD(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkEC(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkForceCodec(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkDisableH264(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkDisableH263(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkAudioCodecs(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkEnableLog(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkRandomAnswerBox(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkDisableLocal(NMHDR *pNMHDR, LRESULT *pResult);
#endif
#ifdef _GLOBAL_VIDEO
	afx_msg void OnBnClickedPreview();
#endif
#ifdef _GLOBAL_CUSTOM_INI
	afx_msg void OnBnClickedBrowseIni();
	afx_msg void OnEnChangeCustomIni();
	afx_msg void OnBnClickedDefaultIni();
#endif
#ifndef _GLOBAL_NO_RINGING_SOUND
	afx_msg void OnBnClickedBrowse();
	afx_msg void OnEnChangeRingingSound();
	afx_msg void OnBnClickedDefault();
#endif
};

#endif