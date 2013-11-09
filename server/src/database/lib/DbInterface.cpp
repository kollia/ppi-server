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
		int ret;
		DbInterface* pdb;

		n= _instance.size();
		pdb= new DbInterface(process, connection, identifwait);
		if(n == 0)
			LogHolderPattern::init(pdb);
		ret= pdb->openSendConnection();
		if(ret > 0 && ret != 35)
		{
			cerr << pdb->strerror(ret) << endl;
			delete pdb;
			return -1;
		}
		if(ret < 0)
			cout << pdb->strerror(ret) << endl;
		_instance[n]= pdb;
		return n;
	}

	int DbInterface::openConnection(string toopen/*= ""*/)
	{
		if(ExternClientInputTemplate::hasOpenGetConnection())
			return 0;
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
		int err;
		string sRv;
		OMethodStringStream command("setServerConfigureStatus");

		command << sProcess;
		command << nPercent;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
		}
	}

	bool DbInterface::isServerConfigured(string& sProcess, short& nPercent)
	{
		int err;
		bool nRv(false);
		string sRv;
		OMethodStringStream command("isServerConfigured");

		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
		}else
		{
			istringstream result(sRv);

			result >> boolalpha >> nRv;
			result >> sProcess;
			result >> nPercent;
		}
		return nRv;
	}

	bool DbInterface::isDbLoaded()
	{
		int err;
		string sRv;
		OMethodStringStream loaded("isDbLoaded");

		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", loaded, true);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
			return false;
		}
		if(sRv == "1")
			return true;
		return false;
	}

	string DbInterface::allOwreadersInitialed()
	{
		int err;
		string res;
		OMethodStringStream initialed("allOwreadersInitialed");

		res= ExternClientInputTemplate::sendMethod("ppi-db-server", initialed, true);
		err= error(res);
		if(err != 0)
		{
			res= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, res);
				cerr << res << endl;
				return "done";
			}else
				LOG(LOG_WARNING, res);
		}
		return res;
	}

	void DbInterface::writeIntoDb(const string& folder, const string& subroutine, const string& identif/*= "value"*/)
	{
		int err;
		string sRv;
		OMethodStringStream command("writeIntoDb");

		command << folder;
		command << subroutine;
		command << identif;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
		}
	}

	bool DbInterface::setValue(const string& folder, const string& subroutine, double value, const string& account)
	{
		int err;
		string sRv;
		OMethodStringStream command("setValue");

		command << folder;
		command << subroutine;
		command << value;
		command << account;
		sRv= ExternClientInputTemplate::sendMethod("ProcessChecker", command, true);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
			return false;
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
		int err;
		string sRv;
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
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
		}
	}

	string DbInterface::isEntryChanged()
	{
		int err;
		string sRv;
		OMethodStringStream command("isEntryChanged");

		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
		}
		return sRv;
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
		int err;
		string sRv;
		OMethodStringStream command("changedChip");

		command << onServer;
		command << chip;
		command << value;
		command << device;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
		}
	}

	unsigned short DbInterface::existEntry(const string& folder, const string& subroutine, const string& identif, const vector<double>::size_type number)
	{
		int err;
		string sRv;
		OMethodStringStream command("existEntry");

		command << folder;
		command << subroutine;
		command << identif;
		command << number;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
			return 0;
		}
		if(sRv == "exist")
			return 5;
		if(sRv == "noaccess")
			return 4;
		if(sRv == "novalue")
			return 3;
		if(sRv == "noidentif")
			return 2;
		if(sRv == "nosubroutine")
			return 1;
		if(sRv == "nofolder")
			return 0;
		return 0;
	}

	void DbInterface::debugSubroutine(bool debug, const string& folder, const string& subroutine/*= ""*/)
	{
		int err;
		string sRv;
		OMethodStringStream command("debugSubroutine");

		command << debug;
		command << folder;
		if(subroutine != "")
			command << subroutine;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
		}
	}

	void DbInterface::clearFolderDebug()
	{
		int err;
		string sRv;
		OMethodStringStream command("clearFolderDebug");

		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
		}
	}

	double DbInterface::getFolderValue(short& noexist, const string& folder, const string& subroutine, const string& account)
	{
		int err;
		double dRv= 0;
		string sRv;
		OMethodStringStream command("getValue");


		command << folder;
		command << subroutine;
		command << account;
		sRv= ExternClientInputTemplate::sendMethod("ProcessChecker", command, true);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
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
		int err;
		double dRv= 0;
		string sRv;
		OMethodStringStream command("getActEntry");


		command << folder;
		command << subroutine;
		command << identif;
		command << number;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
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
		int err;
		vector<convert_t> vtRv;
		vector<string> vsRv;
		OMethodStringStream command("getNearest");
		convert_t tRv;

		command << subroutine;
		command << definition;
		command << value;
		vsRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, "done", true);

		istringstream oRv;
		for(vector<string>::iterator o= vsRv.begin(); o != vsRv.end(); ++o)
		{
			istringstream oRv(*o);

			err= error(*o);
			if(err != 0)
			{
				string msg;

				msg= strerror(err);
				if(err > 0)
				{
					LOG(LOG_ERROR, msg);
					cerr << "### " << msg << endl;
				}else
				{
					LOG(LOG_WARNING, msg);
					cout << "### " << msg << endl;
				}
			}else
			{
				oRv >> tRv.be;
				oRv >> tRv.bSetTime;
				oRv >> tRv.nMikrosec;
				vtRv.push_back(tRv);
			}
		}
		return vtRv;
	}

	bool DbInterface::needSubroutines(unsigned long connection, string name)
	{
		int err;
		string sRv;
		OMethodStringStream command("needSubroutines");

		command << connection;
		command << name;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
			return false;
		}
		if(sRv == "true")
			return true;
		return false;
	}

	vector<string> DbInterface::getChangedEntrys(unsigned long connection)
	{
		int err;
		vector<string> vRv;
		OMethodStringStream command("getChangedEntrys");

		command << connection;
		vRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, "done", true);
		for(vector<string>::iterator o= vRv.begin(); o != vRv.end(); ++o)
		{
			err= error(*o);
			if(err != 0)
			{
				string msg;

				msg= strerror(err);
				if(err > 0)
				{
					LOG(LOG_ERROR, msg);
					cerr << "### " << msg << endl;
				}else
				{
					LOG(LOG_WARNING, msg);
					cout << "### " << msg << endl;
				}
			}
		}
		return vRv;
	}

	void DbInterface::changeNeededIds(unsigned long oldId, unsigned long newId)
	{
		int err;
		string sRv;
		OMethodStringStream command("changeNeededIds");

		command << oldId;
		command << newId;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
		}
	}

	void DbInterface::chipsDefined(const bool defined)
	{
		int err;
		string sRv;
		OMethodStringStream command("chipsDefined");

		command << defined;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
		}
	}

	void DbInterface::define(const string& server, const string& config)
	{
		int err;
		string sRv;
		OMethodStringStream command("define");

		command << server;
		command << config;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
		}
	}

	void DbInterface::registerChip(const string& server, const string& chip, const string& pin, const string& type, const string& family,
			const double* pdmin/*= NULL*/, const double* pdmax/*= NULL*/, const bool* pbFloat/*= NULL*/, const double* pdCache/*= NULL*/)
	{
		int err;
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
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
		}
	}

	void DbInterface::registerPortID(const string& folder, const string& subroutine, const string& onServer, const string& chip)
	{
		int err;
		string sRv;
		OMethodStringStream command("registerPortID");

		command << folder;
		command << subroutine;
		command << onServer;
		command << chip;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
		}

	}

/*	void DbInterface::useChip(const string& folder, const string& subroutine, const string& onServer, const string& chip)
	{
		int err;
		string sRv;
		OMethodStringStream command("useChip");

		command << folder;
		command << subroutine;
		command << onServer;
		command << chip;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
		}

	}*/

	void DbInterface::registerSubroutine(const string& subroutine, const string& folder, const string& server, const string& chip)
	{
		int err;
		string sRv;
		OMethodStringStream command("registerSubroutine");

		command << subroutine;
		command << folder;
		command << server;
		command << chip;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
		}
	}

	double DbInterface::getRegisteredDefaultChipCache(const string& server, const string& chip, bool& exist)
	{
		int err;
		double dRv;
		string sRv;
		OMethodStringStream command("getRegisteredDefaultChipCache");

		command << server;
		command << chip;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
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
		int err;
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
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
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
		int err;
		double dRv;
		string sRv;
		OMethodStringStream command("getDefaultCache");

		command << min;
		command << max;
		command << bFloat;
		command << folder;
		command << subroutine;
		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
			return 15;
		}

		istringstream oRv(sRv);
		oRv >> dRv;
		return dRv;
	}

	bool DbInterface::existOWServer(const unsigned short sID)
	{
		string res;
		OMethodStringStream command("existOWServer");

		command << sID;
		res= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		if(res == "true")
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
		int err;
		vector<string> vRv;
		OMethodStringStream command("getOWDebugInfo");

		command << ID;
		vRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, "done", true);
		for(vector<string>::iterator o= vRv.begin(); o != vRv.end(); ++o)
		{
			err= error(*o);
			if(err != 0)
			{
				string msg;

				msg= strerror(err);
				if(err > 0)
				{
					LOG(LOG_ERROR, msg);
					cerr << "### " << msg << endl;
				}else
				{
					LOG(LOG_WARNING, msg);
					cout << "### " << msg << endl;
				}
			}
		}
		return vRv;
	}

	void DbInterface::setOWDebug(const unsigned short serverID, const unsigned int connectionID, const bool set)
	{
		int err;
		string res;
		OMethodStringStream command("setOWDebug");

		command << serverID;
		command << connectionID;
		command << set;
		res= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err= error(res);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
		}
	}

	void DbInterface::clearOWDebug(const unsigned int connectionID)
	{
		int err;
		string res;
		OMethodStringStream command("clearOWDebug");

		command << connectionID;
		res= ExternClientInputTemplate::sendMethod("ppi-db-server", command, false);
		err= error(res);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
		}
	}

	string DbInterface::stopall()
	{
		int err;
		string sRv;
		OMethodStringStream command("stop-all");

		sRv= ExternClientInputTemplate::sendMethod("ppi-db-server", command, true);
		err= error(sRv);
		if(err != 0)
		{
			string msg;

			msg= strerror(err);
			if(err > 0)
			{
				LOG(LOG_ERROR, msg);
				cerr << "### " << msg << endl;
			}else
			{
				LOG(LOG_WARNING, msg);
				cout << "### " << msg << endl;
			}
		}
		return sRv;
	}

}
