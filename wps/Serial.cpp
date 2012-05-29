#include "Serial.h"

Serial::Serial() : hSerial(INVALID_HANDLE_VALUE) {
	m_ComTimeouts.ReadIntervalTimeout = 0;
	m_ComTimeouts.ReadTotalTimeoutMultiplier = 1;
	m_ComTimeouts.ReadTotalTimeoutConstant = 200;
	m_ComTimeouts.WriteTotalTimeoutMultiplier = 10;
	m_ComTimeouts.WriteTotalTimeoutConstant = 40;
}

Serial::~Serial() {
	if (hSerial != INVALID_HANDLE_VALUE) {
		CloseHandle(hSerial);
		hSerial = INVALID_HANDLE_VALUE;
	}
}

BOOL Serial::Open(PWCHAR pwPort) {
	DCB dcbSerial = {0};
	wstring strPort;

	if (pwPort == NULL) {
		wprintf(L"[ERROR] Missing port number\n");
		return FALSE;
	}

	if (wcslen(pwPort) < 4 || wcslen(pwPort) > 6) {
		wprintf(L"[ERROR] Wrong port format, should be: COMX (e.g.: COM1, COM2, COM12...)\n");
		return FALSE;
	}

	if (pwPort[0] != 'C' || pwPort[1] != 'O' || pwPort[2] != 'M') {
		wprintf(L"[ERROR] Wrong port name, should be: COMX (e.g.: COM1, COM2, COM12...)\n");
		return FALSE;
	}

	if (hSerial != INVALID_HANDLE_VALUE) {
		wprintf(L"[WARNING] Port already opened, trying to reuse connection\n");
		return TRUE;
	}

	strPort = L"\\\\.\\";
	strPort += pwPort;

	hSerial = CreateFile(strPort.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hSerial == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND) {
			//wprintf(L"[ERROR] Port %s doesn't exist (try to restart GSM modem)\n", pwPort);
			return FALSE;
		}

		wprintf(L"[ERROR] Port %s cannot be opened\n", pwPort);
		return FALSE;
	}

	dcbSerial.DCBlength = sizeof(DCB);

	if (!GetCommState(hSerial, &dcbSerial)) {
		wprintf(L"[ERROR] Cannot get COM port state\n");
		CloseHandle(hSerial);
		hSerial = INVALID_HANDLE_VALUE;
		return FALSE;
	}

	dcbSerial.BaudRate = CBR_115200;
	dcbSerial.ByteSize = 8;
	dcbSerial.StopBits = ONESTOPBIT;
	dcbSerial.Parity = NOPARITY;

	if (!SetCommState(hSerial, &dcbSerial)){
		wprintf(L"[ERROR] Cannot set COM port state\n");
		CloseHandle(hSerial);
		hSerial = INVALID_HANDLE_VALUE;
		return FALSE;
	}

	if (!SetCommTimeouts(hSerial, &m_ComTimeouts)) {
		wprintf(L"[WARNING] Cannot set COM port timeouts, trying with defaults\n");
	}

	EscapeCommFunction(hSerial, SETDTR);

	wprintf(L"[NOTICE] Port %s successfully opened\n", pwPort);
	return TRUE;
}

BOOL Serial::Open(wstring &strPort) {
	return Open((PWCHAR)strPort.c_str());
}

INT Serial::Read(PBYTE pBuffer, UINT uBufSize) {
	DWORD dwBytesRead = 0;

	if (hSerial == INVALID_HANDLE_VALUE) {
		wprintf(L"[ERROR] There is no open COM port to read from\n");
		return -1;
	}

	PBYTE pTmp = pBuffer;

	while (ReadFile(hSerial, pTmp, uBufSize, &dwBytesRead, NULL)) {
		if (dwBytesRead == 0)
			return 0;

		if (pTmp + dwBytesRead < pBuffer + uBufSize)
			pTmp += dwBytesRead;
		else
			return dwBytesRead;
	}

	wprintf(L"[ERROR] Cannot read from COM port\n");
	return -1;
}

INT Serial::Write(PBYTE pBuffer, UINT uBufSize) {
	DWORD dwBytesWritten = 0;

	if (hSerial == INVALID_HANDLE_VALUE) {
		wprintf(L"[ERROR] There is no open COM port to write to\n");
		return -1;
	}

	if (!WriteFile(hSerial, pBuffer, uBufSize, &dwBytesWritten, NULL)){
		wprintf(L"[ERROR] Cannot write to COM port\n");
		return -1;
	}

	return dwBytesWritten;
}

BOOL Serial::Close() {
	if (hSerial == INVALID_HANDLE_VALUE) {
		wprintf(L"[WARNING] Port is not open\n");
		return TRUE;
	}

	if (CloseHandle(hSerial)) {
		wprintf(L"[NOTICE] Port closed successfully\n");
		hSerial = INVALID_HANDLE_VALUE;
		return TRUE;
	}

	wprintf(L"[ERROR] Error closing COM port\n");
	return FALSE;
}

BOOL Serial::SetComReadWaitTime(DWORD dwWait) {
	if (hSerial == INVALID_HANDLE_VALUE) {
		wprintf(L"[WARNING] Port is not open\n");
		return FALSE;
	}

	m_ComTimeouts.ReadTotalTimeoutConstant = dwWait;

	if (!SetCommTimeouts(hSerial, &m_ComTimeouts)) {
		wprintf(L"[WARNING] Cannot set COM port timeout\n");
		return FALSE;
	}

	return TRUE;
}