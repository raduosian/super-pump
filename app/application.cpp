#include <user_config.h>
#include <SmingCore/SmingCore.h>

#include <SmingCore/Network/TcpServer.h>
#include <SmingCore/Network/TcpClient.h>
#include <Wiring/WString.h>

#include "SwitchDevice.h"
#include "auto_types.h"
#include "TempSensor.h"
#include "CommandInterface.h"

#define APP_NAME "Super Pump"
#define VERSION "1.0.3 b"

Timer procTimer;
TcpClient* TcpClient1 = NULL;
CommandInterface CI;

void sendString (String &str);
void init_wifi();

#define FLOOR_PUMP2_PIN 2

void tcpServerClientComplete(TcpClient& client, bool succesfull);
void tcpServerClientConnected (TcpClient* client);
bool tcpServerClientReceive (TcpClient& client, char *data, int size);

TcpServer tcpServer(tcpServerClientConnected, tcpServerClientReceive, tcpServerClientComplete);

String state_str[] = {"NONE", "OFF", "FLOOR_HEATING"};

SwitchDevice FloorPump2(FLOOR_PUMP2_PIN);

HeatSystemState state = OFF;
HeatSystemState new_state = OFF;

bool manual_control = true;
bool restart_cmd = false;
void readSensors();
void checkState ();
void runCommand (const TermoCommand &TC);
void log_status();

void onPrintSystemTime()
{
	Serial.print("Local Time: ");
	Serial.print(SystemClock.getSystemTimeString(eTZ_Local));
	Serial.print(" ");
	Serial.print("UTC Time:   ");
	Serial.println(SystemClock.getSystemTimeString(eTZ_UTC));
}

// Called when time has been received by NtpClient (option 1 or 2)
// Either after manual requestTime() or when
// and automatic request has been made.
void onNtpReceive(NtpClient& client, time_t timestamp)
{
	SystemClock.setTime(timestamp, eTZ_UTC);

	Serial.print("Time synchronized: ");
	Serial.println(SystemClock.getSystemTimeString());
}

void run()
{
	if (restart_cmd == true){
		System.restart();
	}
	checkState();
	log_status();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	//Serial.systemDebugOutput(true); // Allow debug output to serial
	procTimer.initializeMs (10000, run).start();   // every 10 seconds
	init_wifi ();
}

void log_status()
{
	String Status = ("Pump State: ")+ (state_str[state])+"\r\n";
	Serial.print(Status);
	sendString(Status);
}

/* Hard coded logic for automation! Shame!*/
void checkState ()
{
	HeatSystemState old_state = state;
	String mode;
	if (manual_control == true){
		mode = "Manual";
		if (new_state != STATE_NONE && new_state != state){
			state = new_state;
			new_state = STATE_NONE;
			if (state == OFF){
				FloorPump2.switchOff();
			} else if (state == FLOOR_HEATING){
				FloorPump2.switchOn();
			}
		}
	} else{ //automatic change of states based on sensors
	}
	if (old_state != state){
		String msg = mode+" changed state from ";
		msg += state_str[old_state];
		msg += " to ";
		msg += state_str[state];
		msg += "mode ";
		msg += mode+"\r\n";
		Serial.print(msg);
		sendString(msg);
	}
}

void sendString (String &str)
{
	if (TcpClient1 != nullptr){
		 TcpClient1->sendString(str);
	}
}

bool tcpServerClientReceive (TcpClient& client, char *data, int size)
{
	Serial.printf("received %d\r\n", size);
	int rem = size;
	while (rem > 0){
		if (CI.newCommand (data[size-rem])){
			runCommand (CI.getCommand());
		}
		rem--;
	}
	return true;
}

void tcpServerClientComplete(TcpClient& client, bool succesfull)
{
	debugf("Application CompleteCallback : %s \r\n",client.getRemoteIp().toString().c_str() );
}

void tcpServerClientConnected (TcpClient* client)
{
	Serial.printf("server connect Callback : %s\r\n",client->getRemoteIp().toString().c_str());
	TcpClient1 = client;
	sendString (String(APP_NAME)+" version "+VERSION+"\r\n");
	tcpServer.setKeepAlive(65535);
}
