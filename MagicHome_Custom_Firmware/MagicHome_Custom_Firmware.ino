#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include <ESP8266HTTPClient.h>
#include <EEPROM.h>
#include <PWMFader.h>


//########## CONFIG ##########


//LED Pins
const int RPin = 14;
const int GPin = 5;
const int BPin = 12;
const int WPin = 13;


//API Key (Internal one, the External key will be this String encrypted by the MAC and ChipIP)
String apiKey = "J5scc8te";

//Wifi SSID/PW for AP
String _ssid = "SmartHome RGBW";
String _pass = "espconfig";

//IP Config
IPAddress ip(10, 10, 10, 10);
IPAddress mask(255, 0, 0, 0);

//Servers
ESP8266WebServer server(80); //Web Server Port
WiFiServer TelnetServer(23); //Telnet Server Port
#define MAX_SRV_CLIENTS 10   //Max Clients
WiFiClient serverClients[MAX_SRV_CLIENTS];
bool serverClientsAuthorized[MAX_SRV_CLIENTS];
bool serverClientsResponseEnabled[MAX_SRV_CLIENTS];



int memoryAddresses[15] = {
	0,   //ssid(String)						=> 64  =>   0-64	0
	65,  //password(String)					=> 32  =>  65-97	1
	98,  //ip(IP)							=> 15  =>  98-113	2
	114, //subnet(IP)						=> 15  => 114-129	3
	130, //gateway(IP)						=> 15  => 130-145	4

	146, //apikey(String)					=> 32  => 146-178	5

	179, //r(0 - 255)						=> 3   => 179-182	6
	183, //g(0 - 255)						=> 3   => 183-186	7
	187, //b(0 - 255)						=> 3   => 187-190	8
	191, //w(0 - 255)						=> 3   => 191-194	9
	195, //mode(on|off|fade|etc.)			=> 1   => 195-196	10
	197, //transitiontime(0ms - 10000ms)	=> 5   => 197-202	11

	203, //EEPROM data valid ("valid")		=> 5   => 203-208	12
	209, //EEPROM autosave					=> 1   => 209-210	13
	211  //Frequency						=> 5   => 211-216	14

};

//########## END CONFIG ##########


bool allUp = false;
bool lastWebserverMode = false;
PWMFader fader(RPin, GPin, BPin, WPin);
//Current Status
int global_r = 0;
int global_g = 0;
int global_b = 0;
int global_w = 0;
int global_mode = 0;
int global_time = 0;
byte rgb_fade_color = 0;
bool rgb_fader = false;
long lastmillis = 0;
bool eeprom_changed = false;
bool eeprom_autosave = false;


void setup(void)
{
	/** START BOOT SEQUENCE **/

	//Debug Verbindung
	Serial.begin(115200);
	Serial.println("\n\nBOOTING...\n");

	//EEPROM init
	EEPROM.begin(2048); // up to 4096

	//Pins Initialisieren
	pinMode(RPin, OUTPUT);
	pinMode(GPin, OUTPUT);
	pinMode(BPin, OUTPUT);
	pinMode(WPin, OUTPUT);
	analogWrite(RPin, 0);
	analogWrite(GPin, 0);
	analogWrite(BPin, 0);
	analogWrite(WPin, 0);



	//Boot Delay
	delay(1000);

	//EEPROM DATEN VALIDIEREN
	String dataValidation = memGetString(memoryAddresses[12]);
	if (dataValidation != "valid")
	{
		Serial.println("Invalid EEPROM data, running firsttime setup...");
		clearEEPROM();
	}

	
	//PWM Frequenz ändern
	analogWriteFreq(memGetString(memoryAddresses[14]).toInt()); //25khz - nicht mehr hörbar
	//analogWriteFreq(100);

	//PWM Range ändern
	analogWriteRange(255);


	//EEPROM AUTOSAVE RESTORE
	String as = memGetString(memoryAddresses[13]);
	if(as == "1") eeprom_autosave = true;


	//reset session spesific telnet options
	for (int i = 0; i < MAX_SRV_CLIENTS; i++)
	{
		serverClientsAuthorized[i] = false;
		serverClientsResponseEnabled[i] = false;
	}


	//Versuchen zu gespeichertem WLAN zu verbinden
	String ssid = memGetString(memoryAddresses[0]);
	String pass = memGetString(memoryAddresses[1]);
	if (ssid != "" && pass != "") {
		Serial.println("######################################");
		Serial.println("Found Configuration");
		Serial.println("SSID: " + ssid);
		//Serial.println("PASSWD: "+pass);
		Serial.println("IP: " + memGetString(memoryAddresses[2]));
		Serial.println("SUBNET: " + memGetString(memoryAddresses[3]));
		Serial.println("GATEWAY: " + memGetString(memoryAddresses[4]));
		Serial.println("APIKEY: " + string_to_hex(encryptDecrypt(apiKey)));
		Serial.println("######################################");


		//Wenn SSID gespeichert ist
		connectToWifi(ssid, pass);
		//Warten ob verbindung hergestellt werden kann
		int counter = 0;
		while (!wifiIsConnected()) {
			delay(500);
			Serial.print(".");
			if (counter++ == 10*120) break; //10 minuten versuchen
		}
		Serial.print("\n");

		//Verbunden?
		if (!wifiIsConnected()) {
			//Konnte nach 10 Sekunden nicht zu WLAN verbinden
			Serial.println("Could not connect to saved WiFi");
			//Access Point zur Konfiguration und Status uebersicht Starten
			registerAP();
			//Webserver fuer HTML Seiten starten
			registerWebserver(false);
		}
		else {
			//Erfolgreich verbunden
			allUp = true;
			Serial.println("Connected to saved WiFi " + ssid);
			Serial.println("IP is: " + WiFi.localIP().toString());

			Serial.println("Restoring LED Mode...");
			String tmp_r = memGetString(memoryAddresses[6]); //R
			String tmp_g = memGetString(memoryAddresses[7]); //G
			String tmp_b = memGetString(memoryAddresses[8]); //B
			String tmp_w = memGetString(memoryAddresses[9]); //W
			String tmp_mode = memGetString(memoryAddresses[10]); //MODE
			String tmp_time = memGetString(memoryAddresses[11]); //TIME
			ledSetString(tmp_r, tmp_g, tmp_b, tmp_w, tmp_mode, tmp_time);

			//Start Servers
			registerTelnet();
			registerWebserver(true);
		}
	}
	else {
		//Wenn SSID nicht gespeichert ist
		Serial.println("No WiFi saved");
		//Access Point zur Konfiguration und Status Uebersicht Starten
		registerAP();
		//Webserver fuer HTML Seiten starten
		registerWebserver(false);
	}


}

void loop(void)
{
	if (allUp)
	{
		if (wifiIsConnected())
		{
			server.handleClient();
			handleTelnet();
			ledUpdate();
			if (eeprom_autosave && round(millis() / 10) % 100 == 0 && lastmillis != millis() && eeprom_changed && !fader.isFading()) {
				lastmillis = millis();
				eeprom_changed = false;
				eepromCommit();
			}
		}
		else
		{
			//Neu Verbinden...
			Serial.println("Conection lost!");
			delaySec(5); //5 Sek Warten
			Serial.println("Trying to reconnect...");
			WiFi.reconnect(); //Versuchen neu zu verbinden
			delaySec(5); //5 Sek Warten
		}
	}
	else
	{
		//Requests for Config Webinterface
		server.handleClient();
	}
}
