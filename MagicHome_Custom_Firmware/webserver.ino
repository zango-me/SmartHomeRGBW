
void registerWebserver(bool apiMode) {
	lastWebserverMode = apiMode;
	if (apiMode)
	{
		server.on("/deleteconfig", handleDeleteConfig);
		server.on("/get", handleApiGet);
		server.on("/set", handleApiSet);
		//server.on("/save", handleApiSave);
	}
	else
	{
		server.on("/connect", handleConnect);
		server.on("/ssidlist.html", handleSSIDList);
	}

	server.on("/", handleRoot);
	server.on("/app.css", handleAppCSS);

	server.onNotFound(handleNotFound);

	server.begin();
	Serial.println("Webserver started");
}

void stopWebserver() {
	Serial.println("Stoppig Webserver");
	lastWebserverMode = false;
	server.close();
	server.stop();
}







//GLOBAL


void handleNotFound() {
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";
	for (uint8_t i = 0; i < server.args(); i++) {
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	}
	server.send(404, "text/plain", message);
}



void handleRoot() {
	if (lastWebserverMode)
	{
		handleApiRoot();
	}
	else
	{
		handleConfigRoot();
	}
}




String cssApp = " \
				body{ \
				background-color: #ddd; \
				font-family: Calibri, Arial, sans-serif; \
				text-align: center; \
				} \
				\
				input{ \
				max-width: 100%; \
				width: 400px; \
				padding: 5px 10px; \
				border-radius: 3px; \
				border: 1px solid #999; \
				} \
				\
				input[type=submit] { \
				padding: 15px 10px; \
				border-radius: 0; \
				border: none; \
				background-color: #67cc8d; \
				cursor: pointer; \
				} \
				\
				input[type=submit]:hover { \
				background-color: #00cc6f; \
				} \
				\
				iframe { \
				border: 1px solid #999; \
				border-radius: 4px; \
				} \
				\
				.btn { \
				border-radius: 3px; \
				color: #ffffff; \
				font-size: 14px; \
				background: #d95534; \
				padding: 7px 15px 7px 15px; \
				text-decoration: none; \
				} \
				\
				.btn:hover { \
				background: #c2221d; \
				text-decoration: none; \
				} \
				";

void handleAppCSS() {
	server.send(200, "text/html", cssApp);
}











//API HTML


void handleApiRoot() {
	server.send(200, "text/html", " \
								  <html> \
								  <head> \
								  <title>SmartHome RGBW Api</title> \
								  <link rel='stylesheet' href='app.css'> \
								  <meta name='viewport' content='width=device-width, initial-scale=1.0'> \
								  </head> \
								  <body> \
								  <div id='main'> \
								  <h1>API</h1> \
								  <h2>SmartHome RGBW Controller</h2> \
								  <h3>Please refer to the documentation for usage instructions.</h3><br /> \
								  <div style='color: #666;'> \
								  <h3>Current configuration</h3> \
								  SSID: <b>" + memGetString(memoryAddresses[0]) + "</b><br /> \
								  IP: <b>" + memGetString(memoryAddresses[2]) + "</b><br /> \
								  Subnet: <b>" + memGetString(memoryAddresses[3]) + "</b><br /> \
								  Gateway: <b>" + memGetString(memoryAddresses[4]) + "</b><br /> \
								  MAC: <b>" + getMacString() + "</b><br /><br /> \
								  <a class='btn' onclick=\"return confirm('Are you sure you want to delete the configuration and reboot? (This will also start the configuration AP again)');\" href='/deleteconfig'>Delete configuration and restart</a><br /><br /> \
								  </div> \
								  </div> \
								  </body> \
								  </html> \
								  ");
}


void handleDeleteConfig() {
	if (!lastWebserverMode) {
		handleNotFound();
		return;
	}
	server.send(200, "text/html", " \
								  <html> \
								  <head> \
								  <title>SmartHome RGBW Api</title> \
								  <link rel='stylesheet' href='app.css'> \
								  <meta name='viewport' content='width=device-width, initial-scale=1.0'> \
								  </head> \
								  <body> \
								  <div id='main'> \
								  <b>Configuration is deleted</b><br /> \
								  <span>Please reconfigure now</span> \
								  </div> \
								  </body> \
								  </html> \
								  ");


	//Auf �brige requests warten
	unsigned long startMicros = micros();
	while (startMicros + getMicrosFromSeconds(6) > micros()) {
		server.handleClient(); //Requests abarbeiten (z.B. css Datei)
	}


	//Stop Webserver
	stopWebserver();

	//Close AP
	stopAP();

	//Clear Config from EEPROM
	clearEEPROM();

	//Reboot ESP
	ESP.restart();

}







/*
void handleApiSave() {
	if (!lastWebserverMode) {
		handleNotFound();
		return;
	}
	eepromCommit();
	server.send(200, "application/json", "{\"success\":true}");
}
*/

void handleApiGet() {
	if (!lastWebserverMode) {
		handleNotFound();
		return;
	}
	server.send(200, "application/json", "{\"r\":" + String(global_r) + ", \"g\":" + String(global_g) + ", \"b\":" + String(global_b) + ", \"w\":" + String(global_w) + ", \"mode\":" + String(global_mode) + ", \"time\":" + String(global_time) + ", \"mac\":\"" + getMacString() + "\", \"autosave\":" + (eeprom_autosave?"true":"false") + "}");
}



void handleApiSet() {
	if (!lastWebserverMode) {
		handleNotFound();
		return;
	}
	String r = "";
	String g = "";
	String b = "";
	String w = "";
	String mode = "";
	String time = "";
	String key = "";
	String autosave = "";
	String save = "";
	String freq = "";

	String error = "";

	//Einlesen von GET/POST Daten
	for (uint8_t i = 0; i < server.args(); i++) {
		if (server.argName(i) == "r") {
			r = server.arg(i);
		}
		if (server.argName(i) == "g") {
			g = server.arg(i);
		}
		if (server.argName(i) == "b") {
			b = server.arg(i);
		}
		if (server.argName(i) == "w") {
			w = server.arg(i);
		}
		if (server.argName(i) == "mode") {
			mode = server.arg(i);
		}
		if (server.argName(i) == "time") {
			time = server.arg(i);
		}
		if (server.argName(i) == "key") {
			key = server.arg(i);
		}
		if (server.argName(i) == "autosave") {
			autosave = server.arg(i);
		}
		if (server.argName(i) == "save") {
			save = server.arg(i);
		}
		if (server.argName(i) == "freq") {
			freq = server.arg(i);
		}
	}

	r.trim();
	g.trim();
	b.trim();
	w.trim();
	mode.trim();
	time.trim();
	key.trim();
	autosave.trim();
	save.trim();
	freq.trim();

	if (key != "") {

		if (encryptDecrypt(hex_to_string(key)) == apiKey) {

			if (r != "") {
				if (!isNumeric(r) || r.toInt() < 0 || r.toInt() > 255) {
					r = "";
					error = "r outside of valid range";
				}
			}


			if (g != "") {
				if (!isNumeric(g) || g.toInt() < 0 || g.toInt() > 255) {
					g = "";
					error = "g outside of valid range";
				}
			}


			if (b != "") {
				if (!isNumeric(b) || b.toInt() < 0 || b.toInt() > 255) {
					b = "";
					error = "b outside of valid range";
				}
			}


			if (w != "") {
				if (!isNumeric(w) || w.toInt() < 0 || w.toInt() > 255) {
					w = "";
					error = "w outside of valid range";
				}
			}


			if (mode != "") {
				if (!isNumeric(mode) || mode.toInt() < 0 || mode.toInt() > 2) {
					mode = "";
					error = "mode outside of valid range";
				}
			}


			if (time != "") {
				if (!isNumeric(time) || time.toInt() < 0 || time.toInt() > 10000) {
					time = "";
					error = "time outside of valid range";
				}
			}


			ledSetString(r, g, b, w, mode, time);

			
			if (autosave != "")
			{
				if (!isNumeric(autosave) || autosave.toInt() < 0 || autosave.toInt() > 1) {
					error = "autosave outside of valid range";
				}
				else
				{
					if(autosave=="1")
					{
						memSetString("1", memoryAddresses[13]);
						eeprom_autosave = true;
						eepromCommit();
					}
					else
					{
						memSetString("0", memoryAddresses[13]);
						eeprom_autosave = false;
						eepromCommit();
					}
				}
			}
			if (save != "")
			{
				if (!isNumeric(save) || save.toInt() < 0 || save.toInt() > 1) {
					error = "save outside of valid range";
				}
				else
				{
					if(save=="1")
					{
						eepromCommit();
					}
				}
			}
			if (freq != "") {
				if (!isNumeric(freq) || freq.toInt() < 100 || freq.toInt() > 25000)
				{
					error = "freq outside of valid range";
				}
				else
				{
					analogWriteFreq(freq.toInt());
					memSetString(freq, memoryAddresses[14]);
					eepromCommit();
				}
			}
		}
		else
		{
			error = "apikey is invalid";
		}
	}
	else
	{
		error = "please set the apikey";
	}


	if (error != "")
	{
		server.send(200, "application/json", "{\"success\":false,\"message\":\"" + error + "\"}");
	}
	else
	{
		server.send(200, "application/json", "{\"success\":true}");
	}
}



























//CONFIG HTML


void handleConfigRoot() {
	server.send(200, "text/html", " \
								  <html> \
								  <head> \
								  <title>WiFi Configuration</title> \
								  <link rel='stylesheet' href='app.css'> \
								  <meta name='viewport' content='width=device-width, initial-scale=1.0'> \
								  </head> \
								  <body> \
								  <div id='main'> \
								  <h1>Configuration</h1> \
								  <h2>SmartHome RGBW Controller</h2> \
								  <h3 style='color: #666;'>API KEY: <b style='color: #AA0000;'>" + string_to_hex(encryptDecrypt(apiKey)) + "</b></h3> \
								  <h3 style='color: #666;'>MAC is: <b style='color: #333333;'>" + getMacString() + "</b></h3> \
								  <h3>Please enter the following information</h3> \
								  <form action='connect' method='POST'> \
								  <b>Your WiFi login credentials</b><br /> \
								  <small>This WiFi needs to be 2.4GHz.</small><br /><br /> \
								  <label for='ssid'>SSID</label><br /> \
								  <input type='text' name='ssid' id='ssid' placeholder='SSID' maxlength='32' required='required' value='" + memGetString(memoryAddresses[0]) + "'><br /><br /> \
								  <label for='pass'>Password</label><br /> \
								  <input type='text' name='pass' id='pass' placeholder='Password' maxlength='32'><br /><br /> \
								  <iframe src='ssidlist.html'></iframe><br /><br /> \
								  <label for='ip'>IP</label><br /> \
								  <input type='text' name='ip' id='ip' placeholder='192.168.2.201' maxlength='15' required='required' value='" + memGetString(memoryAddresses[2]) + "'><br /><br /> \
								  <label for='snm'>Subnetmask</label><br /> \
								  <input type='text' name='snm' id='snm' placeholder='255.255.255.0' maxlength='15' required='required' value='" + memGetString(memoryAddresses[3]) + "'><br /><br /> \
								  <label for='gw'>Gateway</label><br /> \
								  <input type='text' name='gw' id='gw' placeholder='192.168.2.1' maxlength='15' required='required' value='" + memGetString(memoryAddresses[4]) + "'><br /><br /> \
								  <input type='submit' value=' Save configuration '><br /> \
								  </form> \
								  </div> \
								  </body> \
								  </html> \
								  ");
}








void handleSSIDList() {
	if (lastWebserverMode) {
		handleNotFound();
		return;
	}
	String WifiNetworks[50];
	int wifiArrayIndex = 0;
	int n = WiFi.scanNetworks();
	if (n > 0) {
		for (int i = 0; i < n; ++i)
		{
			if (wifiArrayIndex >= 49) continue;
			boolean isInArray = false;
			for (int j = 0; j < 50; j++) {
				if (WifiNetworks[j] == WiFi.SSID(i)) isInArray = true;
			}
			if (!isInArray) WifiNetworks[wifiArrayIndex++] = WiFi.SSID(i);
			//Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
		}
	}
	String html =
		"<html>"
		"	<head>"
		"		<title>WiFi Configuration</title>"
		"		<link rel='stylesheet' href='app.css'>"
		"	</head>"
		"	<body>"
		"		<b>Available Networks</b><br/>";
	for (int i = 0; i < wifiArrayIndex; ++i)
	{
		html += WifiNetworks[i] + "<br/>";
	}
	html += "</body></html>";
	server.send(200, "text/html", html);
}













String htmlConnect = "<h3>Tryig to connect to WiFi...</h3><b>Closing Configuration WiFi now, will reopen in 30 seconds if connection fails.</b>";
String htmlConnectError = "<h3>Please enter your WiFi Credentials and a valid IP Configuration for the Controller!</h3><a href='/'><b>Back</b></a>";
String htmlMessageHead = " \
						 <html> \
						 <head> \
						 <title>WiFi Configuration</title> \
						 <link rel='stylesheet' href='app.css'> \
						 <meta name='viewport' content='width=device-width, initial-scale=1.0'> \
						 </head> \
						 <body> \
						 <div id='main'> \
						 <h1>Configuration</h1> \
						 ";
String htmlMessageFoot = "</div></body></html>";





void handleConnect() {
	if (lastWebserverMode) {
		handleNotFound();
		return;
	}
	String ssid = "";
	String pass = "";
	String ip = "";
	String snm = "";
	String gw = "";

	//Einlesen von POST Daten
	for (uint8_t i = 0; i < server.args(); i++) {
		if (server.argName(i) == "ssid") {
			ssid = server.arg(i);
		}
		if (server.argName(i) == "pass") {
			pass = server.arg(i);
		}
		if (server.argName(i) == "ip") {
			ip = server.arg(i);
		}
		if (server.argName(i) == "snm") {
			snm = server.arg(i);
		}
		if (server.argName(i) == "gw") {
			gw = server.arg(i);
		}
	}


	if (
		(ssid.length() > 0 && ssid.length() <= 64 && pass.length() <= 32) &&
		(ip.length() >= 7 && ip.length() <= 15) &&
		(snm.length() >= 7 && snm.length() <= 15) &&
		(gw.length() >= 7 && gw.length() <= 15) &&
		(isValidIp4(ip) && isValidIp4(snm) && isValidIp4(gw))
		)
	{

		server.send(200, "text/html", htmlMessageHead + htmlConnect + htmlMessageFoot);

		Serial.println("Got Config from Webinterface, waiting for final requests");

		//Auf �brige requests warten
		unsigned long startMicros = micros();
		while (startMicros + getMicrosFromSeconds(6) > micros()) {
			server.handleClient(); //Requests abarbeiten (z.B. css Datei)
		}

		//Stop Webserver
		stopWebserver();

		//Close AP
		stopAP();

		//Save new wifi login information and ip config
		memSetString(ssid, memoryAddresses[0]);
		memSetString(pass, memoryAddresses[1]);
		memSetString(ip, memoryAddresses[2]);
		memSetString(snm, memoryAddresses[3]);
		memSetString(gw, memoryAddresses[4]);
		eepromCommit();

		//Verbindung zum WLAN versuchen
		connectToWifi(ssid, pass);

		//Auf Verbindung warten...
		Serial.println("Connecting to WiFi...");
		int counter = 0;
		while (!wifiIsConnected()) {
			Serial.print(".");
			delay(1000);
			yield(); //Do Wifi actions
			if (counter++ == 30) break; //30 sekunden
		}
		Serial.print("\n");

		//Verbunden?
		if (!wifiIsConnected()) {
			//Konnte nach 10 Sekunden nicht zu WLAN verbinden
			Serial.println("Could not connect to WiFi. Reopening Config AP/Server");
			//Access Point zur Konfiguration und Status �bersicht Starten
			registerAP();
			//Webserver f�r HTML Seiten starten
			registerWebserver(false);
		}
		else {
			//Erfolgreich verbunden
			allUp = true;
			Serial.println("Connected to WiFi " + ssid);
			Serial.println("My IP is: " + WiFi.localIP().toString());

			//Start Servers
			registerTelnet();
			registerWebserver(true);
		}

	}
	else
	{
		server.send(200, "text/html", htmlMessageHead + htmlConnectError + htmlMessageFoot);
	}

}







