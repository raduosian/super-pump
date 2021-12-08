
#include <SmingCore/Digital.h>

class SwitchDevice
{
public:

	SwitchDevice(int pin_)
	{
		pin = pin_;
		pinMode (pin, OUTPUT);
		digitalWrite(pin, 0);
	}
	~SwitchDevice()
	{
	}
	void switchOn ()
	{
		digitalWrite(pin, 1);
		value = 1;
	}
	void switchOff ()
	{
		digitalWrite(pin, 0);
		value = 0;
	}
	int getValue()
	{
		return value;
	}
private:
	int pin;
	int value;
};
