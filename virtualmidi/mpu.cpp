/*****************************************************************************
 * MPU.cpp - UART miniport implementation
 *****************************************************************************
 * Copyright (c) 1998-2000 Microsoft Corporation.  All rights reserved.
 *
 */

#pragma warning (disable : 4127)

#include "private.h"
#include "ksdebug.h"

#define STR_MODULENAME "DMusUART:MPU: "

#define UartFifoOkForWrite(status)  ((status & MPU401_DRR) == 0)
#define UartFifoOkForRead(status)   ((status & MPU401_DSR) == 0)

typedef struct
{
    CMiniportDMusUART  *Miniport;
    PUCHAR              PortBase;
    PVOID               BufferAddress;
    ULONG               Length;
    PULONG              BytesRead;
}
SYNCWRITECONTEXT, *PSYNCWRITECONTEXT;

BOOLEAN  TryMPU(IN PUCHAR PortBase);
NTSTATUS WriteMPU(IN PUCHAR PortBase,IN BOOLEAN IsCommand,IN UCHAR Value);

#pragma code_seg("PAGE")
//  make sure we're in UART mode
NTSTATUS ResetHardware(PUCHAR portBase)
{
	UNREFERENCED_PARAMETER(portBase);
    PAGED_CODE();

	return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
//
// We initialize the UART with interrupts suppressed so we don't
// try to service the chip prematurely.
//
NTSTATUS CMiniportDMusUART::InitializeHardware(PINTERRUPTSYNC interruptSync,PUCHAR portBase)
{
	UNREFERENCED_PARAMETER(interruptSync);
	UNREFERENCED_PARAMETER(portBase);
	PAGED_CODE();
	return STATUS_SUCCESS;
}

#pragma code_seg()
/*****************************************************************************
 * InitMPU()
 *****************************************************************************
 * Synchronized routine to initialize the MPU401.
 */
NTSTATUS
InitMPU
(
    IN      PINTERRUPTSYNC  InterruptSync,
    IN      PVOID           DynamicContext
)
{
    UNREFERENCED_PARAMETER(InterruptSync);
	UNREFERENCED_PARAMETER(DynamicContext);

	return STATUS_SUCCESS;
}

#pragma code_seg()
/*****************************************************************************
 * CMiniportDMusUARTStream::Write()
 *****************************************************************************
 * Writes outgoing MIDI data.
 */
STDMETHODIMP_(NTSTATUS)
CMiniportDMusUARTStream::
Write
(
    IN      PVOID       BufferAddress,
    IN      ULONG       Length,
    OUT     PULONG      BytesWritten
)
{
	MLOG("Stream write. Length: %d",Length);
	ULONG i;
	PUCHAR cbuffer = (PUCHAR) BufferAddress;
	for (i = 0; i < Length; i++) {
		MLOG("Byte %d: %x", i, cbuffer[i]);
	}
    ASSERT(BytesWritten);
    if (!BufferAddress)
    {
        Length = 0;
    }


    if (!m_fCapture)
    {
		MLOG("Notifying miniport driver from stream...");
		m_pMiniport->ForwardOutputFromSource(this, BufferAddress, Length);
		*BytesWritten = Length;
		return STATUS_SUCCESS;
    }
    else   
    {
        return STATUS_INVALID_DEVICE_REQUEST;
    }
}

#pragma code_seg()
/*****************************************************************************
 * SynchronizedDMusMPUWrite()
 *****************************************************************************
 * Writes outgoing MIDI data.
 */
NTSTATUS
SynchronizedDMusMPUWrite
(
    IN      PINTERRUPTSYNC  InterruptSync,
    IN      PVOID           syncWriteContext
)
{
	UNREFERENCED_PARAMETER(InterruptSync);
	UNREFERENCED_PARAMETER(syncWriteContext);

	MLOG("SynchronizedDMusMPUWrite");
	return STATUS_SUCCESS;
}

#define kMPUPollTimeout 2

#pragma code_seg()
/*****************************************************************************
 * TryMPU()
 *****************************************************************************
 * See if the MPU401 is free.
 */
BOOLEAN
TryMPU
(
    IN      PUCHAR      PortBase
)
{
	UNREFERENCED_PARAMETER(PortBase);
	return TRUE;
}

#pragma code_seg()
/*****************************************************************************
 * WriteMPU()
 *****************************************************************************
 * Write a byte out to the MPU401.
 */
NTSTATUS
WriteMPU
(
    IN      PUCHAR      PortBase,
    IN      BOOLEAN     IsCommand,
    IN      UCHAR       Value
)
{
	UNREFERENCED_PARAMETER(PortBase);
	UNREFERENCED_PARAMETER(IsCommand);
	UNREFERENCED_PARAMETER(Value);

	return STATUS_SUCCESS;
}

#pragma code_seg()
/*****************************************************************************
 * SnapTimeStamp()
 *****************************************************************************
 *
 * At synchronized execution to ISR, copy miniport's volatile m_InputTimeStamp
 * to stream's m_SnapshotTimeStamp and zero m_InputTimeStamp.
 *
 */
STDMETHODIMP_(NTSTATUS)
SnapTimeStamp(PINTERRUPTSYNC InterruptSync,PVOID pStream)
{
    UNREFERENCED_PARAMETER(InterruptSync);

    CMiniportDMusUARTStream *pMPStream = (CMiniportDMusUARTStream *)pStream;

    //  cache the timestamp
    pMPStream->m_SnapshotTimeStamp = pMPStream->m_pMiniport->m_InputTimeStamp;

    //  if the window is closed, zero the timestamp
    if (pMPStream->m_pMiniport->m_MPUInputBufferHead ==
        pMPStream->m_pMiniport->m_MPUInputBufferTail)
    {
        pMPStream->m_pMiniport->m_InputTimeStamp = 0;
    }

    return STATUS_SUCCESS;
}

/*****************************************************************************
 * CMiniportDMusUARTStream::SourceEvtsToPort()
 *****************************************************************************
 *
 * Reads incoming MIDI data, feeds into DMus events.
 * No need to touch the hardware, just read from our SW FIFO.
 *
 */
VOID CMiniportDMusUARTStream::ForwardEventsToPort(IN PVOID inputBuffer, IN ULONG Length) {
	MLOG("Input stream, forward events to port (%p,%d)...", inputBuffer, Length);
	PDMUS_KERNEL_EVENT  aDMKEvt;
	PBYTE bbuffer = (PBYTE)inputBuffer;
	NTSTATUS ntStatus = STATUS_SUCCESS;

	m_AllocatorMXF->GetMessage(&aDMKEvt);
	if (Length > sizeof(PBYTE)) {
		MLOG("Message is too long. Max: %d. Current: %d", sizeof(PBYTE), Length);
	}
	else {
		MLOG("Copying data to kernel event...");
		for (aDMKEvt->cbEvent = 0; aDMKEvt->cbEvent < Length; aDMKEvt->cbEvent++) {
			aDMKEvt->uData.abData[aDMKEvt->cbEvent] = bbuffer[aDMKEvt->cbEvent];
		}
		MLOG("Snap timestamp...");
		ntStatus = SnapTimeStamp(NULL, PVOID(this));
		aDMKEvt->ullPresTime100ns = m_SnapshotTimeStamp;
		aDMKEvt->usChannelGroup = 1;
		aDMKEvt->usFlags = DMUS_KEF_EVENT_INCOMPLETE;
		MLOG("Fowarding message to port driver...")
		(void)m_sinkMXF->PutMessage(aDMKEvt);

	}
}

STDMETHODIMP_(NTSTATUS)
CMiniportDMusUARTStream::SourceEvtsToPort()
{
    NTSTATUS    ntStatus;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    _DbgPrintF(DEBUGLVL_BLAB, ("SourceEvtsToPort"));

    if (m_fCapture)
    {
        ntStatus = STATUS_SUCCESS;
        if (m_pMiniport->m_MPUInputBufferHead != m_pMiniport->m_MPUInputBufferTail)
        {
            PDMUS_KERNEL_EVENT  aDMKEvt,eventTail,eventHead = NULL;

            while (m_pMiniport->m_MPUInputBufferHead != m_pMiniport->m_MPUInputBufferTail)
            {
                (void) m_AllocatorMXF->GetMessage(&aDMKEvt);
                if (!aDMKEvt)
                {
                    _DbgPrintF(DEBUGLVL_TERSE, ("SourceEvtsToPort can't allocate DMKEvt"));
                    return STATUS_INSUFFICIENT_RESOURCES;
                }

                //  put this event at the end of the list
                if (!eventHead)
                {
                    eventHead = aDMKEvt;
                }
                else
                {
                    eventTail = eventHead;
                    while (eventTail->pNextEvt)
                    {
                        eventTail = eventTail->pNextEvt;
                    }
                    eventTail->pNextEvt = aDMKEvt;
                }
                //  read all the bytes out of the buffer, into event(s)
                for (aDMKEvt->cbEvent = 0; aDMKEvt->cbEvent < sizeof(PBYTE); aDMKEvt->cbEvent++)
                {
                    if (m_pMiniport->m_MPUInputBufferHead == m_pMiniport->m_MPUInputBufferTail)
                    {
//                        _DbgPrintF(DEBUGLVL_TERSE, ("SourceEvtsToPort m_MPUInputBufferHead met m_MPUInputBufferTail, overrun"));
                        break;
                    }
                    aDMKEvt->uData.abData[aDMKEvt->cbEvent] = m_pMiniport->m_MPUInputBuffer[m_pMiniport->m_MPUInputBufferHead];
                    m_pMiniport->m_MPUInputBufferHead++;
                    if (m_pMiniport->m_MPUInputBufferHead >= kMPUInputBufferSize)
                    {
                        m_pMiniport->m_MPUInputBufferHead = 0;
                    }
                }
            }

            if (m_pMiniport->m_UseIRQ)
            {
                ntStatus = m_pMiniport->m_pInterruptSync->CallSynchronizedRoutine(SnapTimeStamp,PVOID(this));
            }
            else    //  !m_UseIRQ
            {
                ntStatus = SnapTimeStamp(NULL,PVOID(this));
            }       //  !m_UseIRQ
            aDMKEvt = eventHead;
            while (aDMKEvt)
            {
                aDMKEvt->ullPresTime100ns = m_SnapshotTimeStamp;
                aDMKEvt->usChannelGroup = 1;
                aDMKEvt->usFlags = DMUS_KEF_EVENT_INCOMPLETE;
                aDMKEvt = aDMKEvt->pNextEvt;
            }
            (void)m_sinkMXF->PutMessage(eventHead);
        }
    }
    else    //  render stream
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("SourceEvtsToPort called on render stream"));
        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    }
    return ntStatus;
}

#pragma code_seg()
/*****************************************************************************
 * DMusMPUInterruptServiceRoutine()
 *****************************************************************************
 * ISR.
 */
NTSTATUS
DMusMPUInterruptServiceRoutine
(
    IN      PINTERRUPTSYNC  InterruptSync,
    IN      PVOID           DynamicContext
)
{
    UNREFERENCED_PARAMETER(InterruptSync);

    _DbgPrintF(DEBUGLVL_BLAB, ("DMusMPUInterruptServiceRoutine"));
    ULONGLONG   startTime;

    ASSERT(DynamicContext);

    NTSTATUS            ntStatus;
    BOOL                newBytesAvailable;
    CMiniportDMusUART   *that;
    NTSTATUS            clockStatus;

    that = (CMiniportDMusUART *) DynamicContext;
    newBytesAvailable = FALSE;
    ntStatus = STATUS_UNSUCCESSFUL;

    UCHAR portStatus = 0xff;

    //
    // Read the MPU status byte.
    //
    if (that->m_pPortBase)
    {
        portStatus =
            READ_PORT_UCHAR(that->m_pPortBase + MPU401_REG_STATUS);

        //
        // If there is outstanding work to do and there is a port-driver for
        // the MPU miniport...
        //
        if (UartFifoOkForRead(portStatus) && that->m_pPort)
        {
            startTime = PcGetTimeInterval(0);
            while ( (PcGetTimeInterval(startTime) < GTI_MILLISECONDS(50))
                &&  (UartFifoOkForRead(portStatus)) )
            {
                UCHAR uDest = READ_PORT_UCHAR(that->m_pPortBase + MPU401_REG_DATA);
                if (    (that->m_KSStateInput == KSSTATE_RUN)
                    &&  (that->m_NumCaptureStreams)
                   )
                {
                    LONG    buffHead = that->m_MPUInputBufferHead;
                    if (   (that->m_MPUInputBufferTail + 1 == buffHead)
                        || (that->m_MPUInputBufferTail + 1 - kMPUInputBufferSize == buffHead))
                    {
                        _DbgPrintF(DEBUGLVL_TERSE,("*****MPU Input Buffer Overflow*****"));
                    }
                    else
                    {
                        if (!that->m_InputTimeStamp)
                        {
                            clockStatus = that->m_MasterClock->GetTime(&that->m_InputTimeStamp);
                            if (STATUS_SUCCESS != clockStatus)
                            {
                                _DbgPrintF(DEBUGLVL_TERSE,("GetTime failed for clock 0x%08p",that->m_MasterClock));
                            }
                        }
                        newBytesAvailable = TRUE;
                        //  ...place the data in our FIFO...
                        that->m_MPUInputBuffer[that->m_MPUInputBufferTail] = uDest;
                        ASSERT(that->m_MPUInputBufferTail < kMPUInputBufferSize);

                        that->m_MPUInputBufferTail++;
                        if (that->m_MPUInputBufferTail >= kMPUInputBufferSize)
                        {
                            that->m_MPUInputBufferTail = 0;
                        }
                    }
                }
                //
                // Look for more MIDI data.
                //
                portStatus =
                    READ_PORT_UCHAR(that->m_pPortBase + MPU401_REG_STATUS);
            }   //  either there's no data or we ran too long
            if (newBytesAvailable)
            {
                //
                // ...notify the MPU port driver that we have bytes.
                //
                that->m_pPort->Notify(that->m_pServiceGroup);
            }
            ntStatus = STATUS_SUCCESS;
        }
    }

    return ntStatus;
}
