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


#include "../../util/structures.h"
#include "../../util/OMethodStringStream.h"
#include "../../util/IMethodStringStream.h"

#include "../../logger/lib/logstructures.h"
#include "../../logger/lib/LogInterface.h"

#include "DbInterface.h"

using namespace util;

namespace ppi_database
{

	map<short, DbInterface*> DbInterface::_instance;

	short DbInterface::initial(const string& process, IClientConnectArtPattern* connection)
	{
		short n;
		int ret;
		DbInterface* pdb;

		n= _instance.size();
		pdb= new DbInterface(process, connection);
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

	bool DbInterface::isDbLoaded()
	{
		int err;
		string sRv;
		OMethodStringStream loaded("isDbLoaded");

		sRv= sendMethod("ppi-db-server", loaded, true);
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

	void DbInterface::writeIntoDb(const string folder, const string subroutine)
	{
		int err;
		string sRv;
		OMethodStringStream command("writeIntoDb");

		command << folder;
		command << subroutine;
		sRv= sendMethod("ppi-db-server", command, false);
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

	void DbInterface::setValue(string folder, string subroutine, double value)
	{
		int err;
		string sRv;
		OMethodStringStream command("setValue");

		command << folder;
		command << subroutine;
		command << value;
		sRv= sendMethod("ProcessChecker", command, true);
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

	void DbInterface::fillValue(string folder, string subroutine, string identif, double value, bool bNew/*= true*/)
	{
		vector<double> values;

		values.push_back(value);
		fillValue(folder, subroutine, identif, values, bNew);
	}

	void DbInterface::fillValue(string folder, string subroutine, string identif, vector<double> values, bool bNew/*= true*/)
	{
		int err;
		string sRv;
		OMethodStringStream command("fillValue");

		command << folder;
		command << subroutine;
		command << identif;
		command << bNew;
		for(vector<double>::iterator o= values.begin(); o != values.end(); ++o)
			command << *o;
		sRv= sendMethod("ppi-db-server", command, false);
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
		unsigned int nRv= 0;
		string sRv;
		OMethodStringStream command("isEntryChanged");

		sRv= sendMethod("ppi-db-server", command, false);
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

	bool DbInterface::existFolder(const string& folder)
	{
		unsigned short res;

		res= existEntry(folder, "", "", 0);
		if(res > 0)// result should be 1 (subroutine do not exist)
			return true; // in this case, folder exist
		return false;
	}

	void DbInterface::useChip(const string& folder, const string& subroutine, const string& onServer, const string& chip)
	{
		int err;
		string sRv;
		OMethodStringStream command("useChip");

		command << folder;
		command << subroutine;
		command << onServer;
		command << chip;
		sRv= sendMethod("ppi-db-server", command, false);
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

	void DbInterface::changedChip(const string& onServer, const string& chip, const double value, const bool device)
	{
		int err;
		string sRv;
		OMethodStringStream command("changedChip");

		command << onServer;
		command << chip;
		command << value;
		command << device;
		sRv= sendMethod("ppi-db-server", command, false);
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
		unsigned short Rv= 0;
		string sRv;
		OMethodStringStream command("existEntry");

		command << folder;
		command << subroutine;
		command << identif;
		command << number;
		sRv= sendMethod("ppi-db-server", command, false);
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

	void DbInterface::debugFolder(const string& folder)
	{
		int err;
		string sRv;
		OMethodStringStream command("debugFolder");

		command << folder;
		sRv= sendMethod("ppi-db-server", command, false);
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

		sRv= sendMethod("ppi-db-server", command, false);
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
		sRv= sendMethod("ppi-db-server", command, false);
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
		vsRv= sendMethod("ppi-db-server", command, "done", true);

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
		sRv= sendMethod("ppi-db-server", command, false);
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
		vRv= sendMethod("ppi-db-server", command, "done", true);
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
		sRv= sendMethod("ppi-db-server", command, false);
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
		sRv= sendMethod("ppi-db-server", command, false);
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
		sRv= sendMethod("ppi-db-server", command, true);
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
		sRv= sendMethod("ppi-db-server", command, true);
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

	void DbInterface::registerSubroutine(const string& subroutine, const string& folder, const string& server, const string& chip)
	{
		int err;
		string sRv;
		OMethodStringStream command("registerSubroutine");

		command << subroutine;
		command << folder;
		command << server;
		command << chip;
		sRv= sendMethod("ppi-db-server", command, true);
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

	const double DbInterface::getRegisteredDefaultChipCache(const string& server, const string& chip, bool& exist)
	{
		int err;
		double dRv;
		string sRv;
		OMethodStringStream command("getRegisteredDefaultChipCache");

		command << server;
		command << chip;
		sRv= sendMethod("ppi-db-server", command, true);
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
		return getRegisteredDefaultChipA(false, server, "", "", chip);
	}

	DbInterface::chips_t DbInterface::getRegisteredDefaultChip(const string& server, const string& family, const string& type, const string& chip)
	{
		return getRegisteredDefaultChipA(true, server, family, type, chip);
	}

	DbInterface::chips_t DbInterface::getRegisteredDefaultChipA(bool bAll, const string& server, const string& family, const string& type, const string& chip)
	{
		int err;
		chips_t tRv;
		double errorcode;
		string sRv;
		string method("getRegisteredDefaultChip");
		if(bAll)
			method+= "4";
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
		sRv= sendMethod("ppi-db-server", command, true);
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

	const double DbInterface::getDefaultCache(const double min, const double max, const bool bFloat, const string& folder/*= ""*/, const string& subroutine/*= ""*/)
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
		sRv= sendMethod("ppi-db-server", command, true);
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
		res= sendMethod("ppi-db-server", command, true);
		if(res == "true")
			return true;
		return false;
	}

	vector<string> DbInterface::getOWDebugInfo(const unsigned short ID)
	{
		int err;
		vector<string> vRv;
		OMethodStringStream command("getOWDebugInfo");

		command << ID;
		vRv= sendMethod("ppi-db-server", command, "done", true);
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
		res= sendMethod("ppi-db-server", command, false);
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
		OMethodStringStream command("setOWDebug");

		command << connectionID;
		res= sendMethod("ppi-db-server", command, false);
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

		sRv= sendMethod("ppi-db-server", command, true);
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
