

bool isNumeric(String str) {
	for (byte i = 0; i < str.length(); i++)
	{
		if (!isDigit(str.charAt(i))) return false;
	}
	return true;
}

int getMicrosFromSeconds(int Seconds) {
	return Seconds * 1000000;
}

void delaySec(int _delay) {
	int counter = 0;
	while (counter < _delay * 1000) {
		delay(10);
		yield();
		counter = counter + 10;
	}
}

String urlencode(String str)
{
	String encodedString = "";
	char c;
	char code0;
	char code1;
	char code2;
	for (int i = 0; i < str.length(); i++) {
		c = str.charAt(i);
		if (c == ' ') {
			encodedString += '+';
		}
		else if (isalnum(c)) {
			encodedString += c;
		}
		else {
			code1 = (c & 0xf) + '0';
			if ((c & 0xf) > 9) {
				code1 = (c & 0xf) - 10 + 'A';
			}
			c = (c >> 4) & 0xf;
			code0 = c + '0';
			if (c > 9) {
				code0 = c - 10 + 'A';
			}
			code2 = '\0';
			encodedString += '%';
			encodedString += code0;
			encodedString += code1;
			//encodedString+=code2;
		}
		yield();
	}
	return encodedString;

}

String getMacString()
{
	String mac = WiFi.macAddress();
	mac.replace(":", "");
	return mac;
}

/*
IPAddress getIpFromString(String ip)
{
	int index1 = ip.indexOf('.');
	int index2 = ip.indexOf('.', index1 + 1);
	int index3 = ip.indexOf('.', index2 + 1);

	String value1 = ip.substring(0, index1);
	String value2 = ip.substring(index1 + 1, index2);
	String value3 = ip.substring(index2 + 1, index3);
	String value4 = ip.substring(index3 + 1);

	IPAddress _ip(value1.toInt(), value2.toInt(), value3.toInt(), value4.toInt());
	return _ip;
}
*/


IPAddress getIpFromString(String ip)
{
	IPAddress addr;
	addr.fromString(ip);
	return addr;
}


bool isValidIp4(String ip)
{
	IPAddress addr;
	if (addr.fromString(ip)) {
		return true;
	}
	else {
		return false;
	}
}


void clearEEPROM()
{
	memSetString("", memoryAddresses[0]); //ssid
	memSetString("", memoryAddresses[1]); //password

	memSetString("", memoryAddresses[2]); //ip
	memSetString("", memoryAddresses[3]); //subnetmask
	memSetString("", memoryAddresses[4]); //gateway

	memSetString(apiKey, memoryAddresses[5]); //apikey

	memSetString("0", memoryAddresses[6]); //R
	memSetString("0", memoryAddresses[7]); //G
	memSetString("0", memoryAddresses[8]); //B
	memSetString("0", memoryAddresses[9]); //W
	memSetString("0", memoryAddresses[10]); //mode
	memSetString("2000", memoryAddresses[11]); //transitiontime
	
	memSetString("valid", memoryAddresses[12]); //validation
	
	memSetString("0", memoryAddresses[13]); //autosave

	memSetString("100", memoryAddresses[14]); //freq

	eepromCommit();
}


String boolToString(bool input)
{
	if (input)
	{
		return "true";
	}
	else {
		return "false";
	}
}


