#include "const.h"

#ifndef _GLOBAL_CODECS_ENABLED
#define _GLOBAL_CODECS_ENABLED "speex/16000/1 SILK/16000/1 G722/16000/1 PCMA/8000/1 PCMU/8000/1 speex/8000/1 SILK/8000/1 iLBC/8000/1 GSM/8000/1 G729/8000/1"
#endif

#ifndef _GLOBAL_CUSTOM
#define _GLOBAL_HELP_WEBSITE "http://www.microsip.org/help"
#endif

#ifndef _GLOBAL_NAME_NICE
#define _GLOBAL_NAME_NICE _GLOBAL_NAME
#endif

#ifdef _GLOBAL_ACCOUNT_API
#ifndef _GLOBAL_JSON
#define _GLOBAL_JSON
#endif
#endif

#ifdef _GLOBAL_BALANCE_PLAIN
#ifndef _GLOBAL_BALANCE
#define _GLOBAL_BALANCE
#endif
#endif

#ifdef _GLOBAL_BALANCE_REGISTER
#ifndef _GLOBAL_BALANCE
#define _GLOBAL_BALANCE
#endif
#endif

#ifdef _GLOBAL_BALANCE_OPTIONS
#ifndef _GLOBAL_BALANCE
#define _GLOBAL_BALANCE
#endif
#endif

#ifdef _GLOBAL_BALANCE
#ifndef _GLOBAL_BALANCE_IN_STATUSBAR
#define _GLOBAL_BALANCE_IN_DIALER
#endif
#endif

#ifdef _GLOBAL_BALANCE_IN_DIALER
#ifndef _GLOBAL_PADDING_TOP
#define _GLOBAL_PADDING_TOP 20
#endif
#endif

#ifdef _GLOBAL_BALANCE_PASSWORD_MD5
#ifndef _GLOBAL_MD5
#define _GLOBAL_MD5
#endif
#endif

#if !defined _GLOBAL_ACCOUNT_MINI || _GLOBAL_ACCOUNT_PUBLISH == 1
#define _GLOBAL_SUBSCRIBE
#endif

#if !defined _GLOBAL_TAB_HELP && !defined _GLOBAL_CUSTOM
#define _GLOBAL_TAB_HELP "http://www.microsip.org/help"
#endif
