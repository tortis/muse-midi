/*
  ==============================================================================
   This file is part of the Springbeats Virtual MIDI Driver (sbvmidi).
   Copyright (c) 2016 - Springbeats SARL.
   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3
   Details of these licenses can be found at: www.gnu.org/licenses
   Sbvmidi is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
   ------------------------------------------------------------------------------
   To release a closed-source product which uses sbvmidi, commercial licenses are
   available: visit www.springbeats.com for more information.
  ==============================================================================
*/
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
