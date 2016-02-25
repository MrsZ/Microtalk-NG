#include "stdafx.h"
#include "microsipDlg.h"
#include "settings.h"

#ifdef _GLOBAL_ACCOUNT_REG
#include "addons/Reg1.cpp"
#include "addons/Reg2.cpp"
#include "addons/Reg3.cpp"
#endif

#if defined _GLOBAL_JSON || defined _GLOBAL_BALANCE_BEE || defined _GLOBAL_JSONRPC
#include "addons/jsoncpp/json_reader.cpp"
#include "addons/jsoncpp/json_value.cpp"
#include "addons/jsoncpp/json_writer.cpp"
#endif

#include "addons/CXMLFile/XMLFile.cpp"

#ifdef _GLOBAL_BALANCE
#include "addons/balance/balance.cpp"
#endif

#ifdef _GLOBAL_BALANCE_PLAIN
#include "addons/balance/balancePlain.cpp"
#endif

#ifdef _GLOBAL_BALANCE_REGISTER
#include "addons/balance/balanceRegister.cpp"
#endif

#ifdef _GLOBAL_BALANCE_OPTIONS
#include "addons/balance/balanceOptions.cpp"
#endif

#ifdef _GLOBAL_CONFERENCE_DIALOG
#include "addons/Conference.cpp"
#endif

#ifdef _GLOBAL_PAGE_BUTTONS
#include "addons/itu/Buttons.cpp"
#endif

#ifdef _GLOBAL_JSONRPC
#include "addons/jsonrpc/jsonrpc.cpp"
#endif

#ifdef _GLOBAL_ACCOUNT_API
#include "addons/AccountAPI/AccountAPI.cpp"
#endif

#ifdef _GLOBAL_MD5
#include "addons/md5/md5Capi.cpp"
#endif
