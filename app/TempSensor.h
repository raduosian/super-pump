
#ifndef _TEMP_SENSOR_INCLUDED
#define _TEMP_SENSOR_INCLUDED

#include <Libraries/OneWire/OneWire.h>
#include <Libraries/DS18S20/ds18s20.h>
#include <Wiring/WString.h>
#include <Wiring/WVector.h>
#define NONE -1
#define INVALID_TEMP -1000.0f
#define TEMP_BUFFER_SIZE 5

/*
 * Read temperature from a 18b20 sensor, store them in a queue
 * D0 - 16		D5	14
 * D1 -  5		D6	12
 * D2 -  4		D7	13
 * D3 -  0		D8	15
 * D4 -  2
 * */

struct SensorData {
	SensorData()
	{
		for (uint8_t i = 0; i < TEMP_BUFFER_SIZE; i++) {
			tempBuffer [i] = INVALID_TEMP;
		}
		currentIndex = 0;
	}
	void record(float temp)
	{
		tempBuffer [currentIndex] = temp;
		if (++currentIndex >= TEMP_BUFFER_SIZE) {
			currentIndex = 0;
		}
	}
	float get_temp ()
	{
		/*compute temperature from samples */
		float sumTemp = 0.0;
		int samplesCount = 0;
		for (uint8_t i = 0; i < TEMP_BUFFER_SIZE; i++) {
			if (tempBuffer [i] != INVALID_TEMP){
				sumTemp += tempBuffer[i];
				samplesCount++;
			}
		}
		if (samplesCount > 0){
			return sumTemp/samplesCount;
		} else {
			return INVALID_TEMP;
		}
	}
	String address;
	float tempBuffer[TEMP_BUFFER_SIZE];
	float temperature;
	unsigned int currentIndex;
};

class TempSensor
{
public:
	TempSensor(int pin_ = NONE)
	{
		//address = address_;
		//if (pin_ != NONE && initialized == false)
		//	init(pin_);
		address_ix = getNextIndex();

	}
	String getAddress()
	{
		return sensors[address_ix].address;
	}

	float getTemp ()
	{
		return sensors[address_ix].get_temp();
	}

	static void init (int pin_)
	{
		if (initialized == true){
			//print already initialized
		} else {
			Serial.println("Initializing DSBus");
			DSBus.Init(pin_);
			DSBus.StartMeasure(); // first measure start,result after 1.2 seconds * number of sensors
			initialized = true;
		}
		//TempSensor::sensors = Vector<SensorData> ();
	}

	static float readTemp()
	{
		float temp = INVALID_TEMP;
		uint8_t i;
		if (!DSBus.MeasureStatus())  {// the last measurement completed
			if (DSBus.GetSensorsCount()) {  // is minimum 1 sensor detected ?
				for(i=0; i < DSBus.GetSensorsCount(); i++)  { // prints for all sensors
					if (DSBus.IsValidTemperature(i))   {// temperature read correctly ?
						temp = DSBus.GetCelsius(i);
						if (i < max_index) {
							sensors[i].record(temp);
						} else {
							Serial.print ("Not used sensor: ");
							Serial.print(temp);
							Serial.print("\r\n");// Celsius, (");
						}
					}
					else {
						errors++;
						Serial.println("Temperature not valid");
					}
				}
			DSBus.StartMeasure();  // next measure, result after 1.2 seconds * number of sensors
			} else {
				Serial.println("No sensors found!");
			}
		}
		else
			Serial.println("No valid measurement so far! wait please");
		if (temp == INVALID_TEMP)
			errors++;
		if (errors > 3){
			initialized = false;
		}
		return temp;
	}

	static DS18S20 DSBus;
	static bool initialized;
	static int errors;
	static int pin;
	static Vector<SensorData> sensors;
	static int max_index;
	static int getNextIndex()
	{
		sensors.addElement(new SensorData());
		return max_index++;
	}

private:
	int address_ix;
	float temperature;
};

#endif
