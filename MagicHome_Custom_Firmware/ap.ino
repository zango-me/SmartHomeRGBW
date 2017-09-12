
void registerAP()
{
	Serial.println("Configuring access point...");

	// use in case of mode problem
	WiFi.disconnect();
	// switch to Station mode
	if (WiFi.getMode() != WIFI_AP) {
		WiFi.mode(WIFI_AP);
	}

	//Open Access Point
	WiFi.softAP((_ssid + " " + getMacString()).c_str(), _pass.c_str());
	WiFi.softAPConfig(ip, ip, mask);

	//Print IP
	IPAddress apIP = WiFi.softAPIP();

	Serial.print("AP IP address: ");
	Serial.println(apIP);

	Serial.print("AP SSID: ");
	Serial.println(_ssid + " " + getMacString());

	Serial.print("AP PW: ");
	Serial.println(_pass);

}


void stopAP()
{
	WiFi.softAPdisconnect(true);
}
