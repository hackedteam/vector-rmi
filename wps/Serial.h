#pragma once

#include <Windows.h>
#include <string>
#include <stdio.h>
#include <iostream>

using namespace std;

class Serial {
private:
	HANDLE hSerial;
	COMMTIMEOUTS m_ComTimeouts;

public:
	Serial();
	~Serial();

	// Open the specified COM port. Format is "COM1, COM2, COM87 etc..."
	BOOL Open(PWCHAR pwPort);
	BOOL Open(wstring &strPort);

	// Read uBufSize bytes into pBuffer from COM port, returns -1 in case of failure
	INT Read(PBYTE pBuffer, UINT uBufSize);	

	// Write uBufSize bytes from pBuffer into COM port, returns -1 in case of failure
	INT Write(PBYTE pBuffer, UINT uBufSize);

	// Close and open COM port
	BOOL Close();

	// Set max wait time in ms for read operations, return FALSE in case of failure
	BOOL SetComReadWaitTime(DWORD dwWait);
};