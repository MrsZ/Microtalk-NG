/*
accountSettings.account:
local_port public_addr stun
*/

#include "stdafx.h"
#include "microsip.h"
#include "microsipDlg.h"
#include "LoginDialog.h"
#include "Mmsystem.h"
#include "settings.h"
#include "global.h"
#include "ModelessMessageBox.h"
#include "utf.h"
#include "jumplist.h"
#include "atlenc.h"


#include <io.h>
#include <afxinet.h>
#include <ws2tcpip.h>
#include <Dbt.h>

#include <Strsafe.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CmicrosipDlg *microsipDlg;

static UINT BASED_CODE indicators[] =
{
	IDS_STATUSBAR
};

static bool timerContactBlinkState = false;

static CString gethostbyaddrThreadResult;
static DWORD WINAPI gethostbyaddrThread( LPVOID lpParam ) 
{
	CString *addr = (CString *)lpParam;
	struct hostent *he = NULL;
	struct in_addr inaddr;
	inaddr.S_un.S_addr = inet_addr(CStringA(*addr));
	if (inaddr.S_un.S_addr != INADDR_NONE && inaddr.S_un.S_addr != INADDR_ANY) {
		he = gethostbyaddr((char *) &inaddr, 4, AF_INET);
		if (he) {
			gethostbyaddrThreadResult = he->h_name;
		}
	}
	delete addr;
	return 0;
}

static void on_reg_state2(pjsua_acc_id acc_id,  pjsua_reg_info *info)
{
	if (!IsWindow(microsipDlg->m_hWnd))	{
		return;
	}
	CString *str = NULL;
#ifdef _GLOBAL_BALANCE_REGISTER
	str = new CString();
	BalanceRegister(info, str);
#endif
	PostMessage(microsipDlg->m_hWnd, MYWM_ON_REG_STATE2, (WPARAM) info->cbparam->code, (LPARAM) str);
}

LRESULT CmicrosipDlg::onRegState2(WPARAM wParam,LPARAM lParam)
{
	int code = wParam;
#ifdef _GLOBAL_BALANCE_REGISTER
	CString *str = (CString *) lParam;
	if (*str != _T("-") ) {
#ifdef _GLOBAL_BALANCE_IN_DIALER
		BalanceDialer(*str);
#endif
#ifdef _GLOBAL_BALANCE_IN_STATUSBAR
		balance = *str;
#endif
	}
	delete str;
#endif

	if (code == 200) {
#if defined _GLOBAL_BALANCE_PLAIN || defined _GLOBAL_BALANCE_OPTIONS
		OnTimer(IDT_TIMER_BALANCE);
#endif
#ifndef _GLOBAL_NO_CONTACTS
		if (accountSettings.usersDirectory.Find(_T("%s"))!=-1) {
			UsersDirectoryLoad();
		}
#endif
	}

	UpdateWindowText(_T(""), IDI_DEFAULT, true);
	
	return 0;
}


static void on_call_state(pjsua_call_id call_id, pjsip_event *e)
{
	if (!IsWindow(microsipDlg->m_hWnd))	{
		return;
	}
	pjsua_call_info *call_info = new pjsua_call_info();
	if (pjsua_call_get_info(call_id, call_info) != PJ_SUCCESS || call_info->state == PJSIP_INV_STATE_NULL) {
		return;
	}

	CString *str = new CString();
	CString adder;

#ifndef _GLOBAL_ACCOUNT_MINI
	if (call_info->state!=PJSIP_INV_STATE_DISCONNECTED && call_info->state!=PJSIP_INV_STATE_CONNECTING && call_info->remote_contact.slen>0) {
		SIPURI contactURI;
		ParseSIPURI(PjToStr(&call_info->remote_contact,TRUE),&contactURI);
		CString contactDomain = RemovePort(contactURI.domain);
		struct hostent *he = NULL;
		if (IsIP(contactDomain)) {
			HANDLE hThread;
			CString *addr = new CString;
			*addr = contactDomain;
			hThread = CreateThread(NULL,0, gethostbyaddrThread, addr, 0, NULL);
			if (WaitForSingleObject(hThread, 500)==0) {
				contactDomain = gethostbyaddrThreadResult;
			}
		}
		adder.AppendFormat(_T("%s, "),contactDomain);
	}
#endif

	switch (call_info->state) {
	case PJSIP_INV_STATE_CALLING:
		str->Format(_T("%s..."),Translate(_T("Calling")));
		break;
	case PJSIP_INV_STATE_INCOMING:
		str->SetString(Translate(_T("Incoming call")));
		break;
	case PJSIP_INV_STATE_EARLY:
		str->SetString(Translate( PjToStr(&call_info->last_status_text).GetBuffer() ));
		break;
	case PJSIP_INV_STATE_CONNECTING:
		str->Format(_T("%s..."),Translate(_T("Connecting")));
		break;
	case PJSIP_INV_STATE_CONFIRMED:
		str->SetString(Translate(_T("Connected")));
#ifndef _GLOBAL_ACCOUNT_MINI
		for (unsigned i=0;i<call_info->media_cnt;i++) {
			if (call_info->media[i].type == PJMEDIA_TYPE_AUDIO || call_info->media[i].type == PJMEDIA_TYPE_VIDEO) {
				pjsua_stream_info psi;
				if (pjsua_call_get_stream_info(call_info->id, call_info->media[i].index, &psi) == PJ_SUCCESS) {
					if (call_info->media[i].type == PJMEDIA_TYPE_AUDIO) {
						adder.AppendFormat(_T("%s@%dkHz %dkbit/s, "),PjToStr (&psi.info.aud.fmt.encoding_name),psi.info.aud.fmt.clock_rate/1000, psi.info.aud.param->info.avg_bps/1000);
						if (psi.info.aud.proto==PJMEDIA_TP_PROTO_RTP_SAVP) {
							adder.Append(_T("SRTP, "));
						}
					} else {
						adder.AppendFormat(_T("%s %dkbit/s, "),
							PjToStr (&psi.info.vid.codec_info.encoding_name),
							psi.info.vid.codec_param->enc_fmt.det.vid.max_bps/1000
							);
					}
				}
			}
		}
#endif
		break;

	case PJSIP_INV_STATE_DISCONNECTED:
		call_deinit_tonegen(call_info->id);
		if (call_info->last_status == 200) {
			str->SetString(Translate(_T("Call ended")));
		} else {
			str->SetString(PjToStr(&call_info->last_status_text).GetBuffer());
			if (*str == _T("Decline")) {
				str->SetString(_T("Declined"));
			} else if (str->Find(_T("(PJ_ERESOLVE)"))!=-1) {
				str->SetString(_T("Cannot get IP address of the called host."));
			}
			str->SetString(Translate(str->GetBuffer()));
		}
		break;
	}

	if (!str->IsEmpty() && !adder.IsEmpty()) {
		str->AppendFormat(_T(" (%s)"), adder.Left(adder.GetLength()-2));
	}

	PostMessage(microsipDlg->m_hWnd, MYWM_ON_CALL_STATE, (WPARAM) call_info, (LPARAM) str);
}

LRESULT CmicrosipDlg::onCallState(WPARAM wParam,LPARAM lParam)
{
	pjsua_call_info *call_info = (pjsua_call_info *) wParam;
	CString *str = (CString *) lParam;

	CString number = PjToStr(&call_info->remote_info, TRUE);
	SIPURI sipuri;
	ParseSIPURI(number,&sipuri);

	if (call_info->state == PJSIP_INV_STATE_CONFIRMED) {
		if (! callTimer) {
			OnTimerCall(true);
			callTimer = SetTimer(IDT_TIMER_CALL,1000,NULL);
		}
		if (!accountSettings.cmdCallStart.IsEmpty() ) {
			ShellExecute(NULL, NULL, accountSettings.cmdCallStart, sipuri.user, NULL, SW_HIDE);
		}
	}
	if (call_info->state == PJSIP_INV_STATE_DISCONNECTED) {
		destroyDTMFPlayer(NULL,NULL,NULL,NULL);
		if (call_info->last_status == 200) {
			if (accountSettings.localDTMF) {
				onPlayerPlay(MSIP_SOUND_HANGUP, 0);
			}
		} else {
			if (accountSettings.singleMode && call_info->last_status != 487 &&  (call_info->last_status != 603 || call_info->role == PJSIP_ROLE_UAC)) {
				BaloonPopup(*str, *str, NIIF_INFO);
			}
		}
		if (!accountSettings.cmdCallEnd.IsEmpty() ) {
			ShellExecute(NULL, NULL, accountSettings.cmdCallEnd, sipuri.user, NULL, SW_HIDE);
		}
	}

	if (!accountSettings.singleMode) {
		if (call_info->state!=PJSIP_INV_STATE_CONFIRMED) {
			if (call_info->state!=PJSIP_INV_STATE_DISCONNECTED) {
				UpdateWindowText(*str, IDI_ONLINE);
			} else {
				UpdateWindowText(_T("-"));
			}
		}
	}

	if (call_info->role==PJSIP_ROLE_UAC && accountSettings.localDTMF) {
		if (call_info->last_status == 180 && !call_info->media_cnt) {
			if (toneCalls.IsEmpty()) {
					OnTimer(IDT_TIMER_TONE);
					SetTimer(IDT_TIMER_TONE,4500,NULL);
					toneCalls.AddTail(call_info->id);
			} else if (toneCalls.Find(call_info->id)==NULL) {
				toneCalls.AddTail(call_info->id);
			}
		} else {
			POSITION position = toneCalls.Find(call_info->id);
			if (position!=NULL) {
				toneCalls.RemoveAt(position);
				if (toneCalls.IsEmpty()) {
					KillTimer(IDT_TIMER_TONE);
					PlayerStop();
				}
			}
		}
	}
	
	bool doNotShowMessagesWindow =
		call_info->state == PJSIP_INV_STATE_INCOMING ||
		call_info->state == PJSIP_INV_STATE_EARLY ||
		call_info->state == PJSIP_INV_STATE_DISCONNECTED;
	MessagesContact* messagesContact = messagesDlg->AddTab(number, _T(""),
		(!accountSettings.singleMode &&
				(call_info->state == PJSIP_INV_STATE_CONFIRMED
				|| call_info->state == PJSIP_INV_STATE_CONNECTING)
		)
		||
		(accountSettings.singleMode
		&&
		(
		(call_info->role==PJSIP_ROLE_UAC && call_info->state != PJSIP_INV_STATE_DISCONNECTED)
		||
				(call_info->role==PJSIP_ROLE_UAS &&
				(call_info->state == PJSIP_INV_STATE_CONFIRMED
				|| call_info->state == PJSIP_INV_STATE_CONNECTING)
				)
		))
		?TRUE:FALSE,
		call_info, accountSettings.singleMode || doNotShowMessagesWindow, call_info->state == PJSIP_INV_STATE_DISCONNECTED
	);

	if (messagesContact) {
		if ( call_info->state == PJSIP_INV_STATE_DISCONNECTED) {
			messagesContact->mediaStatus = PJSUA_CALL_MEDIA_ERROR;
		}
#ifndef _GLOBAL_NO_CALLS
		if (call_info->role==PJSIP_ROLE_UAS) {
			if ( call_info->state == PJSIP_INV_STATE_CONFIRMED) {
				pageCalls->Add(call_info->call_id, messagesContact->number, messagesContact->name, MSIP_CALL_IN);
			} else if ( call_info->state == PJSIP_INV_STATE_INCOMING || call_info->state == PJSIP_INV_STATE_EARLY) {
				pageCalls->Add(call_info->call_id, messagesContact->number, messagesContact->name , MSIP_CALL_MISS);
			}
		} else {
			if ( call_info->state == PJSIP_INV_STATE_CALLING) {
				pageCalls->Add(call_info->call_id, messagesContact->number, messagesContact->name, MSIP_CALL_OUT);
			}
		}
#endif
	}
#ifndef _GLOBAL_NO_CALLS
	if ( call_info->state == PJSIP_INV_STATE_DISCONNECTED) {
		pageCalls->SetDuration(call_info->call_id, call_info->connect_duration.sec);
		if (call_info->last_status != 200) {
			pageCalls->SetInfo(call_info->call_id, *str);
		}
	}
#endif

	if (call_info->role==PJSIP_ROLE_UAS && call_info->state == PJSIP_INV_STATE_CONFIRMED)	{
		if (!accountSettings.cmdCallAnswer.IsEmpty() ) {
			ShellExecute(NULL, NULL, accountSettings.cmdCallAnswer, sipuri.user, NULL, SW_HIDE);
		}
	}

	if (accountSettings.singleMode)	{
		if (call_info->state == PJSIP_INV_STATE_DISCONNECTED) {
			MessagesContact *messagesContactSelected = messagesDlg->GetMessageContact();
			if (!messagesContactSelected || messagesContactSelected->callId==call_info->id || messagesContactSelected->callId==-1) {
				pageDialer->Clear(false);
				pageDialer->UpdateCallButton(FALSE, 0);
			}
			UpdateWindowText(_T("-"));
		} else {
			if (call_info->state!=PJSIP_INV_STATE_CONFIRMED) {
				UpdateWindowText(*str, IDI_ONLINE);
			}
#ifndef _GLOBAL_PAGE_BUTTONS
			int tabN = 0;
#else
			int tabN = call_info->state == PJSIP_INV_STATE_CONFIRMED ? 2 : 0;
#endif
			GotoTab(tabN);
			messagesDlg->OnChangeTab(call_info);
		}
	} else {
		if (call_info->state == PJSIP_INV_STATE_DISCONNECTED) {
			pageDialer->Clear();
		}
	}

	if (messagesContact && !str->IsEmpty())	{
		messagesDlg->AddMessage(messagesContact, *str, MSIP_MESSAGE_TYPE_SYSTEM,
			call_info->state == PJSIP_INV_STATE_INCOMING || call_info->state == PJSIP_INV_STATE_EARLY
			);
	}

	if (call_info->state == PJSIP_INV_STATE_EARLY ||
		call_info->state == PJSIP_INV_STATE_CONNECTING ||
		call_info->state == PJSIP_INV_STATE_CONFIRMED) {
		if (attendedCalls.Find(call_info->id)==NULL) {
			attendedCalls.AddTail(call_info->id);
		}
	} else {
		POSITION position = attendedCalls.Find(call_info->id);
		if (position!=NULL) {
			attendedCalls.RemoveAt(position);
		}
	}
	messagesDlg->GetDlgItem(IDC_ATTENDED_TRANSFER)->EnableWindow(attendedCalls.GetCount()>1);

	if (call_info->state == PJSIP_INV_STATE_DISCONNECTED) {
		messagesDlg->OnEndCall(call_info);
#ifdef _GLOBAL_VIDEO
		if (previewWin && !pjsua_call_get_count()) {
			previewWin->PostMessage(WM_CLOSE, NULL, NULL);
		}
#endif
	}

#if defined _GLOBAL_BALANCE_PLAIN || defined _GLOBAL_BALANCE_OPTIONS
	if (call_info->state == PJSIP_INV_STATE_DISCONNECTED && !pjsua_call_get_count()) {
		SetTimer(IDT_TIMER_BALANCE,2000,NULL);
	}
#endif

	if (call_info->role==PJSIP_ROLE_UAS) { 
		if (call_info->state == PJSIP_INV_STATE_DISCONNECTED || call_info->state == PJSIP_INV_STATE_CONFIRMED) {
			int count = ringinDlgs.GetCount();
			for (int i = 0; i < count; i++ ) {
				RinginDlg *ringinDlg = ringinDlgs.GetAt(i);
				if ( call_info->id == ringinDlg->call_id) {
					if (count==1) {
						PlayerStop();
					}
					ringinDlgs.RemoveAt(i);
					ringinDlg->DestroyWindow();
					break;
				}
			}
		}
	}

	delete call_info;
	delete str;
	return 0;
}

static void on_call_media_state(pjsua_call_id call_id)
{
	pjsua_call_info *call_info = new pjsua_call_info();
	if (pjsua_call_get_info(call_id, call_info) != PJ_SUCCESS || call_info->state == PJSIP_INV_STATE_NULL) {
		return;
	}

	if (call_info->media_status == PJSUA_CALL_MEDIA_ACTIVE
		|| call_info->media_status == PJSUA_CALL_MEDIA_REMOTE_HOLD
		) {
		pjsua_conf_connect(call_info->conf_slot, 0);
		pjsua_conf_connect(0, call_info->conf_slot);
	} else {
		call_deinit_tonegen(call_id);
		pjsua_conf_disconnect(call_info->conf_slot, 0);
		pjsua_conf_disconnect(0, call_info->conf_slot);
	}
	PostMessage(microsipDlg->m_hWnd, MYWM_ON_CALL_MEDIA_STATE, (WPARAM) call_info, 0);
}

LRESULT CmicrosipDlg::onCallMediaState(WPARAM wParam,LPARAM lParam)
{
	pjsua_call_info *call_info = (pjsua_call_info *) wParam;

	messagesDlg->UpdateHoldButton(call_info);

	CString message;
	CString number = PjToStr(&call_info->remote_info, TRUE);

	MessagesContact* messagesContact = messagesDlg->AddTab(number, _T(""), FALSE, call_info, TRUE, TRUE);

	if (messagesContact) {
		if (call_info->media_status == PJSUA_CALL_MEDIA_REMOTE_HOLD) {
			message = _T("Call on remote hold");
		}
		if (call_info->media_status == PJSUA_CALL_MEDIA_LOCAL_HOLD) {
			message = _T("Call on local hold");
		}
		if (call_info->media_status == PJSUA_CALL_MEDIA_NONE) {
			message = _T("Call on hold");
		}
		if (messagesContact->mediaStatus != PJSUA_CALL_MEDIA_ERROR && messagesContact->mediaStatus != call_info->media_status && call_info->media_status == PJSUA_CALL_MEDIA_ACTIVE) {
			message = _T("Call is active");
		}
		if (!message.IsEmpty()) {
			messagesDlg->AddMessage(messagesContact, Translate(message.GetBuffer()), MSIP_MESSAGE_TYPE_SYSTEM, TRUE);
		}
		messagesContact->mediaStatus = call_info->media_status;
	}

	if (call_info->media_status == PJSUA_CALL_MEDIA_ACTIVE
		|| call_info->media_status == PJSUA_CALL_MEDIA_REMOTE_HOLD
		) {
		CSliderCtrl *sliderCtrl = (CSliderCtrl *)pageDialer->GetDlgItem(IDC_VOLUME_OUTPUT);
		if (sliderCtrl) {
			int pos = 200-sliderCtrl->GetPos();
			int val = pos>100 ? 100+(pos-100)*3 : 100;
			pjsua_conf_adjust_rx_level(call_info->conf_slot, (float)val/100);
		}
	}

	delete call_info;
	return 0;
}

static void on_incoming_call(pjsua_acc_id acc, pjsua_call_id call_id,
							 pjsip_rx_data *rdata)
{
	pjsua_call_info call_info;
	pjsua_call_get_info(call_id,&call_info);
	SIPURI sipuri;
	ParseSIPURI(PjToStr(&call_info.remote_info, TRUE), &sipuri);
	if (accountSettings.forceCodec) {
		pjsua_call *call;
		pjsip_dialog *dlg;
		pj_status_t status;
		status = acquire_call("on_incoming_call()", call_id, &call, &dlg);
		if (status == PJ_SUCCESS) {
			pjmedia_sdp_neg_set_prefer_remote_codec_order(call->inv->neg, PJ_FALSE);
			pjsip_dlg_dec_lock(dlg);
		}
	}

	pjsua_call_id call_ids[PJSUA_MAX_CALLS];
	unsigned calls_count = PJSUA_MAX_CALLS;
	if (pjsua_enum_calls ( call_ids, &calls_count)==PJ_SUCCESS)  {
		for (unsigned i = 0; i < calls_count; ++i) {
			pjsua_call_info call_info_curr;
			pjsua_call_get_info(call_ids[i], &call_info_curr);
			SIPURI sipuri_curr;
			ParseSIPURI(PjToStr(&call_info_curr.remote_info, TRUE), &sipuri_curr);
			if (call_info_curr.id != call_info.id && 
				sipuri.user+_T("@")+sipuri.domain == sipuri_curr.user+_T("@")+sipuri_curr.domain
				) {
				pjsua_call_hangup(call_info.id, 486, NULL, NULL);
				return;
			}
		}
	}

	if (IsWindow(microsipDlg->m_hWnd)) {
		if (!microsipDlg->callIdIncomingIgnore.IsEmpty() && microsipDlg->callIdIncomingIgnore == PjToStr(&call_info.call_id)) {
			microsipDlg->PostMessage(MYWM_CALL_ANSWER, (WPARAM)call_id, -487);
			return;
		}
		if (!accountSettings.denyIncoming.IsEmpty()) {
			bool reject = false;
			if (accountSettings.denyIncoming==_T("user")) {
				SIPURI sipuri_curr;
				ParseSIPURI(PjToStr(&call_info.local_info, TRUE), &sipuri_curr);
				if (sipuri_curr.user!=accountSettings.account.username) {
					reject = true;
				}
			} else if (accountSettings.denyIncoming==_T("domain")) {
				SIPURI sipuri_curr;
				ParseSIPURI(PjToStr(&call_info.local_info, TRUE), &sipuri_curr);
				if (accountSettings.accountId) {
					if (sipuri_curr.domain!=accountSettings.account.domain) {
						reject = true;
					}
				}
			} else if (accountSettings.denyIncoming==_T("rdomain")) {
				if (accountSettings.accountId) {
					if (sipuri.domain!=accountSettings.account.domain) {
						reject = true;
					}
				}
			} else if (accountSettings.denyIncoming==_T("address")) {
				SIPURI sipuri_curr;
				ParseSIPURI(PjToStr(&call_info.local_info, TRUE), &sipuri_curr);
				if (sipuri_curr.user!=accountSettings.account.username || (accountSettings.account.domain != _T("") && sipuri_curr.domain!=accountSettings.account.domain)) {
					reject = true;
				}
			} else if (accountSettings.denyIncoming==_T("all")) {
				reject = true;
			}
			if (reject) {
				microsipDlg->PostMessage(MYWM_CALL_HANGUP, (WPARAM)call_id, NULL);
				return;
			}
		}

#ifdef _GLOBAL_COPY_INCOMING
		SetClipboardText(sipuri.user);
#endif
		if (!accountSettings.cmdIncomingCall.IsEmpty() ) {
			microsipDlg->SendMessage(MYWM_SHELL_EXECUTE, (WPARAM)accountSettings.cmdIncomingCall.GetBuffer(), (LPARAM)sipuri.user.GetBuffer());
		}
		int autoAnswer = accountSettings.autoAnswer;
		if (autoAnswer==2) {
			pjsip_generic_string_hdr *hsr = NULL;
			const pj_str_t header = pj_str("X-AUTOANSWER");
			hsr = (pjsip_generic_string_hdr*) pjsip_msg_find_hdr_by_name ( rdata->msg_info.msg, &header, NULL);
			if (hsr) {
				CString autoAnswerValue = PjToStr(&hsr->hvalue, TRUE);
				autoAnswerValue.MakeLower();
				if (autoAnswerValue == _T("true") || autoAnswerValue == _T("1")) {
					autoAnswer = 1;
				}
			}
		}
		if (autoAnswer==2) {
			pjsip_generic_string_hdr *hsr = NULL;
			const pj_str_t header = pj_str("Call-Info");
			hsr = (pjsip_generic_string_hdr*) pjsip_msg_find_hdr_by_name ( rdata->msg_info.msg, &header, NULL);
			if (hsr) {
				CString autoAnswerValue = PjToStr(&hsr->hvalue, TRUE);
				autoAnswerValue.MakeLower();
				if (autoAnswerValue.Find(_T("answer-after="))!=-1 || autoAnswerValue.Find(_T("auto answer"))!=-1) {
					autoAnswer = 1;
				}
			}
		}
		BOOL playBeep = FALSE;
		if (autoAnswer!=1 || calls_count>1) {
			if (accountSettings.hidden) {
				microsipDlg->PostMessage(MYWM_CALL_HANGUP, (WPARAM)call_id, NULL);
			} else {
				pjsua_call_info *call_info_copy = new pjsua_call_info();
				pjsua_call_get_info(call_id,call_info_copy);
				CString *userAgent = new CString();
				pjsip_generic_string_hdr *hsr = NULL;
				const pj_str_t header = {"User-Agent",10};
				hsr = (pjsip_generic_string_hdr*) pjsip_msg_find_hdr_by_name ( rdata->msg_info.msg, &header, NULL);
				if (hsr) {
					userAgent->SetString(PjToStr(&hsr->hvalue, TRUE));
				}
				microsipDlg->PostMessage(MYWM_CREATE_RINGING, (WPARAM)call_info_copy, (LPARAM)userAgent);

				microsipDlg->PostMessage(MYWM_CALL_ANSWER, (WPARAM)call_id, -180);
				if (call_get_count_noincoming()) {
					playBeep = TRUE;
				} else {
					if (!accountSettings.ringingSound.GetLength()) {
						microsipDlg->PostMessage(MYWM_ON_PLAYER_PLAY,MSIP_SOUND_RINGIN,0);
					} else {
						microsipDlg->PostMessage(MYWM_ON_PLAYER_PLAY,MSIP_SOUND_CUSTOM,(LPARAM)&accountSettings.ringingSound);
					}
				}
			}
		} else {
			microsipDlg->PostMessage(MYWM_CALL_ANSWER, (WPARAM)call_id, (LPARAM)call_info.rem_vid_cnt);
			if (!accountSettings.hidden) {
				playBeep = TRUE;
			}
		}
		if (playBeep) {
			microsipDlg->PostMessage(MYWM_ON_PLAYER_PLAY,MSIP_SOUND_RINGIN2,0);
		}
	}

#ifdef _GLOBAL_CMD_INCOMING_OPEN_URL
	CString url;
	url = _T(_GLOBAL_CMD_INCOMING_OPEN_URL);
	CString str;
	if (acc == accountSettings.accountId && !accountSettings.account.server.IsEmpty()) {
		str = accountSettings.account.server;
	} else {
		str = _T("localhost");
	}
	url.Replace(_T("{server}"),str);
	CString num = !sipuri.name.IsEmpty()?sipuri.name:sipuri.user;
	num.Trim('0');
	url.Replace(_T("{callerid}"),CString(urlencode(Utf8EncodeUcs2(num))));
	OpenURL(url);
#endif
}

static void on_nat_detect(const pj_stun_nat_detect_result *res)
{
	if (res->status != PJ_SUCCESS) {
		pjsua_perror(THIS_FILE, "NAT detection failed", res->status);
	} else {
		if (res->nat_type == PJ_STUN_NAT_TYPE_SYMMETRIC) {
			if (IsWindow(microsipDlg->m_hWnd)) {
				CString message;
				pjsua_acc_config acc_cfg;
				pj_pool_t *pool;
				pool = pjsua_pool_create("acc_cfg-pjsua", 1000, 1000);
				if (pool) {
					if (pjsua_acc_get_config(account, pool, &acc_cfg)==PJ_SUCCESS) {
						acc_cfg.sip_stun_use = PJSUA_STUN_USE_DISABLED;
						acc_cfg.media_stun_use = PJSUA_STUN_USE_DISABLED;
						if (pjsua_acc_modify (account, &acc_cfg)==PJ_SUCCESS) {
							message = _T("STUN was automatically disabled.");
#ifndef _GLOBAL_CUSTOM
							message.Append(_T(" For more info visit MicroSIP website, help page."));
#endif

						}
					}
					pj_pool_release(pool);
				}
				microsipDlg->BaloonPopup(_T("Symmetric NAT detected!"), message);
			}
		}
		PJ_LOG(3, (THIS_FILE, "NAT detected as %s", res->nat_type_name));
	}
}

#ifndef _GLOBAL_NO_CONTACTS
static void on_buddy_state(pjsua_buddy_id buddy_id)
{
	if (IsWindow(microsipDlg->m_hWnd)) {
		pjsua_buddy_info buddy_info;
		if (pjsua_buddy_get_info (buddy_id, &buddy_info) == PJ_SUCCESS) {
			Contact *contact = (Contact *) pjsua_buddy_get_user_data(buddy_id);
			int image;
			bool ringing = false;
			switch (buddy_info.status)
			{
			case PJSUA_BUDDY_STATUS_OFFLINE:
				image=1;
				break;
			case PJSUA_BUDDY_STATUS_ONLINE:
				if (PjToStr(&buddy_info.status_text)==_T("Ringing")) {
					ringing = true;
				}
				if (PjToStr(&buddy_info.status_text)==_T("On the phone")) {
					image=4;
				} else if (buddy_info.rpid.activity == PJRPID_ACTIVITY_AWAY)
				{
					image=2;
				} else if (buddy_info.rpid.activity == PJRPID_ACTIVITY_BUSY)
				{
					image=6;

				} else 
				{
					image=3;
				}
				break;
			default:
				image=0;
				break;
			}
			contact->image = image;
			contact->ringing = ringing;
			microsipDlg->PostMessage(MYWM_ON_BUDDY_STATE,(WPARAM)contact);
		}
	}
}
#endif

#ifndef _GLOBAL_NO_MESSAGING
static void on_pager(pjsua_call_id call_id, const pj_str_t *from, const pj_str_t *to, const pj_str_t *contact, const pj_str_t *mime_type, const pj_str_t *body)
{
	if (IsWindow(microsipDlg->m_hWnd)) {
		CString *number = new CString();
		CString *message = new CString();
		number->SetString(PjToStr(from, TRUE));
		message->SetString(PjToStr(body, TRUE));
		message->Trim();
		microsipDlg->PostMessage(MYWM_ON_PAGER, (WPARAM)number, (LPARAM)message);
	}
}

static void on_pager_status(pjsua_call_id call_id, const pj_str_t *to, const pj_str_t *body, void *user_data, pjsip_status_code status, const pj_str_t *reason)
{
	if (status != 200) {
		if (IsWindow(microsipDlg->m_hWnd)) {
			CString *number = new CString();
			CString *message = new CString();
			number->SetString(PjToStr(to, TRUE));
			message->SetString(PjToStr(reason, TRUE));
			message->Trim();
			microsipDlg->PostMessage(MYWM_ON_PAGER_STATUS, (WPARAM)number, (LPARAM)message);
		}
	}
}
#endif

static void on_call_transfer_request2(pjsua_call_id call_id, const pj_str_t *dst, pjsip_status_code *code, pjsua_call_setting *opt)
{
	opt->vid_cnt = 0;
}

static void on_call_transfer_status(pjsua_call_id call_id,
									int status_code,
									const pj_str_t *status_text,
									pj_bool_t final,
									pj_bool_t *p_cont)
{
	pjsua_call_info *call_info = new pjsua_call_info();
	if (pjsua_call_get_info(call_id, call_info) != PJ_SUCCESS || call_info->state == PJSIP_INV_STATE_NULL) {
		return;
	}

	CString *str = new CString();
	str->Format(_T("%s: %s"),
		Translate(_T("Transfer status")),
		PjToStr(status_text, TRUE)
		);
	if (final) {
		str->AppendFormat(_T(" [%s]"), Translate(_T("final")));
	}

	if (status_code/100 == 2) {
		*p_cont = PJ_FALSE;
	}

	call_info->last_status = (pjsip_status_code)status_code;

	PostMessage(microsipDlg->m_hWnd, MYWM_ON_CALL_TRANSFER_STATUS, (WPARAM) call_info, (LPARAM) str);
}

LRESULT CmicrosipDlg::onCallTransferStatus(WPARAM wParam,LPARAM lParam)
{
	pjsua_call_info *call_info = (pjsua_call_info *) wParam;
	CString *str = (CString *) lParam;

	MessagesContact* messagesContact = NULL;
	CString number = PjToStr(&call_info->remote_info, TRUE);
	messagesContact = microsipDlg->messagesDlg->AddTab(number, _T(""), FALSE, call_info, TRUE, TRUE);
	if (messagesContact) {
		microsipDlg->messagesDlg->AddMessage(messagesContact, *str);
	}
	if (call_info->last_status/100 == 2) {
		if (messagesContact) {
			messagesDlg->AddMessage(messagesContact, Translate(_T("Call transfered successfully, disconnecting call")));
		}
		call_hangup_fast(call_info->id);
	}
	delete call_info;
	delete str;
	return 0;
}

CmicrosipDlg::~CmicrosipDlg(void)
{
	KillTimer(IDT_TIMER_0);
}

void CmicrosipDlg::PostNcDestroy()
{
	accountSettings.SettingsSave();
	PJDestroy();

	CBaseDialog::PostNcDestroy();
	RemoveJumpList();
	if (tnd.hWnd) {
		Shell_NotifyIcon(NIM_DELETE, &tnd);
	}
	UnloadLangPackModule();
	delete this;
}

void CmicrosipDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CmicrosipDlg, CBaseDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_QUERYENDSESSION()
	ON_WM_TIMER()
	ON_WM_MOVE()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_CONTEXTMENU()
	ON_WM_DEVICECHANGE()
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_MESSAGE(MYWM_NOTIFYICON,onTrayNotify)
	ON_MESSAGE(MYWM_CREATE_RINGING,onCreateRingingDlg)
	ON_MESSAGE(MYWM_ON_ACCOUNT,OnAccount)
	ON_MESSAGE(MYWM_ON_REG_STATE2,onRegState2)
	ON_MESSAGE(MYWM_ON_CALL_STATE,onCallState)
	ON_MESSAGE(MYWM_ON_CALL_MEDIA_STATE,onCallMediaState)
	ON_MESSAGE(MYWM_ON_CALL_TRANSFER_STATUS,onCallTransferStatus)
	ON_MESSAGE(MYWM_ON_PLAYER_PLAY,onPlayerPlay)
#ifndef _GLOBAL_NO_MESSAGING
	ON_MESSAGE(MYWM_ON_PAGER,onPager)
	ON_MESSAGE(MYWM_ON_PAGER_STATUS,onPagerStatus)
#endif
#ifndef _GLOBAL_NO_CONTACTS
	ON_MESSAGE(MYWM_ON_BUDDY_STATE,onBuddyState)
	ON_MESSAGE(MYWM_USERS_DIRECTORY,onUsersDirectoryLoaded)
#endif
#ifdef _GLOBAL_BALANCE_PLAIN
	ON_MESSAGE(MYWM_ON_BALANCE_PLAIN,onBalancePlainLoaded)
#endif
#ifdef _GLOBAL_BALANCE_OPTIONS
	ON_MESSAGE(MYWM_ON_BALANCE_OPTIONS,onBalanceOptionsLoaded)
#endif
	ON_MESSAGE(MYWM_SHELL_EXECUTE,onShellExecute)
	ON_MESSAGE(WM_POWERBROADCAST,onPowerBroadcast)
	ON_MESSAGE(WM_COPYDATA,onDialNumber)
	ON_MESSAGE(MYWM_CALL_ANSWER,onCallAnswer)
	ON_MESSAGE(MYWM_CALL_HANGUP,onCallHangup)
	ON_MESSAGE(MYWM_SET_PANE_TEXT,onSetPaneText)
#ifndef _GLOBAL_NO_ACCOUNT
	ON_COMMAND(ID_ACCOUNT_ADD,OnMenuAccountAdd)
	ON_COMMAND_RANGE(ID_ACCOUNT_CHANGE_RANGE,ID_ACCOUNT_CHANGE_RANGE+99,OnMenuAccountChange)
	ON_COMMAND_RANGE(ID_ACCOUNT_EDIT_RANGE,ID_ACCOUNT_EDIT_RANGE+99,OnMenuAccountEdit)
#endif
	ON_COMMAND(ID_SETTINGS,OnMenuSettings)
	ON_COMMAND(ID_ALWAYS_ON_TOP,OnMenuAlwaysOnTop)
#ifndef _GLOBAL_NO_LOG
	ON_COMMAND(ID_LOG,OnMenuLog)
#endif
	ON_COMMAND(ID_EXIT,OnMenuExit)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CmicrosipDlg::OnTcnSelchangeTab)
	ON_NOTIFY(TCN_SELCHANGING, IDC_TAB, &CmicrosipDlg::OnTcnSelchangingTab)
	ON_COMMAND(ID_WEBSITE,OnMenuWebsite)
#ifdef _GLOBAL_VIDEO
#endif
END_MESSAGE_MAP()


// CmicrosipDlg message handlers

void CmicrosipDlg::OnBnClickedOk() 
{
}

CmicrosipDlg::CmicrosipDlg(CWnd* pParent /*=NULL*/)
: CBaseDialog(CmicrosipDlg::IDD, pParent) 
{
	this->m_hWnd = NULL;
	microsipDlg = this;
	m_tabPrev = -1;

	Create (IDD, pParent);
}

BOOL CmicrosipDlg::OnInitDialog()
{
	CBaseDialog::OnInitDialog();
	if ( lstrcmp(theApp.m_lpCmdLine,_T("/hidden")) == 0) {
		accountSettings.hidden = TRUE;
		theApp.m_lpCmdLine = NULL;
	}

	LoadLangPackModule();

	pj_ready = false;

	callTimer = 0;

#ifndef _GLOBAL_NO_SETTINGS
	settingsDlg = NULL;
#endif
	messagesDlg = new MessagesDlg(this);
	transferDlg = NULL;

#ifdef _GLOBAL_ACCOUNT_REG
	reg1Dlg = NULL;
	reg2Dlg = NULL;
	reg3Dlg = NULL;
	
#endif
#ifdef _GLOBAL_BALANCE_BEE
	regUserId = 0;
#endif

	m_idleCounter = 0;
	m_isAway = FALSE;

#ifdef _GLOBAL_VIDEO
	previewWin = NULL;
#endif


	SetTimer(IDT_TIMER_0,5000,NULL);

	if (!accountSettings.hidden) {

		SetupJumpList();

		m_hIcon = theApp.LoadIcon(IDR_MAINFRAME);
		iconSmall = (HICON)LoadImage(
			AfxGetInstanceHandle(),
			MAKEINTRESOURCE(IDR_MAINFRAME),
			IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED );
		PostMessage(WM_SETICON,ICON_SMALL,(LPARAM)iconSmall);

		TranslateDialog(this->m_hWnd);

		// Set the icon for this dialog.  The framework does this automatically
		//  when the application's main window is not a dialog
		SetIcon(m_hIcon, TRUE);			// Set big icon
		SetIcon(m_hIcon, FALSE);		// Set small icon

		// TODO: Add extra initialization here

		// add tray icon
		CString str;
#ifndef _GLOBAL_TRAY_HINT
		str.Format(_T("%s %s"), _T(_GLOBAL_NAME_NICE), _T(_GLOBAL_VERSION));
#else
		str = _T(_GLOBAL_TRAY_HINT);
#endif
		tnd.cbSize = NOTIFYICONDATA_V3_SIZE;
		tnd.hWnd = this->GetSafeHwnd();
		tnd.uID = IDR_MAINFRAME;
		tnd.uCallbackMessage = MYWM_NOTIFYICON;
		tnd.uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP; 
#if !defined _GLOBAL_CUSTOM || defined _GLOBAL_ICON_INACTIVE
		iconInactive = LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_INACTIVE));
		tnd.hIcon = iconInactive;
#else 
#ifdef _GLOBAL_TRAY_ICON
		tnd.hIcon = (HICON)LoadImage(
			AfxGetInstanceHandle(),
			MAKEINTRESOURCE(IDI_TRAY),
			IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED );
#else
		tnd.hIcon = iconSmall;
#endif
#endif
		lstrcpyn(tnd.szTip, (LPCTSTR)str, sizeof(tnd.szTip));
		DWORD dwMessage = NIM_ADD;
		Shell_NotifyIcon(dwMessage, &tnd);
	} else {
		tnd.hWnd = NULL;
	}

	m_bar.Create(this);
	m_bar.SetIndicators(indicators,1);
	m_bar.SetPaneInfo(0,IDS_STATUSBAR, SBPS_STRETCH , 0);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, IDS_STATUSBAR);

	m_bar.SetPaneText( 0, _T(""));

	AutoMove(IDC_TAB,0,0,100,0);
	AutoMove(m_bar.m_hWnd,0,100,100,0);

	//--set window pos
	CRect screenRect;
	screenRect.left = GetSystemMetrics (SM_XVIRTUALSCREEN);
	screenRect.top = GetSystemMetrics (SM_YVIRTUALSCREEN);
	screenRect.right = GetSystemMetrics (SM_CXVIRTUALSCREEN) - screenRect.left;
	screenRect.bottom = GetSystemMetrics (SM_CYVIRTUALSCREEN) - screenRect.top;

	CRect rect;
	GetWindowRect(&rect);
	int mx;
	int my;
#ifndef _GLOBAL_WIDTH
	//int mW = accountSettings.mainW>0?accountSettings.mainW:325;
	int mW = accountSettings.mainW>0?accountSettings.mainW:rect.Width();
#else
	int mW = accountSettings.mainW>0?accountSettings.mainW:_GLOBAL_WIDTH;
#endif
	int mH = accountSettings.mainH>0?accountSettings.mainH:rect.Height();
	// coors not specified, first run
	if (!accountSettings.mainX && !accountSettings.mainY) {
		CRect primaryScreenRect;
		SystemParametersInfo(SPI_GETWORKAREA,0,&primaryScreenRect,0);
		mx = primaryScreenRect.Width()-mW;
		my = primaryScreenRect.Height()-mH;
	} else {
		int maxLeft = screenRect.right-mW;
		if (accountSettings.mainX>maxLeft) {
			mx = maxLeft;
		} else {
			mx = accountSettings.mainX < screenRect.left ? screenRect.left : accountSettings.mainX;
		}
		int maxTop = screenRect.bottom-mH;
		if (accountSettings.mainY>maxTop) {
			my = maxTop;
		} else {
			my = accountSettings.mainY < screenRect.top ? screenRect.top : accountSettings.mainY;
		}
	}

	//--set messages window pos/size
	messagesDlg->GetWindowRect(&rect);
	int messagesX;
	int messagesY;
	int messagesW = accountSettings.messagesW>0?accountSettings.messagesW:550;
	int messagesH = accountSettings.messagesH>0?accountSettings.messagesH:mH;
	// coors not specified, first run
	if (!accountSettings.messagesX && !accountSettings.messagesY) {
		accountSettings.messagesX = mx - messagesW;
		accountSettings.messagesY = my;
	}
	int maxLeft = screenRect.right-messagesW;
	if (accountSettings.messagesX>maxLeft) {
		messagesX = maxLeft;
	} else {
		messagesX = accountSettings.messagesX < screenRect.left ? screenRect.left : accountSettings.messagesX;
	}
	int maxTop = screenRect.bottom-messagesH;
	if (accountSettings.messagesY>maxTop) {
		messagesY = maxTop;
	} else {
		messagesY = accountSettings.messagesY < screenRect.top ? screenRect.top : accountSettings.messagesY;
	}

	messagesDlg->SetWindowPos(NULL, messagesX, messagesY, messagesW, messagesH, SWP_NOZORDER);
	SetWindowPos(accountSettings.alwaysOnTop?&CWnd::wndTopMost:&CWnd::wndNoTopMost, mx, my, mW, mH, NULL);

	CTabCtrl* tab = (CTabCtrl*) GetDlgItem(IDC_TAB);
	tab->SetMinTabWidth(1);
	TC_ITEM tabItem;
	tabItem.mask = TCIF_TEXT | TCIF_PARAM;

	pageDialer = new Dialer(this);
	tabItem.pszText = Translate(_T("Dialpad"));
	tabItem.lParam = (LPARAM)pageDialer;
	tab->InsertItem( 99, &tabItem );
	pageDialer->SetWindowPos(NULL, 0, 32, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	AutoMove(pageDialer->m_hWnd,50,50,0,0);

#ifndef _GLOBAL_NO_CALLS
	pageCalls = new Calls(this);
	tabItem.pszText = Translate(_T("Calls"));
	tabItem.lParam = (LPARAM)pageCalls;
	tab->InsertItem( 99, &tabItem );
	pageCalls->SetWindowPos(NULL, 0, 32, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	AutoMove(pageCalls->m_hWnd,0,0,100,100);
#endif

#ifndef _GLOBAL_NO_CONTACTS
	pageContacts = new Contacts(this);
	tabItem.pszText = Translate(_T("Contacts"));
	tabItem.lParam = (LPARAM)pageContacts;
	tab->InsertItem( 99, &tabItem );
	pageContacts->SetWindowPos(NULL, 0, 32, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	AutoMove(pageContacts->m_hWnd,0,0,100,100);
#endif

#ifdef _GLOBAL_PAGE_BUTTONS
	pageButtons = new Buttons(this);
	tabItem.pszText = Translate(_T("Language"));
	tabItem.lParam = (LPARAM)pageButtons;
	tab->InsertItem( 99, &tabItem );
	pageButtons->SetWindowPos(NULL, 0, 32, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	AutoMove(pageButtons->m_hWnd,50,50,0,0);
#endif

#ifndef _GLOBAL_NO_ACCOUNT
#ifndef _GLOBAL_TAB_ACCOUNT
	tabItem.pszText = Translate(_T("Menu"));
#else
	tabItem.pszText = _T("$");
#endif
	tabItem.lParam = -1;
	tab->InsertItem( 99, &tabItem );
#endif
#ifdef _GLOBAL_TAB_HELP
	tabItem.pszText = _T("?");
	tabItem.lParam = -2;
	tab->InsertItem( 99, &tabItem );
#endif

	tab->SetCurSel(accountSettings.activeTab);

	BOOL minimized = !lstrcmp(theApp.m_lpCmdLine, _T("/minimized"));
	if (minimized) {
		theApp.m_lpCmdLine = _T("");
	}

	if (!accountSettings.hidden) {
#ifndef _GLOBAL_MAXIMIZED
		ShowWindow(SW_SHOW); // need show before hide, to hide properly
#else
		ShowWindow(SW_SHOWMAXIMIZED); // need show before hide, to hide properly
		//ShowWindow(SW_SHOW);
		//ShowWindow(SW_MAXIMIZE);
#endif
	}

	m_startMinimized = !firstRun && minimized;

#ifdef _GLOBAL_NO_AUTO_REGISTER
	disableAutoRegister = TRUE;
#else
	disableAutoRegister = FALSE;
#endif
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CmicrosipDlg::BaloonPopup(CString title, CString message, DWORD flags)
{
	if (tnd.hWnd) {
		lstrcpyn(tnd.szInfo, message, sizeof(tnd.szInfo));
		lstrcpyn(tnd.szInfoTitle, title, sizeof(tnd.szInfoTitle));
		tnd.uFlags = tnd.uFlags | NIF_INFO; 
		tnd.dwInfoFlags = flags;
		DWORD dwMessage = NIM_MODIFY;
		Shell_NotifyIcon(dwMessage, &tnd);
	}
}

#ifndef _GLOBAL_NO_ACCOUNT
void CmicrosipDlg::OnMenuAccountAdd()
{
	if (!accountSettings.hidden) {
		AccountDlg *dlg = new AccountDlg(this);
		dlg->Load(0);
	}
}
void CmicrosipDlg::OnMenuAccountChange(UINT nID)
{
	int idNew = nID - ID_ACCOUNT_CHANGE_RANGE + 1;
	CString s;
	int idOld = accountSettings.accountId;
	if (idOld) {
		microsipDlg->PJAccountDelete();
	}
	if (idOld != idNew ) {
		accountSettings.accountId = idNew;
		accountSettings.AccountLoad(accountSettings.accountId,&accountSettings.account);
	}
	accountSettings.SettingsSave();
	if (accountSettings.accountId) {
		microsipDlg->PJAccountAdd();
	}
}

void CmicrosipDlg::OnMenuAccountEdit(UINT nID)
{
	AccountDlg *dlg = new AccountDlg(this);
	int id = accountSettings.accountId>0 ? accountSettings.accountId : nID - ID_ACCOUNT_EDIT_RANGE + 1;
	dlg->Load(id);
}
#endif

void CmicrosipDlg::OnMenuSettings()
{
#ifndef _GLOBAL_NO_SETTINGS
	if (!accountSettings.hidden) {
		if (!settingsDlg)
		{
			settingsDlg = new SettingsDlg(this);
		} else {
			settingsDlg->SetForegroundWindow();
		}
	}
#endif
}

void CmicrosipDlg::OnMenuAlwaysOnTop()
{
	accountSettings.alwaysOnTop = 1 - accountSettings.alwaysOnTop;
	AccountSettingsPendingSave();
	SetWindowPos(accountSettings.alwaysOnTop?&this->wndTopMost:&this->wndNoTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
}

#ifndef _GLOBAL_NO_LOG
void CmicrosipDlg::OnMenuLog()
{
	ShellExecute(NULL, NULL, accountSettings.logFile, NULL, NULL, SW_SHOWNORMAL);
}
#endif

void CmicrosipDlg::OnMenuExit()
{
	DestroyWindow ();
}

LRESULT CmicrosipDlg::onTrayNotify(WPARAM wParam,LPARAM lParam)
{
	UINT uMsg = (UINT) lParam; 
	switch (uMsg ) 
	{ 
	case WM_LBUTTONUP:
		if (this->IsWindowVisible() && !IsIconic())
		{
			if (wParam) {
				this->ShowWindow(SW_HIDE);
			} else {
				SetForegroundWindow();
			}
		} else
		{
			if (!accountSettings.hidden) {
				if (IsIconic()) {
					ShowWindow(SW_RESTORE);
				} else {
					ShowWindow(SW_SHOW);
				}
				SetForegroundWindow();
				CTabCtrl* tab = (CTabCtrl*) GetDlgItem(IDC_TAB);
				int nTab = tab->GetCurSel();
				TC_ITEM tci;
				tci.mask = TCIF_PARAM;
				tab->GetItem(nTab, &tci);
				CWnd* pWnd = (CWnd *)tci.lParam;
				pWnd->SetFocus();
			}
		}
		break;
	case WM_RBUTTONUP:
		MainPopupMenu();
		break;
	} 
	return TRUE;
}

void CmicrosipDlg::MainPopupMenu() 
{
	CPoint point;    
	GetCursorPos(&point);
	CMenu menu;
	menu.LoadMenu(IDR_MENU_TRAY);
	CMenu* tracker = menu.GetSubMenu(0);
	TranslateMenu(tracker->m_hMenu);

	CMenu editMenu;
	editMenu.CreatePopupMenu();
	Account account;
	CString str;
	int i = 0;
	bool checked = false;
	while (true) {
		if (!accountSettings.AccountLoad(i+1,&account)) {
			break;
		}

#ifndef _GLOBAL_ACCOUNT_DOMAIN
		str.Format(_T("%s@%s"),account.username,account.domain);
#else
#ifdef _GLOBAL_ACCOUNT_API
		if (!account.apiLogin.IsEmpty()) {
			str = account.apiLogin;
#else
		if (!account.username.IsEmpty()) {
			str = account.username;
#endif
		} else {
			str.Format(_T("%s %d"),Translate(_T("Account")),i+1);
		}
#endif
		tracker->InsertMenu(ID_ACCOUNT_ADD, (accountSettings.accountId == i+1 ? MF_CHECKED : 0), ID_ACCOUNT_CHANGE_RANGE+i, str);
		editMenu.AppendMenu(0, ID_ACCOUNT_EDIT_RANGE+i, str);
		if (!checked) {
			checked = accountSettings.accountId == i+1;
		}
		i++;
	}
	CString rab = Translate(_T("Edit account"));
	rab.Append(_T("\tCtrl+M"));
	if (i==1) {
		MENUITEMINFO menuItemInfo;
		menuItemInfo.cbSize = sizeof(MENUITEMINFO);
		menuItemInfo.fMask=MIIM_STRING;
		menuItemInfo.dwTypeData = Translate(_T("Make active"));
		tracker->SetMenuItemInfo(ID_ACCOUNT_CHANGE_RANGE,&menuItemInfo);
		tracker->InsertMenu(ID_ACCOUNT_ADD, 0, ID_ACCOUNT_EDIT_RANGE, rab);
	} else if (i>1) {
		tracker->InsertMenu(ID_ACCOUNT_ADD, MF_SEPARATOR);
		if (checked) {
			tracker->InsertMenu(ID_ACCOUNT_ADD, 0, ID_ACCOUNT_EDIT_RANGE, rab);
		} else {
			tracker->InsertMenu(ID_ACCOUNT_ADD, MF_POPUP, (UINT)editMenu.m_hMenu, Translate(_T("Edit account")));
		}
	}
#ifdef _GLOBAL_ONE_ACCOUNT
	if (i) {
		tracker->EnableMenuItem(ID_ACCOUNT_ADD, MF_DISABLED | MF_GRAYED);
	}
#endif
	if (!accountSettings.enableLog) {
		tracker->EnableMenuItem(ID_LOG, MF_DISABLED | MF_GRAYED);
	}
	if (accountSettings.alwaysOnTop) {
		tracker->CheckMenuItem(ID_ALWAYS_ON_TOP,MF_CHECKED);
	}
	SetForegroundWindow();
	tracker->TrackPopupMenu( 0, point.x, point.y, this );
	PostMessage(WM_NULL, 0, 0);
}

LRESULT CmicrosipDlg::onCreateRingingDlg(WPARAM wParam,LPARAM lParam)
{
	pjsua_call_info* call_info = (pjsua_call_info*) wParam;
	CString *userAgent = (CString *)lParam;

	RinginDlg* ringinDlg = new RinginDlg(this);

	if (call_info->rem_vid_cnt) {
		((CButton*)ringinDlg->GetDlgItem(IDC_VIDEO))->EnableWindow(TRUE);
	}
	ringinDlg->GotoDlgCtrl(ringinDlg->GetDlgItem(IDC_CALLER_NAME));

	ringinDlg->call_id = call_info->id;
	SIPURI sipuri;
	CStringW rab;
	CString str;
	CString info;

	info = PjToStr(&call_info->remote_info, TRUE);

	ParseSIPURI(info, &sipuri);
#ifndef _GLOBAL_NO_CONTACTS
	CString name = pageContacts->GetNameByNumber(GetSIPURI(sipuri.user, true));
#else
	CString name;
#endif
	if (name .IsEmpty()) {
		name = !sipuri.name.IsEmpty() ? sipuri.name : (!sipuri.user.IsEmpty()?sipuri.user:sipuri.domain);
	}
	ringinDlg->GetDlgItem(IDC_CALLER_NAME)->SetWindowText(name);

#if !defined _GLOBAL_ACCOUNT_MINI && !defined _GLOBAL_NO_RINGING_DETAILS
	int c = 0;
	int len=0;

	info=(!sipuri.user.IsEmpty()?sipuri.user+_T("@"):_T(""))+sipuri.domain;
	if (!sipuri.name.IsEmpty() && sipuri.name!=name) {
		info = sipuri.name + _T(" <") + info + _T(">");
	}
	len+=info.GetLength();
	str.Format(_T("%s\r\n\r\n"), info);

	info = PjToStr(&call_info->local_info, TRUE);
	ParseSIPURI(info, &sipuri);
	info=(!sipuri.user.IsEmpty()?sipuri.user+_T("@"):_T(""))+sipuri.domain;
	len+=info.GetLength();
	str.AppendFormat(_T("%s: %s\r\n\r\n"), Translate(_T("To")), info);

	if (len<70) {
		c++;
	}

	if (!userAgent->IsEmpty()) {
		str.AppendFormat(_T("%s: %s"), Translate(_T("User-Agent")), *userAgent);
	} else {
		c++;
	}

	for (int i=0; i<c; i++) {
		str = _T("\r\n") + str;
	}

	
#else
	str = !sipuri.user.IsEmpty()?sipuri.user:sipuri.domain;
#endif
	if (str != name) {
		ringinDlg->GetDlgItem(IDC_CALLER_ADDR)->SetWindowText(str);
	} else {
		ringinDlg->GetDlgItem(IDC_CALLER_ADDR)->EnableWindow(FALSE);
	}
	ringinDlgs.Add(ringinDlg);

	delete call_info;
	delete userAgent;

	return 0;
}

#ifndef _GLOBAL_NO_MESSAGING
LRESULT CmicrosipDlg::onPager(WPARAM wParam,LPARAM lParam)
{
	CString *number=(CString *)wParam;
	CString *message=(CString *)lParam;
	MessagesContact* messagesContact = messagesDlg->AddTab(*number);
	if (messagesContact) {
		messagesDlg->AddMessage(messagesContact, *message, MSIP_MESSAGE_TYPE_REMOTE);
		if (accountSettings.localDTMF) {
			onPlayerPlay(MSIP_SOUND_MESSAGE_IN, 0);
		}
	}
	delete number;
	delete message;
	return 0;
}

LRESULT CmicrosipDlg::onPagerStatus(WPARAM wParam,LPARAM lParam)
{
	CString *number=(CString *)wParam;
	CString *message=(CString *)lParam;
	MessagesContact* messagesContact = microsipDlg->messagesDlg->AddTab(*number);
	if (messagesContact) {
		microsipDlg->messagesDlg->AddMessage(messagesContact, *message);
	}
	delete number;
	delete message;
	return 0;
}
#endif

#ifndef _GLOBAL_NO_CONTACTS
LRESULT CmicrosipDlg::onBuddyState(WPARAM wParam,LPARAM lParam)
{
	Contact *contact = (Contact *)wParam;
	CListCtrl *list= (CListCtrl*)pageContacts->GetDlgItem(IDC_CONTACTS);
	int n = list->GetItemCount();
	for (int i=0; i<n; i++) {
		if (contact==(Contact *)list->GetItemData(i)) {
			list->SetItem(i, 0, LVIF_IMAGE, 0, contact->image, 0, 0, 0);
		}
	}
	if (contact->ringing) {
		SetTimer(IDT_TIMER_CONTACTS_BLINK,500,NULL);
		OnTimerContactBlink();
	}
	return 0;
}
#endif

LRESULT CmicrosipDlg::onShellExecute(WPARAM wParam,LPARAM lParam)
{
	ShellExecute(NULL, NULL, (LPTSTR)wParam, (LPTSTR)lParam, NULL, SW_HIDE);
	return 0;
}

LRESULT CmicrosipDlg::onPowerBroadcast(WPARAM wParam,LPARAM lParam)
{
	if (wParam == PBT_APMRESUMEAUTOMATIC)
	{
		BOOL err = TRUE;
		int nCount = 0;
		WSADATA wsaData;
		int wsaError = -1;
		while (err && nCount < 3) {
			err = FALSE;
			if (wsaError) {
				wsaError = WSAStartup(MAKEWORD(2,2), &wsaData);
				if (wsaError==WSASYSNOTREADY) {
					err = TRUE;
				} else if (wsaError) {
					break;
				}
			}
			if (!err) {
				char buf[256]={0};
				if ( gethostname(buf, 256) != 0) {
					err = TRUE;
				}
				else {
					struct addrinfo* l_addrInfo = NULL;
					struct addrinfo l_addrInfoHints;
					ZeroMemory(&l_addrInfoHints, sizeof(addrinfo));
					l_addrInfoHints.ai_socktype = SOCK_STREAM;
					l_addrInfoHints.ai_family = PF_INET;
					if ( getaddrinfo(buf,NULL, &l_addrInfoHints,&l_addrInfo) == 0 ) {
						if (l_addrInfo) {
							struct sockaddr_in *ipv4 = (struct sockaddr_in *)l_addrInfo->ai_addr;
							char * ip = inet_ntoa(ipv4->sin_addr);
							if ( !strcmp(ip, "127.0.0.1") )
							{
								err = TRUE;
#ifdef _GLOBAL_LOCAL_SIP_SERVER
							} else {
								accountSettings.account.server = ip;
								accountSettings.account.domain = ip;
#endif
							}
						} else {
							err = TRUE;
						}
					} else {
						err = TRUE;
					}
				}
			}
			if (err) {
				Sleep(3000);
				nCount++;
			}
		}
		WSACleanup();

		PJCreate();
		PJAccountAddLocal();
		if (lstrlen(theApp.m_lpCmdLine)) {
			DialNumberFromCommandLine(theApp.m_lpCmdLine);
			theApp.m_lpCmdLine = NULL;
		}

		PJAccountAdd();

#ifdef _GLOBAL_JSONRPC
		JSONRPCCreate();
#endif
	} else if (wParam == PBT_APMSUSPEND)
	{
		PJDestroy();
	}
	return TRUE;
}

LRESULT CmicrosipDlg::OnAccount(WPARAM wParam,LPARAM lParam)
{
#ifndef _GLOBAL_NO_ACCOUNT
	AccountDlg *dlg = new AccountDlg(this);
	dlg->Load(accountSettings.accountId);
	if (wParam && dlg) {
		CEdit *edit = (CEdit*)dlg->GetDlgItem(IDC_EDIT_PASSWORD);
		if (edit) {
			edit->SetFocus();
			int nLength = edit->GetWindowTextLength();
			edit->SetSel(nLength,nLength);
		}
	}
#endif
	return 0;
}

void CmicrosipDlg::OnTimerCall (bool manual)
{
	int duration = !manual ? messagesDlg->GetCallDuration() : 0;
	if (duration!=-1) {
		CString str;
		str.Format(_T("%s %s"),Translate(_T("Connected")), GetDuration(duration, true));
		UpdateWindowText(str, IDI_ACTIVE);
	} else {
		if (KillTimer(IDT_TIMER_CALL)) {
			callTimer = 0;
		}
	}
}

#ifndef _GLOBAL_NO_CONTACTS
void CmicrosipDlg::OnTimerContactBlink ()
{
	CListCtrl *list= (CListCtrl*)pageContacts->GetDlgItem(IDC_CONTACTS);
	int n = list->GetItemCount();
	bool ringing = false;
	for (int i=0; i<n; i++) {
		Contact *contact = (Contact *)list->GetItemData(i);
		if (contact->ringing) {
			list->SetItem(i, 0, LVIF_IMAGE, 0, timerContactBlinkState?5:contact->image, 0, 0, 0);
			ringing = true;
		} else {
			list->SetItem(i, 0, LVIF_IMAGE, 0, contact->image, 0, 0, 0);
		}
	}
	if (!ringing) {
		KillTimer(IDT_TIMER_CONTACTS_BLINK);
		timerContactBlinkState=false;
	} else {
		timerContactBlinkState = !timerContactBlinkState;
	}
}
#endif

void CmicrosipDlg::OnTimer (UINT TimerVal)
{
	if (TimerVal == IDT_TIMER_SAVE) {
		KillTimer(IDT_TIMER_SAVE);
		accountSettings.SettingsSave();
#ifndef _GLOBAL_NO_CONTACTS
	} else if (TimerVal == IDT_TIMER_DIRECTORY) {
		UsersDirectoryLoad();
#endif
	} else if (TimerVal == IDT_TIMER_CONTACTS_BLINK) {
#ifndef _GLOBAL_NO_CONTACTS
		OnTimerContactBlink();
#endif
	} else if (TimerVal == IDT_TIMER_CALL) {
		OnTimerCall();
	} else
#if defined _GLOBAL_BALANCE_PLAIN || defined _GLOBAL_BALANCE_OPTIONS
		if (TimerVal == IDT_TIMER_BALANCE) {
			KillTimer(IDT_TIMER_BALANCE);
			if (accountSettings.accountId) {
#ifdef _GLOBAL_BALANCE_PLAIN
				BalancePlain();
#endif
#ifdef _GLOBAL_BALANCE_OPTIONS
				BalanceOptions();
#endif
				SetTimer(IDT_TIMER_BALANCE,15*60000,NULL);
			}
		} else
#endif
			if (TimerVal == IDT_TIMER_0) {
				POINT coord;
				GetCursorPos(&coord);
				if (m_mousePos.x != coord.x || m_mousePos.y != coord.y)
				{
					m_idleCounter = 0;
					m_mousePos = coord;
					if (m_isAway)
					{
						PublishStatus();
					}
				} else {
					m_idleCounter++;
					if (m_idleCounter == 120)
					{
						PublishStatus(false);
					}
				}
			} else
			if ( TimerVal = IDT_TIMER_TONE ) {
				onPlayerPlay(MSIP_SOUND_RINGOUT, 0);
			}
}

void CmicrosipDlg::PJCreate()
{
#ifndef _GLOBAL_NO_CONTACTS
	pageContacts->isSubscribed = FALSE;
#endif
	player_id = PJSUA_INVALID_ID;

	// check updates
#if !defined _GLOBAL_CUSTOM || defined _GLOBAL_UPDATES
	if (accountSettings.updatesInterval != _T("never")) 
	{
		CTime t = CTime::GetCurrentTime();
		time_t time = t.GetTime();
		int days;
		if (accountSettings.updatesInterval ==_T("daily"))
		{
			days = 1;
		} else if (accountSettings.updatesInterval ==_T("monthly"))
		{
			days = 30;
		} else if (accountSettings.updatesInterval ==_T("quarterly"))
		{
			days = 90;
		} else
		{
			days = 7;
		}
		if (accountSettings.checkUpdatesTime + days * 86400 < time)
		{
			CheckUpdates();
			accountSettings.checkUpdatesTime = time;
			accountSettings.SettingsSave();
		}
	}
#endif

	// pj create
	pj_status_t status;
	pjsua_config         ua_cfg;
	pjsua_media_config   media_cfg;
	pjsua_transport_config cfg;

	// Must create pjsua before anything else!
	status = pjsua_create();
	if (status != PJ_SUCCESS)
	{
		return;
	}

	// Initialize configs with default settings.
	pjsua_config_default(&ua_cfg);
	pjsua_media_config_default(&media_cfg);

	CString userAgent;
	if (accountSettings.userAgent.IsEmpty()) {
		userAgent.Format(_T("%s/%s"), _T(_GLOBAL_NAME_NICE), _T(_GLOBAL_VERSION));
		ua_cfg.user_agent = StrToPjStr(userAgent);
	} else {
		ua_cfg.user_agent = StrToPjStr(accountSettings.userAgent);
	}

	ua_cfg.cb.on_reg_state2=&on_reg_state2;
	ua_cfg.cb.on_call_state=&on_call_state;
	
	ua_cfg.cb.on_call_media_state = &on_call_media_state;
	ua_cfg.cb.on_incoming_call = &on_incoming_call;
	ua_cfg.cb.on_nat_detect= &on_nat_detect;
#ifndef _GLOBAL_NO_CONTACTS
	ua_cfg.cb.on_buddy_state = &on_buddy_state;
#endif
#ifndef _GLOBAL_NO_MESSAGING
	ua_cfg.cb.on_pager = &on_pager;
	ua_cfg.cb.on_pager_status = &on_pager_status;
#endif
	ua_cfg.cb.on_call_transfer_request2 = &on_call_transfer_request2;	
	ua_cfg.cb.on_call_transfer_status = &on_call_transfer_status;

	ua_cfg.srtp_secure_signaling=0;

	if (accountSettings.account.stun.GetLength()) 
	{
		ua_cfg.stun_srv_cnt=1;
		ua_cfg.stun_srv[0] = StrToPjStr( accountSettings.account.stun );
	}

	media_cfg.enable_ice = PJ_FALSE;
	media_cfg.no_vad = accountSettings.vad ? PJ_FALSE : PJ_TRUE;
	media_cfg.ec_tail_len = accountSettings.ec ? PJSUA_DEFAULT_EC_TAIL_LEN : 0;

	media_cfg.clock_rate=44100;
	media_cfg.channel_count=1;

#ifdef _GLOBAL_PTIME
	media_cfg.ptime = _GLOBAL_PTIME;
#endif
#ifdef _GLOBAL_JB_MAX
	media_cfg.jb_max=_GLOBAL_JB_MAX;
#endif
#ifdef _GLOBAL_JB_MAX_PRE
	media_cfg.jb_max_pre=_GLOBAL_JB_MAX_PRE;
#endif
#ifdef _GLOBAL_JB_INIT
	media_cfg.jb_init=_GLOBAL_JB_INIT;
#endif


	// Initialize pjsua
#ifndef _GLOBAL_NO_LOG
	if (accountSettings.enableLog) {
		pjsua_logging_config log_cfg;
		pjsua_logging_config_default(&log_cfg);
		CStringA buf;
		int len = accountSettings.logFile.GetLength()+1;
		LPSTR pBuf = buf.GetBuffer(len);
		WideCharToMultiByte(CP_ACP,0,accountSettings.logFile.GetBuffer(),-1,pBuf,len,NULL,NULL);
		log_cfg.log_filename = pj_str(pBuf);
		buf.ReleaseBuffer();
		status = pjsua_init(&ua_cfg, &log_cfg, &media_cfg);
	} else {
		status = pjsua_init(&ua_cfg, NULL, &media_cfg);
	}
#else
	status = pjsua_init(&ua_cfg, NULL, &media_cfg);
#endif
	
	if (status != PJ_SUCCESS)
	{
		pjsua_destroy();
		return;
	}

	// Start pjsua
	status = pjsua_start();
	if (status != PJ_SUCCESS)
	{
		pjsua_destroy();
		return;
	}

	pj_ready = true;

	// Set snd devices
	UpdateSoundDevicesIds();

	//Set aud codecs prio
#ifdef _GLOBAL_CODECS_ENABLED
	if (accountSettings.audioCodecs.IsEmpty())
	{
		accountSettings.audioCodecs = _T(_GLOBAL_CODECS_ENABLED);
	}
#endif
	if (accountSettings.audioCodecs.GetLength())
	{
		pjsua_codec_info codec_info[64];
		unsigned count = 64;
		pjsua_enum_codecs(codec_info, &count);
		for (unsigned i=0;i<count;i++)
		{
			pjsua_codec_set_priority(&codec_info[i].codec_id,0);
		}
		CString resToken;
		int curPos = 0;
		int i = 255;
		resToken= accountSettings.audioCodecs.Tokenize(_T(" "),curPos);
		while (!resToken.IsEmpty()) {
			pj_str_t codec_id = StrToPjStr(resToken);
			pjsua_codec_set_priority(&codec_id, i);
			resToken= accountSettings.audioCodecs.Tokenize(_T(" "),curPos);
			i--;
		}
	}

#ifdef _GLOBAL_VIDEO
	//Set vid codecs prio
	if (accountSettings.videoCodec.GetLength())
	{
		pj_str_t codec_id = StrToPjStr(accountSettings.videoCodec);
		pjsua_vid_codec_set_priority(&codec_id,255);
	}
	int bitrate;
	if (accountSettings.disableH264) {
		pjsua_vid_codec_set_priority(&pj_str("H264"),0);
	} else
	{
		const pj_str_t codec_id = {"H264", 4};
		pjmedia_vid_codec_param param;
		pjsua_vid_codec_get_param(&codec_id, &param);
#ifdef _GLOBAL_H264_BITRATE
		if (accountSettings.bitrateH264.IsEmpty()) {
			accountSettings.bitrateH264 = _T(_GLOBAL_H264_BITRATE);
		}
#endif
		if (atoi(CStringA(accountSettings.bitrateH264))) {
			bitrate = 1000 * atoi(CStringA(accountSettings.bitrateH264));
			param.enc_fmt.det.vid.avg_bps = bitrate;
			param.enc_fmt.det.vid.max_bps = bitrate;
		}
		/*
		param.enc_fmt.det.vid.size.w = 140;
		param.enc_fmt.det.vid.size.h = 80;
		param.enc_fmt.det.vid.fps.num = 30;
		param.enc_fmt.det.vid.fps.denum = 1;
		param.dec_fmt.det.vid.size.w = 640;
		param.dec_fmt.det.vid.size.h = 480;
		param.dec_fmt.det.vid.fps.num = 30;
		param.dec_fmt.det.vid.fps.denum = 1;
		*/
		/*
		// Defaut (level 1e, 30):
		param.dec_fmtp.cnt = 2;
		param.dec_fmtp.param[0].name = pj_str("profile-level-id");
		param.dec_fmtp.param[0].val = pj_str("42e01e");
		param.dec_fmtp.param[1].name = pj_str("packetization-mode");
		param.dec_fmtp.param[1].val = pj_str("1");
		//*/
		pjsua_vid_codec_set_param(&codec_id, &param);
	}
	if (accountSettings.disableH263) {
		pjsua_vid_codec_set_priority(&pj_str("H263"),0);
	} else {
#ifdef _GLOBAL_H263_BITRATE
		if (accountSettings.bitrateH263.IsEmpty()) {
			accountSettings.bitrateH263 = _T(_GLOBAL_H263_BITRATE);
		}
#endif
		if (atoi(CStringA(accountSettings.bitrateH263))) {
			const pj_str_t codec_id = {"H263", 4};
			pjmedia_vid_codec_param param;
			pjsua_vid_codec_get_param(&codec_id, &param);
			bitrate = 1000 * atoi(CStringA(accountSettings.bitrateH263));
			param.enc_fmt.det.vid.avg_bps = bitrate;
			param.enc_fmt.det.vid.max_bps = bitrate;
			pjsua_vid_codec_set_param(&codec_id, &param);
		}
	}
#endif

	// Create transport
	transport_udp_local = -1;
	transport_udp = -1;
	transport_tcp = -1;
	transport_tls = -1;

	pjsua_transport_config_default(&cfg);
	cfg.public_addr = StrToPjStr( accountSettings.account.publicAddr );

#ifdef _GLOBAL_DSCP_CONTROL
	cfg.qos_params.flags = PJ_QOS_PARAM_HAS_DSCP;
	cfg.qos_params.dscp_val = _GLOBAL_DSCP_CONTROL;
#endif

	if (!accountSettings.disableLocalAccount) {
		cfg.port=5060;
		status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &cfg, &transport_udp_local);
		if (status != PJ_SUCCESS) {
			transport_udp_local = -1;
		}
	}
	cfg.port=0;
	pjsua_transport_create(PJSIP_TRANSPORT_UDP, &cfg, &transport_udp);
	if (transport_udp_local == -1) {
		transport_udp_local = transport_udp;
	}
	

#ifdef _GLOBAL_ACCOUNT_TRANSPORT
	if (_GLOBAL_ACCOUNT_TRANSPORT != "udp") {
#endif
	cfg.port = accountSettings.disableLocalAccount ? 0 : 5060;
	status = pjsua_transport_create(PJSIP_TRANSPORT_TCP, &cfg, &transport_tcp);
	if (status != PJ_SUCCESS && cfg.port) {
		cfg.port=0;
		pjsua_transport_create(PJSIP_TRANSPORT_TCP, &cfg, &transport_tcp);
	}

	cfg.port = accountSettings.disableLocalAccount ? 0 : 5061;
	status = pjsua_transport_create(PJSIP_TRANSPORT_TLS, &cfg, &transport_tls);
	if (status != PJ_SUCCESS && cfg.port) {
		cfg.port=0;
		pjsua_transport_create(PJSIP_TRANSPORT_TLS, &cfg, &transport_tls);
	}
#ifdef _GLOBAL_ACCOUNT_TRANSPORT
	}
#endif
	if (pageDialer) {
		pageDialer->OnVScroll( 0, 0, NULL);
	}

#ifndef _GLOBAL_NO_CONTACTS
	if (accountSettings.usersDirectory.Find(_T("%s"))==-1) {
		UsersDirectoryLoad();
	}
#endif

	account = PJSUA_INVALID_ID;
	account_local = PJSUA_INVALID_ID;
}

void CmicrosipDlg::UpdateSoundDevicesIds()
{
	audio_input=-1;
	audio_output=-2;
	audio_ring=-2;

	unsigned count = 64;
	pjmedia_aud_dev_info aud_dev_info[64];
	pjsua_enum_aud_devs(aud_dev_info, &count);
	for (unsigned i=0;i<count;i++)
	{
		CString audDevName(aud_dev_info[i].name);
		if (aud_dev_info[i].input_count && !accountSettings.audioInputDevice.Compare(audDevName)) {
			audio_input = i;
		}
		if (aud_dev_info[i].output_count) {
			if (!accountSettings.audioOutputDevice.Compare(audDevName)) {
				audio_output = i;
			}
			if (!accountSettings.audioRingDevice.Compare(audDevName)) {
				audio_ring = i;
			}
		}
	}
}

void CmicrosipDlg::PJDestroy()
{
#ifdef _GLOBAL_JSONRPC
	JSONRPCDestroy();
#endif
	if (pj_ready) {
#ifndef _GLOBAL_NO_CONTACTS
		if (pageContacts) {
			pageContacts->PresenceUnsubsribe();
		}
#endif
		call_deinit_tonegen(-1);

		toneCalls.RemoveAll();
		
		if (IsWindow(m_hWnd)) {
			KillTimer(IDT_TIMER_TONE);
		}

		PlayerStop();

		attendedCalls.RemoveAll();

		pj_ready = false;

		pjsua_destroy();
	}
}

void CmicrosipDlg::PJAccountConfig(pjsua_acc_config *acc_cfg)
{
	pjsua_acc_config_default(acc_cfg);
#ifdef _GLOBAL_KEEPALIVE_UDP
	acc_cfg->ka_interval = _GLOBAL_KEEPALIVE_UDP;
#endif
#ifdef _GLOBAL_VIDEO
	acc_cfg->vid_in_auto_show = PJ_TRUE;
	acc_cfg->vid_out_auto_transmit = PJ_TRUE;
	acc_cfg->vid_cap_dev = VideoCaptureDeviceId();
	acc_cfg->vid_wnd_flags = PJMEDIA_VID_DEV_WND_BORDER | PJMEDIA_VID_DEV_WND_RESIZABLE;
#endif
#ifdef _GLOBAL_DSCP_MEDIA
	acc_cfg->rtp_cfg.qos_params.flags = PJ_QOS_PARAM_HAS_DSCP;
	acc_cfg->rtp_cfg.qos_params.dscp_val = _GLOBAL_DSCP_MEDIA;
#endif
}

void CmicrosipDlg::PJAccountAdd()
{
	if (!accountSettings.accountId) {
		return;
	}

#ifdef _GLOBAL_ACCOUNT_API
	if (accountSettings.account.apiLogin.IsEmpty()) {
		OnAccount(0,0);
		return;
	}
	AccountAPIQuery(&accountSettings.account);
#endif

	if (accountSettings.account.username.IsEmpty()) {
		OnAccount(0,0);
		return;
	}

#ifdef _GLOBAL_BALANCE_BEE
	regUserId = 0;
#endif

#ifndef _GLOBAL_ACCOUNT_PIN
	CString usernameLocal;
#ifndef _GLOBAL_ACCOUNT_API
	usernameLocal = accountSettings.account.username;
#else
	usernameLocal = accountSettings.account.apiLogin;
#endif

	CString title = _T(_GLOBAL_NAME_NICE);
	if (!accountSettings.account.displayName.IsEmpty())
	{
		title.Append( _T(" - ") + accountSettings.account.displayName);
	} else if (usernameLocal.GetLength())
	{
		title.Append( _T(" - ") + usernameLocal);
	}
#endif
	SetWindowText(title);

	pjsua_acc_config acc_cfg;
	PJAccountConfig(&acc_cfg);

	if (accountSettings.account.srtp ==_T("optional")) {
		acc_cfg.use_srtp = PJMEDIA_SRTP_OPTIONAL;
	} else if (accountSettings.account.srtp ==_T("mandatory")) {
		acc_cfg.use_srtp = PJMEDIA_SRTP_MANDATORY;
	} else {
		acc_cfg.use_srtp = PJMEDIA_SRTP_DISABLED;
	}

	acc_cfg.ice_cfg_use = PJSUA_ICE_CONFIG_USE_CUSTOM;
	acc_cfg.ice_cfg.enable_ice = accountSettings.account.ice ? PJ_TRUE : PJ_FALSE;

	acc_cfg.rtp_cfg.public_addr = StrToPjStr( accountSettings.account.publicAddr );
	acc_cfg.allow_via_rewrite=accountSettings.account.allowRewrite ? PJ_TRUE : PJ_FALSE;
	acc_cfg.allow_sdp_nat_rewrite=acc_cfg.allow_via_rewrite;
	acc_cfg.allow_contact_rewrite=acc_cfg.allow_via_rewrite ? 2 : PJ_FALSE;
	acc_cfg.publish_enabled = accountSettings.account.publish ? PJ_TRUE : PJ_FALSE;

	transport = MSIP_TRANSPORT_AUTO;
	if (accountSettings.account.transport==_T("udp") && transport_udp!=-1) {
		acc_cfg.transport_id = transport_udp;
	} else if (accountSettings.account.transport==_T("tcp") && transport_tcp!=-1) {
		transport = MSIP_TRANSPORT_TCP;
	} else if (accountSettings.account.transport==_T("tls") && transport_tls!=-1) {
		transport = MSIP_TRANSPORT_TLS;
	}

	CString str;
	str.Format(_T("%s..."),Translate(_T("Connecting")));
	UpdateWindowText(str);

	CString proxy;
	if (!accountSettings.account.proxy.IsEmpty()) {
		acc_cfg.proxy_cnt = 1;
		proxy.Format(_T("sip:%s"),accountSettings.account.proxy);
#ifdef _GLOBAL_ACCOUNT_PORT
		proxy.AppendFormat(_T(":%d"),accountSettings.account.port);
#endif
		AddTransportSuffix(proxy);
		acc_cfg.proxy[0] = StrToPjStr( proxy );
	}

	CString localURI;
	if (!accountSettings.account.displayName.IsEmpty()) {
		localURI = _T("\"") + accountSettings.account.displayName + _T("\" ");
	}
#ifdef _GLOBAL_USERNAME_SUFFIX
	localURI += GetSIPURI(accountSettings.account.username + _T(_GLOBAL_USERNAME_SUFFIX));
#else
	localURI += GetSIPURI(accountSettings.account.username);
#endif
	acc_cfg.id = StrToPjStr(localURI);
	acc_cfg.cred_count = 1;
#ifdef _GLOBAL_USERNAME_SUFFIX
	CString username;
	if (!accountSettings.account.authID.IsEmpty()) {
		username = accountSettings.account.authID;
	} else {
		username = accountSettings.account.username + _T(_GLOBAL_USERNAME_SUFFIX);
	}
	acc_cfg.cred_info[0].username = StrToPjStr( username );
#else
	acc_cfg.cred_info[0].username = StrToPjStr( !accountSettings.account.authID.IsEmpty()? accountSettings.account.authID : accountSettings.account.username );
#endif
	acc_cfg.cred_info[0].realm = pj_str("*");
	acc_cfg.cred_info[0].scheme = pj_str("Digest");
	acc_cfg.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
	acc_cfg.cred_info[0].data = StrToPjStr( accountSettings.account.password );

	CString regURI;
	if (accountSettings.account.server.IsEmpty()) {
		acc_cfg.register_on_acc_add = PJ_FALSE;
	} else {
		regURI.Format(_T("sip:%s"),accountSettings.account.server);
		AddTransportSuffix(regURI);
		acc_cfg.reg_uri = StrToPjStr( regURI );
	}
	pj_status_t status = pjsua_acc_add(&acc_cfg, PJ_TRUE, &account);
	if (status == PJ_SUCCESS) {
		PublishStatus(true, acc_cfg.register_on_acc_add && true);
	} else {
		ShowErrorMessage(status);
		UpdateWindowText();
	}
}

void CmicrosipDlg::PJAccountAddLocal()
{
	if (!accountSettings.disableLocalAccount) {
		pj_status_t status;
		pjsua_acc_config acc_cfg;
		PJAccountConfig(&acc_cfg);
		acc_cfg.priority--;
		pjsua_transport_data *t = &pjsua_var.tpdata[0];
		CString localURI;
		localURI.Format(_T("<sip:%s>"), PjToStr(&t->local_name.host));
		acc_cfg.id = StrToPjStr(localURI);
		pjsua_acc_add(&acc_cfg, PJ_TRUE, &account_local);
		acc_cfg.priority++;
	}
	SetWindowText(_T(_GLOBAL_NAME_NICE));
	UpdateWindowText();
}

void CmicrosipDlg::PJAccountDelete()
{
#ifndef _GLOBAL_NO_CONTACTS
	if (pageContacts) {
		pageContacts->PresenceUnsubsribe();
	}
#endif
	if (account != PJSUA_INVALID_ID) {
		pjsua_acc_del(account);
		account = PJSUA_INVALID_ID;
	}
	accountSettings.accountId = 0;
	SetWindowText(_T(_GLOBAL_NAME_NICE));
#ifdef _GLOBAL_BALANCE_IN_DIALER
	if (pageDialer) {
		pageDialer->GetDlgItem(IDC_BALANCE)->SetWindowText( _T(_GLOBAL_NAME) );
	}
#endif
#ifdef _GLOBAL_BALANCE_IN_STATUSBAR
	balance.Empty();
#endif
	UpdateWindowText();
}

void CmicrosipDlg::PJAccountDeleteLocal()
{
	if (account_local != PJSUA_INVALID_ID) {
		pjsua_acc_del(account_local);
		account_local = PJSUA_INVALID_ID;
	}
}

void CmicrosipDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	CTabCtrl* tab = (CTabCtrl*) GetDlgItem(IDC_TAB);
	int nTab = tab->GetCurSel();
	TC_ITEM tci;
	tci.mask = TCIF_PARAM;
	tab->GetItem(nTab, &tci);
	if (tci.lParam>0) {
		CWnd* pWnd = (CWnd *)tci.lParam;
		if (m_tabPrev!=-1) {
			tab->GetItem(m_tabPrev, &tci);
			if (tci.lParam>0) {
				((CWnd *)tci.lParam)->ShowWindow(SW_HIDE);
			}
		}
		pWnd->ShowWindow(SW_SHOW);
		pWnd->SetFocus();
		if (nTab!=accountSettings.activeTab) {
			accountSettings.activeTab=nTab;
			AccountSettingsPendingSave();
		}
	} else {
#if !defined _GLOBAL_NO_ACCOUNT || defined _GLOBAL_TAB_HELP
#ifndef _GLOBAL_NO_ACCOUNT
		if (tci.lParam == -1) {
#ifndef _GLOBAL_TAB_ACCOUNT
			MainPopupMenu();
#else
			OpenURL(_T(_GLOBAL_TAB_ACCOUNT));
#endif
		}
#endif
#ifdef _GLOBAL_TAB_HELP	
		else if (tci.lParam == -2) {
			OpenURL(_T(_GLOBAL_TAB_HELP));
		}
#endif
#endif
		if (m_tabPrev!=-1) {
			tab->SetCurSel(m_tabPrev);
			tab->Invalidate();
		}
	}
	*pResult = 0;
}

void CmicrosipDlg::OnTcnSelchangingTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	CTabCtrl* tab = (CTabCtrl*) GetDlgItem(IDC_TAB);
	m_tabPrev = tab->GetCurSel();
	*pResult = FALSE;
}

void CmicrosipDlg::UpdateWindowText(CString text, int icon, bool afterRegister)
{
	if (text.IsEmpty() && pjsua_call_get_count()>0) {
		return;
	}
	CString str;
	if (text.IsEmpty() || text==_T("-")) {
		if (pjsua_acc_is_valid(account))
		{
			pjsua_acc_info info;
			pjsua_acc_get_info(account,&info);
			str = PjToStr(&info.status_text);
			if ( str != _T("Default status message") ) {
				bool noReg = str==_T("does not register");
				if (noReg || str==_T("OK"))
				{
					str = Translate(m_isAway ? _T("Away") : _T("Online"));
#ifndef _GLOBAL_ACCOUNT_SIP_SERVER
					if (noReg) {
						str.AppendFormat(_T(" (%s)"), Translate(_T("outgoing")));
					}
#endif
#ifdef _GLOBAL_BALANCE_IN_STATUSBAR
					if (!balance.IsEmpty()) {
						str.AppendFormat(_T(" - your balance %s"), balance);
					}
#endif
					icon = m_isAway ? IDI_AWAY : IDI_ONLINE;
#ifndef _GLOBAL_NO_CONTACTS
					pageContacts->PresenceSubsribe();
#endif
					if (!dialNumberDelayed.IsEmpty())
					{
						DialNumber(dialNumberDelayed);
						dialNumberDelayed=_T("");
					}
				} else if (str == _T("In Progress")) {
					str.Format(_T("%s..."),Translate(_T("Connecting")));
				} else if (info.status == 401 || info.status == 403) {
#ifndef _GLOBAL_NO_ACCOUNT
					onTrayNotify(NULL,WM_LBUTTONUP);
					if (afterRegister) {
						PostMessage(MYWM_ON_ACCOUNT,1);
					}
#endif
					icon = IDI_OFFLINE;
#ifndef _GLOBAL_ACCOUNT_PIN
					str = Translate(_T("Incorrect password"));
#else
					str = Translate(_T("Incorrect Pin No"));
#endif
				} else {
					str = Translate(str.GetBuffer());
				}
				str = str.GetBuffer();
			} else {
				str.Format( _T("%s: %d"), Translate(_T("Response code")),info.status );
			}
		} else {
#ifndef _GLOBAL_STAUSBAR_CAPTION
			str = _T(_GLOBAL_NAME_NICE);
#else
			str = _T(_GLOBAL_STAUSBAR_CAPTION);
#endif
			icon = IDI_DEFAULT;
		}
	} else
	{
		str = text;
	}

	CString* pPaneText = new CString();
	*pPaneText = str;
	PostMessage(MYWM_SET_PANE_TEXT, NULL, (LPARAM)pPaneText);

	if (icon!=-1) {
		HICON hIcon = (HICON)LoadImage(
			AfxGetInstanceHandle(),
			MAKEINTRESOURCE(icon),
			IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED );
		m_bar.GetStatusBarCtrl().SetIcon(0, hIcon);
		
#if !defined _GLOBAL_CUSTOM || defined _GLOBAL_ICON_INACTIVE
		tnd.uFlags = tnd.uFlags & ~NIF_INFO;
		if ((!accountSettings.accountId && !accountSettings.disableLocalAccount) || (icon!=IDI_DEFAULT && icon!=IDI_OFFLINE)) {
			if (tnd.hIcon != iconSmall) {
				tnd.hIcon = iconSmall;
				Shell_NotifyIcon(NIM_MODIFY,&tnd);
			}
		} else {
			if (tnd.hIcon != iconInactive) {
				tnd.hIcon = iconInactive;
				Shell_NotifyIcon(NIM_MODIFY,&tnd);
			}
		}
#endif
	}
}

void CmicrosipDlg::PublishStatus(bool online, bool init)
{
	if (pjsua_acc_is_valid(account))
	{
		pjrpid_element pr;
		pr.type = PJRPID_ELEMENT_TYPE_PERSON;
		pr.id = pj_str(NULL);
		pr.note = pj_str(NULL);
		pr.note = online ? pj_str("Idle") : pj_str("Away");
		pr.activity = online ? PJRPID_ACTIVITY_UNKNOWN : PJRPID_ACTIVITY_AWAY;
		pjsua_acc_set_online_status2(account, PJ_TRUE, &pr);
		m_isAway = !online;
		if (!init) {
			UpdateWindowText();
		}
	}
}

LRESULT CmicrosipDlg::onDialNumber(WPARAM wParam,LPARAM lParam)
{
	COPYDATASTRUCT *s = (COPYDATASTRUCT*)lParam;
	if (s && s->dwData == 1 ) {
		CString params = (LPCTSTR)s->lpData;
		params.Trim();
		if (!params.IsEmpty()) {
			DialNumberFromCommandLine(params);
		}
	}
	return 0;
}

void CmicrosipDlg::GotoTab(int i, CTabCtrl* tab) {
	if (!tab) {
		tab = (CTabCtrl*) GetDlgItem(IDC_TAB);
	}
	int nTab = tab->GetCurSel();
	if (nTab != i) {
		LRESULT pResult;
		microsipDlg->OnTcnSelchangingTab(NULL, &pResult);
		tab->SetCurSel(i);
		microsipDlg->OnTcnSelchangeTab(NULL, &pResult);
	}
}


void CmicrosipDlg::DialNumberFromCommandLine(CString number) {
	GotoTab(0);
	pjsua_acc_info info;
	number.Trim('"');
	if (number.Mid(0,4).CompareNoCase(_T("tel:"))==0 || number.Mid(0,4).CompareNoCase(_T("sip:"))==0) {
		number = number.Mid(4);
	} else if (number.Mid(0,7).CompareNoCase(_T("callto:"))==0) {
		number = number.Mid(7);
	}
	if (pj_ready && (
		(accountSettings.accountId==0 && pjsua_acc_is_valid(account_local)) || 
		(accountSettings.accountId>0 && pjsua_acc_is_valid(account) && pjsua_acc_get_info(account,&info)==PJ_SUCCESS && info.status==200)
		) ) {
			DialNumber(number);
	} else {
		dialNumberDelayed = number;
	}
}

void CmicrosipDlg::DialNumber(CString params)
{
	CString number;
	CString message;
	int i = params.Find(_T(" "));
	if (i!=-1) {
		number = params.Mid(0,i);
		message = params.Mid(i+1);
		message.Trim();
	} else {
		number = params;
	}
	number.Trim();
	if (!number.IsEmpty()) {
		if (message.IsEmpty()) {
			MakeCall(number);
		} else {
			messagesDlg->SendMessage(NULL, message, number);
		}
	}
}

void CmicrosipDlg::MakeCall(CString number)
{
	number.Trim();
	pageDialer->SetNumber(number);
	pageDialer->OnBnClickedCall();
}

void CmicrosipDlg::PlayerStop()
{
	if (player_id != PJSUA_INVALID_ID) {
		pjsua_conf_disconnect(pjsua_player_get_conf_port(player_id),0);
		if (pjsua_player_destroy(player_id)==PJ_SUCCESS) {
			player_id = PJSUA_INVALID_ID;
		}
	}
}

LRESULT CmicrosipDlg::onPlayerPlay(WPARAM wParam,LPARAM lParam)
{
	CString filename;
	BOOL noLoop;
	BOOL inCall;
	switch(wParam){
		case MSIP_SOUND_CUSTOM:
			filename = *(CString*)lParam;
			noLoop = FALSE;
			inCall = FALSE;
			break;
		case MSIP_SOUND_MESSAGE_IN:
			filename = _T("messagein.wav");
			noLoop = TRUE;
			inCall = FALSE;
			break;
		case MSIP_SOUND_MESSAGE_OUT:
			filename = _T("messageout.wav");
			noLoop = TRUE;
			inCall = FALSE;
			break;
		case MSIP_SOUND_HANGUP:
			filename = _T("hangup.wav");
			noLoop = TRUE;
			inCall = TRUE;
			break;
		case MSIP_SOUND_RINGIN:
			filename = _T("ringin.wav");
			noLoop = FALSE;
			inCall = FALSE;
			break;
		case MSIP_SOUND_RINGIN2:
			filename = _T("ringin2.wav");
			noLoop = TRUE;
			inCall = TRUE;
			break;
			case MSIP_SOUND_RINGOUT:
			filename = _T("ringout.wav");
			noLoop = TRUE;
			inCall = TRUE;
			break;
		default:
				noLoop = TRUE;
				inCall = FALSE;
	}
	PlayerPlay(filename, noLoop, inCall);
	return 0;
}

struct pjsua_player_eof_data
{
    pj_pool_t          *pool;
    pjsua_player_id player_id;
};

static PJ_DEF(pj_status_t) on_pjsua_wav_file_end_callback(pjmedia_port* media_port, void* args)
{
    pj_status_t status;
    struct pjsua_player_eof_data *eof_data = (struct pjsua_player_eof_data *)args;
    status = pjsua_player_destroy(eof_data->player_id);
    PJ_LOG(3,(THIS_FILE, "End of Wav File, media_port: %d", media_port));
    if (status == PJ_SUCCESS) {
        return -1;//Here it is important to return value other than PJ_SUCCESS
    }
    return PJ_SUCCESS;
}

void CmicrosipDlg::PlayerPlay(CString filename, BOOL noLoop, BOOL inCall)
{
	PlayerStop();
	if (!filename.IsEmpty()) {
		pj_str_t file = StrToPjStr(filename);
		if (pjsua_player_create( &file, noLoop?PJMEDIA_FILE_NO_LOOP:0, &player_id)== PJ_SUCCESS) {
			pjmedia_port *player_media_port;
			if ( pjsua_player_get_port(player_id, &player_media_port) == PJ_SUCCESS ) {
				pj_pool_t *pool = pjsua_pool_create("microsip_eof_data", 512, 512);
				struct pjsua_player_eof_data *eof_data = PJ_POOL_ZALLOC_T(pool, struct pjsua_player_eof_data);
				eof_data->pool = pool;
				eof_data->player_id = player_id;
				pjmedia_wav_player_set_eof_cb(player_media_port, eof_data, &on_pjsua_wav_file_end_callback);
				if (
					(!tone_gen && pjsua_conf_get_active_ports()<=2)
					||
					(tone_gen && pjsua_conf_get_active_ports()<=3)
					) {
						SetSoundDevice(inCall?audio_output:audio_ring);
				}
				pjsua_conf_connect(pjsua_player_get_conf_port(player_id),0);
			}
		}
	}
}

void CmicrosipDlg::SetSoundDevice(int outDev){
	int in,out;
	if (pjsua_get_snd_dev(&in,&out)!=PJ_SUCCESS || audio_input!=in || outDev!=out ) {
		pjsua_set_snd_dev(audio_input, outDev);
	}
}

LRESULT CmicrosipDlg::onCallAnswer(WPARAM wParam,LPARAM lParam)
{
	pjsua_call_id call_id = wParam;
	if (lParam<0) {
		pjsua_call_answer(call_id, -lParam, NULL, NULL);
		return 0;
	}

	pjsua_call_info call_info;
	pjsua_call_get_info(call_id,&call_info);

	if (call_info.role==PJSIP_ROLE_UAS && (call_info.state == PJSIP_INV_STATE_INCOMING || call_info.state == PJSIP_INV_STATE_EARLY)) {
		if (accountSettings.singleMode) {
			call_hangup_all_noincoming();
		} else {
			call_hold_all_except(call_id);
		}
		SetSoundDevice(audio_output);
#ifdef _GLOBAL_VIDEO
		if (lParam>0) {
			createPreviewWin();
		}
#endif
		pjsua_call_setting call_setting;
		pjsua_call_setting_default(&call_setting);
		call_setting.vid_cnt=lParam>0 ? 1:0;
		call_setting.flag = 0;
		if (pjsua_call_answer2(call_id, &call_setting, 200, NULL, NULL) == PJ_SUCCESS) {
			callIdIncomingIgnore = PjToStr(&call_info.call_id);
		}
		PlayerStop();
		onTrayNotify(NULL,WM_LBUTTONUP);
	}
	return 0;
}

LRESULT CmicrosipDlg::onCallHangup(WPARAM wParam,LPARAM lParam)
{
	pjsua_call_id call_id = wParam;
	call_hangup_fast(call_id);
	return 0;
}

LRESULT CmicrosipDlg::onSetPaneText(WPARAM wParam,LPARAM lParam)
{
	CString* pString = (CString*)lParam;
    ASSERT(pString != NULL);
    m_bar.SetPaneText(0, *pString);
    delete pString;
	return 0;
}

BOOL CmicrosipDlg::CopyStringToClipboard( IN const CString & str )
{
	// Open the clipboard
	if ( !OpenClipboard() )
		return FALSE;

	// Empty the clipboard
	if ( !EmptyClipboard() )
	{
		CloseClipboard();
		return FALSE;
	}

	// Number of bytes to copy (consider +1 for end-of-string, and
	// properly scale byte size to sizeof(TCHAR))
	SIZE_T textCopySize = (str.GetLength() + 1) * sizeof(TCHAR);

	// Allocate a global memory object for the text
	HGLOBAL hTextCopy = GlobalAlloc( GMEM_MOVEABLE, textCopySize );
	if ( hTextCopy == NULL )
	{
		CloseClipboard();
		return FALSE;
	}

	// Lock the handle, and copy source text to the buffer
	TCHAR * textCopy = reinterpret_cast< TCHAR *>( GlobalLock(
		hTextCopy ) );
	ASSERT( textCopy != NULL );
	StringCbCopy( textCopy, textCopySize, str.GetString() );
	GlobalUnlock( hTextCopy );
	textCopy = NULL; // avoid dangling references

	// Place the handle on the clipboard
#if defined( _UNICODE )
	UINT textFormat = CF_UNICODETEXT;  // Unicode text
#else
	UINT textFormat = CF_TEXT;         // ANSI text
#endif // defined( _UNICODE )

	if (SetClipboardData( textFormat, hTextCopy ) == NULL )
	{
		// Failed
		CloseClipboard();
		return FALSE;
	}

	// Release the clipboard
	CloseClipboard();

	// All right
	return TRUE;
}

void CmicrosipDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
#ifndef _GLOBAL_EXIT_ON_CLOSE
	if (nID==SC_CLOSE) {
		ShowWindow(SW_HIDE);
	} else {
		__super::OnSysCommand(nID, lParam);
	}
#else
	__super::OnSysCommand(nID, lParam);
#endif
}

BOOL CmicrosipDlg::OnQueryEndSession()
{
	DestroyWindow();
	return TRUE;
}

void CmicrosipDlg::OnClose()
{
	DestroyWindow ();
}

void CmicrosipDlg::OnContextMenu(CWnd *pWnd, CPoint point)
{
	CPoint local = point;
	ScreenToClient(&local);
	CRect rect;
	GetClientRect(&rect);
	if (rect.Height()-local.y <= 16) {
		MainPopupMenu();
	} else {
		DefWindowProc(WM_CONTEXTMENU,NULL,MAKELPARAM(point.x,point.y));
	}
}

BOOL CmicrosipDlg::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
	if (nEventType==DBT_DEVNODES_CHANGED) {
		if (pj_ready) {
			pjmedia_aud_dev_refresh();
			UpdateSoundDevicesIds();
#ifdef _GLOBAL_VIDEO
			pjmedia_vid_dev_refresh();
#endif
		}
	}
	return FALSE;
}

void CmicrosipDlg::OnMove(int x, int y)
{
	if (IsWindowVisible() && !IsZoomed() && !IsIconic()) {
		CRect cRect;
		GetWindowRect(&cRect);
		accountSettings.mainX = cRect.left;
		accountSettings.mainY = cRect.top;
		AccountSettingsPendingSave();
	}
}

void CmicrosipDlg::OnSize(UINT type, int w, int h)
{
	CBaseDialog::OnSize(type, w, h);
	if (this->IsWindowVisible() && type == SIZE_RESTORED) {
		CRect cRect;
		GetWindowRect(&cRect);
		accountSettings.mainW = cRect.Width();
		accountSettings.mainH = cRect.Height();
		AccountSettingsPendingSave();
	}
}

void CmicrosipDlg::SetupJumpList()
{
JumpList jl(_T(_GLOBAL_NAME_NICE));
jl.AddTasks();
}

void CmicrosipDlg::RemoveJumpList()
{
JumpList jl(_T(_GLOBAL_NAME_NICE));
jl.DeleteJumpList();
}

void CmicrosipDlg::OnMenuWebsite()
{
#ifndef _GLOBAL_CUSTOM
	OpenURL(_T("http://www.microtalkng.com/"));
#endif
#ifdef _GLOBAL_MENU_WEBSITE
	CString url;
	url.Format(_T(_GLOBAL_MENU_WEBSITE),accountSettings.account.username);
	OpenURL(url);
#endif
}

#ifndef _GLOBAL_NO_CONTACTS
LRESULT CmicrosipDlg::onUsersDirectoryLoaded(WPARAM wParam,LPARAM lParam)
{
	URLGetAsyncData *response = (URLGetAsyncData *)wParam;
	int expires = 0;
	if (response->statusCode == 200 && !response->body.IsEmpty()) {
		CXMLFile xmlFile;
		if ( xmlFile.LoadFromStream((BYTE*)response->body.GetBuffer(),response->body.GetLength()) ){
			BOOL ok = FALSE;
			bool changed = false;
			pageContacts->SetCanditates();
			CXMLElement *root = xmlFile.GetRoot();
			CXMLElement *directory = root->GetFirstChild();
			while (directory) {
				if (directory->GetElementType() == XET_TAG) {
					CXMLElement *entry = directory->GetFirstChild();
					while (entry) {
						if (entry->GetElementType() == XET_TAG) {
							CXMLElement *data = entry->GetFirstChild();
							CString number;
							CString name;
							char presence = -1;
							while (data) {
								if (data->GetElementType() == XET_TAG) {
									CString dataName = data->GetElementName();
									CXMLElement *value = data->GetFirstChild();
									if (value->GetElementType() == XET_TEXT) {
										if (dataName.CompareNoCase(_T("name"))==0) {
											name = Utf8DecodeUni(UnicodeToAnsi(value->GetElementName()));
										} else if (dataName.CompareNoCase(_T("extension"))==0 || dataName.CompareNoCase(_T("number"))==0 || dataName.CompareNoCase(_T("telephone"))==0) {
											number = Utf8DecodeUni(UnicodeToAnsi(value->GetElementName()));
										} else if (dataName.CompareNoCase(_T("presence"))==0) {
											CString rab = value->GetElementName();
											presence = !(rab.IsEmpty() || rab == _T("0")
												|| rab.CompareNoCase(_T("no"))==0
												|| rab.CompareNoCase(_T("false"))==0
												|| rab.CompareNoCase(_T("null"))==0);
										}
									}
								}
								data = entry->GetNextChild();
							}
							if (!number.IsEmpty()) {
								if (pageContacts->ContactAdd(number, name, presence, 1, FALSE, TRUE) && !changed) {
									changed = true;
								}
								if (!ok) {
									ok = TRUE;
								}
							}
						}
						entry = directory->GetNextChild();
					}
				}
				directory=root->GetNextChild();
			}
			if (ok) {
				if (pageContacts->DeleteCanditates() || changed) {
					pageContacts->ContactsSave();
				}
			}
		}
		response->headers.MakeLower();
		CString search = _T("\r\ncache-control:");
		int n = response->headers.Find(search);
		if (n>0) {
			n = n+search.GetLength();
			int l = response->headers.Find(_T("\r\n"),n);
			if (l>0) {
				response->headers = response->headers.Mid(n,l-n);
				search = _T("max-age=");
				n = response->headers.Find(search);
				if (n!=-1) {
					response->headers = response->headers.Mid(n+search.GetLength());
					expires = atoi(CStringA(response->headers));
				}
			}
		}
	}
	if (expires<=0) {
		expires = 3600;
	} else if (expires<60) {
		expires = 60;
	} else if (expires>86400) {
		expires = 86400;
	}
	SetTimer(IDT_TIMER_DIRECTORY,1000*expires,NULL);

	PJ_LOG(3, (THIS_FILE, "End UsersDirectoryLoad"));
	return 0;
}
#endif

#ifndef _GLOBAL_NO_CONTACTS
void CmicrosipDlg::UsersDirectoryLoad()
{
	KillTimer(IDT_TIMER_DIRECTORY);
	if (!accountSettings.usersDirectory.IsEmpty()) {
		CString url;
		url.Format(accountSettings.usersDirectory,accountSettings.account.username,accountSettings.account.password,accountSettings.account.server);
		PJ_LOG(3, (THIS_FILE, "Begin UsersDirectoryLoad"));
		URLGetAsync(url,m_hWnd,MYWM_USERS_DIRECTORY);
	}
}
#endif

void CmicrosipDlg::AccountSettingsPendingSave()
{
	KillTimer(IDT_TIMER_SAVE);
	SetTimer(IDT_TIMER_SAVE,5000,NULL);
}

#if !defined _GLOBAL_CUSTOM || defined _GLOBAL_UPDATES
void CmicrosipDlg::CheckUpdates()
{
	CInternetSession session;
	try {
		CHttpFile* pFile;
#ifdef _GLOBAL_UPDATES
		CString url;
#ifndef _GLOBAL_UPDATES_URL
		url.Format(_T("http://update1.microsip.org/?client=%s&version="),_T(_GLOBAL_UPDATES),_T(_GLOBAL_UPDATES));
#else
		url = _GLOBAL_UPDATES_URL;
#endif
#else
		CString url = _T("http://update.microtalkng.com/?version=");
#endif
		url.Append(_T(_GLOBAL_VERSION));
#ifndef _GLOBAL_VIDEO
		url.Append(_T("&lite=1"));
#endif
		pFile = (CHttpFile*)session.OpenURL(url, NULL, INTERNET_FLAG_TRANSFER_ASCII | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE);
		if (pFile)
		{
			DWORD dwStatusCode;
			pFile->QueryInfoStatusCode(dwStatusCode);
			if (dwStatusCode == 202) {
				CString message;
				message.Format(_T("Do you want to update %s?"), _T(_GLOBAL_NAME_NICE));
				if (::MessageBox(this->m_hWnd, Translate(message.GetBuffer()), Translate(_T("Update available")), MB_YESNO|MB_ICONQUESTION) == IDYES)
				{
#ifdef _GLOBAL_UPDATES

					CStringA data;
					CStringA buf;
					char pBuf[256];
					int i;
					do {
						i = pFile->Read(pBuf,255);
						pBuf[i] = 0;
						data.AppendFormat("%s",pBuf);
					} while (i>0);
					if (!data.IsEmpty()) {
						OpenURL(Utf8DecodeUni(data));
					}
#else
					OpenURL(_T("http://www.microtalkng.com/downloads"));
#endif
				}
			}
			pFile->Close();
		}
		session.Close();
	} catch (CInternetException *e) {}
}
#endif

#ifdef _GLOBAL_VIDEO
int CmicrosipDlg::VideoCaptureDeviceId(CString name)
{
	unsigned count = 64;
	pjmedia_vid_dev_info vid_dev_info[64];
	pjsua_vid_enum_devs(vid_dev_info, &count);
	for (unsigned i=0;i<count;i++) {
		if (vid_dev_info[i].fmt_cnt && (vid_dev_info[i].dir==PJMEDIA_DIR_ENCODING || vid_dev_info[i].dir==PJMEDIA_DIR_ENCODING_DECODING)) {
			CString vidDevName(vid_dev_info[i].name);
			if ((!name.IsEmpty() && name == vidDevName)
				||
				(name.IsEmpty() && accountSettings.videoCaptureDevice == vidDevName)) {
				return vid_dev_info[i].id;
			}
		}
	}
	return PJMEDIA_VID_DEFAULT_CAPTURE_DEV;
}

void CmicrosipDlg::createPreviewWin()
{
	if (!previewWin) {
		previewWin = new Preview(this);
	}
	previewWin->Start(VideoCaptureDeviceId());
}
#endif