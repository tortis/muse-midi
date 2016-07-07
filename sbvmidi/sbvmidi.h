/****************************************************************************
* Copyright (C) 2016 Autotheory
* All rights reserved
*
****************************************************************************/

#ifndef _DMUSUART_PRIVATE_H_
#define _DMUSUART_PRIVATE_H_

#include <portcls.h>

DRIVER_INITIALIZE DriverEntry;

DRIVER_ADD_DEVICE AddDevice;

NTSTATUS PropertyHandler_Synth
(
    _In_ PPCPROPERTY_REQUEST
);

NTSTATUS ValidatePropertyRequest
(
    _In_ PPCPROPERTY_REQUEST pRequest,
    _In_ ULONG               ulValueSize,
    _In_ BOOLEAN             fValueRequired
);

#endif  //  _DMusUART_PRIVATE_H_
