#include "CommandInterface.h"
#include <HardwareSerial.h>

#include "SwitchDevice.h"
#include "TempSensor.h"
#include <algorithm>

extern TempSensor BoilerSensor;
extern TempSensor PipeSensor;
extern SwitchDevice Centralina;
extern SwitchDevice FloorPump1;
extern SwitchDevice HotWaterPump;
extern SwitchDevice Valve1;

CommandInterface::CommandInterface ()
{
	bValid = false;
	newCmd = false;
}

/**/
bool CommandInterface::newCommand (char c)
{
	bool result = false;
	read (c);
	result =  newCmd;
	newCmd = false;
	return result;
}

void CommandInterface::read (char c)
{
	if (c == '\r') {
		Command.clear();
		parseCommandString (RawCmd);
		RawCmd = "";
		RawCmd.remove (0);
	  //if (bValid) { //todo check command validity
		//  newCmd = true;
	  //}
	} else if(int(c) < 32) {
		Serial.printf ("c: %d\r\n", c);
	}
	else
		RawCmd += c;
}

void CommandInterface::parseCommandString (String & scmd)
{
	Vector<String> split;
	Serial.printf("scmd %s %d\r\n",scmd.c_str(), scmd.c_str()[0]);
	int n =  splitString(scmd, int(' '), split);
	if (split.size() == 0){
		Serial.printf("split size %d\r\n", split.size());
		Command.clear ();
		return;
	}
	Command.action = split.at(0);
	if (!isValidActionName(Command.action)){
		String response = "invalid action: ="+Command.action+"="+Command.pname+" "+Command.val+"\r\n";
		Serial.print(response.c_str());
		Command.clear ();
		return;
	}
	if (split.size() > 1) {
		Command.pname = split.at(1);
		if (!isValidParamName (Command.pname)) {
			String response = "invalid param name:="+Command.action+"="+Command.pname+" "+Command.val+"\r\n";
			Serial.print(response.c_str());
			Command.clear ();
			return;
		}
	}
	if (split.size() > 2) {
		Command.val = (split.at(2)).toFloat ();
	}
	newCmd = true;
}

bool CommandInterface::isValidParamName (String &pname)
{
	for (int i = 0; i < sizeof(paramsNames); i++){
		if (paramsNames[i].equals(pname)){
			Serial.print ("found param "); Serial.println(pname);
			return true;
		}
	}
	Serial.print ("not found param "); Serial.println(pname);
	return false;
}

bool CommandInterface::isValidActionName (String &aname)
{
	for (int i = 0; i < sizeof(actionsNames); i++){
		if (actionsNames[i].equals(aname)){
			Serial.print ("found action "); Serial.println(aname);
			return true;
		}
	}
	return false;
}
