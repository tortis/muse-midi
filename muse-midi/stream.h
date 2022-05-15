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
#ifndef _DMUSUART_STREAM_H_
#define _DMUSUART_STREAM_H_

#include "portcls.h"
#include <ntstrsafe.h>
#include "stdunk.h"
#include "dmusicks.h"

class CMiniportDMusUART;

/****************************************************************************
* CMiniportDMusUARTStream
*****************************************************************************
* MPU-401 miniport stream.  This object is associated with the pin and is
* created when the pin is instantiated.  It inherits IMXF
* so it can expose this interface and CUnknown so it automatically gets
* reference counting and aggregation support.
*/
class CMiniportDMusUARTStream :
    public IMXF,
    public CUnknown
{
private:
    CMiniportDMusUART* m_pMiniport;            // Parent.
    REFERENCE_TIME     m_SnapshotTimeStamp;    // Current snapshot of miniport's input timestamp.
    PUCHAR             m_pPortBase;            // Base port address.
    BOOLEAN            m_fCapture;             // Whether this is capture.
    PAllocatorMXF      m_AllocatorMXF;         // source/sink for DMus structs
    PMXF               m_sinkMXF;              // sink for DMus capture
    PMASTERCLOCK       m_MasterClock;          // for input data

public:
    /*************************************************************************
    * The following two macros are from STDUNK.H.  DECLARE_STD_UNKNOWN()
    * defines inline IUnknown implementations that use CUnknown's aggregation
    * support.  NonDelegatingQueryInterface() is declared, but it cannot be
    * implemented generically.  Its definition appears in MINIPORT.CPP.
    * DEFINE_STD_CONSTRUCTOR() defines inline a constructor which accepts
    * only the outer unknown, which is used for aggregation.  The standard
    * create macro (in MINIPORT.CPP) uses this constructor.
    */
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CMiniportDMusUARTStream);

    ~CMiniportDMusUARTStream();

    /*************************************************************************
    * IMXF methods
    */
    IMP_IMXF;
    
    NTSTATUS Init
    (
        _In_      CMiniportDMusUART * pMiniport,
        _In_      PUCHAR              pPortBase,
        _In_      BOOLEAN             fCapture,
        _In_      PAllocatorMXF       allocatorMXF,
        _In_      PMASTERCLOCK        masterClock
    );

    NTSTATUS HandlePortParams
    (
        _In_      PPCPROPERTY_REQUEST     Request
    );

    NTSTATUS GetMasterTimeStamp
    (
        _In_ REFERENCE_TIME* time
    );

private:
    NTSTATUS CopyEvent
    (
        _In_ PDMUS_KERNEL_EVENT  sourceEvent,
        _In_ PDMUS_KERNEL_EVENT* targetEvent
    );

    NTSTATUS Write
    (
        _In_  PDMUS_KERNEL_EVENT  writeEvent
    );
};

#endif // _DMUSUART_STREAM_H_
