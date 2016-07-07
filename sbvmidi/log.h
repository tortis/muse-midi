/****************************************************************************
* Copyright (C) 2016 Autotheory
* All rights reserved
*
****************************************************************************/

#ifndef _LOG_H_
#define _LOG_H_

#if DBG==1
#define MLOG(...) //{ DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, __VA_ARGS__); DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "\n");}
#else
#define MLOG(...) //{ DbgPrint( __VA_ARGS__); DbgPrint("\n");}
#endif

#endif // _LOG_H_