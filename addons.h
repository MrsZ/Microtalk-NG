#include "const.h"

#ifdef _GLOBAL_ACCOUNT_REG
#include "addons/Reg1.h"
#include "addons/Reg2.h"
#include "addons/Reg3.h"
#endif

#if defined _GLOBAL_JSON || defined _GLOBAL_BALANCE_BEE || defined _GLOBAL_JSONRPC
#include "addons/jsoncpp/json/json.h"
#endif

#include "addons/CXMLFile/XMLFile.h"

#ifdef _GLOBAL_BALANCE
#include "addons/balance/balance.h"
#endif

#ifdef _GLOBAL_BALANCE_REGISTER
#include "addons/balance/balanceRegister.h"
#endif

#ifdef _GLOBAL_BALANCE_OPTIONS
#include "addons/balance/balanceOptions.h"
#endif

#ifdef _GLOBAL_CONFERENCE_DIALOG
#include "addons/Conference.h"
#endif

#ifdef _GLOBAL_PAGE_BUTTONS
#include "addons/itu/Buttons.h"
#endif

#ifdef _GLOBAL_JSONRPC
#include "addons/jsonrpc/jsonrpc.h"
#endif

#ifdef _GLOBAL_ACCOUNT_API
#include "addons/AccountAPI/AccountAPI.h"
#endif

#ifdef _GLOBAL_MD5
#include "addons/md5/md5Capi.h"
#endif
