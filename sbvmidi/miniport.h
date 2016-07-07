/****************************************************************************
* Copyright (C) 2016 Autotheory
* All rights reserved
*
****************************************************************************/

#ifndef _DMUSUART_MINIPORT_H_
#define _DMUSUART_MINIPORT_H_

#define INITGUID 1
#include "portcls.h"
#include <ntstrsafe.h>
#include "stdunk.h"
#include "dmusicks.h"
#include "stream.h"

/*****************************************************************************
* CMiniportDMusUART
*****************************************************************************
* MPU-401 miniport.  This object is associated with the device and is
* created when the device is started.  The class inherits IMiniportDMus
* so it can expose this interface and CUnknown so it automatically gets
* reference counting and aggregation support.
*/
class CMiniportDMusUART :
    public IMiniportDMus,
    public IMusicTechnology,
    public IPowerNotify,
    public CUnknown
{
private:
    KSSTATE         m_KSStateInput;         // Miniport state (RUN/PAUSE/ACQUIRE/STOP)
    PPORTDMUS       m_pPort;                // Callback interface.
    PUCHAR          m_pPortBase;            // Base port address.
    PSERVICEGROUP   m_pServiceGroup;        // Service group for capture.
    REFERENCE_TIME  m_InputTimeStamp;       // capture data timestamp
    volatile SHORT  m_NumRenderStreams;     // Num active render streams.
    volatile SHORT  m_NumCaptureStreams;    // Num active capture streams.
    GUID            m_MusicFormatTechnology;
    POWER_STATE     m_PowerState;           // Saved power state (D0 = full power, D3 = off)
    PVOID           m_inputStream;
    PVOID           m_outputStream;

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
    DEFINE_STD_CONSTRUCTOR(CMiniportDMusUART);

    ~CMiniportDMusUART();

    /*************************************************************************
    * IMusicTechnology methods
    */
    IMP_IMiniportDMus;

    /*************************************************************************
    * IMusicTechnology methods
    */
    IMP_IMusicTechnology;

    /*************************************************************************
    * IPowerNotify methods
    */
    IMP_IPowerNotify;

    /*************************************************************************
    * CMiniportDMusUART methods
    *
    * These are private member functions used internally by the object.
    * See MINIPORT.CPP for specific descriptions.
    */

    NTSTATUS GetCaptureStream
    (
        _In_  CMiniportDMusUARTStream*  sourceStream,
        _Out_ CMiniportDMusUARTStream** targetStream
    );

    VOID StreamDestroying
    (
        _In_ BOOL  capture
    );

    VOID Notify();

    VOID SetKsState
    (
        _In_ KSSTATE state
    );

    KSSTATE GetKsState();

    PUCHAR GetPortBase();

private:
    NTSTATUS ProcessResources
    (
        _In_      PRESOURCELIST   ResourceList
    );
};

#endif // _DMUSUART_MINIPORT_H_
