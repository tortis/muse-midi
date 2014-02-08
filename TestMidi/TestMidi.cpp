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
	printf("MIDI CALLBACK! Number of callbacks: %d\n",++ncallbacks);
	switch (wMsg) {
	case MIM_CLOSE: printf("MIM_CLOSE\n"); break;
	case MIM_DATA: 
		printf("MIM_DATA\n"); 
		printf("Timestamp: %d\n", dwParam2);
		printf("MIDI data: %x\n", dwParam1);
		break;
	case MIM_ERROR:printf("MIM_ERROR\n"); break;
	case MIM_LONGDATA:printf("MIM_LONGDATA\n"); break;
	case MIM_LONGERROR:printf("MIM_LONGERROR\n"); break;
	case MIM_MOREDATA:printf("MIM_MOREDATA\n"); break;
	case MIM_OPEN:printf("MIM_OPEN\n"); break;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	HMIDIIN inHandle = 0;
	unsigned int num = midiInGetNumDevs();
	printf("Number of MIDI In devices: %d\n ", num);
	printf("Printing MIDI In device names...");
	for (UINT i = 0; i < num; ++i)
	{
		MIDIINCAPS mc = { 0 };

		if (midiInGetDevCaps(i, &mc, sizeof (mc)) == MMSYSERR_NOERROR)
			printf("%ls\n", mc.szPname);
		//If we find our device, set a callback
		if (!wcscmp(mc.szPname, L"MIDI1")) {
			printf("Found input device\n!");

			MMRESULT err = midiInOpen(&inHandle, i,
				(DWORD_PTR)midiInCallback,
				0,
				CALLBACK_FUNCTION);
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
		if (!wcscmp(mc.szPname,L"MIDI1")) {
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
			printf("Starting IN MIDI...");
			MMRESULT res = midiInStart(inHandle);

			printf("Sending chord...");
			midiOutShortMsg(handle, 0x00403C90);
			midiOutShortMsg(handle, 0x00404090);
			midiOutShortMsg(handle, 0x00404390);

			//In a real device, the notes will start sounding here. Wait a bit, then close the notes
			Sleep(1000);

			printf("Turnin off chord...");
			//Turn them off. Volume 0.
			midiOutShortMsg(handle, 0x00003C90);
			midiOutShortMsg(handle, 0x00004090);
			midiOutShortMsg(handle, 0x00004390);
			//First byte is status: 9x is turning a note on, in this case for channel 0. 
			//Second byte is the note itself: 3c=60=Middle Do (C)
			//Third byte is note "velocity", usually implemented as volume
			//Fourth byte (MSB) is unused.
			Sleep(1000);
			midiOutClose(handle);
			if (inHandle != 0) {
				midiInClose(inHandle);
			}

		}
	}
}

