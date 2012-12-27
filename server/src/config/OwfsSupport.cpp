/**
 *   This file 'OwfsSupport.cpp' is part of ppi-server.
 *   Created on: 13.11.2011
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

#ifdef _OWFSLIBRARY

#include <stdlib.h>

#include <iostream>
#include <fstream>
#include <vector>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "../pattern/util/LogHolderPattern.h"

#include "../util/properties/properties.h"

#include "../portserver/libs/chipaccess/maxim/maximchipaccess.h"

#include "../util/GlobalStaticMethods.h"

#include "OwfsSupport.h"

using namespace std;
using namespace util;
using namespace ports;
using namespace boost;

int OwfsSupport::execute(const ICommandStructPattern* params, IInterlacedPropertyPattern* oServerProperties)
{
	bool bFound(false);
	bool bRead(true), bReadOnly(false);
	bool bChanged(false);
	size_t nProcess;
	string examplefile("maxim_examples.conf");
	vector<string> result;
	string currentPath, newPath, snum;
	const string owfsIdentif("OWFS");
	size_t num;
	unsigned short nCount(0);
	unsigned short nback;
	/**
	 * all readed chips or parameter with number
	 */
	map<unsigned short, string> mMenu;
	Properties oExample;
	IChipAccessPattern* chip;
	const IPropertyPattern* const properties(oServerProperties->getSection("owreader", owfsIdentif));
	bool bUsedIds(false), bUsedPins(false), bReadProps(false);
	string prop, sDescription, sName, sId, sPin, sIdPath, sAction;
	vector<string>::size_type actDesc(0), actNames(0), actIds(0), actPins(0), actAction(0);
	defs_t defs;
	map<string, defs_t>::iterator found;
	map<string, string>::iterator foundChip;
	ofstream ofile;
	ifstream ifile;


	if(params->hasOption("file"))
		examplefile= params->getOptionContent("file");
	if(properties == NULL)
	{
		cout << endl;
		cout << "### ERROR: do not found any OWFS owreader property inside server.conf";
		return EXIT_FAILURE;
	}
	ifile.open(examplefile.c_str(), ios::in);
	if(ifile.is_open())
	{
		bRead= true;
		ifile.close();
	}else
		bRead= false;
	ofile.open(examplefile.c_str(), ios::app);
	if(!ofile.is_open())
	{
		bReadOnly= true;
		if(!bRead)
		{
			bRead= false;
			cout << "### WARNING: no access for writing or reading '" << examplefile << "'" << endl;
			cout << "             you can only use the application to see which chips are exist" << endl;
			cout << "             and do not see any pre-defined properties before," << endl;
			cout << "             or create one." << endl;

		}else
		{
			bRead= true;
			cout << "### WARNING: no access for writing inside '" << examplefile << "'" << endl;
			cout << "             you can only read the actually properties!" << endl;
		}
		cout << endl;
		sleep(2);
	}else
	{
		if(!bRead)
			writeHead(ofile, examplefile);
		ofile.close();
		bRead= true;
		bReadOnly= false;
	}
	if(bRead)
	{
		//file.close();
		oExample.setUncomment("#!");
		oExample.readFile(examplefile);

		//cout << "working directory: " << m_sWorkDir << endl;
		while((prop= oExample.nextProp()) != "")
		{
			bReadProps= true;
			if(prop == "description")
			{
				sDescription= oExample.getValue(prop, actDesc);
				++actDesc;

			}else if(prop == "name")
			{
				if(sName != "")
				{
					if(	bUsedIds == false ||
						bUsedPins == false	)
					{
						cout << "### WARNING: do not found any ";
						if(bUsedIds == false)
						{
							cout << "ID ";
							if(bUsedPins == false)
								cout << "or ";
						}
						if(bUsedPins == false)
							cout << "pin ";
						cout << " for subroutine '" << sName <<  "'" << endl;
						cout << "             maybe you have changed anything inside example file?" << endl;
						cout << "             this can make problems by showing definitions." << endl;
					}
					bUsedIds= false;
					bUsedPins= false;
					found= m_mSubroutines.find(sName);
					if(found != m_mSubroutines.end())
					{
						cout << "### WARNING: found more subroutine with name '" << sName <<  "'" << endl;
						cout << "             use only the last for showing definitions." << endl;
					}
					foundChip= m_mConfigured.find(sIdPath);
					if(foundChip != m_mConfigured.end())
					{
						cout << "### WARNING: found more chip with path '" << sIdPath <<  "'" << endl;
						cout << "             use only the last for showing definitions." << endl;
					}
					defs.name= sName;
					defs.desc= sDescription;
					defs.ID= sId;
					defs.pin= sPin;
					defs.action= sAction;
					m_mSubroutines[sName]= defs;
					if(sDescription == "")
						sDescription= "<configured>";
					m_mConfigured[sIdPath]= sDescription;
					sDescription= "";
					sId= "";
					sPin= "";
					sIdPath= "";
					sAction= "";
				}
				sName= oExample.getValue(prop,actNames);
				++actNames;

			}else if(prop == "ID")
			{
				sId= oExample.getValue(prop, actIds);
				++actIds;
				if(sIdPath != "")
					sIdPath= sId + "/" + sIdPath;
				else
					sIdPath= sId;
				bUsedIds= true;

			}else if(prop == "pin")
			{
				sPin= oExample.getValue(prop, actPins);
				++actPins;
				if(sIdPath != "")
					sIdPath+= "/";
				sIdPath+= sPin;
				bUsedPins= true;

			}else if(prop == "action")
			{
				sAction= oExample.getValue(prop, actAction);
				++actAction;
			}
		}
		if(	bReadProps &&
			(	bUsedIds == false ||
				bUsedPins == false	)	)
		{
			cout << "### WARNING: do not found any ";
			if(bUsedIds == false)
			{
				cout << "ID ";
				if(bUsedPins == false)
					cout << "or ";
			}
			if(bUsedPins == false)
				cout << "pin ";
			cout << " for subroutine '" << sName <<  "'" << endl;
			cout << "             maybe you have changed anything inside example file?" << endl;
			cout << "             this can make problems by showing definitions." << endl;
		}
		found= m_mSubroutines.find(sName);
		if(found != m_mSubroutines.end())
		{
			cout << "### WARNING: found more subroutine with name '" << sName <<  "'" << endl;
			cout << "             use only the last for showing definitions." << endl;
		}
		foundChip= m_mConfigured.find(sIdPath);
		if(foundChip != m_mConfigured.end())
		{
			cout << "### WARNING: found more chip with path '" << sIdPath <<  "'" << endl;
			cout << "             use only the last for showing definitions." << endl;
		}
		defs.name= sName;
		defs.desc= sDescription;
		defs.ID= sId;
		defs.pin= sPin;
		defs.action= sAction;
		m_mSubroutines[sName]= defs;
		m_mConfigured[sIdPath]= sDescription;
		//for(map<string, string>::iterator it= m_mConfigured.begin(); it != m_mConfigured.end(); ++it)
		//	cout << it->first << " " << it->second << endl;
		//*******************************************************************************************
	}

	do{
		m_nChip= 0;
		nCount= 0;
		currentPath= newPath;
		if(currentPath == "")
		{// list first from all process
			mMenu.clear();
			OWFSFactory(owfsIdentif, "needprocesses", 0, nProcess, properties);
			m_nChip= 0;
			nCount= 0;
			m_nFirstChip.clear();
			for(size_t n= 1; n <= nProcess; ++n)
			{
				chip= OWFSFactory(owfsIdentif, "chipobject", n, n, properties);
				if(chip->connect())
				{
					if(n == 1)
					{
						cout << endl << endl;
						cout << "       found follow content to configure   " << endl;
						cout << "    ***************************************" << endl;
						cout << endl;
					}
					m_nFirstChip.push_back(m_nChip);
					if(!show(chip, currentPath, bFound, result))
					{
						for(vector<string>::iterator it= result.begin(); it != result.end(); ++it)
						{
							mMenu[nCount]= *it;
							++nCount;
						}
					}
					chip->disconnect();
					cout << endl;
				}
			}
		}else
		{// defined process is given
			mMenu.clear();
			chip= OWFSFactory(owfsIdentif, "chipobject", nProcess, nProcess, properties);
			if(chip->connect())
			{
				cout << endl << endl;
				cout << "       found follow content to configure   " << endl;
				cout << "    ***************************************" << endl;
				cout << "       inside " << currentPath << endl;
				cout << endl;

				if(!show(chip, currentPath, bFound, result))
				{
					for(vector<string>::iterator it= result.begin(); it != result.end(); ++it)
					{
						mMenu[nCount]= *it;
						++nCount;
					}
				}
				chip->disconnect();
				cout << endl;
			}
		}
		if(bFound)
		{
			cout << "        ( ";
			cout.width(3);
			cout << nCount << " )   ";
			if(currentPath != "")
			{
				cout << "BACK      ";
				cout << "go back to definition list before" << endl;
				nback= nCount;
			}else
			{
				cout << "AGAIN     ";
				cout << "search again in root path for new chip's" << endl;
			}
			++nCount;
			cout << "        ( ";
			cout.width(3);
			cout << nCount << " )   ";
			cout << "EXIT      ";
			cout << "ending ppi-mconfig for example definition" << endl;
			cout << endl;

			do{ // reading number for next action
				ostringstream onum;

				cout << "  define number of chip to configure: " << flush;
				getline(std::cin, snum);
				trim(snum);
				istringstream inum(snum);
				inum >> num;
				onum << num;
				if(	snum != onum.str() ||
					num > mMenu.size()+1	)
				{
					cerr << "###ERROR: cannot define entry ('" << snum << "') as correct number" << endl;
					//beep();
					cout << endl;
					snum= "";
				}
			}while(snum == "");

		}else
		{
			if(currentPath == "")
			{
				uid_t uid(getuid());

				if(uid != 1)
				{
					cout << endl;
					cout << "    If you have'nt define in udev or hotplug the USB- or COM-adapter for current user," << endl;
					cout << "    please start ppi-mconfig for OWFS as root or as defined user" << endl;
					cout << endl;
				}else
				{
					cout << endl;
					cout << "    ### ERROR: undefined error occurring by reading maxim/dallas file system" << endl;
					cout << endl;
				}
				return EXIT_FAILURE;
			}else
			{
				cout << endl;
				cout << "    ### ERROR: undefined error occurring by reading '" << currentPath << "'" << endl;
				cout << endl;
				num= (size_t)nback;

			}
		}

		/****************************************************
		 * define next step
		 */
		if(num < mMenu.size())
		{
			if(currentPath == "")
			{// search for needed maximinit process
				nProcess= 0;
				for(vector<unsigned short>::iterator it= m_nFirstChip.begin(); it != m_nFirstChip.end(); ++it)
				{
					if(num < *it)
						break;
					++nProcess;
				}
			}
			newPath= mMenu[(unsigned short)num];
			if(newPath.substr(newPath.length()-1) != "/")
			{
				if(bReadOnly)
				{
					cout << endl;
					cout << "  cannot write in example file!" << endl;
					cout << "  file or current directory is read only" << endl;
					sleep(1);

				}else
				{
					if(writeExample(examplefile, currentPath + newPath, chip))
						bChanged= true;
				}
				newPath= currentPath;
			}else
				newPath= currentPath + newPath;

		}else if(	currentPath != "" &&
					num == mMenu.size()	)
		{
			vector<string> spl;

			nCount= 0;
			newPath= currentPath;
			if(newPath.substr(newPath.length()-1) == "/")
				newPath= newPath.substr(0, newPath.length()-1);
			split(spl, newPath, is_any_of("/"));
			newPath= "";
			for(size_t n= 0; n<spl.size()-1; ++n)
				newPath+= spl[n] + "/";
		}

	}while(num != mMenu.size()+1);
	if(bChanged)
	{
		cout << endl << endl << endl;
		cout << "  now you can use the new generated subroutine blocks from example file '" << examplefile << "'"<< endl;
		cout << "  to copy inside your own measure.conf file" << endl;
		cout << "  and add several new properties like 'while', 'perm' or other one." << endl;
		cout << endl;
		cout << "  Please do not bind direct this example file inside measure.conf" << endl;
		cout << "  or make any own changes in this file (necessary are the sequential arrangement of properties)" << endl;
		cout << "  Because when you do so, all own comments or attached new properties" << endl;
		cout << "  will be lost by reconfigure or delete any chips with the ppi-mconfig tool" << endl;
	}
	cout << endl;
	return EXIT_SUCCESS;
}

int OwfsSupport::show(IChipAccessPattern* chip, string path, bool& found, vector<string>& result)
{
	bool more;
	string spaces;
	string chipType;
	string searchPath;
	vector<string> foundConf;
	string::size_type outSpaceFirst(8);
	string::size_type outNumber(7);
	string::size_type outSpaceBetween(2);
	string::size_type outLengthMenu; // variable
	// again outSpaceBetween
	string::size_type outChipLength(15);// variable have to be always shorter than outContentFull
	bool outNeedContent;
	string::size_type outContentLength(10);
	string::size_type outContentFull(outContentLength + 10); // variable have to be always longer than outChipLength

	result.clear();
	if(chip->command_exec(path, result, more) == 0)
	{
		outNeedContent= false;
		outLengthMenu= 0;
		for(vector<string>::iterator it= result.begin(); it != result.end(); ++it)
		{
			if(it->length() > outLengthMenu)
				outLengthMenu= it->length();
			if(it->substr(it->length()-1) != "/")
				outNeedContent= true;
		}
		for(vector<string>::iterator it= result.begin(); it != result.end(); ++it)
		{
			ostringstream content;

			found= true;
			spaces= "";
			spaces.append(outSpaceFirst, ' ');
			content << spaces;
			content << "( ";
			content.width(3);
			content << m_nChip << " )";
			spaces= "";
			spaces.append(outSpaceBetween, ' ');
			content << spaces;
			content << *it;
			spaces= "";
			spaces.append(outLengthMenu - it->length(), ' ');
			content << spaces;
			spaces= "";
			spaces.append(outSpaceBetween, ' ');
			content << spaces;
			searchPath= path + *it;
			chipType= "";
			if(	path == "" &&
				it->substr(it->length()-1) == "/"	)
			{// write chip type if exist
				chipType= chip->getChipType(*it);
				if(chipType != "")
				{
					content << "chip " << chipType;
					spaces= "";
					spaces.append(outChipLength - 5 - chipType.length(), ' ');
					content << spaces;
				}else
				{
					spaces= "";
					spaces.append(outChipLength, ' ');
					content << spaces;
				}
				if(outNeedContent)
				{
					spaces= "";
					spaces.append(outContentFull - outChipLength, ' ');
					content << spaces;
				}

			}else if(it->substr(it->length()-1) != "/")
			{// write content of pin
				string command("--read ");
				vector<string> scontent;

				if(	!chip->command_exec("--read " + searchPath, scontent, more) &&
					scontent.size() > 0 &&
					scontent[0].length() > 0										)
				{
					//cout << searchPath << " " << scontent[0] << endl;
					content << "content:'";
					trim(scontent[0]);
					if(scontent[0].length() > outContentLength)
					{
						content << scontent[0].substr(0, outContentLength) << " ...'";
					}else
					{
						content << scontent[0];
						content << "'";
						spaces= "";
						spaces.append(outContentFull - scontent[0].length(), ' ');
						content << spaces;
					}
				}else
				{
					spaces= "";
					spaces.append(outContentFull, ' ');
					content << spaces;
				}
			}else
			{
				if(outNeedContent)
				{
					spaces= "";
					spaces.append(outContentFull, ' ');
					content << spaces;

				}else if(path == "")
				{// if no content needed, but it's the root file path
				 // write spaces for an chip
					spaces= "";
					spaces.append(outChipLength, ' ');
					content << spaces;
				}
			}
			spaces= "";
			spaces.append(outSpaceBetween, ' ');
			content << spaces;
			foundConf.clear();
			searchPath= path + *it;
			for(map<string, string>::iterator it= m_mConfigured.begin(); it != m_mConfigured.end(); ++it)
			{
				if(	it->first.length() >= searchPath.length() &&
					it->first.substr(0, searchPath.length()) == searchPath	)
				{
					foundConf.push_back(it->second);
				}
			}
			if(foundConf.size() > 0)
			{
				vector<string>::size_type len;

				content << "(";
				len= foundConf.size();
				if(len == 1)
					content << foundConf[0] << ")";
				else
				{
					for(size_t n= 0; n < len; ++n)
					{
						if(n > 0)
						{
							string::size_type full;

							full=	outSpaceFirst+
									outNumber+
									outSpaceBetween+
									outLengthMenu+
									outSpaceBetween;
							if(outNeedContent)
								full+= outContentFull;
							else if(path == "")
								full+= outChipLength;
							full+= outSpaceBetween + 1;
							spaces= "";
							spaces.append(	full, ' ');
							content << spaces;
						}
						content << foundConf[n];
						if(n != len-1)
							content << endl;
					}
					content << "   )";
				}
			}else
			{// menu path is not defined in example
			 // so maybe chip is from type DS1420
				if(chipType == "DS1420")
					content << "(chip is also used for USB- and some COM-adapters)";
			}
			content << endl;
			cout << content.str();
			++m_nChip;
		}
	}else
		return -1;
	return 0;
}

bool OwfsSupport::writeExample(const string& exampleFile, string chipPath, IChipAccessPattern* chip)
{
	bool bWriteable(false), bReconfigure(false), more;
	ofstream file;
	string id, pin;
	short num;
	string snum;
	vector<string> spl;
	string subroutine;
	string description;
	string action;
	defs_t defs;
	map<string, string>::iterator found;
	map<string, defs_t>::iterator foundSub;
	map<string, defs_t>::iterator reconfigSub;

	split(spl, chipPath, is_any_of("/"));
	if(spl.size() == 1)
	{
		cerr << "### ERROR:    cannot create example with only ID:'" << exampleFile << "'" << endl;
		return false;
	}
	id= spl[0];
	for(size_t n= 1; n<spl.size(); ++n)
	{
		//cout << "pin: " << spl[n] << endl;
		pin+= spl[n] + "/";
	}
	pin= pin.substr(0, pin.length()-1);

	found= m_mConfigured.find(chipPath);
	if(found != m_mConfigured.end())
	{
		bReconfigure= true;
		cout << endl << endl;
		cout << "  pin of chip is configured in example file" << endl;
		cout << "  do you want to reconfigure?" << endl;
		cout << "  or only delete this definition?" << endl;
		cout << endl;
		cout << "  ( 0 )   re-configure" << endl;
		cout << "  ( 1 )   delete" << endl;
		cout << "  ( 2 )   abort" << endl;
		cout << endl;
		do{ // reading number for next action
			ostringstream onum;

			cout << "  >> " << flush;
			getline(std::cin, snum);
			trim(snum);
			istringstream inum(snum);
			inum >> num;
			onum << num;
			if(	snum != onum.str() ||
				num > 2 ||
				num < 0					)
			{
				cerr << "###ERROR: cannot define entry ('" << snum << "') as correct number" << endl;
				//beep();
				cout << endl;
				snum= "";
			}
		}while(snum == "");
		cout << endl;
		if(num == 2)
			return false;
		for(map<string, defs_t>::iterator it= m_mSubroutines.begin(); it != m_mSubroutines.end(); ++it)
		{
			if(	it->second.ID == id &&
				it->second.pin == pin	)
			{
				reconfigSub= it;
				break;
			}
		}
		if(num == 1)
		{
			m_mConfigured.erase(found);
			m_mSubroutines.erase(reconfigSub);
			writeHoleExample(exampleFile);
			return false;
		}
	}
	cout << endl << endl;
	cout << "  write example code for '" << chipPath << "'" << endl;
	do{
		cout << "  pleas insert an name for subroutine";
		if(bReconfigure)
		{
			cout << ": [" << reconfigSub->first << "] " << flush;

		}else // by reconfiguring question was made before
			cout << " (or '-no' for aborting): " << flush;
		getline(std::cin, subroutine);
		trim(subroutine);
		if(subroutine == "-no")
		{
			cout << "  do not configure any chip" << endl;
			sleep(2);
			return false;
		}
		if(	subroutine == "" &&
			bReconfigure == true	)
		{
			subroutine= reconfigSub->first;
		}
		if(subroutine == "")
		{
			cout << "### WARNING: please type some letters before pressing ENTER!" << endl;
			cout << endl;

		}else
		{
			if(	subroutine.find(" ") != string::npos ||
				subroutine.find("\t") != string::npos	)
			{
				cout << "### WARNING: please use only an string with no spaces inside" << endl;
				cout << endl;
				subroutine= "";

			}else if(	bReconfigure == false ||
						subroutine != reconfigSub->first	)
			{
				foundSub= m_mSubroutines.find(subroutine);
				if(foundSub != m_mSubroutines.end())
				{
					cout << "### WARNING: this subroutine name is already defined inside example file," << endl;
					cout << "             please try an other one!" << endl;
					cout << endl;
					subroutine= "";
				}
			}
		}

	}while(	subroutine == "" ||
			glob::replaceName(subroutine, "subroutine", /*change*/false)	);

	cout << "  write now an short description for the chip:" << flush;
	if(bReconfigure)
		cout << "[" << reconfigSub->second.desc << "] " << flush;
	getline(std::cin, description);
	trim(description);
	if(	bReconfigure &&
		description == ""	)
	{
		description= reconfigSub->second.desc;
	}
	if(description == "")
		description= "<configured>";

	if(bReconfigure)
	{
		if(	subroutine == reconfigSub->first &&
			description == reconfigSub->second.desc	)
		{
			cout << "  no changes be made" << endl;
			cout << "  do not configure any chip" << endl;
			sleep(2);
			return false;
		}
		if(subroutine != reconfigSub->first)
		{
			defs.name= subroutine;
			defs.desc= description;
			defs.ID= id;
			defs.pin= pin;
			defs.action= reconfigSub->second.action;
			m_mSubroutines.erase(reconfigSub);
			m_mSubroutines[subroutine]= defs;

		}else
			reconfigSub->second.desc= description;
		return writeHoleExample(exampleFile);
	}
	file.open(exampleFile.c_str(), ios::app);
	if(!file.is_open())
	{
		cerr << "### ERROR:    cannot append any content on example file '" << exampleFile << "'" << endl;
		sleep(2);
		return false;
	}

	if(!chip->command_exec("--write " + chipPath, spl, more))
		bWriteable= true;
	cout << endl << endl;
	defs.name= subroutine;
	defs.desc= description;
	defs.ID= id;
	defs.pin= pin;
	writeSubroutine(file, defs, bWriteable);
	file << flush;
	file.close();
	m_mSubroutines[subroutine]= defs;
	m_mConfigured[chipPath]= description;
	return true;
}

void OwfsSupport::writeSubroutine(ofstream& file, defs_t& defs, const bool bWriteable)
{
	file << "name= " << defs.name << endl;
	file << "#! description= " << defs.desc << endl;
	file << "type= OWFS" << endl;
	file << "ID= " << defs.ID << endl;
	file << "pin= " << defs.pin << endl;
	if(bWriteable)
	{
		file << "# pin of chip is for writing and reading" << endl;
		file << "# please remove one of the action" << endl;
		file << "# because they are not both usable in the same time" << endl;
		defs.action= "read | write";

	}else
	{
		file << "# pin of chip is only for reading" << endl;
		defs.action= "read";
	}
	file << "action= " << defs.action << endl;
	file << endl;
}

bool OwfsSupport::writeHoleExample(const string& exampleFile)
{
	bool bWriteable;
	ofstream file;

	file.open(exampleFile.c_str(), ios::out);
	if(!file.is_open())
	{
		cerr << "### ERROR:    cannot open example file '" << exampleFile << "' for writing" << endl;
		sleep(2);
		return false;
	}
	LOG(LOG_DEBUG, "write hole example file new");

	writeHead(file, exampleFile);
	for(map<string, defs_t>::iterator it= m_mSubroutines.begin(); it != m_mSubroutines.end(); ++it)
	{
		if(it->second.action.find("write") != string::npos)
			bWriteable= true;
		else
			bWriteable= false;
		writeSubroutine(file, it->second, bWriteable);
	}
	file << flush;
	file.close();
	return true;
}

void OwfsSupport::writeHead(ofstream& file, const string& exampleFile)
{
	file << "####################################################################################################" << endl;
	file << "#" << endl;
	file << "#      ppi-server configuration file of measure routines for OWFS maxim/dallas semiconductors" << endl;
	file << "#      generated from ppi-mconfig tool" << endl;
	file << "#" << endl;
	file << "#" << endl;
	file << "#  this configuration file " << exampleFile << " has same structure of folder and subroutines" << endl;
	file << "#  like measure.conf and can be used to copy as example inside some measure.conf files" << endl;
	file << "#" << endl;
	file << "#  please do not bind this file inside measure.conf" << endl;
	file << "#  or make any own changes in this file (necessary are the sequential arrangement of properties)" << endl;
	file << "#  Because when you do so, all own comments or attached new properties" << endl;
	file << "#  will be lost by reconfigure or delete any chips with the ppi-mconfig tool" << endl;
	file << "#" << endl;
	file << "#" << endl;
	file << "####################################################################################################" << endl;
	file << endl;
	file << endl;
	file << endl;
	file << endl;
	file << endl;
}

#endif /* _OWFSLIBRARY */






