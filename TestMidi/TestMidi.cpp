// TestMidi.cpp: define el punto de entrada de la aplicación de consola.
//

#include "stdafx.h"
#include <windows.h>
#include <mmsystem.h>

static ULONG ncallbacks = 0;
void CALLBACK midiInCallback(
	HMIDIOUT hmo,
	UINT wMsg,
	DWORD_PTR dwInstance,
	DWORD_PTR dwParam1,
	DWORD_PTR dwParam2
	) {
	PMIDIHDR header;
	unsigned int i;
	printf("MIDI CALLBACK! Number of callbacks: %d\n",++ncallbacks);
	switch (wMsg) {
	case MIM_CLOSE: printf("MIM_CLOSE\n"); break;
	case MIM_DATA: 
		printf("MIM_DATA\n"); 
		printf("Timestamp: %d\n", dwParam2);
		printf("MIDI data: %X\n", dwParam1);
		break;
	case MIM_ERROR:printf("MIM_ERROR\n"); break;
	case MIM_LONGDATA:
		printf("MIM_LONGDATA\n"); 
		printf("Timestamp: %d\n", dwParam2);
		header = (PMIDIHDR)dwParam1;
		printf("Bytes recorded: %d\n", header->dwBytesRecorded);
		for (i = 0; i < header->dwBytesRecorded;i++) {
			printf("Message[%d]=%x\n", i, header->lpData[i]);
		}
		break;
	case MIM_LONGERROR:printf("MIM_LONGERROR\n"); break;
	case MIM_MOREDATA:printf("MIM_MOREDATA\n"); break;
	case MIM_OPEN:printf("MIM_OPEN\n"); break;
	}
}
#define MIDIHDR_CNT 10
//#define MIDI_PORT L"loopMIDI Port"
#define MIDI_PORT L"MIDI1"

MIDIHDR mMidiHdrs[MIDIHDR_CNT];
int _tmain(int argc, _TCHAR* argv[])
{
	unsigned int num_long_msg = 2;
	unsigned int ncurr_long_msg = 0;
	HMIDIIN inHandle = 0;
	unsigned int num = midiInGetNumDevs();
	printf("Number of MIDI In devices: %d\n ", num);
	printf("Searching for port: %S\n", MIDI_PORT);
	printf("Printing MIDI In device names...");
	for (UINT i = 0; i < num; ++i)
	{
		MIDIINCAPS mc = { 0 };

		if (midiInGetDevCaps(i, &mc, sizeof (mc)) == MMSYSERR_NOERROR)
			printf("%ls\n", mc.szPname);
		//If we find our device, set a callback
		if (!wcscmp(mc.szPname, MIDI_PORT )) {
			printf("Found input device\n!");

			MMRESULT err = midiInOpen(&inHandle, i,
				(DWORD_PTR)midiInCallback,
				0,
				CALLBACK_FUNCTION);
			//Prepare some midi IN buffers
			const int kDataBufLen = 512;
			int idx;
			for (idx = 0; idx < MIDIHDR_CNT; ++idx)
			{
				mMidiHdrs[idx].lpData = (LPSTR) ::malloc(kDataBufLen);
				mMidiHdrs[idx].dwBufferLength = kDataBufLen;
				MMRESULT res;
				res = midiInPrepareHeader(inHandle, &mMidiHdrs[idx], (UINT)sizeof(MIDIHDR));
				res = midiInAddBuffer(inHandle, &mMidiHdrs[idx], sizeof(MIDIHDR));
			}
		}
	}

	num = midiOutGetNumDevs();
	printf("Number of MIDI Out devices: %d\n ", num);
	printf("Printing MIDI Out device names...");
	int id = 0;
	for (UINT i = 0; i < num; ++i)
	{
		MIDIOUTCAPS mc = { 0 };

		if (midiOutGetDevCaps(i, &mc, sizeof (mc)) == MMSYSERR_NOERROR)
			printf("%ls\n", mc.szPname);
		if (!wcscmp(mc.szPname,MIDI_PORT)) {
			printf("Found output device\n!");
			HMIDIOUT handle;
			printf("Opening...");
			MMRESULT ret= midiOutOpen(
				&handle,
				i,
				0,
				0,
				CALLBACK_NULL
				);
			printf("Opened\n");
			if (ret == MMSYSERR_NOERROR) {
				printf("Open successful\n");
			}
			else {
				printf("Open failed: %d", ret);
				return 1;
			}
			printf("Starting IN MIDI...\n");
			MMRESULT res = midiInStart(inHandle);

			printf("Sending chord...\n");
			midiOutShortMsg(handle, 0x00403C90);
			midiOutShortMsg(handle, 0x00404090);
			midiOutShortMsg(handle, 0x00404390);

			//In a real device, the notes will start sounding here. Wait a bit, then close the notes
			Sleep(1000);

			printf("Turning off chord...\n");
			//Turn them off. Volume 0.
			midiOutShortMsg(handle, 0x00003C90);
			midiOutShortMsg(handle, 0x00004090);
			midiOutShortMsg(handle, 0x00004390);
			//First byte is status: 9x is turning a note on, in this case for channel 0. 
			//Second byte is the note itself: 3c=60=Middle Do (C)
			//Third byte is note "velocity", usually implemented as volume
			//Fourth byte (MSB) is unused.
		resend_message:
			ncurr_long_msg++;
			MIDIHDR h = { 0 };
			HANDLE hbuffer = NULL;
			if (ncurr_long_msg > 2) goto end;
			Sleep(1000);
			printf("Sending SysEx message...\n");
			char message[] = { 0xF0, 0x41, 0x10, 0x042, 0x12, 0x040, 0x00, 0x7f, 0x00, 0x01, 0x41, 0xF7};
			hbuffer = GlobalAlloc(GHND, sizeof(message));
			h.lpData=(LPSTR)GlobalLock(hbuffer);
			h.dwBytesRecorded = h.dwBufferLength = sizeof(message);
			h.dwFlags = 0;
			if (midiOutPrepareHeader(handle, &h, sizeof (MIDIHDR)) == MMSYSERR_NOERROR)
			{
				printf("Header prepared, sending...\n");
				memcpy(h.lpData, &message[0], sizeof(message));
				MMRESULT res = midiOutLongMsg(handle, &h, sizeof (MIDIHDR));

				if (res == MMSYSERR_NOERROR)
				{
					while ((h.dwFlags & MHDR_DONE) == 0) {
						printf("Waiting for message...\n");
						Sleep(1);
					}

					int count = 500; // 1 sec timeout

					while (--count >= 0)
					{
						printf("Attempting to unprepare header...\n");
						res = midiOutUnprepareHeader(handle, &h, sizeof (MIDIHDR));

						if (res == MIDIERR_STILLPLAYING) {
							printf("Still playing, wait...\n");
							Sleep(2);
						}
						else {
							printf("Message sent!\n");
							goto resend_message;
							break;
						}
					}
				}
				else {
					printf("Error in midiOutLongMsg\n");
				}
			}
			else {
				printf("MidiOutPrepareHeader failed...\n");
			}
			end:
			Sleep(1000);

			GlobalUnlock(hbuffer);
			GlobalFree(hbuffer);
			printf("Closing...\n");
			midiOutClose(handle);
			if (inHandle != 0) {
				midiInClose(inHandle);
			}

		}
	}
}

