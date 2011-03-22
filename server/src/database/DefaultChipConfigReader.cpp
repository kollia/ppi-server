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
#include <stdlib.h>

#include "DefaultChipConfigReader.h"

#include "../logger/lib/LogInterface.h"

#include "../util/URL.h"
#include "../util/Calendar.h"

#include "../util/properties/configpropertycasher.h"


namespace ports
{
	using namespace util;

	DefaultChipConfigReader* DefaultChipConfigReader::_instance= NULL;

	void DefaultChipConfigReader::init(const string& path)
	{
		if(_instance == NULL)
			_instance= new DefaultChipConfigReader(path);
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
		ostringstream output;
		string fileName, propName, errmsg;
		InterlacedProperties properties;
		vector<IInterlacedPropertyPattern*> sections;

		fileName= URL::addPath(m_sPath, "default.conf", /*always*/true);
		properties.modifier("folder");
		properties.setMsgParameter("folder");
		properties.modifier("subroutine");
		properties.setMsgParameter("subroutine");
		properties.modifier("range");
		properties.setMsgParameter("range", "range of chip");
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
		properties.modifier("dbolder");
		properties.setMsgParameter("dbolder", "older time");
		properties.allowLaterModifier(true);
		if(!properties.readFile(fileName))
		{
			string msg;

			msg= "### ERROR: cannot read default configuration 'default.conf'\n";
			msg+= "           for all chips";
			cerr << msg << endl;
			LOG(LOG_ERROR, msg);
			return;
		}
		sections= properties.getSections();
		for(sit o= sections.begin(); o != sections.end(); ++o)
			readSection("", *o, /*default*/true);
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
		sections= properties.getSections();
		for(sit o= sections.begin(); o != sections.end(); ++o)
			readSection(server, *o, /*default*/false);
		properties.checkProperties();
	}

	void DefaultChipConfigReader::readSection(string server, IInterlacedPropertyPattern *property, const bool bDefault)
	{
		typedef vector<IInterlacedPropertyPattern*>::iterator sit;

		bool read= false;
		bool bwritten= false;
		bool onlyDefault= false;
		int errorcount= 0;
		double errorcode;
		double dRange;
		double dCache= 0;
		vector<IInterlacedPropertyPattern*> sections;
		vector<string> split;
		string propName("cache");
		string family, type, ID, pin, dbolder;
		string range, sfloat, writable, dbwrite;
		string folder, subroutine;
		chips_t chip;
		defValues_t defaultValues;

		if(bDefault)
		{
			server= "###all";
			folder= property->getValue("folder", /*warning*/false);
			if(folder == "")
				folder= "###all";
			else
				onlyDefault= true;
			subroutine= property->getValue("subroutine", /*warning*/false);
			if(subroutine == "")
				subroutine= "###all";
			else
			{
				if(folder == "###all")
					property->needValue("folder");
				onlyDefault= true;
			}
		}
		if(onlyDefault == false)
		{
			family= property->getValue("family", /*warning*/false);
			if(family == "")
				family= "###all";
			type= property->getValue("type", /*warning*/false);
			if(type == "")
				type= "###all";
			ID= property->getValue("ID", /*warning*/false);
			if(ID == "")
				ID= "###all";
			pin= property->getValue("pin", /*warning*/false);
			if(pin == "")
				pin= "###all";
			if(	bDefault
				&&
				(	family != "###all"
					||
					type != "###all"
					||
					ID != "###all"
					||
					pin != "###all"		)	)
			{
				server= property->needValue("server");
			}
		}

		range= property->getValue("range", /*warning*/false);
		if(range != "")
			read= true;
		else if(bDefault)
			range= "1:0";
		propName= "cache";
		dCache= property->getDouble(propName, /*warning*/false);
		if(dCache != 0)
			read= true;

		sfloat= property->getValue("float", /*warning*/false);
		if(sfloat != "")
			read= true;
		else
			sfloat= "false";
		if(!bDefault)
		{
			writable= property->getValue("writable", /*warning*/false);
			if(writable != "")
				read= true;
			errorcount= property->getPropertyCount("errorcode");
			if(errorcount)
				read= true;
		}
		if(	bDefault
			&&
			read	)
		{
			dbwrite= property->needValue("dbwrite");
		}/*else
		{
			dbwrite= property->getValue("dbwrite", *warning*false);
			if(dbwrite != "")
				read= true;
		}*/

		chip.server= server;
		chip.family= family;
		chip.type= type;
		chip.id= ID;
		chip.dmin= 1;
		chip.dmax= 0;
		chip.bFloat= false;
		chip.dCache= dCache;
		chip.bWritable= false;

		if(read)
		{
			string str;

			if(range != "")
			{
				split= ConfigPropertyCasher::split(range, ":");
				if(split.size() < 2)
				{
					string msg;

					msg= property->getMsgHead(/*ERROR*/true);
					msg+= "range should have 2 double values separated with an colon";
					LOG(LOG_ERROR, msg);
					cerr << msg << endl;
				}else
				{
					string str(split[0]);

					str+= " ";
					str+= split[1];
					istringstream istr(str);
					istr >> chip.dmin >> chip.dmax;
				}
			}
			if(	bDefault
				&&
				sfloat == ""	)
			{
				sfloat= "true";
			}
			if(sfloat != "")
			{
				if(sfloat == "true")
					chip.bFloat= true;
				else if(sfloat != "false")
				{
					string msg;

					msg= property->getMsgHead(/*error*/false);
					msg+= "float have to be 'true' or 'false', can not read '";
					msg+= sfloat + "'. Set float to false.";
					LOG(LOG_WARNING, msg);
					cerr << msg << endl;
				}
			}
			if(writable != "")
			{
				if(writable == "true")
					chip.bWritable= true;
				else if(writable != "false")
				{
					string msg;

					msg= property->getMsgHead(/*error*/false);
					msg+= "writable have to be 'true' or 'false', can not read '";
					msg+= writable + "'. Set writable to false.";
					LOG(LOG_WARNING, msg);
					cerr << msg << endl;
				}
			}
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
			if(dbwrite != "")
				chip.older= readOlderSection(property, /*first*/true);
			if(bDefault)
			{
				if(	onlyDefault
					||
					(	server == "###all"
						&&
						family == "###all"
						&&
						type == "###all"
						&&
						ID == "###all"
						&&
						pin == "###all"		)	)
				{
					if(chip.dmin > chip.dmax)
						dRange= 0;
					else
						dRange= chip.dmax - chip.dmin;
					defaultValues.dmin= chip.dmin;
					defaultValues.dmax= chip.dmax;
					defaultValues.bFloat= chip.bFloat;
					defaultValues.dcache= dCache;
					defaultValues.older= chip.older;
					m_mmmmDefaultValues[folder][subroutine][dRange][chip.bFloat]= defaultValues;
				}else
					saveChip(chip, server, family, type, ID, pin);
			}else
				saveChip(chip, server, family, type, ID, pin);
			bwritten= true;
		}

		sections= property->getSections();
		for(sit o= sections.begin(); o != sections.end(); ++o)
		{
			if((*o)->getSectionModifier() == "dbolder") {
				if(!chip.older) {
					chip.older= SHAREDPTR::shared_ptr<otime_t>(new otime_t);
					chip.older->active= false;			// the first older is only
					chip.older->more= 0;				// for current reading
					chip.older->unit= 'D';
					chip.older->dbwrite= "all";
					chip.older->older= SHAREDPTR::shared_ptr<otime_t>(new otime_t);
					chip.older->older->active= false;
					chip.older->older->more= 0;			// the second and all other
					chip.older->older->unit= 'D';		// to thin database
					chip.older->older->dbwrite= "all";
					chip.older->older->older= readOlderSection(*o, /*first*/false);
				}else {
					SHAREDPTR::shared_ptr<otime_t> older;

					older= chip.older;
					while(older->older)
						older= older->older;
					older->older= readOlderSection(*o, /*first*/false);
				}
				if(!bwritten) {
					m_mmmmmChips[server][family][type][ID][pin]= chip;
					bwritten= true;
				}
			}else
				readSection(server, *o, bDefault);
		}
	}

	void DefaultChipConfigReader::saveChip(chips_t chip, const string& server, const string& family, const string& type, const string& ID, const string& pin)
	{
		bool bDefault= false;
		vector<string> pinsplit;
		chips_t *addr;
		chips_t *exist;

		pinsplit= ConfigPropertyCasher::split(pin, ":");
		for(vector<string>::iterator s= pinsplit.begin(); s != pinsplit.end(); ++s)
		{
			chip.pin= *s;
			m_mmmmmChips[server][family][type][ID][*s]= chip;
			if(	!bDefault
				&&
				pin != "###all"	)
			{
				addr= &m_mmmmmChips[server][family][type][ID][*s];
				if(family != "###all")
				{
					exist= m_mDefaults[server]["family"][family];
					if(!exist)
						m_mDefaults[server]["family"][family]= addr;
				}
				if(type != "###all")
				{
					exist= m_mDefaults[server]["type"][type];
					if(!exist)
						m_mDefaults[server]["type"][type]= addr;
				}
				if(ID != "###all")
				{
					exist= m_mDefaults[server]["ID"][ID];
					if(!exist)
						m_mDefaults[server]["ID"][ID]= addr;
				}
				bDefault= true;
			}
		}
	}

	const SHAREDPTR::shared_ptr<DefaultChipConfigReader::otime_t> DefaultChipConfigReader::getLastActiveOlder(const string& folder, const string& subroutine, const bool nonactive)
	{
		SHAREDPTR::shared_ptr<otime_t> older;
		map<string, chips_t*>::const_iterator subIt;
		map<string, map<string, chips_t*> >::const_iterator folderIt;

		folderIt= m_mmAllChips.find(folder);
		if(folderIt == m_mmAllChips.end())
			return SHAREDPTR::shared_ptr<otime_t>();
		subIt= folderIt->second.find(subroutine);
		if(subIt == folderIt->second.end())
			return SHAREDPTR::shared_ptr<otime_t>();
		if(subIt->second->older.get() == NULL)
			return SHAREDPTR::shared_ptr<otime_t>();
		older= subIt->second->older->older;
		while(older)
		{
			if(	older->active == true
				&&
				(	older->older.get() == NULL
					||
					(	older->older
						&&
						older->older->active == false	)	)	)
			{
				break;
			}
			older= older->older;
		}
		if(	older
			&&
			older->active == true	)
		{
			if(nonactive)
				older->active= false;
			return older;
		}
		return SHAREDPTR::shared_ptr<otime_t>();
	}

	const DefaultChipConfigReader::chips_t* DefaultChipConfigReader::getRegisteredDefaultChip(const string& server, const string& chip) const
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

	const DefaultChipConfigReader::chips_t* DefaultChipConfigReader::getRegisteredDefaultChip(const string& server, const string& family, const string& type, const string& chip) const
	{
		map<string, map<string, map<string, chips_t*> > >::const_iterator sIt;
		map<string, map<string, chips_t*> >::const_iterator nIt;
		map<string, chips_t*>::const_iterator tIt;

		sIt= m_mDefaults.find(server);
		if(sIt == m_mDefaults.end())
			return NULL;

		nIt= sIt->second.find("ID");
		if(nIt != sIt->second.end())
		{
			tIt= nIt->second.find(chip);
			if(tIt != nIt->second.end())
				if(tIt->second->pin != "###all")
					return tIt->second;
		}
		nIt= sIt->second.find("type");
		if(nIt != sIt->second.end())
		{
			tIt= nIt->second.find(type);
			if(tIt != nIt->second.end())
				if(tIt->second->pin != "###all")
					return tIt->second;
		}
		nIt= sIt->second.find("family");
		if(nIt != sIt->second.end())
		{
			tIt= nIt->second.find(family);
			if(tIt != nIt->second.end())
				if(tIt->second->pin != "###all")
					return tIt->second;
		}
		return NULL;
	}

	SHAREDPTR::shared_ptr<DefaultChipConfigReader::otime_t> DefaultChipConfigReader::readOlderSection(IInterlacedPropertyPattern* property, const bool first) {

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
			if(	older->more == 0
				||
				(	d != "W"
					&&
					d != "M"
					&&
					d != "Y"	)	)
			{
				ostringstream msg;

				older->unit= 'M';
				if(older->more == 0)
					older->more= 1;
				msg << property->getMsgHead(/*error*/false);
				msg << "dbolder is undefined, check values after all ";
				msg << dec << older->more << " month";
				LOG(LOG_WARNING, msg.str());
				cerr << msg.str() << endl;
			}else
				older->unit= d[0];
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
					older->highest->hightime= 0;
					older->highest->lowest= 0;
					older->highest->lowtime= 0;

				}else if(dbwrite == "fractions") {
					older->fraction= auto_ptr<fraction_t>(new fraction_t);
					older->fraction->bValue= false;
					older->fraction->dbafter= 0;
					older->fraction->dbinterval= 0;
					older->fraction->direction= "";
					older->fraction->writtenvalue= 0;
					older->fraction->deepvalue= 0;
					older->fraction->deeptime= 0;
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
		if(first == true)
		{ // if the older section is the first, the method duplicate the section,
		  // because the first is for normally reading and all other to thin database
			older->older= copyOlder(older);
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
			m_mmmmmChips[server][family][type][chip][pin]= entry;
			m_mmUsedChips[server][chip]= entry; //&m_mmmmmChips[server][family][type][chip][pin];
		}
	}

	SHAREDPTR::shared_ptr<DefaultChipConfigReader::otime_t> DefaultChipConfigReader::copyOlder(const SHAREDPTR::shared_ptr<DefaultChipConfigReader::otime_t> &older) const
	{
		SHAREDPTR::shared_ptr<otime_t>  pRv;

		if(older.get() == NULL)
			return pRv;
		pRv= SHAREDPTR::shared_ptr<otime_t>(new otime_t);
		pRv->active= older->active;
		pRv->more= older->more;
		pRv->unit= older->unit;
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

	const DefaultChipConfigReader::defValues_t DefaultChipConfigReader::getDefaultValues(const double min, const double max, const bool bFloat, const string& folder/*= ""*/, const string& subroutine/*= ""*/) const
	{
		bool bAll= false;
		double dRange;
		map<string, map<string, map<double, map<bool, defValues_t> > > >::const_iterator folderIt;
		map<string, map<double, map<bool, defValues_t> > >::const_iterator subIt;
		map<double, map<bool, defValues_t> >::const_iterator rangeIt;
		map<double, map<bool, defValues_t> >::const_iterator highRange;
		map<double, map<bool, defValues_t> >::const_iterator lowRange;
		map<bool, defValues_t>::const_iterator floatIt;
		defValues_t nullDef = { 0, 0, false, 0, SHAREDPTR::shared_ptr<otime_t>() };

		if(folder != "")
			folderIt= m_mmmmDefaultValues.find(folder);
		if(	folder == ""
			||
			folderIt == m_mmmmDefaultValues.end()	)
		{
			bAll= true;
			folderIt= m_mmmmDefaultValues.find("###all");
			if(folderIt == m_mmmmDefaultValues.end())
				return nullDef;
		}
		if(	bAll == false
			&&
			subroutine != ""	)
		{
			subIt= folderIt->second.find(subroutine);
		}
		if(	bAll == true
			||
			subroutine == ""
			||
			subIt == folderIt->second.end()	)
		{
			subIt= folderIt->second.find("###all");
			if(subIt == folderIt->second.end())
				return nullDef;
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
					return nullDef;
			}
		}else
		{
			floatIt= lowRange->second.find(true);
			if(floatIt == lowRange->second.end())
				return nullDef;
		}
		return floatIt->second;
	}


	SHAREDPTR::shared_ptr<DefaultChipConfigReader::otime_t> DefaultChipConfigReader::getNewDefaultChipOlder(const string& folder, const string& subroutine, const double min, const double max, const bool bFloat) const
	{
		defValues_t def;

		def= getDefaultValues(min, max, bFloat, folder, subroutine);
		if(!def.older) // toDo: check whether can define instnace of shared_ptr as boolean for content
			return SHAREDPTR::shared_ptr<otime_t>();
		return copyOlder(def.older);
	}



	DefaultChipConfigReader::write_t DefaultChipConfigReader::allowDbWriting(const string& folder, const string& subroutine, const double value, const time_t acttime, bool* newOlder/*=NULL*/)
	{
		write_t tRv;
		SHAREDPTR::shared_ptr<otime_t> parento;
		SHAREDPTR::shared_ptr<otime_t> wr;
		chips_t* chip;
		time_t thistime;
		map<string, chips_t*>::const_iterator subIt;
		map<string, map<string, chips_t*> >::const_iterator folderIt;

		tRv.folder= folder;
		tRv.subroutine= subroutine;
		tRv.highest.highest= 0;
		tRv.highest.hightime= 0;
		tRv.highest.lowest= 0;
		tRv.highest.lowtime= 0;
		if(newOlder)
			*newOlder= false;
		tRv.action= "write";
		folderIt= m_mmAllChips.find(folder);
		if(folderIt == m_mmAllChips.end())
		{ // found no specified folder with default chip, so allow writing
			return tRv;
		}
		subIt= folderIt->second.find(subroutine);
		if(subIt == folderIt->second.end())
		{ // found no specified subroutine with default chip, so allow writing
			return tRv;
		}
		chip= subIt->second;
		if(chip->older == NULL)
		{ // no action is defined for default chip, so allow writing
			return tRv;
		}
		time(&thistime);
		wr= chip->older;
		if(newOlder)
		{	// question is for thin database, so don't take the first older,
			// because all other be for thin,
			// and calculate witch older structure to be used
			// to thin
			wr= wr->older;
			parento= wr;
			while(wr->older.get() != NULL)
			{
				if(Calendar::calcDate(/*newer*/false, thistime, wr->more, wr->unit) < acttime)
					break;
				parento= wr;
				wr= wr->older;
			}
			wr= parento;
			if(	wr->active == false
				&&
				wr->older != NULL
				&&
				wr->older->active == true	)
			{
				*newOlder= true;
			}
		}
		wr->active= true;
		if(wr->dbwrite == "all")
			return tRv;
		if(wr->dbwrite == "fractions") {
			double half;
			string direction;

			tRv.action= "fractions";
			if(!wr->fraction->bValue) {
				wr->fraction->bValue= true;
				wr->fraction->writtenvalue= value;
				wr->fraction->deepvalue= value;
				wr->fraction->deeptime= acttime;
				if(wr->fraction->dbafter)
					wr->fraction->write= acttime + wr->fraction->dbafter;
				wr->fraction->direction= "";
				// write highest.highest and highest.hightime
				tRv.highest.highest= value;
				tRv.highest.hightime= acttime;
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
					tRv.highest.highest= wr->fraction->deepvalue;
					tRv.highest.hightime= wr->fraction->deeptime;
					wr->fraction->deepvalue= value;
					wr->fraction->deeptime= acttime;
					return tRv;
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
					tRv.highest.highest= wr->fraction->deepvalue;
					tRv.highest.hightime= wr->fraction->deeptime;
					wr->fraction->deepvalue= value;
					wr->fraction->deeptime= acttime;
					return tRv;
				}else if(direction == "")
				{
					wr->fraction->deepvalue= value;
					wr->fraction->deeptime= acttime;
				}
			}
			if(	wr->fraction->dbafter
				&&
				acttime >= wr->fraction->write	) {

				wr->fraction->write= acttime + wr->fraction->dbafter;
				return tRv;
			}
			tRv.action= "no";
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
				if(	!wr->highest->bValue
					||
					wr->highest->nextwrite < acttime	)
				{
					if(wr->highest->bValue)
					{
						tRv.action= "highest";
						tRv.highest.highest= wr->highest->highest;
						tRv.highest.hightime= wr->highest->hightime;
						tRv.highest.lowest= wr->highest->lowest;
						tRv.highest.lowtime= wr->highest->lowtime;
					}else
						tRv.action= "no";
					wr->highest->nextwrite= Calendar::calcDate(/*newer*/true, acttime, wr->highest->between, wr->highest->t);
					wr->highest->bValue= true;
					wr->highest->highest= value;
					wr->highest->hightime= acttime;
					wr->highest->lowest= value;
					wr->highest->lowtime= acttime;
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
			return tRv;
		}
		// wr->dbwrite have to be action 'kill'
		tRv.action= "no";
		return tRv;
	}

	DefaultChipConfigReader::write_t DefaultChipConfigReader::getLastValues(const unsigned int pos, const bool bolder/*=false*/)
	{
		unsigned int current= 0;
		write_t tRv;
		SHAREDPTR::shared_ptr<otime_t> older;
		map<string, chips_t*>::const_iterator subIt;
		map<string, map<string, chips_t*> >::const_iterator folderIt;

		tRv.action= "kill"; // action kill is defined for reached ending
		tRv.highest.highest= 0;
		tRv.highest.hightime= 0;
		tRv.highest.lowest= 0;
		tRv.highest.lowtime= 0;
	 	for(folderIt= m_mmAllChips.begin(); folderIt != m_mmAllChips.end(); ++folderIt)
		{
			tRv.folder= folderIt->first;
			for(subIt= folderIt->second.begin(); subIt != folderIt->second.end(); ++subIt)
			{
				tRv.subroutine= subIt->first;
				older= subIt->second->older;
				if(bolder)
					older= older->older;
				while(older)
				{
					if(	older->active == true
						&&
						(	older->dbwrite == "highest"
							||
							older->dbwrite == "fractions"	)	)
					{
						if(current == pos)
						{
							if(older->dbwrite == "highest")
							{
								tRv.action= "highest";
								tRv.highest.highest= older->highest->highest;
								tRv.highest.hightime= older->highest->hightime;
								tRv.highest.lowest= older->highest->lowest;
								tRv.highest.lowtime= older->highest->lowtime;
								older->highest->bValue= false;
							}else // fractions
							{
								tRv.action= "fractions";
								tRv.highest.highest= older->fraction->deepvalue;
								tRv.highest.hightime= older->fraction->deeptime;
								older->fraction->bValue= false;
							}
							return tRv;
						}
						++current;
					} // if(current == pos)
					if(!bolder)
						break;
					older= older->older;
				}
			}
		}
		return tRv;
	}

	DefaultChipConfigReader::~DefaultChipConfigReader()
	{
	/*	typedef map<string, map<string, map<string, map<string, map<string, chips_t> > > > >::iterator servIt;
		typedef map<string, map<string, map<string, map<string, chips_t> > > >::iterator famIt;
		typedef map<string, map<string, map<string, chips_t> > >::iterator typIt;
		typedef map<string, map<string, chips_t> >::iterator idIt;
		typedef map<string, chips_t>::iterator pinIt;

		otime_t *next;
		otime_t *del;

		for(servIt s= m_mmmmmChips.begin(); s != m_mmmmmChips.end(); ++s)
		{
			for(famIt f= s->second.begin(); f != s->second.end(); ++f)
			{
				for(typIt t= f->second.begin(); t != f->second.end(); ++t)
				{
					for(idIt i= t->second.begin(); i != t->second.end(); ++i)
					{
						for(pinIt p= i->second.begin(); p != i->second.end(); ++p)
						{
							del= p->second.older;
							while(del)
							{
								next= del->older;
								if(del->highest)
									delete del->highest;
								if(del->fraction)
									delete del->fraction;
								delete del;
								del= next;
							}
						}
					}
				}
			}
		}
		for(idIt i= m_mmUsedChips.begin(); i != m_mmUsedChips.end(); ++i)
		{
			for(pinIt p= i->second.begin(); p != i->second.end(); ++p)
			{
				del= p->second.older;
				while(del)
				{
					next= del->older;
					if(del->highest)
						delete del->highest;
					if(del->fraction)
						delete del->fraction;
					delete del;
					del= next;
				}
			}
		}*/
	}
}
