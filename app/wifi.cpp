#include <user_config.h>
#include <SmingCore/SmingCore.h>

#include <SmingCore/Network/TcpServer.h>
#include <SmingCore/Network/TcpClient.h>
#include <Wiring/WString.h>
#include <SmingCore/Digital.h>

#define WIFISTA
//#define WIFI_SSID "WLAN1-HW"
//#define WIFI_PWD "4FE18EE3968"
#define WIFI_SSID "kinternetb"
#define WIFI_PWD "asuswl500g"
//#define WIFI_SSID "batranul_kinternet"
//#define WIFI_PWD "mamaliga"
//#define WIFI_SSID "kinternet"
//#define WIFI_PWD "451*BCdMf"
//#define WIFI_SSID "Diginesis - Guests" // Put you SSID and Password here
//#define WIFI_SSID "Diginesis-Protime"
//#define WIFI_PWD "67890poi"

extern TcpClient* TcpClient1;
extern TcpServer tcpServer;

// Will be called when WiFi station was disconnected
void connectFail(String ssid, uint8_t ssidLength, uint8_t *bssid, uint8_t reason)
{
	// The different reason codes can be found in user_interface.h. in your SDK.
	debugf("Disconnected from %s. Reason: %d", ssid.c_str(), reason);
	if(TcpClient1 != NULL) TcpClient1->close();
}


// Will be called when WiFi hardware and software initialization was finished
// And system initialization was completed
void ready()
{
	debugf("READY!");
	// If AP is enabled:
	Serial.printf("AP. ip: %s mac: %s",\
			WifiAccessPoint.getIP().toString().c_str(), \
			WifiAccessPoint.getMAC().c_str());
	tcpServer.close();
	tcpServer.listen(9000);
}

#define WIFISTA


void connectOk(IPAddress ip, IPAddress mask, IPAddress gateway)
{
	debugf("I'm CONNECTED");
	Serial.println(ip.toString());
	bool listen = tcpServer.listen(9000);
	Serial.printf("listen = %d\r\n", listen);
}

void init_wifi()
{
#ifdef WIFISTA
	WifiAccessPoint.enable(false);
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here
	// Print available access points
	//WifiStation.startScan(listNetworks);
	// In Sming we can start network scan from init method without additional code
	// Run our method when station was connected to AP (or not connected)

	//WifiStation.waitConnection(connectOk, 30, connectFail); // We recommend 20+ seconds at start
	WifiEvents.onStationGotIP(connectOk);

	// Set callback that should be triggered if we are disconnected or connection attempt failed
	WifiEvents.onStationDisconnect(connectFail);

	//WifiStation.setIP(IPAddress(192, 168, 1, 171));
#else
	WifiStation.enable(false);
	WifiAccessPoint.enable(true);
	WifiAccessPoint.config("IOTSming", "", AUTH_OPEN);
	// Optional: Change IP addresses (and disable DHCP)
	WifiAccessPoint.setIP(IPAddress(192, 168, 2, 1));
	tcpServer.listen(9000);
#endif
}

// Will be called when WiFi station network scan was completed
void listNetworks(bool succeeded, BssList list)
{
	if (!succeeded)	{
		Serial.println("Failed to scan networks");
		return;
	}
	for (int i = 0; i < list.count(); i++)	{
		Serial.print("\tWiFi: ");
		Serial.print(list[i].ssid);
		Serial.print(", ");
		Serial.print(list[i].getAuthorizationMethodName());
		if (list[i].hidden) Serial.print(" (hidden)");
		Serial.println();
	}
}
