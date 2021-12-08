/*
 * CommandInterface.h
 *
 *  Created on: Feb 4, 2018
 *      Author: radu
 */

#ifndef COMMANDINTERFACE_H_
#define COMMANDINTERFACE_H_

#include <Wiring/WString.h>
#include <Wiring/WVector.h>
//#include <vector>

/*
 * Commands are composed by name and parameters
 * Every command ends with \r\n (CRLF). Examples:
 *
 * */
typedef struct _TermoCommand
{
	_TermoCommand (){};
	const _TermoCommand &operator=(const _TermoCommand& other)
	{
		action = other.action;
		pname = other.pname;
		val = other.val;
		return *this;
	};

	String action;
	String pname;
	float val;

	void clear (){
		val = 0.0;
		action= "";
		pname = "";
	}
} TermoCommand;

/*
 *
 * */
class CommandInterface
{
public:
	CommandInterface ();//SerialDriver*);
	~CommandInterface (){};
	void read (char);
	bool newCommand (char);
	const TermoCommand& getCommand () {return Command;};
	void parseCommandString (String & scmd);
	bool isValidActionName (String &aname);
	bool isValidParamName (String &aname);
private:
	TermoCommand Command;
	String RawCmd;
	bool newCmd;
	bool bValid;
	const String actionsNames[3] = {"get", "set", "reset"};
	const String paramsNames[8] = {"fpump", "state", "system", "manual"};
};


#endif /* COMMANDINTERFACE_H_ */
