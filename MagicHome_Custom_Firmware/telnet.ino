void registerTelnet()
{
	TelnetServer.begin();
	TelnetServer.setNoDelay(true);
	Serial.println("Telnet server started");
}

void handleTelnet()
{
	uint8_t i;
	//check if there are any new clients
	if (TelnetServer.hasClient()) {
		for (i = 0; i < MAX_SRV_CLIENTS; i++) {
			//find free/disconnected spot
			if (!serverClients[i] || !serverClients[i].connected()) {
				if (serverClients[i]) {
					serverClients[i].stop();
					serverClientsAuthorized[i] = false;
					serverClientsResponseEnabled[i] = false;
				}
				serverClients[i] = TelnetServer.available();
				if (serverClients[i] && serverClients[i].connected()) {
					serverClients[i].println("welcome");
				}
				//Serial.println("new telnet client: "+(String(i))+" of "+MAX_SRV_CLIENTS);
				continue;
			}
		}
		//no free/disconnected spot so reject
		WiFiClient serverClient = TelnetServer.available();
		serverClient.stop();
	}
	//check clients for data
	for (i = 0; i < MAX_SRV_CLIENTS; i++) {
		if (serverClients[i] && serverClients[i].connected()) {
			if (serverClients[i].available()) {
				//get data from the telnet client and push it to the UART
				while (serverClients[i].available()) {
					String command = serverClients[i].readStringUntil('\n');
					command.trim();
					if (command == "ping")
					{
						serverClients[i].println("pong");
						continue;
					}
					else if (command == "enable response")
					{
						serverClientsResponseEnabled[i] = true;
						serverClients[i].println("ok");
					}
					else if (command == "disable response")
					{
						serverClientsResponseEnabled[i] = false;
						serverClients[i].println("ok");
					}
					else if (command == "get")
					{
						serverClients[i].println(String(global_r) + "|" + String(global_g) + "|" + String(global_b) + "|" + String(global_w) + "|" + String(global_mode) + "|" + String(global_time) + "|" + getMacString() + "|" + (eeprom_autosave?"1":"0") );
						continue;
					}
					else if (command == "exit")
					{
						serverClients[i].stop();
						serverClientsAuthorized[i] = false;
						continue;
					}
					else if (serverClientsAuthorized[i])
					{
						if (command == "save")
						{
							eepromCommit();
							if(serverClientsResponseEnabled[i]) serverClients[i].println("ok");
							continue;
						}
						else if (command == "enable autosave")
						{
							eeprom_autosave = true;
							memSetString("1", memoryAddresses[13]);
							if(serverClientsResponseEnabled[i]) serverClients[i].println("ok");
							eepromCommit();
							continue;
						}
						else if (command == "disable autosave")
						{
							eeprom_autosave = false;
							memSetString("0", memoryAddresses[13]);
							if(serverClientsResponseEnabled[i]) serverClients[i].println("ok");
							eepromCommit();
							continue;
						}
						else
						{
							if(handleCommad(command))
							{
								if(serverClientsResponseEnabled[i]) serverClients[i].println("ok");
							}
							else
							{
								if(serverClientsResponseEnabled[i]) serverClients[i].println("error");
							}
						}
					}
					else
					{
						if (encryptDecrypt(hex_to_string(command)) == apiKey)
						{
							serverClients[i].println("authorized");
							serverClientsAuthorized[i] = true;
						}
						else
						{
							serverClients[i].println("invalid apikey");
						}
					}
				}
			}
		}
	}
}
