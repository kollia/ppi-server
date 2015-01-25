/**
 *   This file is part of ppi-server.
 *
 *   ppi-server is free software: you can redistribute it and/or modify
 *   it under the terms of the Lesser GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   ppi-server is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   Lesser GNU General Public License for more details.
 *
 *   You should have received a copy of the Lesser GNU General Public License
 *   along with ppi-server.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <limits.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>

#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>

#include "../../util/GlobalStaticMethods.h"
#include "../../util/stream/ppivalues.h"
#include "../../util/stream/OMethodStringStream.h"
#include "../../util/stream/IMethodStringStream.h"

#include "../../pattern/util/LogHolderPattern.h"

#include "DbInterface.h"

using namespace util;

namespace ppi_database
{

	map<short, DbInterface*> DbInterface::_instance;

	short DbInterface::initial(const string& process, IClientConnectArtPattern* connection, const int identifwait)
	{
		short n;
		EHObj ret;
		DbInterface* pdb;

		n= _instance.size();
		pdb= new DbInterface(process, connection, identifwait);
		if(n == 0)
			LogHolderPattern::init((ILogPattern*)pdb);
		ret= pdb->openSendConnection();
		if(	ret->hasError() &&
			!ret->fail(IEH::errno_error, ECONNREFUSED)	)
		{
			cerr << glob::addPrefix("ERROR: ", ret->getDescription()) << endl;
			delete pdb;
			return -1;
		}
		if(ret->hasWarning())
			cout << glob::addPrefix("WARNING: ", ret->getDescription()) << endl;
		_instance[n]= pdb;
		return n;
	}

	EHObj DbInterface::openConnection(string toopen/*= ""*/)
	{
		if(ExternClientInputTemplate::hasOpenGetConnection())
		{
			m_pSocketError->clear();
			return m_pSocketError;
		}
		return ExternClientInputTemplate::openSendConnection(toopen);
	}

	DbInterface* DbInterface::instance(const short number/*= 0*/)
	{
		return _instance[number];
	}

	void DbInterface::deleteAll()
	{
		for(map<short, DbInterface*>::iterator it= _instance.begin(); it != _instance.end(); ++it)
			delete it->second;
		_instance.clear();
	}

	void DbInterface::setServerConfigureStatus(const string& sProcess, const short& nPercent)
	{
		string msg;
		SocketErrorHandling err;
		OMethodStringStream command("setServerConfigureStatus");

		command << sProcess;
		command << nPercent;
		msg= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err.setErrorStr(msg);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			msg= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, msg);
				cerr << glob::addPrefix("### ERROR:", msg) << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << glob::addPrefix("### WARNING:", msg) << endl;
			}
		}
	}

	bool DbInterface::isServerConfigured(string& sProcess, short& nPercent)
	{
		bool bRv(false);
		string msg;
		SocketErrorHandling err;
		OMethodStringStream command("isServerConfigured");

		msg= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		err.setErrorStr(msg);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			msg= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, msg);
				cerr << glob::addPrefix("### ERROR:", msg) << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << glob::addPrefix("### WARNING:", msg) << endl;
			}
			bRv= false;
		}else
		{
			istringstream result(msg);

			result >> boolalpha >> bRv;
			result >> sProcess;
			result >> nPercent;
			if(result.fail())
			{
				bRv= false;
				msg= "gives back false answer '" + msg + "'";
				msg= "DbInterface question for isServerConfigured\n" + msg;
				LOG(LOG_ALERT, msg);
				cerr << glob::addPrefix("### ALERT:", msg) << endl;
			}
		}
		return bRv;
	}

	bool DbInterface::isDbLoaded()
	{
		string msg;
		SocketErrorHandling err;
		OMethodStringStream loaded("isDbLoaded");

		msg= ExternClientInputTemplate::sendMethod("ppi-db-server", loaded, true);
		err.setErrorStr(msg);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + loaded.getMethodName());
			msg= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, msg);
				cerr << glob::addPrefix("### ERROR:", msg) << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << glob::addPrefix("### WARNING:", msg) << endl;
			}
		}
		if(msg == "1")
			return true;
		return false;
	}

	string DbInterface::allOwreadersInitialed()
	{
		string msg;
		SocketErrorHandling err;
		OMethodStringStream initialed("allOwreadersInitialed");

		msg= ExternClientInputTemplate::sendMethod("ppi-db-server", initialed, true);
		err.setErrorStr(msg);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + initialed.getMethodName());
			msg= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, msg);
				cerr << glob::addPrefix("### ERROR:", msg) << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << glob::addPrefix("### WARNING:", msg) << endl;
			}
			msg= "false";
		}
		return msg;
	}

	void DbInterface::writeIntoDb(const string& folder, const string& subroutine, const string& identif/*= "value"*/)
	{
		string msg;
		SocketErrorHandling err;
		OMethodStringStream command("writeIntoDb");

		command << folder;
		command << subroutine;
		command << identif;
		msg= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err.setErrorStr(msg);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			msg= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, msg);
				cerr << glob::addPrefix("### ERROR:", msg) << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << glob::addPrefix("### WARNING:", msg) << endl;
			}
		}
	}

	bool DbInterface::setValue(const string& folder, const string& subroutine,
					const IValueHolderPattern& value, const InformObject& account)
	{
		string msg;
		SocketErrorHandling err;
		OMethodStringStream command("setValue");

		command << folder;
		command << subroutine;
		command << value.getValue();
		command << value.getTime();
		command << (int)account.getDirection();
		command << account.getWhoDescription();
		msg= ExternClientInputTemplate::sendMethod("ProcessChecker", command, true);
		err.setErrorStr(msg);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "worker list@" + command.getMethodName());
			msg= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, msg);
				cerr << glob::addPrefix("### ERROR:", msg) << endl;
				return false;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << glob::addPrefix("### WARNING:", msg) << endl;
			}
		}
		return true;
	}

	void DbInterface::fillValue(const string& folder, const string& subroutine, const string& identif, double value, bool bNew/*= true*/)
	{
		vector<double> values;

		values.push_back(value);
		fillValue(folder, subroutine, identif, values, bNew);
	}

	void DbInterface::fillValue(const string& folder, const string& subroutine, const string& identif,
					const vector<double>& values, bool bNew/*= true*/)
	{
		string msg;
		SocketErrorHandling err;
		OMethodStringStream command("fillValue");

		command << folder;
		command << subroutine;
		command << identif;
		command << bNew;
		for(vector<double>::const_iterator o= values.begin(); o != values.end(); ++o)
			command << *o;
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// output on command line to set new value
#if 0
		ostringstream out;

		out << "DbInterface filling from " << getProcessName();
		out << " new " << identif << " inside " << folder << ":" << subroutine;
		out << " with content ";
		for(vector<double>::iterator it= values.begin(); it != values.end(); ++it)
			out << "[" << dec << *it << "] ";
		out << endl;
		cout << out.str();
#endif
		msg= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err.setErrorStr(msg);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			msg= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, msg);
				cerr << glob::addPrefix("### ERROR:", msg) << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << glob::addPrefix("### WARNING:", msg) << endl;
			}
		}
	}

	bool DbInterface::fillDebugSession(const IDbFillerPattern::dbgSubroutineContent_t& content, bool answer)
	{
		string msg;
		SocketErrorHandling err;
		OMethodStringStream command("fillDebugSession");

		/*
		 * data type order below
		 * is specified inside follow methods:
		 * DbInterface::fillDebugSession
		 * ServerDbTransaction::transfer by method == "fillDebugSession"
		 * ClientTransaction::hearingTransfer
		 * ClientTransaction::saveFile
		 * ClientTransaction::loadFile
		 */
		command << content.folder;
		command << content.subroutine;
		command << content.value;
		command << *content.currentTime;
		command << content.content;
		msg= ExternClientInputTemplate::sendMethod("ppi-db-server", command, answer);
		err.setErrorStr(msg);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			msg= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, msg);
				cerr << glob::addPrefix("### ERROR:", msg) << endl;
				return false;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << glob::addPrefix("### WARNING:", msg) << endl;
			}
		}
		return true;
	}

	vector<string> DbInterface::getDebugSessionQueue()
	{
		SocketErrorHandling err;
		vector<string> vRv;
		OMethodStringStream command("getDebugSessionQueue");

		vRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, "done", true);
		err.searchResultError(vRv);
		if(err.fail())
		{
			string sRv;

			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			sRv= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, sRv);
				cerr << glob::addPrefix("### ERROR:", sRv) << endl;
			}else
			{
				LOG(LOG_WARNING, sRv);
				cout << glob::addPrefix("### WARNING:", sRv) << endl;
			}
		}
		return vRv;
	}

	string DbInterface::isEntryChanged()
	{
		string msg;
		SocketErrorHandling err;
		OMethodStringStream command("isEntryChanged");

		msg= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		err.setErrorStr(msg);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			msg= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, msg);
				cerr << glob::addPrefix("### ERROR:", msg) << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << glob::addPrefix("### WARNING:", msg) << endl;
			}
			return "false";
		}
		return msg;
	}

	bool DbInterface::existSubroutine(const string& folder, const string& subroutine/*= ""*/)
	{
		unsigned short res;

		res= existEntry(folder, subroutine, "", 0);
		if(res > 1)// result should be 2 (identification do not exist)
			return true; // in this case, folder exist
		if(	res == 1 &&
			subroutine == ""	)
		{ // subroutine is not given
		  // and folder do exist
			return true;
		}
		return false;
	}

	void DbInterface::changedChip(const string& onServer, const string& chip, const double value, const bool device)
	{
		string msg;
		SocketErrorHandling err;
		OMethodStringStream command("changedChip");

		command << onServer;
		command << chip;
		command << value;
		command << device;
		msg= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err.setErrorStr(msg);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			msg= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, msg);
				cerr << glob::addPrefix("### ERROR:", msg) << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << glob::addPrefix("### WARNING:", msg) << endl;
			}
		}
	}

	unsigned short DbInterface::existEntry(const string& folder, const string& subroutine,
					const string& identif, const vector<double>::size_type number)
	{
		string msg;
		SocketErrorHandling err;
		OMethodStringStream command("existEntry");

		command << folder;
		command << subroutine;
		command << identif;
		command << number;
		msg= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		err.setErrorStr(msg);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			msg= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, msg);
				cerr << glob::addPrefix("### ERROR:", msg) << endl;
				return false;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << glob::addPrefix("### WARNING:", msg) << endl;
			}
		}
		if(msg == "exist")
			return 5;
		if(msg == "noaccess")
			return 4;
		if(msg == "novalue")
			return 3;
		if(msg == "noidentif")
			return 2;
		if(msg == "nosubroutine")
			return 1;
		if(msg == "nofolder")
			return 0;
		return 0;
	}

	void DbInterface::debugSubroutine(bool debug, bool bInform,
					const string& folder, const string& subroutine/*= ""*/)
	{
		string msg;
		SocketErrorHandling err;
		OMethodStringStream command("debugSubroutine");

		command << debug;
		command << bInform;
		if(folder != "")
			command << folder;
		if(subroutine != "")
			command << subroutine;
		msg= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err.setErrorStr(msg);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			msg= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, msg);
				cerr << glob::addPrefix("### ERROR:", msg) << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << glob::addPrefix("### WARNING:", msg) << endl;
			}
		}
	}

	void DbInterface::showThreads(int seconds, bool bClient)
	{
		string msg;
		SocketErrorHandling err;
		OMethodStringStream command("showThreads");

		command << seconds;
		command << bClient;
		msg= ExternClientInputTemplate::sendMethod("ProcessChecker", command, false);
		err.setErrorStr(msg);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "worker list@" + command.getMethodName());
			msg= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, msg);
				cerr << glob::addPrefix("### ERROR:", msg) << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << glob::addPrefix("### WARNING:", msg) << endl;
			}
		}
	}

	void DbInterface::clearFolderDebug()
	{
		string msg;
		SocketErrorHandling err;
		OMethodStringStream command("clearFolderDebug");

		msg= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err.setErrorStr(msg);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			msg= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, msg);
				cerr << glob::addPrefix("### ERROR:", msg) << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << glob::addPrefix("### WARNING:", msg) << endl;
			}
		}
	}

	double DbInterface::getFolderValue(short& noexist, const string& folder,
					const string& subroutine, const InformObject& account)
	{
		double dRv= 0;
		string sRv;
		SocketErrorHandling err;
		OMethodStringStream command("getValue");


		command << folder;
		command << subroutine;
		command << (int)account.getDirection();
		command << account.getWhoDescription();
		sRv= ExternClientInputTemplate::sendMethod("ProcessChecker", command, true);
		err.setErrorStr(sRv);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "worker list@" + command.getMethodName());
			sRv= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, sRv);
				cerr << glob::addPrefix("### ERROR:", sRv) << endl;
			}else
			{
				LOG(LOG_WARNING, sRv);
				cout << glob::addPrefix("### WARNING:", sRv) << endl;
			}
			return 0;
		}

		if(sRv == "nochipaccess")
			noexist= -1;
		else if(sRv == "nocorrectsubroutine")
			noexist= -2;
		else if(sRv == "nosubroutine")
			noexist= -3;
		else if(sRv == "nofolder")
			noexist= -4;
		else
		{
			istringstream oRv(sRv);
			oRv >> dRv;
			noexist= 0;
		}
		return dRv;
	}

	double DbInterface::getActEntry(bool& exist, const string& folder, const string& subroutine, const string& identif, const vector<double>::size_type number/*= 0*/)
	{
		double dRv= 0;
		string sRv;
		SocketErrorHandling err;
		OMethodStringStream command("getActEntry");


		command << folder;
		command << subroutine;
		command << identif;
		command << number;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		err.setErrorStr(sRv);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			sRv= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, sRv);
				cerr << glob::addPrefix("### ERROR:", sRv) << endl;
			}else
			{
				LOG(LOG_WARNING, sRv);
				cout << glob::addPrefix("### WARNING:", sRv) << endl;
			}
			return 0;
		}

		if(sRv != "NULL")
		{
			istringstream oRv(sRv);
			oRv >> dRv;
			exist= true;
		}else
			exist= false;
		return dRv;
	}

	vector<convert_t> DbInterface::getNearest(string subroutine, string definition, double value)
	{
		SocketErrorHandling err;
		vector<convert_t> vtRv;
		vector<string> vsRv;
		OMethodStringStream command("getNearest");
		convert_t tRv;

		command << subroutine;
		command << definition;
		command << value;
		vsRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, "done", true);
		err.searchResultError(vsRv);
		if(err.fail())
		{
			string msg;

			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			msg= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, msg);
				cerr << glob::addPrefix("### ERROR:", msg) << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << glob::addPrefix("### WARNING:", msg) << endl;
			}
		}

		istringstream oRv;
		for(vector<string>::iterator o= vsRv.begin(); o != vsRv.end(); ++o)
		{
			istringstream oRv(*o);

			oRv >> tRv.be;
			oRv >> tRv.bSetTime;
			oRv >> tRv.nMikrosec;
			if(!oRv.fail())
				vtRv.push_back(tRv);
		}
		return vtRv;
	}

	bool DbInterface::needSubroutines(unsigned long connection, string name)
	{
		string sRv;
		SocketErrorHandling err;
		OMethodStringStream command("needSubroutines");

		command << connection;
		command << name;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err.setErrorStr(sRv);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			sRv= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, sRv);
				cerr << glob::addPrefix("### ERROR:", sRv) << endl;
			}else
			{
				LOG(LOG_WARNING, sRv);
				cout << glob::addPrefix("### WARNING:", sRv) << endl;
			}
			return false;
		}
/*		alix: 29/12/2013
 * 		do not need answer now
 * 		perform HEAE command from internet-server
 * 		for faster answer
 */
		return true;
/*		if(sRv == "true")
			return true;
		return false;*/
	}

	vector<string> DbInterface::getChangedEntrys(unsigned long connection)
	{
		SocketErrorHandling err;
		vector<string> vRv;
		OMethodStringStream command("getChangedEntrys");

		command << connection;
		vRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, "done", true);
		err.searchResultError(vRv);
		if(err.fail())
		{
			string msg;

			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			msg= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, msg);
				cerr << glob::addPrefix("### ERROR:", msg) << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << glob::addPrefix("### WARNING:", msg) << endl;
			}
		}
		return vRv;
	}

	void DbInterface::changeNeededIds(unsigned long oldId, unsigned long newId)
	{
		string sRv;
		SocketErrorHandling err;
		OMethodStringStream command("changeNeededIds");

		command << oldId;
		command << newId;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err.setErrorStr(sRv);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			sRv= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, sRv);
				cerr << glob::addPrefix("### ERROR:", sRv) << endl;
			}else
			{
				LOG(LOG_WARNING, sRv);
				cout << glob::addPrefix("### WARNING:", sRv) << endl;
			}
		}
	}

	void DbInterface::chipsDefined(const bool defined)
	{
		string sRv;
		SocketErrorHandling err;
		OMethodStringStream command("chipsDefined");

		command << defined;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err.setErrorStr(sRv);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			sRv= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, sRv);
				cerr << glob::addPrefix("### ERROR:", sRv) << endl;
			}else
			{
				LOG(LOG_WARNING, sRv);
				cout << glob::addPrefix("### WARNING:", sRv) << endl;
			}
		}
	}

	void DbInterface::define(const string& server, const string& config)
	{
		SocketErrorHandling err;
		string sRv;
		OMethodStringStream command("define");

		command << server;
		command << config;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		err.setErrorStr(sRv);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			sRv= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, sRv);
				cerr << glob::addPrefix("### ERROR:", sRv) << endl;
			}else
			{
				LOG(LOG_WARNING, sRv);
				cout << glob::addPrefix("### WARNING:", sRv) << endl;
			}
		}
	}

	void DbInterface::registerChip(const string& server, const string& chip, const string& pin, const string& type, const string& family,
			const double* pdmin/*= NULL*/, const double* pdmax/*= NULL*/, const bool* pbFloat/*= NULL*/, const double* pdCache/*= NULL*/)
	{
		SocketErrorHandling err;
		string sRv;
		OMethodStringStream command("registerChip");

		command << server;
		command << chip;
		command << pin;
		command << type;
		command << family;
		command << pdmin;
		command << pdmax;
		command << pbFloat;
		command << pdCache;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		err.setErrorStr(sRv);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			sRv= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, sRv);
				cerr << glob::addPrefix("### ERROR:", sRv) << endl;
			}else
			{
				LOG(LOG_WARNING, sRv);
				cout << glob::addPrefix("### WARNING:", sRv) << endl;
			}
		}
	}

	void DbInterface::registerPortID(const string& folder, const string& subroutine, const string& onServer, const string& chip)
	{
		SocketErrorHandling err;
		string sRv;
		OMethodStringStream command("registerPortID");

		command << folder;
		command << subroutine;
		command << onServer;
		command << chip;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err.setErrorStr(sRv);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			sRv= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, sRv);
				cerr << glob::addPrefix("### ERROR:", sRv) << endl;
			}else
			{
				LOG(LOG_WARNING, sRv);
				cout << glob::addPrefix("### WARNING:", sRv) << endl;
			}
		}
	}

	void DbInterface::registerSubroutine(const string& subroutine, const string& folder, const string& server, const string& chip)
	{
		SocketErrorHandling err;
		string sRv;
		OMethodStringStream command("registerSubroutine");

		command << subroutine;
		command << folder;
		command << server;
		command << chip;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		err.setErrorStr(sRv);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			sRv= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, sRv);
				cerr << glob::addPrefix("### ERROR:", sRv) << endl;
			}else
			{
				LOG(LOG_WARNING, sRv);
				cout << glob::addPrefix("### WARNING:", sRv) << endl;
			}
		}
	}

	double DbInterface::getRegisteredDefaultChipCache(const string& server, const string& chip, bool& exist)
	{
		SocketErrorHandling err;
		double dRv;
		string sRv;
		OMethodStringStream command("getRegisteredDefaultChipCache");

		command << server;
		command << chip;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		err.setErrorStr(sRv);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			sRv= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, sRv);
				cerr << glob::addPrefix("### ERROR:", sRv) << endl;
			}else
			{
				LOG(LOG_WARNING, sRv);
				cout << glob::addPrefix("### WARNING:", sRv) << endl;
			}
			exist= false;
			return 0;
		}
		if(sRv == "NULL")
		{
			exist= false;
			return 0;
		}
		istringstream oRv(sRv);

		oRv >> dRv;
		return dRv;
	}

	DbInterface::chips_t DbInterface::getRegisteredDefaultChip(const string& server, const string& chip)
	{
		return getRegisteredDefaultChipA(false, server, "", "", chip, "");
	}

	DbInterface::chips_t DbInterface::getRegisteredDefaultChip(const string& server, const string& family,
																const string& type, const string& chip, const string& pin)
	{
		return getRegisteredDefaultChipA(true, server, family, type, chip, pin);
	}

	DbInterface::chips_t DbInterface::getRegisteredDefaultChipA(bool bAll, const string& server, const string& family,
																	const string& type, const string& chip, const string& pin)
	{
		SocketErrorHandling err;
		chips_t tRv;
		double errorcode;
		string sRv;
		string method("getRegisteredDefaultChip");
		if(bAll)
			method+= "5";
		else
			method+= "2";
		OMethodStringStream command(method);

		command << server;
		if(bAll)
		{
			command << family;
			command << type;
		}
		command << chip;
		if(bAll)
			command << pin;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		err.setErrorStr(sRv);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			sRv= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, sRv);
				cerr << glob::addPrefix("### ERROR:", sRv) << endl;
			}else
			{
				LOG(LOG_WARNING, sRv);
				cout << glob::addPrefix("### WARNING:", sRv) << endl;
			}
			sRv= "NULL";
		}
		if(sRv == "NULL")
		{
			tRv.exists= false;
			tRv.dmin= 0;
			tRv.dmax= 0;
			tRv.bFloat= false;
			tRv.dCache= 0;
			tRv.bWritable= false;
			return tRv;
		}
		IParameterStringStream oRv(sRv);

		tRv.exists= true;
		oRv >> tRv.server;
		oRv >> tRv.family;
		oRv >> tRv.type;
		oRv >> tRv.id;
		oRv >> tRv.pin;
		oRv >> tRv.dmin;
		oRv >> tRv.dmax;
		oRv >> tRv.bFloat;
		oRv >> tRv.dCache;
		oRv >> tRv.bWritable;
		while(!oRv.empty())
		{
			oRv >> errorcode;
			tRv.errorcode.push_back(errorcode);
		}
		return tRv;
	}

	double DbInterface::getDefaultCache(const double min, const double max, const bool bFloat, const string& folder/*= ""*/, const string& subroutine/*= ""*/)
	{
		SocketErrorHandling err;
		double dRv;
		string sRv;
		OMethodStringStream command("getDefaultCache");

		command << min;
		command << max;
		command << bFloat;
		command << folder;
		command << subroutine;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		err.setErrorStr(sRv);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			sRv= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, sRv);
				cerr << glob::addPrefix("### ERROR:", sRv) << endl;
			}else
			{
				LOG(LOG_WARNING, sRv);
				cout << glob::addPrefix("### WARNING:", sRv) << endl;
			}
			return 15;
		}

		istringstream oRv(sRv);
		oRv >> dRv;
		return dRv;
	}

	bool DbInterface::existOWServer(const unsigned short sID)
	{
		string sRv;
		SocketErrorHandling err;
		OMethodStringStream command("existOWServer");

		command << sID;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		err.setErrorStr(sRv);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			sRv= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, sRv);
				cerr << glob::addPrefix("### ERROR:", sRv) << endl;
			}else
			{
				LOG(LOG_WARNING, sRv);
				cout << glob::addPrefix("### WARNING:", sRv) << endl;
			}
		}
		if(sRv == "true")
			return true;
		return false;
	}

	vector<string> DbInterface::getStatusInfo(const string& param)
	{
		vector<string> vStatus;
		OMethodStringStream command("getStatusInfo");

		command << param;
		return ExternClientInputTemplate::sendMethod("ppi-db-server", command, "done", true);
	}

	vector<string> DbInterface::getOWDebugInfo(const unsigned short ID)
	{
		SocketErrorHandling err;
		vector<string> vRv;
		OMethodStringStream command("getOWDebugInfo");

		command << ID;
		vRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, "done", true);
		err.searchResultError(vRv);
		if(err.fail())
		{
			string sRv;

			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			sRv= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, sRv);
				cerr << glob::addPrefix("### ERROR:", sRv) << endl;
			}else
			{
				LOG(LOG_WARNING, sRv);
				cout << glob::addPrefix("### WARNING:", sRv) << endl;
			}
		}
		return vRv;
	}

	void DbInterface::setOWDebug(const unsigned short serverID, const unsigned int connectionID, const bool set)
	{
		SocketErrorHandling err;
		string sRv;
		OMethodStringStream command("setOWDebug");

		command << serverID;
		command << connectionID;
		command << set;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err.setErrorStr(sRv);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			sRv= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, sRv);
				cerr << glob::addPrefix("### ERROR:", sRv) << endl;
			}else
			{
				LOG(LOG_WARNING, sRv);
				cout << glob::addPrefix("### WARNING:", sRv) << endl;
			}
		}
	}

	void DbInterface::clearOWDebug(const unsigned int connectionID)
	{
		SocketErrorHandling err;
		string sRv;
		OMethodStringStream command("clearOWDebug");

		command << connectionID;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err.setErrorStr(sRv);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			sRv= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, sRv);
				cerr << glob::addPrefix("### ERROR:", sRv) << endl;
			}else
			{
				LOG(LOG_WARNING, sRv);
				cout << glob::addPrefix("### WARNING:", sRv) << endl;
			}
		}
	}

	string DbInterface::stopall()
	{
		SocketErrorHandling err;
		string sRv;
		OMethodStringStream command("stop-all");

		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		err.setErrorStr(sRv);
		if(err.fail())
		{
			err.addMessage("DbInterface", "sendCommand", "database server@" + command.getMethodName());
			sRv= err.getDescription();
			if(err.hasError())
			{
				LOG(LOG_ERROR, sRv);
				cerr << glob::addPrefix("### ERROR:", sRv) << endl;
			}else
			{
				LOG(LOG_WARNING, sRv);
				cout << glob::addPrefix("### WARNING:", sRv) << endl;
			}
		}
		return sRv;
	}

}
