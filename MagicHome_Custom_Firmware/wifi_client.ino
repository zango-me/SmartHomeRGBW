
void connectToWifi(String ssid, String pass) {

	// use in case of mode problem
	WiFi.disconnect();
	// switch to Station mode
	if (WiFi.getMode() != WIFI_STA) {
		WiFi.mode(WIFI_STA);
	}

	IPAddress __ip = getIpFromString(memGetString(memoryAddresses[2]));
	IPAddress __subnet = getIpFromString(memGetString(memoryAddresses[3]));
	IPAddress __gateway = getIpFromString(memGetString(memoryAddresses[4]));
	//IP config
	WiFi.config(__ip, __gateway, __subnet);
	//Verbindung zum WLAN aufbauen
	WiFi.begin(ssid.c_str(), pass.c_str());
	//Set auto reconnect
	//WiFi.setAutoReconnect(true);
}

boolean wifiIsConnected() {
	if (WiFi.status() == WL_CONNECTED) {
		//Serial.println("DEBUG:"+(String)WiFi.status()+"T");
		return true;
	}
	else {
		//Serial.println("DEBUG:"+(String)WiFi.status()+"F");
		return false;
	}
}