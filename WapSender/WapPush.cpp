#include "WapPush.h"
#include "wbxml2\wbxml.h"

#define ASCII(x) ((x >= 0 && x <= 9) ? x += '0' :  x += 'A' - 0x0A)

WapPush::WapPush() : m_bCOMOpen(FALSE), m_pcWbXml(NULL), m_pcAsciiXml(NULL) {
	ZeroMemory(cBCDNumber, sizeof(cBCDNumber));
}

WapPush::~WapPush() {
	Close();

	if (m_pcWbXml != NULL)
		wbxml_free(m_pcWbXml);

	if (m_pcAsciiXml != NULL)
		delete[] m_pcAsciiXml;
}


BOOL WapPush::Open() {
	if (strPort.empty()) {
		wprintf(L"[ERROR] No COM port set\n");
		m_bCOMOpen = FALSE;
		return FALSE;
	}

	if (m_bCOMOpen)
		return TRUE;

	if (serialObj.Open(strPort)) {
		m_bCOMOpen = TRUE;
		return TRUE;
	}

	m_bCOMOpen = FALSE;
	return FALSE;
}

BOOL WapPush::Close() {
	if (m_bCOMOpen == FALSE)
		return TRUE;

	if (serialObj.Close()) {
		m_bCOMOpen = FALSE;
		return TRUE;
	}

	m_bCOMOpen = TRUE;
	return FALSE;
}

BOOL WapPush::SetPort(PWCHAR pwPort) {
	if (pwPort == NULL || wcslen(pwPort) < 4 || wcslen(pwPort) > 6) {
		wprintf(L"[ERROR] Invalid port format, should be: COM1, COM3, COM12 etc...\n");
		return FALSE;
	}

	if (m_bCOMOpen && strPort.empty() == FALSE) {
		// Convert to uppercase
		transform(strPort.begin(), strPort.end(), strPort.begin(), ptr_fun(toupper));
	}

	if (m_bCOMOpen && strPort.empty() == FALSE && strPort.compare(pwPort)) {
		wprintf(L"[NOTICE] Port changed, closing old connection...\n");
		Close();
	}

	strPort = pwPort;

	// Convert to uppercase
	transform(strPort.begin(), strPort.end(), strPort.begin(), ptr_fun(toupper));

	return TRUE;
}

BOOL WapPush::SetPIN(PWCHAR pwPin) {
	if (pwPin == NULL) {
		strPin.clear();
		return TRUE;
	}

	strPin = pwPin;
	return TRUE;
}

BOOL WapPush::SetNumber(PWCHAR pwNumber) {
	UINT i;

	if (pwNumber == NULL || wcslen(pwNumber) > 15 || wcslen(pwNumber) < 10) {
		wprintf(L"[ERROR] Phone number format is invalid or missing\n");
		return FALSE;
	}

	strNumber = pwNumber;

	wprintf(L"[NOTICE] Phone number set to: %s\n", pwNumber);

	// We have to convert it in BCD format
	if (wcslen(pwNumber) & 1) {
		// Odd number, padding required
		sprintf_s(cBCDNumber, sizeof(cBCDNumber), "%SF", pwNumber);
	} else {
		sprintf_s(cBCDNumber, sizeof(cBCDNumber), "%S", pwNumber);
	}

	for (i = 0; i < strlen(cBCDNumber) - 1; i += 2) {
		CHAR tmp;

		tmp = cBCDNumber[i];
		cBCDNumber[i] = cBCDNumber[i + 1];
		cBCDNumber[i + 1] = tmp;
	}
 
	return TRUE;
}

BOOL WapPush::SetLink(PWCHAR pwLink) {
	if (pwLink == NULL) {
		wprintf(L"[ERROR] HTTP Link format is invalid or missing\n");
		return FALSE;
	}

	strLink = pwLink;
	return TRUE;
}

// Text can be NULL
BOOL WapPush::SetText(PWCHAR pwText) {
	if (pwText == NULL) {
		strText = L"";
	} else {
		strText = pwText;
	}

	return TRUE;
}

BOOL WapPush::SetService(PWCHAR pwService) {
	if (pwService == NULL) {
		wprintf(L"[ERROR] Service format is invalid or missing\n");
		return FALSE;
	}

	strService = pwService;

	// Convert to lowercase
	transform(strService.begin(), strService.end(), strService.begin(), ptr_fun(tolower));

	// Enforce type ("si" or "sl")
	if (strService.compare(L"si") && strService.compare(L"sl")) {
		wprintf(L"[ERROR] Service format is invalid\n");
		return FALSE;
	}

	return TRUE;
}

BOOL WapPush::SetPriority(PWCHAR pwPriority) {
	if (pwPriority == NULL) {
		wprintf(L"[ERROR] Priority format is invalid or missing\n");
		return FALSE;
	}

	strPriority = pwPriority;

	// Convert to lowercase
	transform(strPriority.begin(), strPriority.end(), strPriority.begin(), ptr_fun(tolower));

	// Enforce type
	if (strService.empty() == FALSE) {
		// We have service-type set (SI)
		if (!strService.compare(L"si")) {
			// signal-none, signal-low, signal-medium, signal-high, delete
			if (strPriority.compare(L"signal-none") && strPriority.compare(L"signal-low") && 
				strPriority.compare(L"signal-medium") && strPriority.compare(L"signal-high") && 
				strPriority.compare(L"delete")) {
					wprintf(L"[ERROR] Chosen priority is not compatible with type SI\n");
					return FALSE;
			}
		} else if (!strService.compare(L"sl")) { // SL
			// execute-low, execute-high, cache
			if (strPriority.compare(L"execute-low") && strPriority.compare(L"execute-high") && 
				strPriority.compare(L"cache")) {
					wprintf(L"[ERROR] Chosen priority is not compatible with type SL\n");
					return FALSE;
			}
		} else {
			wprintf(L"[ERROR] Service format is invalid\n");
			return FALSE;
		}
	} else {
		// Service not set, we have to check all types
		if (strPriority.compare(L"signal-none") && strPriority.compare(L"signal-low") && 
			strPriority.compare(L"signal-medium") && strPriority.compare(L"signal-high") && 
			strPriority.compare(L"delete") && strPriority.compare(L"execute-low") && 
			strPriority.compare(L"execute-high") && strPriority.compare(L"cache")) {
				wprintf(L"[ERROR] Chosen priority type is invalid\n");
				return FALSE;
		}
	}

	return TRUE;
}

BOOL WapPush::SetDate(PWCHAR pwDate) {
	if (!strService.compare(L"si") && (pwDate == NULL || wcslen(pwDate) == 0)) {
		wprintf(L"[ERROR] Creation date is mandatory for service type \"si\"\n");
		return FALSE;
	}

	if (pwDate == NULL)
		strDate = L"";
	else
		strDate = pwDate;

	return TRUE;
}

BOOL WapPush::SendMessage(PWCHAR pwPort, PWCHAR pwPIN, PWCHAR pwNumber, PWCHAR pwText, 
						  PWCHAR pwService, PWCHAR pwPriority, PWCHAR pwLink, PWCHAR pwDate) {
	CHAR *pcMsg = NULL;

	if (SetPort(pwPort) == FALSE)
		return FALSE;

	if (SetNumber(pwNumber) == FALSE)
		return FALSE;

	if (SetPIN(pwPIN) == FALSE)
		return FALSE;

	if (SetText(pwText) == FALSE)
		return FALSE;

	if (SetService(pwService) == FALSE)
		return FALSE;

	if (SetPriority(pwPriority) == FALSE)
		return FALSE;

	if (SetLink(pwLink) == FALSE)
		return FALSE;

	if (SetDate(pwDate) == FALSE)
		return FALSE;

	// Let's build a "standard" message (XML to WBXML to ASCII)
	if (BuildWBXML() == FALSE) {
		return FALSE;
	}
	
	wprintf(L"[NOTICE] WBXML Built\n");

	// Let's add: GSM 03.40 Header, UDHI Length, WDP Layer and WSP Layer
	if (AddGSMHeaders() == FALSE) {
		return FALSE;
	}

	wprintf(L"[NOTICE] Final message created\n");

	// Now we can try to send it: let's enter in PDU mode
	if (SendCommandAndCheck("AT+CMGF=0\r\n", "OK") <= 0) {
		wprintf(L"[ERROR] Cannot enter PDU mode\n");
		
		// "Reset" commandline
		SendCommand("\x1A");
		return FALSE;
	}

	wprintf(L"[NOTICE] PDU mode set\n");

	// Better to enable error mode
	if (SendCommandAndCheck("AT+CMEE=1\r\n", "OK") < 0) {
		wprintf(L"[WARNING] Cannot enable ERROR mode\n");
	}

	wprintf(L"[NOTICE] Sending message...\n");

	// Let's tell the modem we want to send an SMS
	pcMsg = new(std::nothrow) CHAR[m_uAsciiXmlLen + 1 + 1];

	if (pcMsg == NULL) {
		wprintf(L"[ERROR] Cannot allocate memory for final message\n");
		return FALSE;
	}

	if ((m_uAsciiXmlLen >> 1) - 1 > 153) {
		wprintf(L"[ERROR] Message to long to be sent\n");
		delete[] pcMsg;
		return FALSE;
	}

	// AT+CMGS=XX
	sprintf_s(pcMsg, m_uAsciiXmlLen + 2, "AT+CMGS=%d\r", (m_uAsciiXmlLen >> 1) - 1);
	
	if (SendCommandAndCheck(pcMsg, ">") <= 0) {
		wprintf(L"[ERROR] Wrong answer from GSM modem\n");
		delete[] pcMsg;
		return FALSE;	
	}

	// Actual payload 0055....
	sprintf_s(pcMsg, m_uAsciiXmlLen + 2, "%s\x1A", m_pcAsciiXml);

	// Let's set a higher timeout for message sending
	serialObj.SetComReadWaitTime(5000);

	if (SendCommandAndCheck(pcMsg, "OK") <= 0) {
		wprintf(L"[ERROR] Cannot send message\n");
		serialObj.SetComReadWaitTime(200);
		delete[] pcMsg;
		return FALSE;
	}

	wprintf(L"[NOTICE] Message delivered\n");

	delete[] pcMsg;
	serialObj.SetComReadWaitTime(200);
	return TRUE;
}

BOOL WapPush::SendMessage(PWCHAR pwPort, PWCHAR pwPIN, PWCHAR pwNumber, PCHAR pcXml) {
	wprintf(L"[ERROR] NOT IMPLEMENTED\n");
	return FALSE;
}

// Text (optional) and Link must be already set at this point
BOOL WapPush::BuildWBXML() {
	CHAR *xml_exp = NULL;
	UINT xml_len = 0;
	WBXMLGenWBXMLParams params;
	WBXMLError ret = WBXML_OK;

	/*
	 * - Service Loading
	 * Possible signals:
	 * execute-low, execute-high, cache
	 */	
	CHAR xml_sl[] =
		"<?xml version=\"1.0\"?>"
		"<!DOCTYPE sl PUBLIC \"-//WAPFORUM//DTD SL 1.0//EN\" \"http://www.wapforum.org/DTD/sl.dtd\">"
		"<sl href=\"%S\" action=\"%S\" />"; // Link - Priority

	/*
	 * - Service Indication
	 * Possible signals:
	 * signal-none, signal-low, signal-medium, signal-high, delete
	 */	
	CHAR xml_si[] = 
		"<?xml version=\"1.0\"?>"
		"<!DOCTYPE si PUBLIC \"-//WAPFORUM//DTD SI 1.0//EN\" \"http://www.wapforum.org/DTD/si.dtd\">"
		"<si>"
		"<indication href=\"%S\" action=\"%S\" created=\"%SZ\" si-id=\"1101\">" // Link - Priority - Date
		"%S" // Text
		"</indication>"
		"</si>";

	if (strNumber.empty()) {
		wprintf(L"[ERROR] Number not set cannot build XML\n");
		return FALSE;
	}

	if (strLink.empty()) {
		wprintf(L"[ERROR] Link not set, cannot build XML\n");
		return FALSE;
	}

	if (strService.empty()) {
		wprintf(L"[ERROR] Service not set, cannot build XML\n");
		return FALSE;
	}

	if (strPriority.empty()) {
		wprintf(L"[ERROR] Priority not set, cannot build XML\n");
		return FALSE;
	}

	if (strDate.empty() && !strService.compare(L"si")) {
		wprintf(L"[ERROR] Creation date not set, cannot build XML\n");
		return FALSE;
	}

	// SI Xml
	if (!strService.compare(L"sl")) {
		xml_len = strlen(xml_sl) + strLink.size() + strPriority.size() + 1;
		xml_exp = new(std::nothrow) CHAR[xml_len];

		if (xml_exp == NULL) {
			wprintf(L"[ERROR] Cannot allocate memory for XML\n");
			return FALSE;
		}

		sprintf_s(xml_exp, xml_len, xml_sl, strLink.c_str(), strPriority.c_str());
	} else if (!strService.compare(L"si")) { // SL Xml
		xml_len = strlen(xml_si) + strLink.size() + strPriority.size() + strDate.size() + strText.size() + 1;
		xml_exp = new(std::nothrow) CHAR[xml_len];

		if (xml_exp == NULL) {
			wprintf(L"[ERROR] Cannot allocate memory for XML\n");
			return FALSE;
		}

		sprintf_s(xml_exp, xml_len, xml_si, strLink.c_str(), strPriority.c_str(), strDate.c_str(), strText.c_str());
	} else {
		wprintf(L"[ERROR] Unknown service type\n");
		return FALSE;
	}

	xml_len = strlen(xml_exp);

	params.wbxml_version = WBXML_VERSION_13;
	params.use_strtbl = TRUE;
	params.keep_ignorable_ws = FALSE;

	if (m_pcWbXml != NULL) {
		wbxml_free(m_pcWbXml);
		m_pcWbXml = NULL;
		m_uWbXmlLen = 0;
	}

	ret = wbxml_conv_xml2wbxml_withlen((PUCHAR)xml_exp, xml_len, (UCHAR **)&m_pcWbXml, &m_uWbXmlLen, &params);
	
	delete[] xml_exp;

	if (ret != WBXML_OK) {
		wprintf(L"[ERROR] %s\n", wbxml_errors_string(ret));
		return FALSE;
	} 

	if (WBXMLToAscii() == FALSE) {
		wprintf(L"[ERROR] Error converting WBXML to ASCII\n");
		return FALSE;
	}

	return TRUE;
}

BOOL WapPush::AddGSMHeaders() {
	// Service Loading
	CHAR cHeaders_sl[] =
		"0041000%X91%s0004%02X"		// --> GSM 03.40 Header (length and number in BCD), payload length
		"0B"						// --> UDHI Length
		"05040B84C0020003F00101"	// --> WDP Layer
		"0A060403B081EA"			// --> WSP Layer
		"%s";						// --> WBXML Message

	// Service Indication
	CHAR cHeaders_si[] =
		"0055010%X91%s00F5A7%02X"	// --> GSM 03.40 Header (length and number in BCD), payload length
		"06"						// --> UDHI Length
		"05040B8423F0"				// --> WDP Layer
		"01060403AE81EA"			// --> WSP Layer
		"%s";						// --> WBXML Message

	CHAR *pcMsg;
	UINT uHeaderLen;
	UINT uPayloadLen;

	if (!strService.compare(L"sl")) {
		uHeaderLen = strlen(cHeaders_sl) + 1 + 15 + strlen(m_pcAsciiXml) + 1;
	} else if (!strService.compare(L"si")) {
		uHeaderLen = strlen(cHeaders_si) + 1 + 15 + strlen(m_pcAsciiXml) + 1;
	} else {
		wprintf(L"[ERROR] Unknown service type\n");
		return FALSE;
	}

	pcMsg = new(std::nothrow) CHAR[uHeaderLen];
	ZeroMemory(pcMsg, uHeaderLen);

	if (pcMsg == NULL) {
		wprintf(L"[ERROR] Error allocating memory for GSM headers\n");
		return FALSE;
	}

	// Calculate payload length
	if (!strService.compare(L"sl")) {
		// Service Loading
		uPayloadLen = strlen("0B") + strlen("05040B84C0020003F00101") + strlen("0A060403B081EA") + 
						strlen(m_pcAsciiXml);

		uPayloadLen >>= 1;
		sprintf_s(pcMsg, uHeaderLen, cHeaders_sl, strNumber.size(), cBCDNumber, uPayloadLen, m_pcAsciiXml);

	} else {
		// Service Indication
		uPayloadLen = strlen("06") + strlen("05040B8423F0") + strlen("01060403AE81EA") + strlen(m_pcAsciiXml);
		uPayloadLen >>= 1;

		sprintf_s(pcMsg, uHeaderLen, cHeaders_si, strNumber.size(), cBCDNumber, uPayloadLen, m_pcAsciiXml);
	}

	uHeaderLen--; // Real length

	// Clean-up old ASCII-XML and replace it with the expanded one
	delete[] m_pcAsciiXml;
	m_pcAsciiXml = pcMsg;
	m_uAsciiXmlLen = strlen(m_pcAsciiXml);

	return TRUE;
}

BOOL WapPush::WBXMLToAscii() {
	UINT i, j;

	if (m_pcWbXml == NULL || m_uWbXmlLen == 0) {
		wprintf(L"[ERROR] There is no WBXML to convert\n");
		return FALSE;
	}

	if (m_pcAsciiXml != NULL) {
		delete[] m_pcAsciiXml;
		m_pcAsciiXml = NULL;
		m_uAsciiXmlLen = 0;
	}

	m_uAsciiXmlLen = m_uWbXmlLen * 2 + 1;
	m_pcAsciiXml = new(std::nothrow) CHAR[m_uAsciiXmlLen];

	if (m_pcAsciiXml == NULL) {
		wprintf(L"[ERROR] Error allocating WBXML-ASCII array\n");
		return FALSE;
	}

	m_uAsciiXmlLen--; // Real length
	m_pcAsciiXml[m_uAsciiXmlLen] = 0;

	// Hex to ASCII
	for (i = 0, j = 0; i < m_uWbXmlLen; i++, j += 2) {
		CHAR tmp = (m_pcWbXml[i] & 0xF0) >> 4;
		m_pcAsciiXml[j] = ASCII(tmp);

		tmp = m_pcWbXml[i] & 0x0F;
		m_pcAsciiXml[j + 1] = ASCII(tmp);
	}

	// Cleanup of m_cWbXML buffer
	if (m_pcWbXml != NULL) {
		wbxml_free(m_pcWbXml);
		m_pcWbXml = NULL;
		m_uWbXmlLen = 0;
	}

	return TRUE;
}

BOOL WapPush::CheckModem(PWCHAR pwPort, PWCHAR pwPIN) {
	INT iCheck;

	if (SetPort(pwPort) == FALSE)
		return FALSE;

	if (SetPIN(pwPIN) == FALSE)
		return FALSE;

	// Check AT
	iCheck = SendCommandAndCheck("AT\r", "OK\r");
	
	if (iCheck == 0) {
		wprintf(L"[ERROR] Cannot send command to GSM modem\n");
		return FALSE;	
	}

	if (iCheck < 0) {
		wprintf(L"[ERROR] AT interface is not ready\n");
		return FALSE;	
	}

	wprintf(L"[CHECK] AT interface is ready\n");

	// Let's see if we need a PIN
	iCheck = SendCommandAndCheck("AT+CPIN?\r", "READY");

	if (iCheck == 0) {
		wprintf(L"[ERROR] Cannot send command to GSM modem\n");
		return FALSE;	
	}

	// In case we insert the provided PIN
	if (iCheck < 0) {
		wprintf(L"[NOTICE] A PIN code is required to operate the SIM card\n");

		if (strPin.empty()) {
			wprintf(L"[ERROR] No PIN has been provided, exiting...\n");
		}

		char cPinBuf[30];
		sprintf_s(cPinBuf, 30, "AT+CPIN=\"%S\"\r", strPin.c_str());

		iCheck = SendCommandAndCheck(cPinBuf, "OK");

		if (iCheck < 0) {
			wprintf(L"[ERROR] Wrong PIN used\n");
			return FALSE;
		}

		if (iCheck == 0) {
			wprintf(L"[ERROR] Error sending PIN\n");
			return FALSE;
		}
	} else {
		wprintf(L"[NOTICE] No PIN code required to operate SIM card\n");
	}

	SendCommandAndCheck("AT+CPIN?\r", "waiting response");		// PIN status
	SendCommandAndCheck("AT+CREG?\r", "waiting response");		// Network registration status
	SendCommandAndCheck("AT+CSQ\r", "waiting response");		// Signal strength
	SendCommandAndCheck("AT+CGMM\r", "waiting response");		// Producer identification
	SendCommandAndCheck("AT+CGMI\r", "waiting response");		// Model identification
	SendCommandAndCheck("AT+CGMR\r", "waiting response");		// Revision information
	SendCommandAndCheck("AT+CGSN\r", "waiting response");		// Serial number
	SendCommandAndCheck("AT+CIMI\r", "waiting response");		// IMSI
	SendCommandAndCheck("AT+CCID\r", "waiting response");		// SIM EF-CCID
	SendCommandAndCheck("AT+GCAP\r", "waiting response");		// Device capabilities
	SendCommandAndCheck("AT+CPAS\r", "waiting response");		// Mobile equipment activity status
	SendCommandAndCheck("AT+CGCLASS?\r", "waiting response");	// GPRS mobile class
	SendCommandAndCheck("AT+CGATT?\r", "waiting response");		// MT attachment status
	SendCommandAndCheck("AT+CSCA?\r", "waiting response");		// SMSC number

	// Let's check to see if we have a SMSC number set
	iCheck = SendCommandAndCheck("AT+CSCA?\r", "+CSCA: ");
		
	if (iCheck < 0) {
		wprintf(L"[ERROR] SMSC found...\n");
		return FALSE;
	}

	if (iCheck == 0) {
		wprintf(L"[ERROR] Error checking for carrier\n");
		return FALSE;
	}

	wprintf(L"[NOTICE] SMSC number correctly set\n");

	return TRUE;
}

BOOL WapPush::CheckCOM(PWCHAR pwPort) {
	INT iCheck;

	if (SetPort(pwPort) == FALSE)
		return FALSE;

	if (m_bCOMOpen)
		return TRUE;

	iCheck = SendCommandAndCheck("AT\r", "OK\r");

	if (iCheck == 0) {
		wprintf(L"[ERROR] Cannot send command to GSM modem\n");
		return FALSE;	
	}

	if (iCheck < 0) {
		wprintf(L"[ERROR] AT interface is not ready\n");
		return FALSE;	
	}

	wprintf(L"[CHECK] AT interface is ready\n");
	return TRUE;
}

INT WapPush::SendCommandAndCheck(PCHAR pcCommand, PCHAR pcCheck) {
#define READBUF_SIZE 1024

	INT iWritten = 0, iRead = 0, iCommLen, iCheckLen;
	BYTE *pBuff = NULL;

	if (m_bCOMOpen == FALSE) {
		if (Open() == FALSE)
			return FALSE;
	}

	if (pcCommand == NULL || pcCheck == NULL) {
		wprintf(L"[ERROR] Command or Check incorrectly set\n");
		return FALSE;
	}

	iCommLen = strlen(pcCommand);
	iCheckLen = strlen(pcCheck);

	if (iCommLen == 0 || iCheckLen == 0) {
		wprintf(L"[ERROR] Command or Check cannot have 0 length\n");
		return FALSE;
	}

	iWritten = serialObj.Write((PBYTE)pcCommand, iCommLen);

	if (iWritten < 0) {
		wprintf(L"[ERROR] Command not sent\n");
		return FALSE;
	}

	pBuff = new(std::nothrow) BYTE[READBUF_SIZE];

	if (pBuff == NULL) {
		wprintf(L"[ERROR] Cannot allocate memory for receiving buffer\n");
		return FALSE;
	}

	ZeroMemory(pBuff, READBUF_SIZE);

	iRead = serialObj.Read(pBuff, READBUF_SIZE);

	if (iRead < 0) {
		wprintf(L"[ERROR] Cannot read response\n");
		delete[] pBuff;
		return FALSE;
	}

	pBuff[READBUF_SIZE - 1] = 0;

	// Checking response
	if (strstr((PCHAR)pBuff, pcCheck) == NULL) {
		wprintf(L"[WARNING] Response received: %S\n", pBuff);
		delete[] pBuff;
		return -1;
	}

	delete[] pBuff;
	return TRUE;
}

INT WapPush::SendCommand(PCHAR pcCommand) {
	INT iWritten = 0, iCommLen;

	if (m_bCOMOpen == FALSE) {
		if (Open() == FALSE)
			return FALSE;
	}

	if (pcCommand == NULL) {
		wprintf(L"[ERROR] Command or Check incorrectly set\n");
		return FALSE;
	}

	iCommLen = strlen(pcCommand);

	if (iCommLen == 0) {
		wprintf(L"[ERROR] Command or Check cannot have 0 length\n");
		return FALSE;
	}

	iWritten = serialObj.Write((PBYTE)pcCommand, iCommLen);

	if (iWritten < 0) {
		wprintf(L"[ERROR] Command not sent\n");
		return FALSE;
	}

	return TRUE;
}