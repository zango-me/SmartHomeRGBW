


String memGetString(const int address)
{
	char value[1000];
	__retrieve(value, address);
	String strValue = value;
	return strValue;
}

void memSetString(String value, const int address)
{
	__store(value.c_str(), address);
	eeprom_changed = true;
}




int __store(const char * str, const int address)
{
	int i = 0;
	char chr;
	do {
		chr = str[i];
		EEPROM.write(i + address, chr);
		i++;
	} while (chr != '\0');
	//EEPROM.commit();
	return i;
}

int __retrieve(char * buffer, const int address)
{
	int i = 0;
	char chr;
	do {
		chr = EEPROM.read(i + address);
		buffer[i] = chr;
		i++;
	} while (chr != '\0');

	return i;
}

void eepromCommit() {
	Serial.println("EEPROM COMMIT");
	EEPROM.commit();
}

