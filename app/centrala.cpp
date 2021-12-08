#include <Libraries/OneWire/OneWire.h>
#include <Libraries/DS18S20/ds18s20.h>
#include <Wiring/WString.h>

#include "CommandInterface.h"


#define HEAT_RELAY_PIN 5
#define PUMP1_RELAY_PIN 4
#define DSBUS_PIN 0

#define INVALID_TEMP -1000.0f
class Centrala {
public:
	Centrala ()
	{
		for (uint8_t i = 0; i < 50; i++) {
			tempBuffer [i] = INVALID_TEMP;
		}
		pinMode (HEAT_RELAY_PIN, OUTPUT);
		pinMode (PUMP1_RELAY_PIN, OUTPUT);

		heatState = false;
		pump1State = false;
		digitalWrite (HEAT_RELAY_PIN, heatState);
		digitalWrite (PUMP1_RELAY_PIN, pump1State);
	}
	void init ()
	{
		heatOnTime = 0;
		heatOffTime = 0;
		pump1OffTime = 0;
		pump1OnTime = 0;

		P1TempOff = 26;
		currentIndex = 50;
		DSBus.Init(DSBUS_PIN);
		DSBus.StartMeasure(); // first measure start,result after 1.2 seconds * number of sensors
	}

	void run ()
	{
		float currentTemp = readTemp();

		if (currentIndex++ >= 50) {
			currentIndex = 0;
		}
		tempBuffer [currentIndex] = currentTemp;

		/*compute temperature from samples */
		float sumTemp = 0.0;
		int samplesCount = 0;
		for (uint8_t i = 0; i < 50; i++) {
			if (tempBuffer [i] != INVALID_TEMP){
				sumTemp += tempBuffer[i];
				samplesCount++;
			}
		}
		if (samplesCount > 0){
			P1Temp = sumTemp/samplesCount;
		}
		processState (P1Temp);
	}

	void processState (float currentTemp)
	{
		if (currentTemp != INVALID_TEMP) {//
			sendTemp (currentTemp);
			if (heatState == false){
				if (pump1State == true){// is OFF
					if (currentTemp < P1TempOff) {
						pump1State = false;
						digitalWrite (PUMP1_RELAY_PIN, pump1State);
					}
				}
			} else {

			}
		} else { //we have not a valid measure of temperature


		}
	}

	float readTemp()
	{
		float temp = INVALID_TEMP;
		uint8_t a;
		uint64_t info;
		String tcp_str;
		if (!DSBus.MeasureStatus())  {// the last measurement completed
			if (DSBus.GetSensorsCount()) {  // is minimum 1 sensor detected ?
				for(a=0; a < DSBus.GetSensorsCount(); a++)  { // prints for all sensors
					Serial.print(" T");
					Serial.print(a+1);
					Serial.print(" = ");
					if (DSBus.IsValidTemperature(a))   // temperature read correctly ?
					{
						temp = DSBus.GetCelsius(a);
						Serial.print (DSBus.GetCelsius(a));
						Serial.print("\r\n");// Celsius, (");
					}
					else
						Serial.println("Temperature not valid");

		    /*  Serial.print(" <Sensor id.");
		      info= TempSensor.GetSensorID(a)>>32;
		      Serial.print((uint32_t)info,16);
		      Serial.print((uint32_t)ReadTemp.GetSensorID(a),16);
		      Serial.println(">");
		      if( TcpClient1 != NULL){
		    	  tcp_str = ((const char*)"TEMP ");
		    	  tcp_str += String((uint32_t)info, 16);
		    	  tcp_str += (const char*)" ";
		    	  tcp_str += ReadTemp.GetCelsius(a);
		    	  tcp_str += (const char*)"\r\n";
		    	  Serial.println(tcp_str);
	    		  TcpClient1->sendString(tcp_str);
	    	  }*/
				}
		    DSBus.StartMeasure();  // next measure, result after 1.2 seconds * number of sensors
			} else {
				Serial.println("No sensors found!");
			}
		}
		else
			Serial.println("No valid measurement so far! wait please");

		return temp;
	}
	bool receive (char *data, int size)
	{
		Serial.printf("received %d\r\n", size);
		int rem = size;
		while (rem > 0){
			if (CI.newCommand (data[size-rem])){
				runCommand (CI.getCommand());
			}
			rem--;
		}
	}
	void setSendFunction (void (*fct) (String &))
	{
		sendString = fct;
	}

	void sendTemp (float temp)
	{
		String tcp_str = ((const char*)"P1TEMP ");
		tcp_str = P1Temp;
		tcp_str += " pump ";
		tcp_str += (int)pump1State;
		tcp_str += " heat ";
		tcp_str += (int)heatState;
		tcp_str += (const char*)"\r\n";
		sendString(tcp_str);
	}
	void sendStatus (String &tcp_str)
	{

		tcp_str = " Heat=";
		tcp_str += (int)heatState;
		tcp_str += " heat ";
		if (heatState){
			tcp_str += " on for ";
			tcp_str += heatOnTime;
		} else {
			tcp_str += " off for ";
			tcp_str += heatOffTime;
		}
		tcp_str += " s ";
		tcp_str = ((const char*)"P1temp=");
		tcp_str += P1Temp;
		tcp_str += " Pump1 ";
		if (pump1State){
			tcp_str += "on for ";
			tcp_str += pump1OnTime;
		} else {
			tcp_str += "off for ";
			tcp_str += pump1OffTime;
		}
		tcp_str += " s ";
		tcp_str += (const char*)"\r\n";
		sendString(tcp_str);
	}
	void runCommand (const TermoCommand &TC)
	{
		String tcp_str;
		String response = " runcommand: invalid command:"+TC.action+" "+TC.pname+" "+TC.val;
		if (TC.action  == "set"){
			if (TC.pname == "heat"){
				bool state = (int)TC.val;
				if (state !=  heatState){
					heatState = state;
					digitalWrite (HEAT_RELAY_PIN, heatState);
					if (state){
						heatOnTime = 0;
					} else {
						heatOffTime = 0;
					}
					response = String("heat set to ")+heatState;
				} else {
					response = String("heat already set to ")+heatState;
				}
			} else if (TC.pname == "pump1"){
				bool state = (int)TC.val;
				if (state !=  pump1State){
					pump1State = state;
					digitalWrite (PUMP1_RELAY_PIN, pump1State);
					if (state){
						pump1OnTime = 0;
					} else {
						pump1OnTime = 0;
					}
					response = String("pump1 set to ")+pump1State;
				} else {
					response = String("pump1 already set to ")+pump1State;
				}
			} else;
		} else if (TC.action == "get"){
			if (TC.pname == "p1temp"){
				response = String("p1temp=")+P1Temp;
			} else if (TC.pname == "status"){
				sendStatus (response);
			} /*else if (TC.pname == "ton"){
				response = String("timeon=")+ timeOn;
			} else if (TC.pname == "toff"){
				response = String("timeoff=")+ timeOff;
			} else if (TC.pname == "ontime"){
				response = String("max on time=")+ maxOnTime;
			} else if (TC.pname == "offtime"){
				response = String("min off time=")+ minOffTime;
			} else;*/
		} else if (TC.action == "reset"){
			if (TC.pname == "system"){
				response = "Restarting Ferrolina by request";
				sendString (response);
				System.restart ();
			} else if (TC.pname == "sensor"){
				response = "Resetting sensors";
				DSBus.Init(0);
				DSBus.StartMeasure();
			}
		}else;

		response +="\r\n";
		sendString (response);
		Serial.print(response.c_str());
	}
private:
	CommandInterface CI;
	DS18S20 DSBus;
	bool heatState;
	bool pump1State;

	uint32_t heatOnTime;
	uint32_t heatOffTime;
	uint32_t pump1OffTime;
	uint32_t pump1OnTime;

	float P1TempOff;
	float P1Temp;
	float tempBuffer[50];
	uint8_t currentIndex;
	float targetTemp;
	void (*sendString)(String &) = nullptr;
};

