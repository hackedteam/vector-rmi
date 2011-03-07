#pragma once

#include <Windows.h>
#include <string>
#include <stdio.h>
#include <algorithm>
#include <functional>

#include "Serial.h"

using namespace std;

/* Encoding type: http://adywicaksono.wordpress.com/2008/05/11/sending-wap-push-service-indication/
<?xml version="1.0"?>
<!DOCTYPE si PUBLIC "-//WAPFORUM//DTD SI 1.0//EN" "http://www.wapforum.org/DTD/si.dtd">
<si>
<indication href="http://89.96.137.6/a.jad" created="2010-06-25T15:23:15Z" si-expires="2099-06-30T00:00:00Z">
Que
</indication>
</si>


<?xml version="1.0"?>
<!DOCTYPE si PUBLIC "-//WAPFORUM//DTD SI 1.0//EN" "http://www.wapforum.org/DTD/si.dtd">
<si>
<indication href="http://89.96.137.6/a.jad" "action="signal-none" si-id="1234">
Que
</indication>
</si>

<?xml version="1.0"?>
<!DOCTYPE si PUBLIC "-//WAPFORUM//DTD SI 1.0//EN"	"http://www.wapforum.org/DTD/si.dtd"><si>
<indication href="http://wap.wappushexample.com" action="signal-none" si-id="1234" >
a new email is available at http://wap.wappushexample.com
</indication>
</si>
*/

class WapPush : public Serial{
private:
	wstring strPort, strPin, strNumber, strLink, strText;
	wstring strService, strPriority, strDate;
	CHAR cBCDNumber[16];
	Serial serialObj;
	BOOL m_bCOMOpen;
	CHAR *m_pcWbXml, *m_pcAsciiXml;
	UINT m_uWbXmlLen, m_uAsciiXmlLen;

public:
	WapPush();
	~WapPush();

	// Both parameters MUST be in ASCII and NULL-terminated
	// TRUE if Check was successful, 0 if sending/receiving failed, -1 when Check fails
	INT SendCommandAndCheck(PCHAR pcCommand, PCHAR pcCheck);

	// Like above but doesn't check for response
	INT SendCommand(PCHAR pcCommand);

	BOOL SendMessage(PWCHAR pwPort, PWCHAR pwPIN, PWCHAR pwNumber, PWCHAR pwText, 
					PWCHAR pwService, PWCHAR pwPriority, PWCHAR pwLink, PWCHAR pwDate);
	BOOL SendMessage(PWCHAR pwPort, PWCHAR pwPIN, PWCHAR pwNumber, PCHAR pcXml);

	BOOL CheckModem(PWCHAR pwPort, PWCHAR pwPIN);

	BOOL CheckCOM(PWCHAR pwPort);

private:
	// Open COM port
	BOOL Open();

	// Close COM port
	BOOL Close();

	// COM port in format COM1, COM4, COM12 etc...
	BOOL SetPort(PWCHAR pwPort);

	// SIM PIN code
	BOOL SetPIN(PWCHAR pwPin);

	// Phone number in international format without + (393471234567)
	BOOL SetNumber(PWCHAR pwNumber);

	// Any valid link: http://quequero.org/rcs.exe
	BOOL SetLink(PWCHAR pwLink);

	// Any text: "This is a test"
	BOOL SetText(PWCHAR pwText);

	// Set service type, can be "si" or "sl"
	BOOL SetService(PWCHAR pwService);

	// Set service priority
	BOOL SetPriority(PWCHAR pwPriority);

	// Set creation date
	BOOL SetDate(PWCHAR pwDate);

	// Builds a valid WBXML from given Number, Link, Text
	BOOL BuildWBXML();

	// Converts che WBXML to an ASCII string
	BOOL WBXMLToAscii();

	// Add all necessary header to the raw WBXML
	BOOL AddGSMHeaders();
};