#include "auto_types.h"
#include "CommandInterface.h"
#include "TempSensor.h"
#include "SwitchDevice.h"

extern SwitchDevice FloorPump2;
extern HeatSystemState state;
extern HeatSystemState new_state;
extern bool restart_cmd;
extern bool manual_control;

void sendString (String &str);

void runCommand (const TermoCommand &TC)
{
	String tcp_str;
	String response = " runCommand: "+TC.action+" "+TC.pname+" "+TC.val;
	if (TC.action  == "set"){
		if (TC.pname == "fpump"){
			if (TC.val)
				FloorPump2.switchOn();
			else FloorPump2.switchOff();
		} else if (TC.pname == "state"){
			if (TC.val >= 0 && TC.val < MAX_STATES){
				auto a = TC.val;
				new_state = (HeatSystemState)(int)a;
				response += String("> new state ")+ new_state+"\r\n";
			}
		} else if (TC.pname == "manual"){
			manual_control = (int)(TC.val);
			response += String("> manual control is ")+ manual_control+"\r\n";
		}else
			response += "\r\n";
	} else if (TC.action  == "reset"){
		if (TC.pname == "system"){
			restart_cmd = true;
			response += String("> restart ");
		}
	}
	sendString (response);
}
