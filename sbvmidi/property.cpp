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
#include "property.h"
#include "miniport.h"
#include "log.h"

/*****************************************************************************
* DirectMusic properties
****************************************************************************/
/*
*  Properties concerning synthesizer functions.
*/
const WCHAR wszDescOut[] = L"DMusic MPU-401 Out ";
const WCHAR wszDescIn[] = L"DMusic MPU-401 In ";


/*****************************************************************************
* PropertyHandler_Synth
****************************************************************************/
#pragma code_seg("PAGE")
NTSTATUS PropertyHandler_Synth
(
    _In_  PPCPROPERTY_REQUEST   pRequest
)
{
    NTSTATUS    ntStatus;

    PAGED_CODE();

    LOG(DEBUG, "PropertyHandler synth");

    if (pRequest->Verb & KSPROPERTY_TYPE_BASICSUPPORT)
    {
        ntStatus = ValidatePropertyRequest(pRequest, sizeof(ULONG), TRUE);
        if (NT_SUCCESS(ntStatus))
        {
            // if return buffer can hold a ULONG, return the access flags
            PULONG AccessFlags = PULONG(pRequest->Value);

            *AccessFlags = KSPROPERTY_TYPE_BASICSUPPORT;
            switch (pRequest->PropertyItem->Id)
            {
            case KSPROPERTY_SYNTH_CAPS:
            case KSPROPERTY_SYNTH_CHANNELGROUPS:
                *AccessFlags |= KSPROPERTY_TYPE_GET;
            }
            switch (pRequest->PropertyItem->Id)
            {
            case KSPROPERTY_SYNTH_CHANNELGROUPS:
                *AccessFlags |= KSPROPERTY_TYPE_SET;
            }
            ntStatus = STATUS_SUCCESS;
            pRequest->ValueSize = sizeof(ULONG);

            switch (pRequest->PropertyItem->Id)
            {
            case KSPROPERTY_SYNTH_PORTPARAMETERS:
                if (pRequest->MinorTarget)
                {
                    *AccessFlags |= KSPROPERTY_TYPE_GET;
                }
                else
                {
                    pRequest->ValueSize = 0;
                    ntStatus = STATUS_INVALID_DEVICE_REQUEST;
                }
            }
        }
    }
    else
    {
        ntStatus = STATUS_SUCCESS;
        switch (pRequest->PropertyItem->Id)
        {
        case KSPROPERTY_SYNTH_CAPS:
            LOG(DEBUG, "PropertyHandler_Synth:KSPROPERTY_SYNTH_CAPS");

            if (pRequest->Verb & KSPROPERTY_TYPE_SET)
            {
                ntStatus = STATUS_INVALID_DEVICE_REQUEST;
            }

            if (NT_SUCCESS(ntStatus))
            {
                ntStatus = ValidatePropertyRequest(pRequest, sizeof(SYNTHCAPS), TRUE);
                if (NT_SUCCESS(ntStatus))
                {
                    SYNTHCAPS *caps = (SYNTHCAPS*)pRequest->Value;
                    int increment;
                    RtlZeroMemory(caps, sizeof(SYNTHCAPS));
                    // XXX Different guids for different instances!
                    //
                    if (pRequest->Node == eSynthNode)
                    {
                        increment = sizeof(wszDescOut) - 2;
                        RtlCopyMemory(caps->Description, wszDescOut, increment);
                        caps->Guid = CLSID_MiniportDriverDMusUART;
                    }
                    else
                    {
                        increment = sizeof(wszDescIn) - 2;
                        RtlCopyMemory(caps->Description, wszDescIn, increment);
                        caps->Guid = CLSID_MiniportDriverDMusUARTCapture;
                    }

                    caps->Flags = SYNTH_PC_EXTERNAL;
                    caps->MemorySize = 0;
                    caps->MaxChannelGroups = 1;
                    caps->MaxVoices = 0xFFFFFFFF;
                    caps->MaxAudioChannels = 0xFFFFFFFF;

                    caps->EffectFlags = 0;

                    CMiniportDMusUART *aMiniport;
                    ASSERT(pRequest->MajorTarget);
                    aMiniport = (CMiniportDMusUART *)(PMINIPORTDMUS)(pRequest->MajorTarget);
                    WCHAR wszDesc2[16];
                    size_t cLen;
                    RtlStringCchPrintfW(wszDesc2, sizeof(wszDesc2) / sizeof(wszDesc2[0]), L"[%03X]\0", PtrToUlong(aMiniport->GetPortBase()));
                    ntStatus = RtlStringCchLengthW(wszDesc2, sizeof(wszDesc2) / sizeof(wszDesc2[0]), &cLen);
                    if (NT_SUCCESS(ntStatus))
                    {
#if _PREFAST_
                        __assume(cLen <= sizeof(wszDesc2));
#endif
                        RtlCopyMemory((WCHAR *)((DWORD_PTR)(caps->Description) + increment),
                            wszDesc2,
                            cLen);
                    }

                    pRequest->ValueSize = sizeof(SYNTHCAPS);
                }
            }

            break;

        case KSPROPERTY_SYNTH_PORTPARAMETERS:
            LOG(DEBUG, "PropertyHandler_Synth:KSPROPERTY_SYNTH_PORTPARAMETERS");
            {
                CMiniportDMusUARTStream *aStream;

                aStream = (CMiniportDMusUARTStream*)(pRequest->MinorTarget);
                if (aStream)
                {
                    ntStatus = aStream->HandlePortParams(pRequest);
                }
                else
                {
                    ntStatus = STATUS_INVALID_DEVICE_REQUEST;
                }
            }
            break;

        case KSPROPERTY_SYNTH_CHANNELGROUPS:
            LOG(DEBUG, "PropertyHandler_Synth:KSPROPERTY_SYNTH_CHANNELGROUPS");

            ntStatus = ValidatePropertyRequest(pRequest, sizeof(ULONG), TRUE);
            if (NT_SUCCESS(ntStatus))
            {
                *(PULONG)(pRequest->Value) = 1;
                pRequest->ValueSize = sizeof(ULONG);
            }
            break;

        case KSPROPERTY_SYNTH_LATENCYCLOCK:
            LOG(DEBUG, "PropertyHandler_Synth:KSPROPERTY_SYNTH_LATENCYCLOCK");

            if (pRequest->Verb & KSPROPERTY_TYPE_SET)
            {
                ntStatus = STATUS_INVALID_DEVICE_REQUEST;
            }
            else
            {
                ntStatus = ValidatePropertyRequest(pRequest, sizeof(ULONGLONG), TRUE);
                if (NT_SUCCESS(ntStatus))
                {
                    REFERENCE_TIME rtLatency = NULL;
                    CMiniportDMusUARTStream *aStream;

                    aStream = (CMiniportDMusUARTStream*)(pRequest->MinorTarget);
                    if (aStream == NULL)
                    {
                        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
                    }
                    else
                    {
                        aStream->GetMasterTimeStamp(&rtLatency);
                        *((PULONGLONG)pRequest->Value) = rtLatency;
                        pRequest->ValueSize = sizeof(ULONGLONG);
                    }
                }
            }
            break;

        default:
            LOG(WARN, "Unhandled property in PropertyHandler_Synth");
            break;
        }
    }

    return ntStatus;
}

/*****************************************************************************
* ValidatePropertyRequest()
*****************************************************************************
* Validates pRequest.
*  Checks if the ValueSize is valid
*  Checks if the Value is valid
*
*  This does not update pRequest->ValueSize if it returns NT_SUCCESS.
*  Caller must set pRequest->ValueSize in case of NT_SUCCESS.
*/
NTSTATUS ValidatePropertyRequest
(
    _In_  PPCPROPERTY_REQUEST     pRequest,
    _In_  ULONG                   ulValueSize,
    _In_  BOOLEAN                 fValueRequired
)
{
    PAGED_CODE();

    NTSTATUS    ntStatus;

    if (pRequest->ValueSize >= ulValueSize)
    {
        if (fValueRequired && NULL == pRequest->Value)
        {
            ntStatus = STATUS_INVALID_PARAMETER;
        }
        else
        {
            ntStatus = STATUS_SUCCESS;
        }
    }
    else  if (0 == pRequest->ValueSize)
    {
        ntStatus = STATUS_BUFFER_OVERFLOW;
    }
    else
    {
        ntStatus = STATUS_BUFFER_TOO_SMALL;
    }

    if (STATUS_BUFFER_OVERFLOW == ntStatus)
    {
        pRequest->ValueSize = ulValueSize;
    }
    else
    {
        pRequest->ValueSize = 0;
    }

    return ntStatus;
} // ValidatePropertyRequest
