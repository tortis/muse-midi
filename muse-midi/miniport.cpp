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
#define INITGUID 1
#include "property.h"
#include "portcls.h"
#include <ntstrsafe.h>
#include "stdunk.h"
#include "dmusicks.h"
#include "ksdebug.h"
#include "stdio.h"
#include "stream.h"
#include "miniport.h"
#include "log.h"

/*****************************************************************************
 * CMiniportDMusUART::GetDescription()
 *****************************************************************************
 * Gets the topology.
 */
#pragma code_seg("PAGE")
NTSTATUS CMiniportDMusUART::GetDescription
(
    _Out_     PPCFILTER_DESCRIPTOR *  OutFilterDescriptor
)
{
    PAGED_CODE();

    ASSERT(OutFilterDescriptor);

    LOG(DEBUG, "GetDescription");

    *OutFilterDescriptor = &MiniportFilterDescriptor;

    return STATUS_SUCCESS;
}

/*****************************************************************************
* CMiniportDMusUART::StreamDestroying()
*****************************************************************************
*/
#pragma code_seg("PAGE")
VOID CMiniportDMusUART::StreamDestroying
(
    _In_ BOOL capture
)
{
    PAGED_CODE();

    LOG(DEBUG, "Destroying stream...");
    if (capture)
    {
        LOG(INFO, "Destroying input stream...");
        m_inputStream = NULL;
        InterlockedDecrement16(&m_NumCaptureStreams);
    }
    else
    {
        LOG(INFO, "Destroying output stream...");
        m_outputStream = NULL;
        InterlockedDecrement16(&m_NumRenderStreams);
    }
}

/*****************************************************************************
* CMiniportDMusUART::GetCaptureStream()
*****************************************************************************
*/
#pragma code_seg()
NTSTATUS CMiniportDMusUART::GetCaptureStream
(
    _In_  CMiniportDMusUARTStream*  sourceStream,
    _Out_ CMiniportDMusUARTStream** targetStream 
)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    if (targetStream == NULL)
    {
        LOG(ERROR, "Invalid stream handle (null value).");
        return STATUS_INVALID_PARAMETER;
    }

    *targetStream = NULL;

    if (sourceStream != m_outputStream)
    {
        LOG(ERROR, "Source is correct: output stream");
        return STATUS_INVALID_PARAMETER;
    }

    if (m_inputStream != NULL)
    {
        LOG(DEBUG, "Input stream is not null, forwarding event...");
        *targetStream = reinterpret_cast<CMiniportDMusUARTStream*>(m_inputStream);
        status = STATUS_SUCCESS;
    }
    
    return status;
}

/*****************************************************************************
 * CMiniportDMusUART::ProcessResources()
 *****************************************************************************
 * Processes the resource list, setting up helper objects accordingly.
 */
#pragma code_seg("PAGE")
NTSTATUS CMiniportDMusUART::ProcessResources
(
    _In_      PRESOURCELIST   ResourceList
)
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(ResourceList);
    LOG(DEBUG, "ProcessResources");
    //Here we are supposed to call: initmpu, resethardware
    //We do not use any physical resouces
    return STATUS_SUCCESS;
}

/*****************************************************************************
* CMiniportDMusUART::PowerChangeNotify()
*****************************************************************************
*/
#pragma code_seg("PAGE")
void CMiniportDMusUART::PowerChangeNotify
(
    _In_    POWER_STATE  PowerState
)
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(PowerState);

    LOG(DEBUG, "CMiniportDMusUART::PoweChangeNotify D%d", PowerState.DeviceState);
} 

/*****************************************************************************
* CMiniportDMusUART::NonDelegatingQueryInterface()
*****************************************************************************
*/
#pragma code_seg("PAGE")
NTSTATUS CMiniportDMusUART::NonDelegatingQueryInterface
(
    _In_           REFIID  Interface,
    _COM_Outptr_   PVOID * Object
)
{
    PAGED_CODE();

    LOG(DEBUG, "Miniport::NonDelegatingQueryInterface");
    ASSERT(Object);

    /*GUID input = (GUID)Interface;
    LOG(DEBUG, "Requested GUI: %x-%x-%x", input.Data1, input.Data2, input.Data3);*/

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        LOG(INFO, "Requested IID_IUnknown interface...");
        *Object = PVOID(PUNKNOWN(PMINIPORTDMUS(this)));
    }
    else if (IsEqualGUIDAligned(Interface,IID_IMiniport))
    {
        LOG(INFO, "Requested IID_IMiniport interface...");
        *Object = PVOID(PMINIPORT(this));
    }
    else if (IsEqualGUIDAligned(Interface,IID_IMiniportDMus))
    {
        LOG(INFO, "Requested IID_IMiniportDMus interface...");

        *Object = PVOID(PMINIPORTDMUS(this));
    }
    else if (IsEqualGUIDAligned(Interface,IID_IMusicTechnology))
    {
        LOG(INFO, "Requested IID_IMusicTechnology interface...");

        *Object = PVOID(PMUSICTECHNOLOGY(this));
    }
    else if (IsEqualGUIDAligned(Interface, IID_IPowerNotify))
    {
        LOG(INFO, "Requested IID_IPowerNotify interface...");

        *Object = PVOID(PPOWERNOTIFY(this));
    }
    else
    {
        LOG(INFO, "Unknown interface, null...");

        *Object = NULL;
    }

    if (*Object)
    {
        // We reference the interface for the caller.
        LOG(DEBUG, "Success in nondelegatingqueryinterface")
        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

/*****************************************************************************
 * CMiniportDMusUART::~CMiniportDMusUART()
 *****************************************************************************
 * Destructor.
 */
#pragma code_seg("PAGE")
CMiniportDMusUART::~CMiniportDMusUART(void)
{
    PAGED_CODE();

    ASSERT(0 == m_NumCaptureStreams);
    ASSERT(0 == m_NumRenderStreams);

    //  reset the HW so we don't get anymore interrupts
  
    if (m_pServiceGroup)
    {
        m_pServiceGroup->Release();
        m_pServiceGroup = NULL;
    }
    if (m_pPort)
    {
        m_pPort->Release();
        m_pPort = NULL;
    }
}

/*****************************************************************************
 * CMiniportDMusUART::Init()
 ********************************************************ii*********************
 * Initializes a the miniport.
 */
#pragma code_seg("PAGE")
NTSTATUS CMiniportDMusUART::Init
(
    _In_opt_  PUNKNOWN        UnknownInterruptSync,
    _In_      PRESOURCELIST   ResourceList,
    _In_      PPORTDMUS       Port_,
    _Out_     PSERVICEGROUP*  ServiceGroup
)
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(UnknownInterruptSync);

    LOG(DEBUG, "Initializing the miniport...");
    ASSERT(ResourceList);
    
    if (!ResourceList)
    {
        LOG(ERROR, "ResourceList is null, configuration error...");
        return STATUS_DEVICE_CONFIGURATION_ERROR;
    }

    ASSERT(Port_);
    ASSERT(ServiceGroup);

    *ServiceGroup = NULL;
    m_pPortBase = 0;

    // This will remain unspecified if the miniport does not get any power
    // messages.
    //
    //Possible problem here.
    m_PowerState.DeviceState = PowerDeviceUnspecified;

    // AddRef() is required because we are keeping this pointer.
    m_pPort = Port_;
    m_pPort->AddRef();

    // Set dataformat.
    if (IsEqualGUIDAligned(m_MusicFormatTechnology, GUID_NULL))
    {
        LOG(DEBUG, "MusicFormatTechnology is null, using technology_port");
        RtlCopyMemory(  &m_MusicFormatTechnology,
                        &KSMUSIC_TECHNOLOGY_PORT,
                        sizeof(GUID));
    }
    RtlCopyMemory(  &PinDataRangesStreamLegacy.Technology,
                    &m_MusicFormatTechnology,
                    sizeof(GUID));
    RtlCopyMemory(  &PinDataRangesStreamDMusic.Technology,
                    &m_MusicFormatTechnology,
                    sizeof(GUID));
    m_InputTimeStamp = 0;
    m_KSStateInput = KSSTATE_STOP;

    NTSTATUS ntStatus = STATUS_SUCCESS;

    m_NumRenderStreams = 0;
    m_NumCaptureStreams = 0;

    LOG(DEBUG, "New service group...");
    ntStatus = PcNewServiceGroup(&m_pServiceGroup,NULL);
    if (NT_SUCCESS(ntStatus) && !m_pServiceGroup)   //  keep any error
    {
        LOG(ERROR, "Could not allocate service group...");
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (NT_SUCCESS(ntStatus))
    {
        *ServiceGroup = m_pServiceGroup;
        m_pServiceGroup->AddRef();

        //
        // Register the service group with the port early so the port is
        // prepared to handle interrupts.
        //
        LOG(DEBUG, "Registering service group (necessary)?");
        m_pPort->RegisterServiceGroup(m_pServiceGroup);
    }

    if (NT_SUCCESS(ntStatus))
    {
        LOG(DEBUG, "Process resources");
        ntStatus = ProcessResources(ResourceList);
    }

    LOG(DEBUG, "Ending init function...");
    if (!NT_SUCCESS(ntStatus))
    {
        LOG(DEBUG, "Ending init function with error.");

        // clean up our mess

        // clean up the service group
        if( m_pServiceGroup )
        {
            m_pServiceGroup->Release();
            m_pServiceGroup = NULL;
        }

        // clean up the out param service group.
        if (*ServiceGroup)
        {
            (*ServiceGroup)->Release();
            (*ServiceGroup) = NULL;
        }

        // release the port
        m_pPort->Release();
        m_pPort = NULL;
    }

    return ntStatus;
}

/*****************************************************************************
 * CMiniportDMusUART::NewStream()
 *****************************************************************************
 * Gets the topology.
 */
#pragma code_seg("PAGE")
NTSTATUS CMiniportDMusUART::NewStream
(
    _Out_     PMXF                  * MXF,
    _In_opt_  PUNKNOWN                OuterUnknown,
    _In_      POOL_TYPE               PoolType,
    _In_      ULONG                   PinID,
    _In_      DMUS_STREAM_TYPE        StreamType,
    _In_      PKSDATAFORMAT           DataFormat,
    _Out_     PSERVICEGROUP         * ServiceGroup,
    _In_      PAllocatorMXF           AllocatorMXF,
    _In_      PMASTERCLOCK            MasterClock,
    _Out_     PULONGLONG              SchedulePreFetch
)
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(DataFormat);
    UNREFERENCED_PARAMETER(PinID);
    NTSTATUS ntStatus = STATUS_SUCCESS;

    // In 100 ns, we want stuff as soon as it comes in
    *SchedulePreFetch = 0;
    if (((m_NumCaptureStreams < kMaxNumCaptureStreams)
        && (StreamType == DMUS_STREAM_MIDI_CAPTURE))
        || ((m_NumRenderStreams < kMaxNumLegacyRenderStreams + kMaxNumDMusicRenderStreams)
        && (StreamType == DMUS_STREAM_MIDI_RENDER)))
    {
        LOG(DEBUG, "Creating new stream...");
        CMiniportDMusUARTStream *pStream =
            new(PoolType)CMiniportDMusUARTStream(OuterUnknown);
        if (pStream == NULL)
        {
            LOG(DEBUG, "Insufficient resources...");
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        LOG(DEBUG, "Stream allocated., initializing");
        pStream->AddRef();

        ntStatus = pStream->Init(this, m_pPortBase, (StreamType == DMUS_STREAM_MIDI_CAPTURE), AllocatorMXF, MasterClock);
        if (NT_SUCCESS(ntStatus))
        {
            *MXF = PMXF(pStream);
            (*MXF)->AddRef();

            LOG(DEBUG, "Stream correctly initialized.");
            if (StreamType == DMUS_STREAM_MIDI_CAPTURE)
            {
                m_inputStream = pStream;
                LOG(DEBUG, "Stream type is DMUS capture, adding service group.");
                m_NumCaptureStreams++;
                *ServiceGroup = m_pServiceGroup;
                (*ServiceGroup)->AddRef();
            }
            else
            {
                m_outputStream = pStream;
                m_NumRenderStreams++;
                *ServiceGroup = NULL;
            }
        }

        pStream->Release();
    }
    else
    {
        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
        if (StreamType == DMUS_STREAM_MIDI_CAPTURE)
        {
           LOG(ERROR, "NewStream failed, too many capture streams");
        }
        else if (StreamType == DMUS_STREAM_MIDI_RENDER)
        {
            LOG(ERROR, "NewStream failed, too many render streams");
        }
        else
        {
            LOG(ERROR, "NewStream invalid stream type");
        }
    }

    return ntStatus;
}

/*****************************************************************************
 * CMiniportDMusUART::SetTechnology()
 *****************************************************************************
 * Sets pindatarange technology.
 */
#pragma code_seg("PAGE")
NTSTATUS CMiniportDMusUART::SetTechnology
(
    _In_ const GUID* Technology
)
{
    PAGED_CODE();

    LOG(DEBUG, "SetTechnology");
    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

    // Fail if miniport has already been initialized.
    //
    if (NULL == m_pPort)
    {
        RtlCopyMemory(&m_MusicFormatTechnology, Technology, sizeof(GUID));
        ntStatus = STATUS_SUCCESS;
    }

    return ntStatus;
} // SetTechnology

/*****************************************************************************
 * CMiniportDMusUART::Service()
 *****************************************************************************
 * DPC-mode service call from the port.
 */
#pragma code_seg()
void CMiniportDMusUART::Service()
{
    LOG(DEBUG, "Service");
}

/*****************************************************************************
* CMiniportDMusUART::DataRangeIntersection()
*****************************************************************************
*/
#pragma code_seg("PAGE_CODE")
NTSTATUS CMiniportDMusUART::DataRangeIntersection
(
    _In_      ULONG           PinId,
    _In_      PKSDATARANGE    DataRange,
    _In_      PKSDATARANGE    MatchingDataRange,
    _In_      ULONG           OutputBufferLength,
    _Out_opt_ PVOID           ResultantFormat,
    _Out_     PULONG          ResultantFormatLength
)
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(PinId);
    UNREFERENCED_PARAMETER(DataRange);
    UNREFERENCED_PARAMETER(MatchingDataRange);
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(ResultantFormat);
    UNREFERENCED_PARAMETER(ResultantFormatLength);

    return STATUS_NOT_IMPLEMENTED;
}

/*****************************************************************************
* CMiniportDMusUART::SetKsState()
*****************************************************************************
* Sets ks input stream state.
*/
#pragma code_seg("PAGE")
VOID CMiniportDMusUART::SetKsState
(
    _In_ KSSTATE state
)
{
    PAGED_CODE();

    m_KSStateInput = state;
}

/*****************************************************************************
* CMiniportDMusUART::GetKsState()
*****************************************************************************
* Gets ks input stream state.
*/
#pragma code_seg("PAGE")
KSSTATE CMiniportDMusUART::GetKsState()
{
    PAGED_CODE();

    return m_KSStateInput;
}

/*****************************************************************************
* Notify()
******************************************************************************
*/
#pragma code_seg()
void CMiniportDMusUART::Notify()
{
    m_pPort->Notify(m_pServiceGroup);
}

/*****************************************************************************
* GetPortBase()
******************************************************************************
*/
#pragma code_seg("PAGE")
PUCHAR CMiniportDMusUART::GetPortBase()
{
    PAGED_CODE();

    return m_pPortBase;
}
