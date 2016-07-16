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
#include "stream.h"
#include "miniport.h"
#include "Log.h"
#include "sbvmidi.h"

/*****************************************************************************
* CMiniportDMusUARTStream::~CMiniportDMusUARTStream()
*****************************************************************************
* Destructs a stream.
*/
#pragma code_seg("PAGE")
CMiniportDMusUARTStream::~CMiniportDMusUARTStream(void)
{
    PAGED_CODE();

    LOG(DEBUG, ("~CMiniportDMusUARTStream"));

    if (m_AllocatorMXF)
    {
        m_AllocatorMXF->Release();
        m_AllocatorMXF = NULL;
    }

    if (m_pMiniport)
    {
        m_pMiniport->StreamDestroying(m_fCapture);
        m_pMiniport->Release();
    }
}


/*****************************************************************************
 * CMiniportDMusUARTStream::Write()
 *****************************************************************************
 * Writes outgoing MIDI data by forwarding it to the miniport driver,
 * which will delegate to the input stream
 */
#pragma code_seg()
NTSTATUS CMiniportDMusUARTStream::Write
(
    _In_  PDMUS_KERNEL_EVENT  writeEvent
)
{
    LOG(DEBUG, "Stream write.");

    NTSTATUS status = STATUS_SUCCESS;

    if (writeEvent == NULL)
    {
        LOG(ERROR, "Invalid writeEvent handle (null value).");
        return STATUS_INVALID_PARAMETER;
    }

    if (m_fCapture)
    {
        LOG(DEBUG, "Notifying miniport driver from stream...");
        status = m_sinkMXF->PutMessage(writeEvent);
        if (!NT_SUCCESS(status))
        {
            LOG(ERROR, "Failed to put message status=0x%x.", status);
        }
    }
    
    if (!m_fCapture || !NT_SUCCESS(status))
    {
        m_AllocatorMXF->PutMessage(writeEvent);
    }

    return status;
}

/*****************************************************************************
* CMiniportDMusUARTStream::CopyEvent()
*****************************************************************************
*/
#pragma code_seg()
NTSTATUS CMiniportDMusUARTStream::CopyEvent
(
    _In_  PDMUS_KERNEL_EVENT  sourceEvent,
    _Out_ PDMUS_KERNEL_EVENT *captureEvent
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PDMUS_KERNEL_EVENT newEvent = NULL;

    if (captureEvent == NULL)
    {
        LOG(ERROR, "Invalid captureEvent handle (null value).");
        return STATUS_INVALID_PARAMETER;
    }

    *captureEvent = NULL;

    if ((sourceEvent->cbEvent == 0) || (sourceEvent->uData.pbData == NULL) ||
        (sourceEvent->usFlags & DMUS_KEF_PACKAGE_EVENT))
    {
        return STATUS_SUCCESS;
    }

    __try
    {
        status = m_AllocatorMXF->GetMessage(&newEvent);
        if (!NT_SUCCESS(status) || (newEvent == NULL))
        {
            LOG(ERROR, "Invalid captureEvent handle (null value).");
            __leave;
        }

        if (sourceEvent->cbEvent > sizeof(PBYTE))
        {
            status = m_AllocatorMXF->GetBuffer(&newEvent->uData.pbData);
            if (!NT_SUCCESS(status) || (newEvent->uData.pbData == NULL))
            {
                LOG(ERROR, "Failed to get message buffer status=0x%x.", status);
                status = STATUS_INSUFFICIENT_RESOURCES;
                __leave;
            }

            RtlCopyMemory(newEvent->uData.pbData, sourceEvent->uData.pbData, sourceEvent->cbEvent);
        }
        else
        {
            newEvent->uData.pbData = sourceEvent->uData.pbData;
        }

        LOG(DEBUG, "Snap timestamp...");
        status = GetMasterTimeStamp(&newEvent->ullPresTime100ns);
        if (!NT_SUCCESS(status))
        {
            LOG(ERROR, "Failed to get timestamp status=0x%x.", status);
            __leave;
        }

        LOG(DEBUG, "Copying data to kernel event...");
        newEvent->cbEvent = sourceEvent->cbEvent;
        newEvent->usFlags = sourceEvent->usFlags;
        newEvent->usChannelGroup = 1;
    }
    __finally
    {
        if (!NT_SUCCESS(status))
        {
            if (newEvent != NULL)
            {
                if (!NT_SUCCESS(m_AllocatorMXF->PutMessage(newEvent)))
                {
                    LOG(ERROR, "Failed to put message back to allocator status=0x%x.", status);
                }
            }

            *captureEvent = newEvent;
        }
        else
        {
            *captureEvent = newEvent;
            m_pMiniport->Notify();
        }
    }

    return status;
}

/*****************************************************************************
 * CMiniportDMusUARTStream::SnapTimeStamp()
 *****************************************************************************
 *
 * At synchronized execution to ISR, copy miniport's volatile m_InputTimeStamp
 * to stream's m_SnapshotTimeStamp and zero m_InputTimeStamp.
 *
 */
#pragma code_seg()
NTSTATUS CMiniportDMusUARTStream::GetMasterTimeStamp
(
    _In_ REFERENCE_TIME* time
)
{
    return m_MasterClock->GetTime(time);
}

/*****************************************************************************
* CMiniportDMusUARTStream::Init()
*****************************************************************************
* Initializes a stream.
*/
#pragma code_seg("PAGE")
NTSTATUS CMiniportDMusUARTStream::Init
(
    _In_      CMiniportDMusUART * pMiniport,
    _In_      PUCHAR              pPortBase,
    _In_      BOOLEAN             fCapture,
    _In_      PAllocatorMXF       allocatorMXF,
    _In_      PMASTERCLOCK        masterClock
)
{
    PAGED_CODE();

    ASSERT(pMiniport);
    // ASSERT(pPortBase);

    LOG(DEBUG, ("Init"));

    m_pMiniport = pMiniport;
    m_pMiniport->AddRef();

    m_MasterClock = masterClock;

    m_pPortBase = pPortBase;
    m_fCapture = fCapture;

    m_SnapshotTimeStamp = 0;

    if (allocatorMXF)
    {
        allocatorMXF->AddRef();
        m_AllocatorMXF = allocatorMXF;
        m_sinkMXF = m_AllocatorMXF;
    }
    else
    {
        LOG(ERROR, "Failed to init stream.");
        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;
}

/*****************************************************************************
* CMiniportDMusUARTStream::ConnectOutput()
*****************************************************************************
* Writes outgoing MIDI data.
*/
#pragma code_seg("PAGE")
NTSTATUS CMiniportDMusUARTStream::ConnectOutput
(
    _In_  PMXF sinkMXF
)
{
    PAGED_CODE();

    NTSTATUS status = STATUS_UNSUCCESSFUL;

    LOG(DEBUG, "ConnectOutput");

    if (m_fCapture)
    {
        if (sinkMXF != NULL)
        {
            LOG(DEBUG, "Assigning m_sinKMXF");
            m_sinkMXF = sinkMXF;
            status = STATUS_SUCCESS;
        }
        else
        {
            LOG(ERROR, "ConnectOutput Failed");
        }
    }
    else
    {
        LOG(ERROR, "ConnectOutput called on renderer; failed");
    }

    return status;
}

/*****************************************************************************
* CMiniportDMusUARTStream::DisconnectOutput()
*****************************************************************************
* Writes outgoing MIDI data.
*/
#pragma code_seg("PAGE")
NTSTATUS CMiniportDMusUARTStream::DisconnectOutput
(
    _In_ PMXF sinkMXF
)
{
    PAGED_CODE();

    NTSTATUS status = STATUS_UNSUCCESSFUL;

    LOG(DEBUG, "DisconnectOutput");
    if (m_fCapture)
    {
        if ((m_sinkMXF == sinkMXF) || (!sinkMXF))
        {
            LOG(DEBUG, "Assigning allocator to sink")
                m_sinkMXF = m_AllocatorMXF;
            status = STATUS_SUCCESS;
        }
        else
        {
            LOG(ERROR, "DisconnectOutput failed");
        }
    }
    else
    {
        LOG(ERROR, "DisconnectOutput called on renderer; failed");
    }

    return status;
}

/*****************************************************************************
* CMiniportDMusUARTStream::HandlePortParams()
*****************************************************************************
* Writes an outgoing MIDI message.
*/
#pragma code_seg("PAGE")
NTSTATUS CMiniportDMusUARTStream::HandlePortParams
(
    _In_  PPCPROPERTY_REQUEST     pRequest
)
{
    PAGED_CODE();

    LOG(DEBUG, "HandlePortParams");
    NTSTATUS ntStatus;

    if (pRequest->Verb & KSPROPERTY_TYPE_SET)
    {
        LOG(ERROR, "Invalid property!")
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    ntStatus = ValidatePropertyRequest(pRequest, sizeof(SYNTH_PORTPARAMS), TRUE);
    if (NT_SUCCESS(ntStatus))
    {
        RtlCopyMemory(pRequest->Value, pRequest->Instance, sizeof(SYNTH_PORTPARAMS));

        PSYNTH_PORTPARAMS Params = (PSYNTH_PORTPARAMS)pRequest->Value;

        if (Params->ValidParams & ~SYNTH_PORTPARAMS_CHANNELGROUPS)
        {
            Params->ValidParams &= SYNTH_PORTPARAMS_CHANNELGROUPS;
        }

        if (!(Params->ValidParams & SYNTH_PORTPARAMS_CHANNELGROUPS))
        {
            Params->ChannelGroups = 1;
        }
        else if (Params->ChannelGroups != 1)
        {
            Params->ChannelGroups = 1;
        }

        pRequest->ValueSize = sizeof(SYNTH_PORTPARAMS);
    }

    return ntStatus;
}

/*****************************************************************************
* CMiniportDMusUARTStream::SetState()
*****************************************************************************
* Sets the state of the channel.
*/
#pragma code_seg("PAGE")
NTSTATUS CMiniportDMusUARTStream::SetState
(
    _In_ KSSTATE NewState
)
{
    PAGED_CODE();

    LOG(DEBUG, "SetState %d", NewState);

    if (NewState == KSSTATE_RUN)
    {
        LOG(DEBUG, "KSSTATE_RUN for stream");
    }

    if (m_fCapture)
    {
        m_pMiniport->SetKsState(NewState);
    }

    return STATUS_SUCCESS;
}

/*****************************************************************************
* CMiniportDMusUARTStream::NonDelegatingQueryInterface()
*****************************************************************************
* Obtains an interface.  This function works just like a COM QueryInterface
* call and is used if the object is not being aggregated.
*/
#pragma code_seg("PAGE")
NTSTATUS CMiniportDMusUARTStream::NonDelegatingQueryInterface
(
    _In_  REFIID  Interface,
    _Out_ PVOID*  Object
)
{
    PAGED_CODE();

    LOG(DEBUG, "Stream::NonDelegatingQueryInterface");
    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface, IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(this));
    }
    else if (IsEqualGUIDAligned(Interface, IID_IMXF))
    {
        *Object = PVOID(PMXF(this));
    }
    else
    {
        *Object = NULL;
    }

    if (*Object)
    {
        // We reference the interface for the caller.
        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

/*****************************************************************************
* CMiniportDMusUARTStream::PutMessage()
*********************************p********************************************
* Obtains an interface.  This function works just like a COM QueryInterface
* call and is used if the object is not being aggregated.
*/
#pragma code_seg()
NTSTATUS CMiniportDMusUARTStream::PutMessage
(
    _In_ PDMUS_KERNEL_EVENT pDMKEvt
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PDMUS_KERNEL_EVENT captureEvent = NULL;
    CMiniportDMusUARTStream* captureStream = NULL;

    LOG(DEBUG, "PutMessage with kernel event %p", pDMKEvt);

    if (pDMKEvt != NULL)
    {
        status = m_pMiniport->GetCaptureStream(this, &captureStream);
        if (NT_SUCCESS(status))
        {
            status = captureStream->CopyEvent(pDMKEvt, &captureEvent);
            if (!NT_SUCCESS(status))
            {
                LOG(ERROR, "Failed to copy event status=0x%x.", status);
                captureEvent = NULL;
            }
        }

    }

    status = m_AllocatorMXF->PutMessage(pDMKEvt);
    if (!NT_SUCCESS(status))
    {
        LOG(ERROR, "Failed to put message status=0x%x.", status);
    }

    if (captureEvent != NULL)
    {
        //Basically just write and free the allocator
        status = captureStream->Write(captureEvent);
        if (!NT_SUCCESS(status))
        {
            LOG(ERROR, "Failed to write to capture stream status=0x%x.", status);
        }
    }

    return status;
}
