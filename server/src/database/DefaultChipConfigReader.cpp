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

/*
 * DefaultChipConfigReader.cpp
 *
 *  Created on: 13.03.2009
 *      Author: Alexander Kolli
 */

#include <iostream>
#include <sstream>

#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/regex.hpp>

#include "DefaultChipConfigReader.h"

#include "../pattern/util/LogHolderPattern.h"

#include "../util/URL.h"
#include "../util/Calendar.h"

#include "../util/properties/configpropertycasher.h"


namespace ports
{
	using namespace util;
	using namespace boost;

	DefaultChipConfigReader* DefaultChipConfigReader::_instance= NULL;

	void DefaultChipConfigReader::init(const string& path, bool bUseRegex)
	{
		if(_instance == NULL)
			_instance= new DefaultChipConfigReader(path, bUseRegex);
	}

	inline void DefaultChipConfigReader::deleteObj()
	{
		if(_instance)
		{
			delete _instance;
			_instance= NULL;
		}
	}

	void DefaultChipConfigReader::initialDefault()
	{
		typedef vector<IInterlacedPropertyPattern*>::iterator sit;

		bool berror;
		chips_t chip;
		ostringstream output;
		string fileName, propName, errmsg;
		InterlacedProperties properties;
		vector<IInterlacedPropertyPattern*> sections;

		properties.modifier("folder");
		properties.setMsgParameter("folder");
		properties.modifier("subroutine");
		properties.setMsgParameter("subroutine");
		properties.modifier("server");
		properties.setMsgParameter("server");
		properties.modifier("family");
		properties.setMsgParameter("family", "family code");
		properties.modifier("type");
		properties.setMsgParameter("type", "chip type");
		properties.modifier("ID");
		properties.setMsgParameter("ID", "chip ID");
		properties.modifier("pin");
		properties.setMsgParameter("pin", "chip pin");
		properties.modifier("range");
		properties.setMsgParameter("range", "range of chip");
		properties.modifier("dbolder");
		properties.setMsgParameter("dbolder", "older time");
		properties.allowLaterModifier(true);
		fileName= URL::addPath(m_sPath, "default.conf", /*always*/true);
		if(!properties.readFile(fileName))
		{
			string msg;

			msg= "### ERROR: cannot read default configuration 'default.conf'\n";
			msg+= "           for all chip's and folders";
			cerr << msg << endl;
			LOG(LOG_ERROR, msg);
			return;
		}
		sections= properties.getSections();
		//cout << "read " << sections.size() << " sctions inside default.conf" << endl;
		//for(sit o= sections.begin(); o != sections.end(); ++o)
		//	cout << "         section " << (*o)->getSectionModifier() << " with value '" << (*o)->getSectionValue() << "'" << endl;
		for(sit o= sections.begin(); o != sections.end(); ++o)
			readSection(*o, chip);
		berror= properties.checkProperties(&errmsg);
		if(errmsg != "")
		{
			output << endl;
			output << "###  some ";
			if(berror)
				output << "ERRORs and maybe ";
			output << "WARNINGs occurred by reading 'conf/default.conf' configure file" << endl;
			output << "     ------------------------------------------------------------------------------------" << endl;
			output << errmsg << endl << endl;
			if(berror)
				cerr << output.str();
			else
				cout << output.str();
		}
	}

	void DefaultChipConfigReader::define(const string& server, const string& config)
	{
		typedef vector<IInterlacedPropertyPattern*>::iterator sit;

		string fileName, propName;
		chips_t chip;
		InterlacedProperties properties;
		vector<IInterlacedPropertyPattern*> sections;

		fileName= URL::addPath(m_sPath, config, /*always*/true);
		properties.modifier("family");
		properties.setMsgParameter("family", "family code");
		properties.modifier("type");
		properties.setMsgParameter("type", "chip type");
		properties.modifier("ID");
		properties.setMsgParameter("ID", "chip ID");
		properties.modifier("pin");
		properties.setMsgParameter("pin", "chip pin");
		//properties.modifier("dbolder");
		//properties.setMsgParameter("dbolder", "older time");
		properties.allowLaterModifier(true);
		if(!properties.readFile(fileName))
		{
			string msg;

			msg= "### ERROR: cannot read default configuration '";
			msg+= fileName + "'\n           for server ";
			msg+= server;
			cerr << msg << endl;
			LOG(LOG_ERROR, msg);
			return;
		}
		chip.server= server;
		sections= properties.getSections();
		for(sit o= sections.begin(); o != sections.end(); ++o)
			readSection(*o, chip);
		properties.checkProperties();
	}

	void DefaultChipConfigReader::readSection(IInterlacedPropertyPattern *property, chips_t chip)
	{
		typedef vector<IInterlacedPropertyPattern*>::iterator sit;

		/**
		 * whether data of an chip was read
		 */
		bool bChipReadings(false);
		/**
		 * currently section which reading
		 */
		string section(property->getSectionModifier());
		/**
		 * number of currently section
		 *   1 - folder
		 *   2 - subroutine
		 *   3 - server
		 *   4 - family
		 *   5 - type
		 *   6 - ID
		 *   7 - pin
		 *   8 - range
		 */
		unsigned short nSection;
		/**
		 * intern chip ID genarated for all default chip's
		 */
		unsigned int nInternChipID(0);
		bool bErrWarn;
		int errorcount= 0;
		double errorcode;
		double dRange;
		double dCache= 0;
		vector<IInterlacedPropertyPattern*> sections;
		SHAREDPTR::shared_ptr<otime_t> older;
		vector<string> split;
		string propName("cache");
		string server, family, type, ID, pin, dbolder;
		string range, sfloat, writable, dbwrite;
		string folder, subroutine, errwarnmsg;
		defValues_t defaultValues;

		if(section == "folder")
			nSection= 1;
		else if(section == "subroutine")
			nSection= 2;
		else if(section == "server")
			nSection= 3;
		else if(section == "family")
			nSection= 4;
		else if(section == "type")
			nSection= 5;
		else if(section == "ID")
			nSection= 6;
		else if(section == "pin")
			nSection= 7;
		else if(section == "range")
			nSection= 8;
		if(nSection <= 1) // folder
		{
			folder= property->getValue("folder", /*warning*/false);
			if(folder != "")
			{
				string corr;
				//folder= "###all";
				if(section != "folder")
				{
					sections= property->getSections();
					for(sit o= sections.begin(); o != sections.end(); ++o)
						readSection(*o, chip);
					return;
				}
				corr= folder;
				if(!m_bUseRegex)
				{
					replace_all(folder, "^", "\\^");
					replace_all(folder, "$", "\\$");
					replace_all(folder, "(", "\\(");
					replace_all(folder, ")", "\\)");
					replace_all(folder, "[", "\\[");
					replace_all(folder, "]", "\\]");
					replace_all(folder, "{", "\\{");
					replace_all(folder, "}", "\\}");
					replace_all(folder, "?", ".");
					replace_all(folder, "*", ".*");
					folder= "^" + folder + "$";
				}
				try{
					regex exp(folder);

					regex_match("anything", exp);
					// when correct
					chip.folder= folder;

				}catch(...)
				{
					string errmsg("default folder definition '");

					errmsg+= corr + "' wrong defined, making regex error";
					cerr << "### ERROR: " << errmsg << endl;
					cerr << "           so do not read defaults for this case" << endl;
					LOG(LOG_ERROR, errmsg + "\nso do not read defaults for this case of folders");
					return;
				}
			}
		}
		if(nSection <= 2) // subroutine
		{
			subroutine= property->getValue("subroutine", /*warning*/false);
			if(subroutine != "")
			{
				string corr;
				//subroutine= "###all";
				if(chip.folder == "")
				{
					property->needValue("folder");
					return;
				}
				if(section != "subroutine")
				{
					sections= property->getSections();
					for(sit o= sections.begin(); o != sections.end(); ++o)
						readSection(*o, chip);
					return;
				}
				corr= subroutine;
				if(!m_bUseRegex)
				{
					replace_all(subroutine, "^", "\\^");
					replace_all(subroutine, "$", "\\$");
					replace_all(subroutine, "(", "\\(");
					replace_all(subroutine, ")", "\\)");
					replace_all(subroutine, "[", "\\[");
					replace_all(subroutine, "]", "\\]");
					replace_all(subroutine, "{", "\\{");
					replace_all(subroutine, "}", "\\}");
					replace_all(subroutine, "?", ".");
					replace_all(subroutine, "*", ".*");
					subroutine= "^" + subroutine + "$";
				}
				try{
					regex exp(subroutine);

					regex_match("anything", exp);
					// when correct
					chip.subroutine= subroutine;

				}catch(...)
				{
					string errmsg("default subroutine definition '");

					errmsg+= corr + "' wrong defined, making regex error";
					cerr << "### ERROR: " << errmsg << endl;
					cerr << "           so do not read defaults for this case" << endl;
					LOG(LOG_ERROR, errmsg + "\nso do not read defaults for this case of subroutines");
					return;
				}
			}
		}
		if(nSection <= 3) // server
		{
			server= property->getValue("server", /*warning*/false);
			if(server != "")
			{
				//server= "###all";
				if(section != "server")
				{
					sections= property->getSections();
					for(sit o= sections.begin(); o != sections.end(); ++o)
						readSection(*o, chip);
					return;
				}
				chip.server= server;
			}
		}
		if(nSection <= 4) // family
		{
			family= property->getValue("family", /*warning*/false);
			if(family != "")
			{
				//family= "###all";
				if(section != "family")
				{
					sections= property->getSections();
					for(sit o= sections.begin(); o != sections.end(); ++o)
						readSection(*o, chip);
					return;
				}
				chip.family= family;
			}
		}
		if(nSection <= 5) // type
		{
			type= property->getValue("type", /*warning*/false);
			if(type != "")
			{
				//type= "###all";
				if(section != "type")
				{
					sections= property->getSections();
					for(sit o= sections.begin(); o != sections.end(); ++o)
						readSection(*o, chip);
					return;
				}
				chip.type= type;
			}
		}
		if(nSection <= 6) // ID
		{
			ID= property->getValue("ID", /*warning*/false);
			if(ID != "")
			{
				//ID= "###all";
				if(section != "ID")
				{
					sections= property->getSections();
					for(sit o= sections.begin(); o != sections.end(); ++o)
						readSection(*o, chip);
					return;
				}
				chip.id= ID;
			}
		}
		if(nSection <= 7) // pin
		{
			pin= property->getValue("pin", /*warning*/false);
			if(pin != "")
			{
				//pin= "###all";
				if(section != "pin")
				{
					sections= property->getSections();
					for(sit o= sections.begin(); o != sections.end(); ++o)
						readSection(*o, chip);
					return;
				}
				chip.pin= pin;
			}
		}
		range= property->getValue("range", /*warning*/false);
		if(range != "")
		{
			if(section != "range")
			{
				sections= property->getSections();
				for(sit o= sections.begin(); o != sections.end(); ++o)
					readSection(*o, chip);
				return;
			}
			split= ConfigPropertyCasher::split(range, ":");
			if(split.size() < 2)
			{
				string msg;

				msg= property->getMsgHead(/*ERROR*/true);
				msg+= "range should have 2 double values separated with an colon, define as hole range (1:0)";
				LOG(LOG_ERROR, msg);
				cerr << msg << endl;
				chip.dmin= 1;
				chip.dmax= 0;
			}else
			{
				string str(split[0]);

				str+= " ";
				str+= split[1];
				istringstream istr(str);
				istr >> chip.dmin >> chip.dmax;
			}
		}else
		{
			range= "1:0";
			chip.dmin= 1;
			chip.dmax= 0;
		}
		propName= "cache";
		dCache= property->getDouble(propName, /*warning*/false);
		if(propName == "cache")// no error
			chip.dCache= dCache;
		sfloat= property->getValue("float", /*warning*/false);
		if(sfloat != "")
		{
			if(sfloat == "false")
				chip.bFloat= false;
			else if(sfloat != "true")
			{
				string msg;

				msg= property->getMsgHead(/*error*/false);
				msg+= "float have to be 'true' or 'false', can not read '";
				msg+= sfloat + "'. Set float to false.";
				LOG(LOG_WARNING, msg);
				cerr << msg << endl;
			}
		}else
			chip.bFloat= true;
		dbwrite= property->needValue("dbwrite");
		sections= property->getSections();//should be only dbolder sections
		if(	dbwrite != "" ||
			sections.size() > 0	)
		{
			SHAREDPTR::shared_ptr<otime_t> older;

			if(dbwrite == "")
				dbwrite= "all";
			older= readOlderSection(property);
			chip.older= older;
			chip.older->more= 0;  // Currently state of reading has the unit of the next dbodler section
			chip.older->unit= 'D';// so change it to current with 0 days
			for(sit o= sections.begin(); o != sections.end(); ++o)
			{
				older->older= readOlderSection(*o);
				older= older->older;
			}
		}
		bErrWarn= false;
		errwarnmsg= property->getMsgHead(/*error*/true);
		bChipReadings= false;
		if(chip.server != "")
			bChipReadings= true;
		else
			chip.server= "###all";
		if(chip.family != "")
		{
			if(chip.server == "###all")
			{
				bErrWarn= true;
				errwarnmsg+= "\n           family code";
			}
			bChipReadings= true;
		}else
			chip.family= "###all";
		if(chip.type != "")
		{
			if(chip.server == "###all")
			{
				bErrWarn= true;
				errwarnmsg+= ",\n           chip type";
			}
			bChipReadings= true;
		}else
			chip.type= "###all";
		if(chip.id != "")
		{
			if(chip.server == "###all")
			{
				bErrWarn= true;
				errwarnmsg+= ",\n           chip ID";
			}
			bChipReadings= true;
		}else
			chip.id= "###all";
		if(chip.pin != "")
		{
			if(chip.server == "###all")
			{
				bErrWarn= true;
				errwarnmsg+= ",\n           pin(s) of chip";
			}
			bChipReadings= true;
		}else
			chip.pin= "###all";
		if(bErrWarn)
		{
			errwarnmsg+= ",\n           cannot be set when no server defined\n";

		}else
		{
			if(chip.folder != "")
			{
				if(	chip.server != "###all" &&
					chip.subroutine != ""		)
				{
					bErrWarn= true;
					errwarnmsg+= "\n           folder cannot be set when server be defined\n";
				}
			}else
				chip.folder= "###all";
			if(chip.subroutine != "")
			{
				if(chip.server != "###all")
				{
					bErrWarn= true;
					errwarnmsg+= "\n           folder/subroutine cannot be set when server be defined\n";
				}
			}else
				chip.subroutine= "###all";
		}
		if(bErrWarn)
		{
			errwarnmsg+= "           so do not read this section";
			LOG(LOG_WARNING, errwarnmsg);
			cerr << errwarnmsg << endl;
			return;
		}
		if(chip.dmin == 0 && chip.dmax == 0)
		{// for hole range
			chip.dmin= 1;
			chip.dmax= 0;
		}
		if(bChipReadings)
		{
			writable= property->getValue("writable", /*warning*/false);
			if(writable != "")
			{
				if(writable == "false")
					chip.bWritable= false;
				else if(writable != "true")
				{
					string msg;

					msg= property->getMsgHead(/*error*/false);
					msg+= "writable have to be 'true' or 'false', can not read '";
					msg+= writable + "'. Set writable to false.";
					LOG(LOG_WARNING, msg);
					cerr << msg << endl;
				}
			}
			errorcount= property->getPropertyCount("errorcode");
			if(errorcount)
			{
				string prop;

				for(int i= 0; i < errorcount; ++i)
				{
					prop= "errorcode";
					errorcode= property->getDouble(prop, (vector<string>::size_type)i, /*warning*/false);
					if(prop != "#ERROR")
						chip.errorcode.push_back(errorcode);
				}
			}
		}
		older= chip.older;// do not save older inside default chips with server configuration
		chip.older= SHAREDPTR::shared_ptr<otime_t>();
		if(bChipReadings)
			nInternChipID= saveChip(chip);
		chip.internReaderChipID= nInternChipID;

		bool btake(true);
		string msg;
		map<unsigned int, map<string, map<string, map<double , map<bool, defValues_t> > > > >::iterator itChip;
		map<string, map<string, map<double , map<bool, defValues_t> > > >::iterator itFolder;
		map<string, map<double , map<bool, defValues_t> > >::iterator itSubroutine;
		map<double , map<bool, defValues_t> >::iterator itRange;
		map<bool, defValues_t>::iterator itFloat;

		if(chip.dmin > chip.dmax)
			dRange= 0;
		else
			dRange= chip.dmax - chip.dmin;
		defaultValues.dmin= chip.dmin;
		defaultValues.dmax= chip.dmax;
		defaultValues.bFloat= chip.bFloat;
		defaultValues.dcache= chip.dCache;
		defaultValues.internReaderChipID= nInternChipID;
		defaultValues.older= older;
		itChip= m_mmmmDefaultValues.find(nInternChipID);
		if(itChip != m_mmmmDefaultValues.end())
		{
			if(nInternChipID != 0)
				msg= "an " + chip.server + "/" + chip.id + " with ";
			itFolder= itChip->second.find(chip.folder);
			if(itFolder != itChip->second.end())
			{
				if(folder != "")
					msg+= "folder '" + folder + "' ";
				itSubroutine= itFolder->second.find(chip.subroutine);
				if(itSubroutine != itFolder->second.end())
				{
					if(subroutine != "")
						msg+= "subroutine '" + subroutine + "' ";
					itRange = itSubroutine->second.find(dRange);
					if(itRange != itSubroutine->second.end())
					{
						string sRange;
						ostringstream oRange;;

						oRange << dRange;
						msg+= "range " + oRange.str() + " ";
						itFloat= itRange->second.find(chip.bFloat);
						if(itFloat != itRange->second.end())
						{
							msg+= "for ";
							if(!chip.bFloat)
								msg+= "non ";
							msg+= "floating values";
							//"### WARNING: "
							msg= "found second default value from " + msg + " inside [xxx]default.conf";
							cout << "### WARNING: " << msg << endl;
							cout << "             take only the first entry's" << endl;
							LOG(LOG_WARNING, msg + "\ntake only the first entry's");
							btake= false;
						}
					}
				}
			}
		}
		if(btake)
			m_mmmmDefaultValues[nInternChipID][chip.folder][chip.subroutine][dRange][chip.bFloat]= defaultValues;
	}

	unsigned int DefaultChipConfigReader::saveChip(chips_t chip)
	{
		map<string, map<string, map<string, map<string, map<string, chips_t> > > > >::iterator servIt;
		map<string, map<string, map<string, map<string, chips_t> > > >::iterator famIt;
		map<string, map<string, map<string, chips_t> > >::iterator typIt;
		map<string, map<string, chips_t> >::iterator idIt;
		map<string, chips_t>::iterator pinIt;
		vector<string> pinsplit;
		string pin(chip.pin);
		unsigned int nInternChip(0);
		const chips_t* pDefaultChip;
		bool bFound;

		servIt= m_mmmmmChips.find(chip.server);
		if(servIt != m_mmmmmChips.end())
		{
			famIt= servIt->second.find(chip.family);
			if(famIt != servIt->second.end())
			{
				typIt= famIt->second.find(chip.type);
				if(typIt != famIt->second.end())
				{
					idIt= typIt->second.find(chip.id);
					if(	idIt != typIt->second.end() &&
						idIt->second.size() > 0		)
					{
						nInternChip= idIt->second.begin()->second.internReaderChipID;
					}
				}
			}
		}
		if(nInternChip == 0)
		{
			++m_nInternChip;
			nInternChip= m_nInternChip;
		}
		chip.internReaderChipID= nInternChip;
		pinsplit= ConfigPropertyCasher::split(chip.pin, ":");
		for(vector<string>::iterator s= pinsplit.begin(); s != pinsplit.end(); ++s)
		{
			bFound= false;
			chip.pin= *s;
			pDefaultChip= getRegisteredDefaultChip(chip.server, chip.family, chip.type, chip.id, chip.pin);
			if(pDefaultChip)
			{
				if(	pDefaultChip->server == chip.server &&
					pDefaultChip->family == chip.family &&
					pDefaultChip->type == chip.type &&
					pDefaultChip->pin == chip.pin			)
				{// when chip was found, do not save again inside chips map
					bFound= true;
				}
				if(chip.older == NULL)
					chip.older= pDefaultChip->older;
			}
			if(!bFound)
				m_mmmmmChips[chip.server][chip.family][chip.type][chip.id][*s]= chip;
		}
		return nInternChip;
	}

	const SHAREDPTR::shared_ptr<otime_t> DefaultChipConfigReader::getLastActiveOlder(const string& folder,
																const string& subroutine, const bool nonactive)
	{
		map<string, SHAREDPTR::shared_ptr<otime_t> >::iterator it;
		SHAREDPTR::shared_ptr<otime_t> older;
		map<string, chips_t*>::const_iterator subIt;
		map<string, map<string, chips_t*> >::const_iterator folderIt;

		it= m_mCurrentValues.find(folder + ":" + subroutine);
		if(it != m_mCurrentValues.end())
			return it->second->older;
		return SHAREDPTR::shared_ptr<otime_t>();
	}

	const chips_t* DefaultChipConfigReader::getRegisteredDefaultChip(const string& server, const string& chip) const
	{
		map<string, map<string, chips_t> >::const_iterator sIt;
		map<string, chips_t>::const_iterator cIt;

		sIt= m_mmUsedChips.find(server);
		if(sIt == m_mmUsedChips.end())
			return NULL;

		cIt= sIt->second.find(chip);
		if(cIt == sIt->second.end())
			return NULL;

		return &cIt->second;
	}

	const double* DefaultChipConfigReader::getRegisteredDefaultChipCache(const string& server, const string& chip) const
	{
		const chips_t *tchip;

		tchip= getRegisteredDefaultChip(server, chip);
		if(tchip == NULL)
			return NULL;
		return &tchip->dCache;
	}

	const chips_t* DefaultChipConfigReader::getRegisteredDefaultChip(string server, string family,
																		string type, string chip, string pin) const
	{
		bool btype(false), bchip[2]= {false, false }, bpin[3]= {false, false, false };
		string ifamily(family), itype(type), ichip(chip), ipin(pin);
		map<string, map<string, map<string, map<string, map<string, chips_t> > > > >::const_iterator servIt;
		map<string, map<string, map<string, map<string, chips_t> > > >::const_iterator famIt;
		map<string, map<string, map<string, chips_t> > >::const_iterator typIt;
		map<string, map<string, chips_t> >::const_iterator idIt;
		map<string, chips_t>::const_iterator pinIt;

		servIt= m_mmmmmChips.find(server);
		if(servIt == m_mmmmmChips.end())
			return NULL;
		//cout << endl;
		while(true)
		{
			//cout << "search for server'" << server << "' family'" << ifamily << "' type'" << itype << "' chip'" << ichip << "' pin'" << ipin << "'" << endl;
			famIt= servIt->second.find(ifamily);
			if(famIt == servIt->second.end())
			{
				if(ifamily != "###all")
				{
					ifamily= "###all";
					continue;
				}
				return NULL;
			}
			typIt= famIt->second.find(itype);
			if(typIt == famIt->second.end())
			{
				if(itype != "###all")
				{
					itype= "###all";
					continue;
				}
				if(	!btype &&
					ifamily != "###all")
				{
					ifamily= "###all";
					itype= type;
					btype= true;
					continue;
				}
				return NULL;
			}
			idIt= typIt->second.find(ichip);
			if(	idIt == typIt->second.end())
			{
				if(ichip != "###all")
				{
					ichip= "###all";
					continue;
				}
				if(	!bchip[0] &&
					itype != "###all")
				{
					ifamily= family;
					itype= "###all";
					ichip= chip;
					bchip[0]= true;
					continue;
				}
				if(	!bchip[1] &&
					ifamily != "###all")
				{
					ifamily= "###all";
					itype= type;
					ichip= chip;
					bchip[1]= true;
					continue;
				}
				return NULL;
			}
			pinIt= idIt->second.find(ipin);
			if(pinIt == idIt->second.end())
			{
				if(ipin != "###all")
				{
					ipin= "###all";
					continue;
				}
				if(	!bpin[0] &&
					ichip != "###all")
				{
					ifamily= family;
					itype= type;
					ichip= "###all";
					ipin= pin;
					bpin[0]= true;
					continue;
				}
				if(	!bpin[1] &&
					itype != "###all")
				{
					ifamily= family;
					itype= "###all";
					ichip= chip;
					ipin= pin;
					bpin[1]= true;
					continue;
				}
				if(	!bpin[2] &&
					ifamily != "###all")
				{
					ifamily= "###all";
					itype= type;
					ichip= chip;
					ipin= pin;
					bpin[2]= true;
					continue;
				}
				return NULL;
			}
			return &pinIt->second;
		}
		return NULL;
	}

	SHAREDPTR::shared_ptr<otime_t> DefaultChipConfigReader::readOlderSection(IInterlacedPropertyPattern* property)
	{
		SHAREDPTR::shared_ptr<otime_t> older(new otime_t);
		string dbolder, dbwrite, dbinterval, dbafter;

		older->active= false;
		older->more= 0;
		older->unit= 'D';
		dbolder= property->getValue("dbolder", /*warning*/false);
		dbwrite= property->getValue("dbwrite", /*warning*/false);
		dbinterval= property->getValue("dbinterval", /*warning*/false);
		dbafter= property->getValue("dbafter", /*warning*/false);

		if(dbolder != "") {
			string d;
			istringstream istr(dbolder);

			istr >> older->more;
			istr >> d;
			if(	older->more > 365 ||
				(	d != "D" &&
					d != "W" &&
					d != "M" &&
					d != "Y"	)	)
			{
				ostringstream msg;

				older->unit= 'M';
				older->more= 2;
				msg << property->getMsgHead(/*error*/false);
				msg << "dbolder is undefined, check values after all month";
				LOG(LOG_WARNING, msg.str());
				cerr << msg.str() << endl;
			}else
			{	// calculate for all units one more,
				// because calculation should beginning
				// at the end of an unit
				older->more+= 1;
				older->unit= d[0];
			}
		}
		if(dbwrite != "") {
			if(	dbwrite != "all"
				&&
				dbwrite != "fractions"
				&&
				dbwrite.substr(0, 8) != "highest:"
				&&
				dbwrite != "kill"					)
			{
				string msg;

				msg= property->getMsgHead(/*error*/false);
				msg+= "dbwrite can be 'all', 'fractions', 'highest:<range>' or 'kill', can not read '";
				msg+= dbwrite+ "'. Set dbwrite to all.";
				LOG(LOG_WARNING, msg);
				cerr << msg << endl;
				older->dbwrite= "all";
			}else
			{
				if(dbwrite.substr(0, 8) == "highest:")
					older->dbwrite= "highest";
				else
					older->dbwrite= dbwrite;
				if(older->dbwrite == "highest")
				{
					string d;
					istringstream istr(dbwrite.substr(8));


					older->highest= auto_ptr<highest_t>(new highest_t);
					older->highest->between= 0;
					istr >> older->highest->between;
					istr >> d;
					if(	older->highest->between == 0
						||
						(	d != "h"
							&&
							d != "D"
							&&
							d != "W"
							&&
							d != "M"
							&&
							d != "Y"	)				)
					{
						ostringstream msg;

						if(older->highest->between == 0)
							older->highest->between= 1;
						older->highest->t= 'D';
						msg << property->getMsgHead(/*error*/false);
						msg << "dbwrite is undefined, check values after all ";
						msg << dec << older->highest->between << " day(s)";
						LOG(LOG_WARNING, msg.str());
						cerr << msg.str() << endl;
					}else
						older->highest->t= d[0];

					older->highest->bValue= false;
					older->highest->highest= 0;
					older->highest->hightime.tv_sec= 0;
					older->highest->hightime.tv_usec= 0;
					older->highest->lowest= 0;
					older->highest->lowtime.tv_sec= 0;
					older->highest->lowtime.tv_usec= 0;

				}else if(dbwrite == "fractions") {
					older->fraction= auto_ptr<fraction_t>(new fraction_t);
					older->fraction->bValue= false;
					older->fraction->dbafter= 0;
					older->fraction->dbinterval= 0;
					older->fraction->direction= "";
					older->fraction->writtenvalue= 0;
					older->fraction->deepvalue= 0;
					older->fraction->deeptime.tv_sec= 0;
					older->fraction->deeptime.tv_usec= 0;
					older->fraction->write= 0;
				}

			}
		}
		if(dbinterval != "") {
			if(dbwrite != "fractions") {
				string msg(property->getMsgHead(/*error*/false));

				msg+= "dbinterval only be used when dbwrite is set to 'fractions'";
				LOG(LOG_WARNING, msg);
				cerr << msg << endl;
			}else {
				older->fraction->dbinterval= atof(dbinterval.c_str());
			}
		}
		if(dbafter != "") {
			if(dbwrite != "fractions") {
				string msg(property->getMsgHead(/*error*/false));

				msg+= "dbafter only be used when dbwrite is set to 'fractions";
				LOG(LOG_WARNING, msg);
				cerr << msg << endl;
			}else {
				string d;
				istringstream istr(dbafter);

				older->fraction->dbafter= 0;
				istr >> older->fraction->dbafter;
				istr >> d;
				if(older->fraction->dbafter < 1) {
					older->fraction->dbafter= 60 * 60; // 1 hour
					d= "";
				}
				if(d == "m")
					older->fraction->dbafter*= 60;
				else if(d == "h")
					older->fraction->dbafter*= 60 * 60;
				else if(d == "D")
					older->fraction->dbafter*= 60 * 60 * 12;
				else if(d != "s")
				{
					string msg;

					msg= property->getMsgHead(/*error*/false);
					msg+= "dbafter value is undefined save value after every 1 hour";
					LOG(LOG_WARNING, msg);
					cerr << msg << endl;
				}
			}
		}
		return older;
	}

	void DefaultChipConfigReader::registerChip(const string& server, const string& chip, const string& pin,
												const string& type, const string& family, const double* pdmin/*= NULL*/,
												const double* pdmax/*= NULL*/, const bool* pbFloat/*=NULL*/, const double* pdCache/*= NULL*/)
	{
		chips_t entry;
		map<string, map<string, map<string, map<string, map<string, chips_t> > > > >::iterator servIt;
		map<string, map<string, map<string, map<string, chips_t> > > >::iterator famIt;
		map<string, map<string, map<string, chips_t> > >::iterator typIt;
		map<string, map<string, chips_t> >::iterator idIt;
		map<string, chips_t>::iterator pinIt;

		if(pdmin == NULL)
		{
			servIt= m_mmmmmChips.find(server);
			if(servIt == m_mmmmmChips.end()) {
				string msg;

				msg= "ALERT ERROR: found no defined running server '";
				msg+= server + "' for chip '";
				msg+= chip + "'";
				LOG(LOG_ALERT, msg);
				cerr << msg << endl;
				return;
			}
			famIt= servIt->second.find(family);
			if(famIt == servIt->second.end()) {
				famIt= servIt->second.find("###all");
				if(famIt == servIt->second.end()) {
					string msg;

					msg= "ERROR: found no family code with spezification '";
					msg+= family + "' for chip '";
					msg+= chip + "'";
					LOG(LOG_ERROR, msg);
					cerr << msg << endl;
					return;

				}
			}
			typIt= famIt->second.find(type);
			if(typIt == famIt->second.end()) {
				typIt= famIt->second.find("###all");
				if(typIt == famIt->second.end()) {
					string msg;

					msg= "ERROR: found no type code with spezification '";
					msg+= type + "' for chip '";
					msg+= chip + "'";
					LOG(LOG_ERROR, msg);
					cerr << msg << endl;
					return;

				}
			}
			idIt= typIt->second.find(chip);
			if(idIt == typIt->second.end()) {
				idIt= typIt->second.find("###all");
				if(idIt == typIt->second.end()) {
					string msg;

					msg= "ERROR: do not found chip '";
					msg+= chip + "'";
					LOG(LOG_ERROR, msg);
					cerr << msg << endl;
					return;

				}
			}
			pinIt= idIt->second.find(pin);
			if(pinIt == idIt->second.end()) {
				pinIt= idIt->second.find("###all");
				if(pinIt == idIt->second.end()) {
					string msg;

					msg= "WARNING: found no correct pin '";
					msg+= pin + "' for chip '";
					msg+= chip + "'";
					LOG(LOG_INFO, msg);
					return;

				}
			}
			entry= pinIt->second;
			entry.server= server;
			entry.family= family;
			entry.type= type;
			entry.id= chip;
			entry.pin= pin;
			if(pdmin)
				entry.dmin= *pdmin;
			if(pdmax)
				entry.dmax= *pdmax;
			if(pbFloat)
				entry.bFloat= *pbFloat;
			if(pdCache)
				entry.dCache= *pdCache;
			entry.older= copyOlder(pinIt->second.older);
			m_mmUsedChips[server][chip]= entry;
		}else
		{
			bool bFloat;
			double min, max;

			if(!pbFloat)
				bFloat= false;
			else
				bFloat= *pbFloat;
			if(!pdmax)
				max= 0;
			else
				max= *pdmax;
			if(!pdmin)
				min= 1;
			else
				min= *pdmin;
			entry.server= server;
			entry.id= chip;
			entry.family= family;
			entry.type= type;
			entry.pin= pin;
			entry.dmin= min;
			entry.dmax= max;
			entry.bFloat= bFloat;
			if(pdCache)
				entry.dCache= *pdCache;
			else
				entry.dCache= 0;
			if(chip.substr(0, 11) != "###DefChip ")
			{
				const chips_t* defChip;

				defChip= getRegisteredDefaultChip(server, family, type, chip, pin);
				if(defChip)
				{
					entry.internReaderChipID= defChip->internReaderChipID;
					entry.older= getNewDefaultChipOlder(/*folder*/"###all", /*subroutine*/"###all", min, max, bFloat, entry.internReaderChipID);
				}
			}
			m_mmmmmChips[server][family][type][chip][pin]= entry;
			m_mmUsedChips[server][chip]= entry;
		}
	}

	SHAREDPTR::shared_ptr<otime_t> DefaultChipConfigReader::copyOlder(const SHAREDPTR::shared_ptr<otime_t> &older) const
	{
		SHAREDPTR::shared_ptr<otime_t>  pRv;

		if(older.get() == NULL)
			return pRv;
		pRv= SHAREDPTR::shared_ptr<otime_t>(new otime_t);
		pRv->active= older->active;
		pRv->more= older->more;
		pRv->unit= older->unit;
		pRv->dbwrite= older->dbwrite;
		if(older->fraction.get())
		{
			pRv->fraction= auto_ptr<fraction_t>(new fraction_t);
			*(pRv->fraction)= *(older->fraction);
		}
		if(older->highest.get())
		{
			pRv->highest= auto_ptr<highest_t>(new highest_t);
			*pRv->highest= *older->highest;
		}
		pRv->older= copyOlder(older->older);
		return pRv;
	}

	void DefaultChipConfigReader::registerSubroutine(const string& subroutine, const string& folder, const string& server, const string& chip)
	{
		const chips_t* pDefaultChip;
		map<string, chips_t>::iterator usedIt;
		map<string, map<string, chips_t> >::iterator servIt;

		servIt= m_mmUsedChips.find(server);
		if(servIt == m_mmUsedChips.end()) {
			string msg;

			msg= "ERROR: found no server '";
			msg+= server + "' for chip '";
			msg+= chip + "' in used chips";
			LOG(LOG_ERROR, msg);
			cerr << msg << endl;
			return;
		}
		usedIt= servIt->second.find(chip);
		if(usedIt == servIt->second.end()) {
			string msg;

			msg= "ERROR: found no chip '";
			msg+= chip + "' for server '";
			msg+= server + "' in used chips";
			LOG(LOG_ERROR, msg);
			cerr << msg << endl;
			return;
		}
		if(usedIt->second.older == NULL)
		{
			if(chip.substr(0, 10) != "###DefChip")
			{
				pDefaultChip= getRegisteredDefaultChip(usedIt->second.server, usedIt->second.family,
														usedIt->second.type, usedIt->second.id, usedIt->second.pin);
				if(	pDefaultChip != NULL &&
					pDefaultChip->older != NULL	)
				{
					usedIt->second.older= pDefaultChip->older;
					m_mmAllChips[folder][subroutine]= &usedIt->second;
					return;
				}
			}
			usedIt->second.older= getNewDefaultChipOlder(folder, subroutine, usedIt->second.dmin, usedIt->second.dmax, usedIt->second.bFloat);
		}
		m_mmAllChips[folder][subroutine]= &usedIt->second;
	}

	double DefaultChipConfigReader::getDefaultCache(const double min, const double max, const bool bFloat, const string& folder/*= ""*/, const string& subroutine/*= ""*/) const
	{
		defValues_t defValues;

		defValues= getDefaultValues(min, max, bFloat, folder, subroutine);
		return defValues.dcache;
	}

	const defValues_t DefaultChipConfigReader::getDefaultValues(const double min, const double max, const bool bFloat,
																string folder/*= ""*/, string subroutine/*= ""*/, unsigned int nInternChipID/*= 0*/) const
	{
		bool bAll= false;
		double dRange;
		map<unsigned int, map<string, map<string, map<double , map<bool, defValues_t> > > > >::const_iterator itChip;
		map<string, map<string, map<double, map<bool, defValues_t> > > >::const_iterator folderIt;
		map<string, map<double, map<bool, defValues_t> > >::const_iterator subIt;
		map<double, map<bool, defValues_t> >::const_iterator rangeIt;
		map<double, map<bool, defValues_t> >::const_iterator highRange;
		map<double, map<bool, defValues_t> >::const_iterator lowRange;
		map<bool, defValues_t>::const_iterator floatIt;
		defValues_t nullDef = { 0, 0, false, 0, 0, SHAREDPTR::shared_ptr<otime_t>() };

		if(	folder != "" &&
			folder != "###all" &&
			subroutine != "" &&
			subroutine != "###all")
		{
			defValues_t tRv;
			map<string, chips_t*>::const_iterator itSubroutine;
			map<string, map<string, chips_t*> >::const_iterator itFolder;

			itFolder= m_mmAllChips.find(folder);
			if(itFolder != m_mmAllChips.end())
			{
				itSubroutine= itFolder->second.find(subroutine);
				if(itSubroutine != itFolder->second.end())
				{
					tRv.bFloat= itSubroutine->second->bFloat;
					tRv.dcache= itSubroutine->second->dCache;
					tRv.dmax= itSubroutine->second->dmax;
					tRv.dmin= itSubroutine->second->dmin;
					tRv.internReaderChipID= itSubroutine->second->internReaderChipID;
					nInternChipID= itSubroutine->second->internReaderChipID;
					tRv.older= itSubroutine->second->older;
					if(itSubroutine->second->older != NULL)
						return tRv;
				}
			}
		}
		do{
			itChip= m_mmmmDefaultValues.find(nInternChipID);
			if(itChip == m_mmmmDefaultValues.end())
			{
				if(nInternChipID != 0)
				{
					itChip= m_mmmmDefaultValues.find(0);
					if(itChip == m_mmmmDefaultValues.end())
						return nullDef;
				}else
					return nullDef;
			}
			if(folder != "")
			{
				for(folderIt= itChip->second.begin(); folderIt != itChip->second.end(); ++folderIt)
				{
					regex expr(folderIt->first);

					if(regex_match(folder, expr))
						break;
				}
			}
			if(	folder == "" ||
				folder == "###all" ||
				folderIt == itChip->second.end()	)
			{
				bAll= true;
				folderIt= itChip->second.find("###all");
				if(folderIt == itChip->second.end())
					return nullDef;
			}
			if(	bAll == false &&
				subroutine != "" &&
				subroutine != "###all"	)
			{
				for(subIt= folderIt->second.begin(); subIt != folderIt->second.end(); ++subIt)
				{
					regex expr(subIt->first);

					if(regex_match(subroutine, expr))
						break;
				}
			}
			if(	bAll == true
				||
				subroutine == "" ||
				subroutine == "###all" ||
				subIt == folderIt->second.end()	)
			{
				subIt= folderIt->second.find("###all");
				if(subIt == folderIt->second.end())
				{
					if(folder != "")
					{// search default again with no folder
						folder= "";
						subroutine= "";
						continue;
					}
					return nullDef;
				}
			}
			highRange= subIt->second.end();
			lowRange= highRange;
			if(min > max)
				dRange= 0;
			else
				dRange= max - min;
			for(rangeIt= subIt->second.begin(); rangeIt != subIt->second.end(); ++rangeIt)
			{
				if(dRange == rangeIt->first)
				{
					lowRange= rangeIt;
					break;
				}
				if(rangeIt->first == 0)
					highRange= rangeIt;
				if(	rangeIt->first > dRange
					&&
					(	lowRange == subIt->second.end()
						||
						(	dRange < rangeIt->first
							&&
							lowRange->first > rangeIt->first	)	)	)
				{
					lowRange= rangeIt;
				}
			}
			if(	highRange == subIt->second.end()
				&&
				lowRange == subIt->second.end()	)
			{
				if(subroutine != "")
				{// search default again with no subroutine
					subroutine= "";
					continue;
				}
				if(folder != "")
				{// search default again with no folder
					folder= "";
					continue;
				}
				return nullDef;
			}
			if(lowRange == subIt->second.end())
				lowRange= highRange;
			if(bFloat == false)
			{
				floatIt= lowRange->second.find(false);
				if(floatIt == lowRange->second.end())
				{
					floatIt= lowRange->second.find(true);
					if(floatIt == lowRange->second.end())
					{
						if(subroutine != "")
						{// search default again with no subroutine
							subroutine= "";
							continue;
						}
						if(folder != "")
						{// search default again with no folder
							folder= "";
							continue;
						}
						return nullDef;
					}
				}
			}else
			{
				floatIt= lowRange->second.find(true);
				if(floatIt == lowRange->second.end())
				{
					if(subroutine != "")
					{// search default again with no subroutine
						subroutine= "";
						continue;
					}
					if(folder != "")
					{// search default again with no folder
						folder= "";
						continue;
					}
					return nullDef;
				}
			}

			// found default, leaf while loop
			break;

		}while(floatIt != lowRange->second.end());
		if(floatIt == lowRange->second.end())
			return nullDef;// find no default
		return floatIt->second;
	}


	SHAREDPTR::shared_ptr<otime_t> DefaultChipConfigReader::getNewDefaultChipOlder(const string& folder, const string& subroutine,
																					const double min, const double max, const bool bFloat,
																					const unsigned int internChipID) const
	{
		defValues_t def;

		def= getDefaultValues(min, max, bFloat, folder, subroutine, internChipID);
		if(!def.older) // toDo: check whether can define instance of shared_ptr as boolean for content
			return SHAREDPTR::shared_ptr<otime_t>();
		return copyOlder(def.older);
	}



	write_t DefaultChipConfigReader::allowDbWriting(const string& folder, const string& subroutine, const double value,
					const timeval acttime, const string& thinning/*= ""*/, bool* newOlder/*=NULL*/)
	{
		bool bThinning(false);
		string sFolderSub(folder + ":" + subroutine);
		write_t tRv;
		SHAREDPTR::shared_ptr<otime_t> wr;
		chips_t* chip;
		time_t thistime;
		map<string, chips_t*>::const_iterator subIt;
		map<string, map<string, chips_t*> >::const_iterator folderIt;
		map<string, SHAREDPTR::shared_ptr<otime_t> >::iterator itCurrent;

		if(thinning != "")
			bThinning= true;
		tRv.folder= folder;
		tRv.subroutine= subroutine;
		tRv.highest.highest= 0;
		tRv.highest.hightime.tv_sec= 0;
		tRv.highest.hightime.tv_usec= 0;
		tRv.highest.lowest= 0;
		tRv.highest.lowtime.tv_sec= 0;
		tRv.highest.lowtime.tv_usec= 0;
		if(newOlder)
			*newOlder= false;
		tRv.action= "no";
		folderIt= m_mmAllChips.find(folder);
		if(folderIt == m_mmAllChips.end())
		{ // found no specified folder which is registered, so do not write into database
			return tRv;
		}
		subIt= folderIt->second.find(subroutine);
		if(subIt == folderIt->second.end())
		{ // found no specified action structure which is registered, so do not write into database
			return tRv;
		}
		tRv.action= "write";
		chip= subIt->second;
		if(chip->older == NULL)
		{ // no action is defined for default chip, so allow writing
			string errmsg("found no specified action structure for registered folder:subroutine '");

			errmsg+= folder + ":" + subroutine + "'\n";
			errmsg+= "           so do not allow writing into database";
			if(newOlder)
				errmsg+= " by thinning";
			LOG(LOG_ALERT,  errmsg);
			cerr << "### ALERT: " << errmsg << endl;
			return tRv;
		}
		time(&thistime);
		wr= chip->older;
		if(bThinning)
		{
			while(wr->older.get() != NULL)
			{
				Calendar::time_e unit;

				wr->older->active= false;
				if(wr->older->unit == 's')
					unit= Calendar::seconds;
				else if(wr->older->unit == 'm')
					unit= Calendar::minutes;
				else if(wr->older->unit == 'h')
					unit= Calendar::hours;
				else if(wr->older->unit == 'D')
					unit= Calendar::days;
				else if(wr->older->unit == 'W')
					unit= Calendar::weeks;
				else if(wr->older->unit == 'M')
					unit= Calendar::months;
				else if(wr->older->unit == 'Y')
					unit= Calendar::years;
				else
				{
					string msg(&wr->older->unit);

					msg= "undefined time unit '" + msg + "' for calendar\nset older calculation to one year";
					TIMELOG(LOG_ALERT, "time_units"+string(&wr->older->unit), msg);
					unit= Calendar::years;
					wr->older->more= 1;
				}
				if(Calendar::calcDate(/*newer*/false, thistime, wr->older->more, unit) < acttime.tv_sec)
				{
					SHAREDPTR::shared_ptr<otime_t> setFalse;

					wr->active= true;
					setFalse= wr;
					while(setFalse->older)
					{
						setFalse= setFalse->older;
						setFalse->active= false;
					}
					break;
				}
				wr= wr->older;
			}
			if(	wr->active == false
				&&
				wr->older != NULL
				&&
				wr->older->active == true	)
			{
				*newOlder= true;
			}
		}
		LOCK(m_CURRENTVALMUTEX);
		itCurrent= m_mCurrentValues.find(sFolderSub);
		if(itCurrent == m_mCurrentValues.end())
		{
			SHAREDPTR::shared_ptr<otime_t> curVal;

			curVal= SHAREDPTR::shared_ptr<otime_t>(new otime_t);
			*curVal= *chip->older;
			if(curVal->dbwrite == "fractions")
			{
				curVal->fraction= SHAREDPTR::shared_ptr<fraction_t>(new fraction_t);
				*curVal->fraction= *chip->older->fraction;
			}
			if(curVal->dbwrite == "highest")
			{
				curVal->highest= SHAREDPTR::shared_ptr<highest_t>(new highest_t);
				*curVal->highest= *chip->older->highest;
			}
			curVal->older= SHAREDPTR::shared_ptr<otime_t>();
			if(bThinning)
			{
				curVal->older= SHAREDPTR::shared_ptr<otime_t>(new otime_t);
				*curVal->older= *wr;
				if(wr->dbwrite == "fractions")
				{
					curVal->older->fraction= SHAREDPTR::shared_ptr<fraction_t>(new fraction_t);
					*curVal->older->fraction= *wr->fraction;
				}
				if(wr->dbwrite == "highest")
				{
					curVal->older->highest= SHAREDPTR::shared_ptr<highest_t>(new highest_t);
					*curVal->older->highest= *wr->highest;
				}
				wr= curVal->older;
			}else
			{
				curVal->older= SHAREDPTR::shared_ptr<otime_t>();
				wr= curVal;
			}
			m_mCurrentValues[sFolderSub]= curVal;

		}else
		{
			if(bThinning)
			{
				if(itCurrent->second->older == NULL)
				{
					itCurrent->second->older= SHAREDPTR::shared_ptr<otime_t>(new otime_t);
					*itCurrent->second->older= *wr;
					if(wr->dbwrite == "fractions")
					{
						itCurrent->second->older->fraction= SHAREDPTR::shared_ptr<fraction_t>(new fraction_t);
						*itCurrent->second->older->fraction= *wr->fraction;
					}
					if(wr->dbwrite == "highest")
					{
						itCurrent->second->older->highest= SHAREDPTR::shared_ptr<highest_t>(new highest_t);
						*itCurrent->second->older->highest= *wr->highest;
					}
				}
				if(	itCurrent->second->older->more != wr->more ||
					itCurrent->second->older->unit != wr->unit		)
				{
					// write first last entries of fractions or highest
					// before change to an new older settings
					if(	itCurrent->second->older->dbwrite == "fractions" &&
						timercmp(&itCurrent->second->older->lastwrite, &itCurrent->second->older->fraction->deeptime, !=)	)
					{
						wr= itCurrent->second->older;
						tRv.action= wr->dbwrite;
						tRv.highest.hightime= wr->fraction->deeptime;
						tRv.highest.highest= wr->fraction->deepvalue;
						wr->lastwrite= wr->fraction->deeptime;
						if(newOlder)
							*newOlder= true;
						UNLOCK(m_CURRENTVALMUTEX);
						return tRv;

					}else if(	itCurrent->second->older->dbwrite == "highest" &&
								timercmp(&itCurrent->second->older->highest->lowtime, &itCurrent->second->older->lastwrite, >)	)
					{
						wr= itCurrent->second->older;
						tRv.action= wr->dbwrite;
						tRv.highest.hightime= wr->highest->hightime;
						tRv.highest.highest= wr->highest->highest;
						tRv.highest.lowtime= wr->highest->lowtime;
						tRv.highest.lowest= wr->highest->lowest;
						wr->lastwrite= wr->highest->lowtime;
						if(newOlder)
							*newOlder= true;
						UNLOCK(m_CURRENTVALMUTEX);
						return tRv;
					}
					*itCurrent->second->older= *wr;
					if(wr->dbwrite == "fractions")
					{
						itCurrent->second->older->fraction= SHAREDPTR::shared_ptr<fraction_t>(new fraction_t);
						*itCurrent->second->older->fraction= *wr->fraction;
					}
					if(wr->dbwrite == "highest")
					{
						itCurrent->second->older->highest= SHAREDPTR::shared_ptr<highest_t>(new highest_t);
						*itCurrent->second->older->highest= *wr->highest;
					}
				}
				wr= itCurrent->second->older;
			}else
				wr= itCurrent->second;
		}
		if(	bThinning &&
			thinning != "value")
		{
			if(wr->more == 0)
				tRv.action= "write";
			else
				tRv.action= "no";
			UNLOCK(m_CURRENTVALMUTEX);
			return tRv;
		}
		if(wr->dbwrite == "all")
		{
			wr->lastwrite= acttime;
			tRv.highest.hightime= acttime;
			tRv.highest.highest= value;
			UNLOCK(m_CURRENTVALMUTEX);
			return tRv;
		}
		if(wr->dbwrite == "fractions") {
			double half;
			string direction;

			tRv.action= "fractions";
			if(wr->fraction.get() == NULL)
			{
				wr->fraction= auto_ptr<fraction_t>(new fraction_t);
				wr->fraction->bValue= false;
			}
			if(!wr->fraction->bValue)
			{
				wr->fraction->bValue= true;
				wr->fraction->writtenvalue= value;
				wr->fraction->deepvalue= value;
				wr->fraction->deeptime= acttime;
				wr->lastwrite= acttime;
				if(wr->fraction->dbafter)
					wr->fraction->write= acttime.tv_sec + wr->fraction->dbafter;
				wr->fraction->direction= "";
				// write highest.highest and highest.hightime
				tRv.highest.highest= value;
				tRv.highest.hightime= acttime;
				UNLOCK(m_CURRENTVALMUTEX);
				return tRv;
			}
			if(	wr->fraction->dbafter &&
				timercmp(&wr->lastwrite, &wr->fraction->deeptime, !=) &&
				wr->fraction->deeptime.tv_sec >= wr->fraction->write	)
			{
				wr->fraction->write= wr->fraction->deeptime.tv_sec + wr->fraction->dbafter;
				wr->lastwrite= wr->fraction->deeptime;
				wr->fraction->writtenvalue= wr->fraction->deepvalue;
				tRv.highest.hightime=  wr->fraction->deeptime;
				tRv.highest.highest= wr->fraction->deepvalue;
				if(newOlder)
					*newOlder= true;
				UNLOCK(m_CURRENTVALMUTEX);
				return tRv;
			}
			if(	(	wr->fraction->direction == "up"
					&&
					value > wr->fraction->deepvalue	)
				||
				(	wr->fraction->direction == "down"
					&&
					value < wr->fraction->deepvalue	)	)
			{
				wr->fraction->deepvalue= value;
				wr->fraction->deeptime= acttime;
			}
			half= wr->fraction->dbinterval /2;
			if(value > (wr->fraction->writtenvalue + half)) {
				// direction is up
				direction= wr->fraction->direction;
				wr->fraction->direction= "up";
				wr->fraction->writtenvalue= value;
				if(direction == "down")
				{
					bool bdis(false);

					if(timercmp(&wr->fraction->deeptime, &wr->lastwrite, !=))
					{
						bdis= true;
						tRv.highest.highest= wr->fraction->deepvalue;
						tRv.highest.hightime= wr->fraction->deeptime;
						wr->lastwrite= wr->fraction->deeptime;
						wr->fraction->write= wr->fraction->deeptime.tv_sec + wr->fraction->dbafter;
					}
					wr->fraction->deepvalue= value;
					wr->fraction->deeptime= acttime;
					if(bdis)
					{
						UNLOCK(m_CURRENTVALMUTEX);
						return tRv;
					}

				}else if(direction == "")
				{
					wr->fraction->deepvalue= value;
					wr->fraction->deeptime= acttime;
				}

			}else if(value < (wr->fraction->writtenvalue - half)	) {
				// direction is down
				direction= wr->fraction->direction;
				wr->fraction->direction= "down";
				wr->fraction->writtenvalue= value;
				if(direction == "up")
				{
					bool bdis(false);

					if(timercmp(&wr->fraction->deeptime, &wr->lastwrite, !=))
					{
						bdis= true;
						tRv.highest.highest= wr->fraction->deepvalue;
						tRv.highest.hightime= wr->fraction->deeptime;
						wr->lastwrite= wr->fraction->deeptime;
						wr->fraction->write= wr->fraction->deeptime.tv_sec + wr->fraction->dbafter;
					}
					wr->fraction->deepvalue= value;
					wr->fraction->deeptime= acttime;
					if(bdis)
					{
						UNLOCK(m_CURRENTVALMUTEX);
						return tRv;
					}

				}else if(direction == "")
				{
					wr->fraction->deepvalue= value;
					wr->fraction->deeptime= acttime;
				}
			}
			if(	wr->fraction->dbafter
				&&
				acttime.tv_sec >= wr->fraction->write	) {

				wr->fraction->write= acttime.tv_sec + wr->fraction->dbafter;
				wr->lastwrite= acttime;
				wr->fraction->writtenvalue= value;
				tRv.highest.hightime= acttime;
				tRv.highest.highest= value;
				UNLOCK(m_CURRENTVALMUTEX);
				return tRv;
			}
			tRv.action= "no";
			UNLOCK(m_CURRENTVALMUTEX);
			return tRv;
		}
		if(wr->dbwrite == "highest") {

			bool iserror= false;

			for(vector<double>::iterator it= chip->errorcode.begin(); it != chip->errorcode.end(); ++it)
			{
				if(*it == value)
				{
					iserror= true;
					break;
				}
			}
			if(!iserror)
			{
				if(wr->highest.get() == NULL)
				{
					wr->highest= SHAREDPTR::shared_ptr<highest_t>(new highest_t);
					wr->highest->bValue= false;
				}
				if(	!wr->highest->bValue
					||
					wr->highest->nextwrite < acttime.tv_sec	)
				{
					Calendar::time_e unit;

					if(wr->highest->bValue)
					{
						tRv.action= "highest";
						tRv.highest.highest= wr->highest->highest;
						tRv.highest.hightime= wr->highest->hightime;
						tRv.highest.lowest= wr->highest->lowest;
						tRv.highest.lowtime= wr->highest->lowtime;
						if(timercmp(&wr->highest->hightime, &wr->highest->lowtime, >))
							wr->lastwrite= wr->highest->hightime;
						else
							wr->lastwrite= wr->highest->lowtime;
					}else
						tRv.action= "no";

					if(wr->highest->t == 's')
						unit= Calendar::seconds;
					else if(wr->highest->t == 'm')
						unit= Calendar::minutes;
					else if(wr->highest->t == 'h')
						unit= Calendar::hours;
					else if(wr->highest->t == 'D')
						unit= Calendar::days;
					else if(wr->highest->t == 'W')
						unit= Calendar::weeks;
					else if(wr->highest->t == 'M')
						unit= Calendar::months;
					else if(wr->highest->t == 'Y')
						unit= Calendar::years;
					else
					{
						string msg(&wr->highest->t);

						msg= "undefined highest t time unit '" + msg + "' for calendar\nset older calculation to one year";
						TIMELOG(LOG_ALERT, "highest_t_time_units"+string(&wr->highest->t), msg);
						unit= Calendar::years;
						wr->highest->between= 1;
					}
					wr->highest->nextwrite= Calendar::calcDate(/*newer*/true, acttime.tv_sec, wr->highest->between, unit);
					wr->highest->bValue= true;
					wr->highest->highest= value;
					wr->highest->hightime= acttime;
					wr->highest->lowest= value;
					wr->highest->lowtime= acttime;
					UNLOCK(m_CURRENTVALMUTEX);
					return tRv;
				}
				if(wr->highest->highest < value) {
					wr->highest->highest= value;
					wr->highest->hightime= acttime;
				}else if(wr->highest->lowest > value) {
					wr->highest->lowest= value;
					wr->highest->lowtime= acttime;
				}
			}
			tRv.action= "no";
			UNLOCK(m_CURRENTVALMUTEX);
			return tRv;
		}
		// wr->dbwrite have to be action 'kill'
		tRv.action= "no";
		UNLOCK(m_CURRENTVALMUTEX);
		return tRv;
	}

	void DefaultChipConfigReader::setDbOlderNull()
	{
		LOCK(m_CURRENTVALMUTEX);
		for(map<string, SHAREDPTR::shared_ptr<otime_t> >::iterator it= m_mCurrentValues.begin(); it != m_mCurrentValues.end(); ++it)
		{
			if(it->second->older != NULL)
				it->second->older= SHAREDPTR::shared_ptr<otime_t>();
		}
		UNLOCK(m_CURRENTVALMUTEX);
	}

	write_t DefaultChipConfigReader::getLastValues(const bool bolder/*=false*/)
	{
		bool bwrite;
		write_t tRv;
		vector<string> splSub;
		SHAREDPTR::shared_ptr<otime_t> older;

		LOCK(m_CURRENTVALMUTEX);
		for(map<string, SHAREDPTR::shared_ptr<otime_t> >::iterator it= m_mCurrentValues.begin(); it != m_mCurrentValues.end(); ++it)
		{
			if(bolder)
				older= it->second->older;
			else
				older= it->second;
			if(older != NULL)
			{
				bwrite= false;
				split(splSub, it->first, is_any_of(":"));
				tRv.folder= splSub[0];
				tRv.subroutine= splSub[1];
				if(older->dbwrite == "highest")
				{
					bwrite= true;
					tRv.action= "highest";
					tRv.highest.highest= older->highest->highest;
					if(timercmp(&older->highest->hightime, &older->lastwrite, >))
					{
						tRv.highest.hightime= older->highest->hightime;
						bwrite= true;
					}else
						timerclear(&tRv.highest.hightime);
					tRv.highest.lowest= older->highest->lowest;
					if(timercmp(&older->highest->lowtime, &older->lastwrite, >))
					{
						tRv.highest.lowtime= older->highest->lowtime;
						bwrite= true;
					}else
						timerclear(&tRv.highest.lowtime);

				}else if(older->dbwrite == "fractions")
				{
					if(older->fraction.get() != NULL)
					{
						tRv.action= "fractions";
						tRv.highest.highest= older->fraction->deepvalue;
						tRv.highest.hightime= older->fraction->deeptime;
						if(timercmp(&older->fraction->deeptime, &older->lastwrite, >))
							bwrite= true;
					}

				}// else if dbwrite is all or kill -> nothing to do
				if(bolder)
					it->second->older= SHAREDPTR::shared_ptr<otime_t>();
				else if(bwrite)// erase value only when should written,
					m_mCurrentValues.erase(it); // because otherwise maybe iterate map will be inconsistent
				if(bwrite)
				{
					UNLOCK(m_CURRENTVALMUTEX);
					return tRv;
				}
			}
		}
		UNLOCK(m_CURRENTVALMUTEX);
		tRv.action= "kill"; // action kill is defined for reached ending
		tRv.highest.highest= 0;
		timerclear(&tRv.highest.hightime);
		tRv.highest.lowest= 0;
		timerclear(&tRv.highest.lowtime);

		return tRv;
	}

	DefaultChipConfigReader::~DefaultChipConfigReader()
	{
		DESTROYMUTEX(m_CURRENTVALMUTEX);
	}
}
