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

#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

//#include "../util/debug.h"
#include "../ports/subroutines/Switch.h"

#include "../util/GlobalStaticMethods.h"

#include "../util/properties/interlacedproperties.h"

#include "../ports/subroutines/Folder.h"

#include "LircSupport.h"

using namespace std;
using namespace util;
using namespace subroutines;
using namespace boost;

int LircSupport::execute(const ICommandStructPattern* params)
{
	bool receive(params->hasOption("receiver"));
	bool transmit(params->hasOption("transmitter"));
	bool learn(params->hasOption("learn"));
	int vertical= 3;
	string param, opname, remote;
	string lircconf("/etc/lirc/lircd.conf");
	string userreadperm("read"), userwriteperm("change");
	string readperm("confread"), writeperm("confchange");
	remotecodes_t r;

	if(params->hasOption("file"))
		lircconf= params->getOptionContent("file");
	if(params->hasOption("readperm"))
		userreadperm= params->getOptionContent("readperm");
	if(params->hasOption("changeperm"))
		userwriteperm= params->getOptionContent("changeperm");
	if(params->hasOption("configreadperm"))
		readperm= params->getOptionContent("configreadperm");
	if(params->hasOption("configchangeperm"))
		writeperm= params->getOptionContent("configchangeperm");
	if(params->hasOption("remote"))
		remote= params->getOptionContent("remote");
	if(params->hasOption("vertical"))
	{
		opname= "vertical";
		vertical= params->getOptionIntContent(opname);
		if(	opname == "##ERROR" ||
			vertical < 1			)
		{
			cerr << "wrong content '" << params->getOptionContent("vertical") << "' for option vertical be set" << endl;
			cerr << "type -? for help" << endl;
			return EXIT_FAILURE;
		}
	}

	if(	receive &&
		transmit	)
	{
		cout << "do not set option for transmitting and receiving" << endl;
		cout << "no correct command be set" << endl;
		cout << "type -? for help" << endl;
		return EXIT_FAILURE;
	}
	if(	receive &&
		learn		)
	{
		cout << "option -l or --learn is only for transmitting" << endl;
		cout << "no correct command be set" << endl;
		cout << "type -? for help" << endl;
		return EXIT_FAILURE;
	}
	if(learn)
		transmit= true;
	if(!transmit)
		receive= true;

	cout << endl << "### read all configured remote controls from ";
	if(params->hasOption("file"))
		cout << "pre-defined ";
	else
		cout << "standard ";
	cout << "lircd: '" << lircconf << "'" << endl;
	r= readLircd(lircconf);
	if(!r.success)
		return EXIT_FAILURE;
	if(!learn)
	{
		if(createConfigLayoutFiles(transmit, vertical, userreadperm, userwriteperm, readperm, writeperm, r, remote) == false)
			return EXIT_FAILURE;
	}// if(!learn)
	return EXIT_SUCCESS;
}

LircSupport::remotecodes_t LircSupport::readLircd(const string& lircd) const
{
	// read all permission groups for folder:subroutines in file measure.conf
	typedef vector<IInterlacedPropertyPattern*>::iterator sit;

	unsigned short count;
	unsigned long line(0);
	string act_code;
	string name, newname, code; //, newcode;
	string org_name, org_code, code_name;
	remotecodes_t r;
	InterlacedProperties oLirc;
	vector<string>::size_type uncommentedlink, uncommentedname;
	vector<IInterlacedPropertyPattern*> rsection, csection;
	map<string, vector<string> >::iterator foundr;
	vector<string>::iterator foundc;

	r.success= false;
	oLirc.setComment("#");
	//oLirc.setDoc("/*", "*/");
	oLirc.setUncomment("#!");
	oLirc.setDelimiter(" ");
	oLirc.modifier("begin", "remote");
	oLirc.modifier("begin", "codes");
	oLirc.allowLaterModifier(true);
	if(!oLirc.readFile(lircd))
	{
		cerr << endl;
		cerr << "###ERROR: cannot read file '" << lircd << "' to configurate receiver" << endl;
		r.success= false;
		return r;
	}
	rsection= oLirc.getSections();

	for(sit vfit= rsection.begin(); vfit != rsection.end(); ++vfit)
	{
		map<string, string> codealias;
		map<string, string> displaynames;

		csection= (*vfit)->getSections();
		org_name= (*vfit)->getValue("name");
		name= org_name;
		glob::replaceName(name);
		newname= name;
		foundr= r.remotes.find(newname);
		count= 0;
		while(foundr != r.remotes.end())
		{
			ostringstream oname;

			++count;
			oname << name << "[" << dec << count << "]";
			newname= oname.str();
			foundr= r.remotes.find(newname);
		}
		r.remotealias[newname]= org_name;
		name= newname;
		//cout << "read remote '" << name << "'" << endl;
		line= 0;
		for(sit vsit= csection.begin(); vsit != csection.end(); ++vsit)
		{
			uncommentedlink= 0;
			uncommentedname= 0;
			while((code= (*vsit)->nextProp()) != "")
			{
				bool readProp= true;

				if(code == "end")
				{
					string value;

					value= (*vsit)->getValue("begin", /*warning*/false);
					if(	value == "codes" ||
						value == "remote"	)
					{
						readProp= false;
					}
				}
				if(readProp)
				{
					if((*vsit)->wasCommented(code) != "")
					{
						if(code == "name")
						{
							code_name= (*vsit)->getValue(code, uncommentedname, /*warning*/false);
							if(	line != 0 &&
								line == (*vsit)->getPropertyLine(code, uncommentedname)	)
							{
								displaynames[act_code]= code_name;
								line= 0;
							}else
							{
								cerr << "### WARNING: cannot find associated code for pre-defined name '" << code_name << "'" << endl;
								cerr << "             (name should be defined in same line of file than code)" << endl << endl;
							}
							++uncommentedname;

						}else if(code == "###LINK")
						{
							string rem, co;
							vector<string> sp;

							code= (*vsit)->getValue(code, uncommentedlink, /*warning*/false);
							split(sp, code , is_any_of(":"));
							++uncommentedlink;
							if(sp.size() == 2)
							{
								rem= sp[0];
								co= sp[1];
								org_code= co;
								glob::replaceName(rem);
								glob::replaceName(co);
								newname= co;
								foundc= find(r.remotes[name].begin(), r.remotes[name].end(), newname);
								count= 0;
								while(foundc != r.remotes[name].end())
								{
									ostringstream oname;

									++count;
									oname << co << "[" << dec << count << "]";
									newname= oname.str();
									foundc= find(r.remotes[name].begin(), r.remotes[name].end(), newname);
								}
								codealias[newname]= org_code;
								co= newname;
								r.kill[rem].push_back(name);
								r.remotes[name].push_back(rem +":"+ co);
								//cout << "       with code '" << co << "' linked to remote '" << rem << "'" << endl;
							}else
								cerr << "### warning: undefined uncommented code '" << code << "' found in remoute '" << name << "'" << endl;

						}else if(code == "###NULL")
						{
							r.remotes[name].push_back(code + "::");
							//cout << "       NULL FIELD" << endl;
						}
					}else
					{
						line= (*vsit)->getPropertyLine(code);
						org_code= code;
						glob::replaceName(code);
						newname= code;
						foundc= find(r.remotes[name].begin(), r.remotes[name].end(), newname);
						count= 0;
						while(foundc != r.remotes[name].end())
						{
							ostringstream oname;

							++count;
							oname << code << "[" << dec << count << "]";
							newname= oname.str();
							foundc= find(r.remotes[name].begin(), r.remotes[name].end(), newname);
						}
						codealias[newname]= org_code;
						act_code= newname;
						code= newname;
						//cout << "       with code '" << code << "'" << endl;
						r.remotes[name].push_back(code);
					}
				}// if(readProp)
			}// nextProp()
		}// iterate over codes
		r.rcodealias[name]= codealias;
		r.dcodes[name]= displaynames;
	}// iterate over remote

	r.success= true;
	return r;
}

void LircSupport::writeHeader(ofstream& file, const string& filename) const
{
	file << "##################################################################################" << endl;
	file << "#" << endl;
	file << "#      ppi-server configuration file of measure routines for lirc" << endl;
	file << "#      generated from ppi-mconfig deamon" << endl;
	file << "#" << endl;
	file << "#" << endl;
	file << "#  this configuration file " << filename << ".conf have same structure of folder and subroutines" << endl;
	file << "#  like measure.conf to use any receiver or transmitter inside ppi-server" << endl;
	file << "#  to receive or transmit signals over lircd." << endl;
	file << "#  Copy this file into the configuration sub folder (conf) of ppi-server" << endl;
	if(filename == "lirc")
		file << "#  and make there inside of measure.conf an link to this file ('file= " << filename << ".conf')" << endl;
	else
		file << "#  and also the corresponding file " << filename << ".desktop to the client directory" << endl;
	file << "#  An description to use lirc on command line or better understanding, read http://www.lirc.org" << endl;
	file << "#" << endl;
	file << "#" << endl;
	file << "#   TYPES:       properties and actions:" << endl;
	file << "#       LIRC         properties: <ID>, <pin>, [priority], [cache], <begin|while|end>" << endl;
	file << "#                    actions= <receive|send|send_once>, [db]" << endl;
	file << "#              [access to an chip with the unique address from maxim on property <id>    ]" << endl;
	file << "#" << endl;
	file << "#   PROPERTIES:" << endl;
	file << "#       ID         - name of remote group in /etc/lirc/lircd.conf after begin remote" << endl;
	file << "#       pin        - receiving/transmitting code between begin end codes" << endl;
	file << "#       priority   - priority of writing" << endl;
	file << "#                    all pins are holding in an queue to write in serie." << endl;
	file << "#                    if in the queue more than one chips/pins" << endl;
	file << "#                    higher priority's will be concerned of writing before the other" << endl;
	file << "#                    (allowed priority 1 (highest) to priority 9999 (lowest))" << endl;
	file << "#       begin      - do writing if begin status occurring" << endl;
	file << "#                    the value can be an defined-value" << endl;
	file << "#       while      - do writing while state be set" << endl;
	file << "#                    the value can be an defined-value" << endl;
	file << "#       end        - set pin to 0 if expression be correct" << endl;
	file << "#                    the value can be an defined-value" << endl;
	file << "#       perm       - permission group to read or change this subroutine" << endl;
	file << "#" << endl;
	file << "#  ACTIONS:" << endl;
	file << "#       receive    - receiving code from device" << endl;
	file << "#       send_once  - sending one signal over device" << endl;
	file << "#       send       - sending start and end signal over device" << endl;
	file << "#       db         - writing actual state on database" << endl;
	file << "#" << endl;
	file << "#" << endl;
	file << "###################################################################################" << endl;
	file << endl;
	file << endl;
	file << endl;
}

bool LircSupport::createConfigLayoutFiles(const bool transmit, const int vertical,
		const string& userreadperm, const string& userwriteperm,
		const string& readperm, const string& writeperm, const remotecodes_t& r, const string& forremote) const
{
	bool bwritten(false), ballwritten(true);
	Switch* pSwitch;
	auto_ptr<Folder> folder;
	ofstream file;
	map<string, vector<string> >::const_iterator remit;

	if(forremote == "")
	{
		file.open("lirc.conf", ios::out);
		if(!file.is_open())
		{
			cerr << "### ERROR:    cannot create lirc.conf for writing" << endl;
			return false;
		}

		// write header of file for lirc.conf
		writeHeader(file, "lirc");

		if(transmit)
		{//  write by transmitting record button
			folder= auto_ptr<Folder>(new Folder(file, "TRANSMIT_RECEIVE_main_settings"));
			pSwitch= folder->getSwitch("record");
			pSwitch->description("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
			pSwitch->description(" record all changes on remote access");
			pSwitch->action("db");
			pSwitch->pperm(writeperm);
			pSwitch->description();
			pSwitch->description();
			pSwitch->description();
		}

		for(remit= r.remotes.begin(); remit != r.remotes.end(); ++remit)
		{
			if(r.kill.find(remit->first) == r.kill.end())
			{
				cout << "###" << endl;
				cout << "###   write configuration for remote control '" << remit->first << "':" << endl;
				if(createRemoteConfFile(remit->first, r, readperm, writeperm, userreadperm, userwriteperm, transmit))
				{

					file << "#" << endl;
					file << "#  configuration file for remote control " << remit->first << endl;
					file << "# ----------------------------------------------------------------------------------------" << endl;
					file << "file= " << remit->first << ".conf" << endl;
					file << endl;
					if(createRemoteDesktopFile(remit->first, r, vertical, transmit, writeperm))
						bwritten= true;
				}else
					ballwritten= false;
			}
		}
		file.close();
	}else
	{
		map<string, vector<string> >::const_iterator found;

		found= r.kill.find(forremote);
		cout << "###" << endl;
		if(found == r.kill.end())
		{
			cout << "###   write configuration for remote control '" << forremote << "':" << endl;
			if(createRemoteConfFile(forremote, r, readperm, writeperm, userreadperm, userwriteperm, transmit))
			{
				if(r.kill.find(forremote) == r.kill.end())
				{
					if(createRemoteDesktopFile(forremote, r, vertical, transmit, writeperm))
						bwritten= true;
				}else
					ballwritten= false;
			}else
				ballwritten= false;
		}else
		{
			cerr << "### ERROR: given file '" << forremote << "' is only an linked remote from remote ";
			for(vector<string>::const_iterator it= found->second.begin(); it != found->second.end(); ++it)
				cerr << *it << " ";
			cerr << "           pleas take an other one." << endl;
			ballwritten= false;
		}
	}
	if(bwritten == false)
		return false;
	return true;
}

bool LircSupport::createRemoteDesktopFile(const string& remote, const remotecodes_t& r, const int vertical,
		const bool transmit, const string& perm) const
{
	int actrows= 0;
	ofstream file;
	string filename(remote + ".desktop");
	string code, org_code;
	string org_remote;

	map<string, string>::const_iterator foundRemouteAlias, foundCodeAlias;
	map<string, map<string, string> >::const_iterator foundCodeAliasMap, linkCodeAliasMap, displayCodeMap;
	map<string, vector<string> >::const_iterator remit;

	file.open(filename.c_str(), ios::out);
	if(!file.is_open())
	{
		cerr << "### ERROR:    cannot create " << filename << " for writing" << endl;
		return false;
	}
	cout << "                                 " << filename << endl;

	remit= r.remotes.find(remote);
	foundRemouteAlias= r.remotealias.find(remote);
	org_remote= foundRemouteAlias->second;
	foundCodeAliasMap= r.rcodealias.find(remote);
	displayCodeMap= r.dcodes.find(remote);
	filename= remote + ".desktop";
	file << "<layout>" << endl;
	file << "  <head>" << endl;
	file << "    <title name=\"" << org_remote << "\" />" << endl;
	file << "    <meta name=\"permission\" content=\"" <<perm << "\" />" << endl;
	file << "  </head>" << endl;
	file << "  <body>" << endl;
	if(transmit)
	{
		file << "    <table boder=\"1\" width=\"100\">" << endl;
		file << "      <tr>" << endl;
		file << "        <td>" << endl;
		file << "          <input type=\"togglebutton\" value=\"record\" "
					"result=\"TRANSMIT_RECEIVE_main_settings:record\" />" << endl;
		file << "        </td>" << endl;
		file << "      </tr>" << endl;
		file << "    </table><br />" << endl;
	}
	file << "    <table border=\"1\">" << endl;
	file << "      <tr>" << endl;
	file << "        <td>" << endl;
	file << "          <table border=\"0\" >" << endl;
	actrows= 0;
	for(vector<string>::const_iterator coit= remit->second.begin(); coit != remit->second.end(); ++coit)
	{
		code= *coit;
		displayCodeMap= r.dcodes.find(remote);
		foundCodeAliasMap= r.rcodealias.find(remote);
		foundCodeAlias= displayCodeMap->second.find(code);
		if(foundCodeAlias == displayCodeMap->second.end())
		{
			foundCodeAlias= foundCodeAliasMap->second.find(code);
			if(foundCodeAlias != foundCodeAliasMap->second.end())
				org_code= foundCodeAlias->second;
			else
				org_code= code;
		}else
			org_code= foundCodeAlias->second;
		if(actrows == 0)
			file << "            <tr>" << endl;
		file << "              <td>" << endl;
		//cout << "read " << remote << " " << code << endl;
		if(code != "###NULL::")
		{
			file << "                ";
			file << "<input type=\"checkbox\" result=\"";
			if(code.find(":") != string::npos)
			{// code is an linked subroutine
				vector<string> sp;

				split(sp, code , is_any_of(":"));
				displayCodeMap= r.dcodes.find(sp[0]);
				foundCodeAliasMap= r.rcodealias.find(sp[0]);
				foundCodeAlias= displayCodeMap->second.find(sp[1]);
				if(foundCodeAlias == displayCodeMap->second.end())
				{
					foundCodeAlias= foundCodeAliasMap->second.find(sp[1]);
					if(foundCodeAlias != foundCodeAliasMap->second.end())
						org_code= foundCodeAlias->second;
					else
						org_code= sp[1];
				}else
					org_code= foundCodeAlias->second;
				code= sp[1];
			}

			file << remote << "_" << code << ":receive\"";
			file << " readonly=\"readonly\" />" << endl;
			file << "                ";
			file << "<input type=\"togglebutton\"";
			file << " value=\"" << org_code << "\"";
			file << " result=\"" << remote << "__show:" << code << "\" />" << endl;

		}else//code is an NULL field
			file << "                &#160;" << endl;
		file << "              </td>" << endl;
		++actrows;
		if(actrows == vertical)
		{
			file << "            </tr>" << endl;
			actrows= 0;
		}
	}
	if(actrows != 0)
		file << "            </tr>" << endl;
	file << "          </table>" << endl;
	file << "        </td>" << endl;
	file << "      </tr>" << endl;
	file << "    </table>" << endl;
	//file << "    <table border=\"1\">" << endl;
	//file << "      <tr>" << endl;
	//file << "        <td>" << endl;
	file << "          <table>" << endl;
	file << "            <tr>" << endl;
	file << "              <td align=\"right\">" << endl;
	file << "                button:" << endl;
	file << "              </td>" << endl;
	file << "              <td>" << endl;
	file << "                <input type=\"spinner\" result=\"" << remote << "__choice:group\" width=\"20\" min=\"1\" max=\"" << remit->second.size() << "\" />" << endl;
	file << "                <input type=\"text\" result=\"" << remote << "__choice:correct_group\" value=\"\" disabled=\"disabled\" width=\"20\"/>" << endl;
	file << "              </td>" << endl;
	file << "            </tr>" << endl;
	file << "            <tr>" << endl;
	file << "              <td align=\"right\">" << endl;
	file << "                steps:" << endl;
	file << "              </td>" << endl;
	file << "              <td>" << endl;
	file << "                <input type=\"spinner\" result=\"" << remote << "__choice:steps\" />" << endl;
	file << "              </td>" << endl;
	file << "            </tr>" << endl;
	file << "            <tr>" << endl;
	file << "              <td>" << endl;
	file << "              </td>" << endl;
	file << "              <td>" << endl;
	file << "                <select size=\"1\" result=\"" << remote << "__choice:steps_action\">" << endl;
	file << "                  <option value=\"0\">UP STOP</option>" << endl;
	file << "                  <option value=\"1\">DOWN STOP</option>" << endl;
	file << "                  <option value=\"2\">UP LOOP</option>" << endl;
	file << "                  <option value=\"3\">DOWN LOOP</option>" << endl;
	file << "                </select>" << endl;
	file << "              </td>" << endl;
	file << "            </tr>" << endl;
	file << "            <tr>" << endl;
	file << "              <td>" << endl;
	file << "              </td>" << endl;
	file << "              <td>" << endl;
	file << "                <select size=\"1\" result=\"" << remote << "__choice:transmit_action\">" << endl;
	file << "                  <option value=\"0\">SEND ONCE</option>" << endl;
	file << "                  <option value=\"1\">SEND</option>" << endl;
	file << "                  <option value=\"2\">WAIT after</option>" << endl;
	file << "                  <option value=\"3\">BACK time</option>" << endl;
	file << "                </select>" << endl;
	file << "              </td>" << endl;
	file << "            </tr>" << endl;
	file << "            <tr>" << endl;
	file << "              <td align=\"right\">" << endl;
	file << "                <input type=\"checkbox\" result=\"" << remote << "__choice:receive\" readonly=\"readonly\" />" << endl;
	file << "              </td>" << endl;
	file << "              <td>" << endl;
	file << "                <select size=\"1\" result=\"" << remote << "__choice:correct_group\" disabled=\"disabled\">" << endl;
	actrows= 0;
	for(vector<string>::const_iterator coit= remit->second.begin(); coit != remit->second.end(); ++coit)
	{
		code= *coit;
		if(code != "###NULL::")
		{
			if(code.find(":") == string::npos)
			{
				displayCodeMap= r.dcodes.find(remote);
				foundCodeAliasMap= r.rcodealias.find(remote);
				foundCodeAlias= displayCodeMap->second.find(code);
				if(foundCodeAlias == displayCodeMap->second.end())
				{
					foundCodeAlias= foundCodeAliasMap->second.find(code);
					if(foundCodeAlias != foundCodeAliasMap->second.end())
						org_code= foundCodeAlias->second;
					else
						org_code= code;
				}else
					org_code= foundCodeAlias->second;
			}else
			{// code is an linked subroutine
				vector<string> sp;

				split(sp, code , is_any_of(":"));
				displayCodeMap= r.dcodes.find(sp[0]);
				foundCodeAliasMap= r.rcodealias.find(sp[0]);
				foundCodeAlias= displayCodeMap->second.find(sp[1]);
				if(foundCodeAlias == displayCodeMap->second.end())
				{
					foundCodeAlias= foundCodeAliasMap->second.find(sp[1]);
					if(foundCodeAlias != foundCodeAliasMap->second.end())
						org_code= foundCodeAlias->second;
					else
						org_code= sp[1];
				}else
					org_code= foundCodeAlias->second;
				code= sp[1];
			}
			++actrows;
			file << "                  <option value=\"" << actrows << "\">" << org_code << "</option>" << endl;
		}
	}
	file << "                </select>" << endl;
	file << "                <br />" << endl;
	file << "                <input type=\"button\" value=\"PRESS\" result=\"" << remote << "__choice:count\" />" << endl;
	file << "              </td>" << endl;
	file << "            </tr>" << endl;
	file << "            <tr>" << endl;
	file << "              <td align=\"right\">" << endl;
	file << "                first touch show:" << endl;
	file << "              </td>" << endl;
	file << "              <td>" << endl;
	file << "                <input type=\"checkbox\" result=\"" << remote << "__choice:display_first\" />" << endl;
	file << "              </td>" << endl;
	file << "            </tr>" << endl;
	file << "            <tr>" << endl;
	file << "              <td align=\"right\">" << endl;
	file << "                wait double<br />" << endl;
	file << "                after pressed again:" << endl;
	file << "              </td>" << endl;
	file << "              <td>" << endl;
	file << "                <input type=\"checkbox\" result=\"" << remote << "__choice:double\" />" << endl;
	file << "              </td>" << endl;
	file << "            </tr>" << endl;
	file << "            <tr>" << endl;
	file << "              <td align=\"right\">" << endl;
	file << "                wait after:" << endl;
	file << "              </td>" << endl;
	file << "              <td>" << endl;
	file << "                <input type=\"text\" value=\"sec\" format=\"#1.3\" result=\"" << remote << "__choice:after\" /><br />" << endl;
	file << "                <input type=\"text\" value=\"items\" result=\"" << remote << "__choice:calculate_lirc\" />" << endl;
	file << "              </td>" << endl;
	file << "            </tr>" << endl;
	file << "            <tr>" << endl;
	file << "              <td>" << endl;
	file << "              </td>" << endl;
	file << "              <td>" << endl;
	file << "                <input type=\"button\" value=\"CALCULATE\" result=\"" << remote << "__choice:calcbutton\" />" << endl;
	file << "              </td>" << endl;
	file << "            </tr>" << endl;
	file << "            <tr>" << endl;
	file << "              <td align=\"right\">" << endl;
	file << "                back time:" << endl;
	file << "              </td>" << endl;
	file << "              <td>" << endl;
	file << "                <input type=\"text\" value=\"sec\" format=\"#1.3\" result=\"" << remote << "__choice:back_time\" />" << endl;
	file << "              </td>" << endl;
	file << "            </tr>" << endl;
	file << "            <tr>" << endl;
	file << "              <td align=\"right\">" << endl;
	file << "                actual step:" << endl;
	file << "              </td>" << endl;
	file << "              <td>" << endl;
	file << "                <input type=\"spinner\" result=\"" << remote << "__choice:actual_step\" />" << endl;
	file << "              </td>" << endl;
	file << "            </tr>" << endl;
	file << "          </table>" << endl;
	//file << "        </td>" << endl;
	//file << "      </tr>" << endl;
	//file << "    </table>" << endl;
	file << "  </body>" << endl;
	file.close();
	return true;
}

bool LircSupport::createRemoteConfFile(const string& remote, const remotecodes_t& r,
		const string& readperm, const string& writeperm,
		const string& userreadperm, const string& userwriteperm, const bool transmit) const
{
	bool setDefault;
	bool allowdirect(false);
	unsigned short correct_group;
	Switch* pSwitch;
	Value* pValue;
	Set* pSet;
	Timer* pTimer;
	//Shell* pShell;
	//Debug* pDebug;
	Lirc* pLirc;
	auto_ptr<Folder> folder;
	ofstream file;
	string filename(remote + ".conf");
	string code, org_code;
	string org_remote;

	map<string, string>::const_iterator foundRemouteAlias, foundCodeAlias;
	map<string, map<string, string> >::const_iterator foundCodeAliasMap, displayCodeMap; //, linkCodeAliasMap;
	map<string, vector<string> >::const_iterator remit;

	remit= r.remotes.find(remote);
	if(remit == r.remotes.end())
	{
		cerr << "### ERROR:    cannot found remote control '" << remote;
		cerr << "' inside lircd.conf or pre-defined file" << endl << endl;
		return false;
	}
	foundRemouteAlias= r.remotealias.find(remote);
	org_remote= foundRemouteAlias->second;
	foundCodeAliasMap= r.rcodealias.find(remote);

	file.open(filename.c_str(), ios::out);
	if(!file.is_open())
	{
		cerr << "### ERROR:    cannot create " << filename << " for writing" << endl;
		return false;
	}
	cout << "                                 " << filename << endl;

	// writing header information
	writeHeader(file, remote);

	folder= auto_ptr<Folder>(new Folder(file, remote + "__show"));
	folder->description();
	folder->description();
	folder->description("###################################################################################");
	folder->description("###################################################################################");
	folder->description("####");
	folder->description("####");
	folder->description("####   write all folder for remote control");
	folder->description("####          '" + org_remote + "'");
	if(org_remote != remote)
		folder->description("####    alias as '" + remote + "'");
	folder->description("####");
	folder->description("####");
	folder->description("###################################################################################");
	folder->description("###################################################################################");
	folder->description();
	folder->description();

	file << "# this under comments should be templates to copy in your own measure.conf" << endl;
	file << "# All subroutines ending with _receive you can use in subroutines witch has begin, while or end parameter" << endl;
	if(transmit)
	{
		file << "#     subroutines ending with _send_once use in SET subroutines to send one signal to case or the measuered time in GUI" << endl;
		file << "#     by ending with _send begin to sending signal. Don't forget to SEt this subroutine to 0 for stopping" << endl;
	}
	file << "#" << endl;

	/**************************************************************************************************
	 *
	 * write first in lirc.conf or any defined file all remote controls with existing codes
	 * only for administrator to fill in by new folder codes with copy and paste
	 *
	 **************************************************************************************************/
	file << "# for remote " << org_remote << endl;
	for(vector<string>::const_iterator coit= remit->second.begin(); coit != remit->second.end(); ++coit)
	{
		code= *coit;
		if(code != "###NULL::")
		{
			if(code.find(":") == string::npos)
			{
				displayCodeMap= r.dcodes.find(remote);
				foundCodeAliasMap= r.rcodealias.find(remote);
				foundCodeAlias= displayCodeMap->second.find(code);
				if(foundCodeAlias == displayCodeMap->second.end())
				{
					foundCodeAlias= foundCodeAliasMap->second.find(code);
					if(foundCodeAlias != foundCodeAliasMap->second.end())
						org_code= foundCodeAlias->second;
					else
						org_code= code;
				}else
					org_code= foundCodeAlias->second;
			}else
			{// code is an linked subroutine
				vector<string> sp;

				split(sp, code , is_any_of(":"));
				displayCodeMap= r.dcodes.find(sp[0]);
				foundCodeAliasMap= r.rcodealias.find(sp[0]);
				foundCodeAlias= displayCodeMap->second.find(sp[1]);
				if(foundCodeAlias == displayCodeMap->second.end())
				{
					foundCodeAlias= foundCodeAliasMap->second.find(sp[1]);
					if(foundCodeAlias != foundCodeAliasMap->second.end())
						org_code= foundCodeAlias->second;
					else
						org_code= sp[1];
				}else
					org_code= foundCodeAlias->second;
				code= sp[1];
			}
			file << "#" << endl;
			file << "#        button '" << org_code << "'" << endl;
			if(allowdirect)
				file << "#            " << remote << "_" << code << ":receive" << endl;
			if(transmit)
			{
				file << "#            " << remote << "_" << code << ":count" << endl;
				if(allowdirect)
				{
					file << "#            " << remote << "_" << code << ":send_once" << endl;
					file << "#            " << remote << "_" << code << ":send" << endl;
				}
			}
			file << "#            " << remote << "_" << code << ":actual_step" << endl;
		}
	}
	file << "#" << endl;
	file << endl;

	/******************************************************************************************************
	 * declaration of all buttons seen on client to activate
	 */
	folder->description("declaration of all buttons from transmitter seen on client to activate");
	folder->flush();
	folder->description("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	folder->description();
	setDefault= false;
	for(vector<string>::const_iterator coit= remit->second.begin(); coit != remit->second.end(); ++coit)
	{
		code= *coit;
		if(code != "###NULL::")
		{
			if(code.find(":") == string::npos)
			{
				displayCodeMap= r.dcodes.find(remote);
				foundCodeAliasMap= r.rcodealias.find(remote);
				foundCodeAlias= displayCodeMap->second.find(code);
				if(foundCodeAlias == displayCodeMap->second.end())
				{
					foundCodeAlias= foundCodeAliasMap->second.find(code);
					if(foundCodeAlias != foundCodeAliasMap->second.end())
						org_code= foundCodeAlias->second;
					else
						org_code= code;
				}else
					org_code= foundCodeAlias->second;
			}else
			{// code is an linked subroutine
				vector<string> sp;

				split(sp, code , is_any_of(":"));
				displayCodeMap= r.dcodes.find(sp[0]);
				foundCodeAliasMap= r.rcodealias.find(sp[0]);
				foundCodeAlias= displayCodeMap->second.find(sp[1]);
				if(foundCodeAlias == displayCodeMap->second.end())
				{
					foundCodeAlias= foundCodeAliasMap->second.find(sp[1]);
					if(foundCodeAlias != foundCodeAliasMap->second.end())
						org_code= foundCodeAlias->second;
					else
						org_code= sp[1];
				}else
					org_code= foundCodeAlias->second;
				code= sp[1];
			}
			pSwitch= folder->getSwitch(code);
			pSwitch->description("button '" + org_code + "'");
			pSwitch->pend("true");
			if(!setDefault)
			{
				pSwitch->pdefault(1);
				setDefault= true;
			}
			pSwitch->pperm(writeperm);
			pSwitch->description();
		}
	}
	folder->description();
	folder->description();

	/******************************************************************************************************
	 * choice folder for changes, on right side of all buttons
	 */
	folder->description(" change properties for transmitter on client.");
	folder->description(" Fields to change, seen on right side of all transmitter buttons");
	folder= auto_ptr<Folder>(new Folder(file, remote + "__choice"));
	folder->flush();
	folder->description("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

	pValue= folder->getValue("correct_group");
	pValue->description("correct unique group number of button");
	pValue->flush();
	createSubroutineLink(file, pValue->getName(), remote, remit->second, "correct_group");
	pValue->action("int");
	pValue->pperm(readperm);
	pValue->description();

	pValue= folder->getValue("group");
	pValue->description("group of button which hold steps");
	pValue->flush();
	createSubroutineLink(file, pValue->getName(), remote, remit->second, "correct_group");
	pValue->action("int");
	pValue->pperm(writeperm);
	pValue->description();

	pValue= folder->getValue("transmit_action");
	pValue->description("action for transmitter: send_once, send, wait_after and back");
	pValue->flush();
	createSubroutineLink(file, pValue->getName(), remote, remit->second, "correct_group");
	pValue->action("int");
	pValue->pperm(writeperm);
	pValue->description();

	pValue= folder->getValue("count");
	pValue->description("button for client to send signal over transmitter");
	pValue->flush();
	createSubroutineLink(file, pValue->getName(), remote, remit->second, "correct_group");
	pValue->action("int");
	pValue->pperm(writeperm);
	pValue->description();

	pValue= folder->getValue("receive");
	pValue->description("receive signal from receiver or button was pressed");
	pValue->flush();
	createSubroutineLink(file, pValue->getName(), remote, remit->second, "correct_group");
	pValue->action("int");
	pValue->pperm(readperm);
	pValue->description();

	pValue= folder->getValue("calcbutton");
	pValue->description("button to calculating 'wait after' time for few steps");
	pValue->flush();
	createSubroutineLink(file, pValue->getName(), remote, remit->second, "correct_group");
	pValue->action("int");
	pValue->pperm(writeperm);
	pValue->description();

	pSwitch= folder->getSwitch("double");
	pSwitch->description("repeat when sending is set to SEND ONCE");
	pSwitch->flush();
	createSubroutineLink(file, pSwitch->getName(), remote, remit->second, "correct_group");
	pSwitch->pperm(writeperm);
	pSwitch->description();

	pValue= folder->getValue("after");
	pValue->description("measure time to wait after signal for next step");
	pValue->flush();
	createSubroutineLink(file, pValue->getName(), remote, remit->second, "correct_group");
	pValue->pperm(writeperm);
	pValue->description();

	pValue= folder->getValue("calculate_lirc");
	pValue->description("calculating 'wait after' lirc steps from receiving highest lirc count thru steps");
	pValue->flush();
	createSubroutineLink(file, pValue->getName(), remote, remit->second, "correct_group");
	pValue->action("int");
	pValue->pperm(writeperm);
	pValue->description();

	pValue= folder->getValue("back_time");
	pValue->description("how long the time after last pressed should measured");
	pValue->flush();
	createSubroutineLink(file, pValue->getName(), remote, remit->second, "correct_group");
	pValue->pperm(writeperm);
	pValue->description();

	pValue= folder->getValue("display_first");
	pValue->description("how often the interval of subroutine after is to wait");
	pValue->flush();
	createSubroutineLink(file, pValue->getName(), remote, remit->second, "correct_group");
	pValue->action("int");
	pValue->pperm(writeperm);
	pValue->description();

	pValue= folder->getValue("steps");
	pValue->description("count of exist steps");
	pValue->flush();
	createSubroutineLink(file, pValue->getName(), remote, remit->second, "correct_group");
	pValue->action("int");
	pValue->pperm(writeperm);
	pValue->description();

	pValue= folder->getValue("steps_action");
	pValue->description("action to count steps in which direction UP_STOP, DOWN_STOP, UP_LOOP, DOWN_LOOP");
	pValue->flush();
	createSubroutineLink(file, pValue->getName(), remote, remit->second, "correct_group");
	pValue->action("int");
	pValue->pperm(writeperm);
	pValue->description();

	pValue= folder->getValue("actual_step");
	pValue->description("count of actual step");
	pValue->flush();
	createSubroutineLink(file, pValue->getName(), remote, remit->second, "correct_group");
	pValue->action("int");
	pValue->pperm(writeperm);
	pValue->description();

	pValue->description();
	pValue->description();
	pValue->description();

	/******************************************************************************************************
	 * extra folders for every code on remote control
	 */
	correct_group= 0;
	for(vector<string>::const_iterator coit= remit->second.begin(); coit != remit->second.end(); ++coit)
	{
		//ostringstream group;
		string display_name;

		code= *coit;
		if(code != "###NULL::")
		{
			if(code.find(":") == string::npos)
			{
				displayCodeMap= r.dcodes.find(remote);
				foundCodeAliasMap= r.rcodealias.find(remote);
				foundCodeAlias= foundCodeAliasMap->second.find(code);
				org_code= foundCodeAlias->second;
				foundCodeAlias= displayCodeMap->second.find(code);
				if(foundCodeAlias == displayCodeMap->second.end())
				{
					foundCodeAlias= foundCodeAliasMap->second.find(code);
					if(foundCodeAlias != foundCodeAliasMap->second.end())
						display_name= foundCodeAlias->second;
					else
						display_name= code;
				}else
					display_name= foundCodeAlias->second;
			}else
			{// code is an linked subroutine
				vector<string> sp;

				split(sp, code , is_any_of(":"));
				displayCodeMap= r.dcodes.find(sp[0]);
				foundCodeAliasMap= r.rcodealias.find(sp[0]);
				foundCodeAlias= foundCodeAliasMap->second.find(code);
				if(foundCodeAlias != foundCodeAliasMap->second.end())
					org_code= foundCodeAlias->second;
				else
					org_code= code;
				foundCodeAlias= displayCodeMap->second.find(sp[1]);
				if(foundCodeAlias == displayCodeMap->second.end())
				{
					foundCodeAlias= foundCodeAliasMap->second.find(sp[1]);
					if(foundCodeAlias != foundCodeAliasMap->second.end())
						display_name= foundCodeAlias->second;
					else
						display_name= sp[1];
				}else
					display_name= foundCodeAlias->second;
				code= sp[1];
			}

			folder= auto_ptr<Folder>(new Folder(file, remote + "_" + code));
			folder->description("propteries for button '" + display_name + "'");
			folder->flush();
			folder->description("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
			folder->description();

			++correct_group;
			pValue= folder->getValue("correct_group");
			pValue->description("unique group number of button");
			pValue->pdefault(static_cast<double>(correct_group));
			pValue->pperm(readperm);
			pValue->description();
			pValue->description();
			pValue->description();

			folder->description("-------------------------------------------------------------------------------------------------");
			folder->description("--------------------------  begin of read sending from client  ----------------------------------");
			pSwitch= folder->getSwitch("count");
			pSwitch->description("button for client to send signal over transmitter");
			pSwitch->pperm(userwriteperm);
			pSwitch->description();

			pSwitch= folder->getSwitch("button");
			pSwitch->description("button should be the same for hole folder");
			pSwitch->pwhile("count");
			pSwitch->description();

			pTimer= folder->getTimer("pressed");
			pTimer->description("calculating length of pressed client button");
			pTimer->pwhile("button");
			pTimer->action("micro | measure");
			folder->description("--------------------------  end of read sending from client  ------------------------------------");
			folder->description("#################################################################################################");
			folder->description();
			folder->description();
			folder->description();


			folder->description("-------------------------------------------------------------------------------------------------");
			folder->description("---------------------------  begin of read signal from receiver  --------------------------------");
			pLirc= folder->getLirc("receive_signal_on");
			pLirc->description("get signal from receiver");
			pLirc->premote(org_remote);
			pLirc->pcode(org_code);
			pLirc->action("receive");
			pLirc->description();

			pValue= folder->getValue("receive_signal");
			pValue->description("signal should be the same for hole folder");
			pValue->pwhile("receive_signal_on");
			pValue->action("int");
			pValue->pperm(userreadperm);
			pValue->description();

			pSet= folder->getSet("lirc_set_back");
			pSet->description("was receive_signal really 0 before?");
			pSet->pfrom("last_next_lirc");
			pSet->pset("next_lirc");
			pSet->pset("last_lirc_count");
			pSet->pwhile("next_lirc=0 & receive_signal>last_lirc");
			pSet->description();

			pValue= folder->getValue("next_lirc");
			pValue->description("calculating first count signal by receive");
			pValue->description("(when calculate_lirc has 6 counts -> first count is 1, 12, 18, 24, ... -> calculated as 1, 2, 3, 4, ...)");
			pValue->pvalue(0);
			pValue->pvalue(1);
			pValue->pvalue("receive_signal / calculate_lirc");
			pValue->pwhile("receive_signal=0 ? 0 : receive_signal<calculate_lirc ? 1 : 2");
			// when first count should be 1, 7, 13, 19, ...
			// value= 0
			// value= (receive_signal-1) / calculate_lirc + 1
			// while= receive_signal=0 ? 0 : 1
			pValue->description();

			pSwitch= folder->getSwitch("lirc_set");
			pSwitch->description("value is true when any first count is reached");
			pSwitch->pwhile("next_lirc!=0 & next_lirc != last_lirc_count");
			pSwitch->description();

			pValue= folder->getValue("last_lirc_count");
			pValue->description("last count of next_lirc to calculate lirc_set");
			pValue->pwhile("next_lirc");
			pValue->action("int");
			pValue->description();

			pValue= folder->getValue("last_lirc");
			pValue->description("last count of receive_signal");
			pValue->pvalue("last_lirc");
			pValue->pvalue("receive_signal");
			pValue->pwhile("receive_signal=0 ? 0 : 1");
			pValue->action("int");
			pValue->description();

			pValue= folder->getValue("last_next_lirc");
			pValue->description("last count of next_lirc");
			pValue->pwhile("next_lirc!=0 ? next_lirc : last_next_lirc");
			pValue->action("int");
			pValue->description();

			pValue= folder->getValue("lirc_high");
			pValue->description("highest lirc signal was last sending");
			pValue->pwhile("receive_signal ? receive_signal : lirc_high");
			pValue->action("int");
			folder->description("---------------------------  end of read signal from receiver  ----------------------------------");
			folder->description("#################################################################################################");
			folder->description();
			folder->description();
			folder->description();

			pValue= folder->getValue("what");
			pValue->description("whether button was last pressed (value 0)");
			pValue->description("or lirc has received as last a signal (value 1)");
			pValue->pvalue(0, "button was pressed (field count was activated)");
			pValue->pvalue(1, "lirc received signal");
			pValue->pvalue("what");
			pValue->pwhile("button ? 0 : receive_signal ? 1 : 2");
			pValue->action("int");
			pValue->description();

			pSwitch= folder->getSwitch("active");
			pSwitch->description("whether folder is active running by get signal from any client or receiver");
			pSwitch->pbegin("receive");
			pSwitch->description();

			folder->description();
			folder->description();
			folder->description("-------------------------------------------------------------------------------------------------");
			folder->description("--------------  begin of calculation from first_touch, receive any and first_off  ---------------");
			pSwitch= folder->getSwitch("first_touch");
			pSwitch->description("first signal from receiver or client button");
			pSwitch->description("(this value is not the same like 'first touch show' on jclient)");
			pSwitch->pwhile("receive=0 & (button | (lirc_set & receive_signal<=calculate_lirc))");
			pSwitch->description();

			pSwitch= folder->getSwitch("first_off");
			pSwitch->description("first passing by no signal receiver or client button");
			pSwitch->pwhile("receive_signal=0 & button=0 & receive=1");
			pSwitch->pperm(readperm);
			pSwitch->description();

			pSwitch= folder->getSwitch("receive");
			pSwitch->description("receive signal from receiver or button was pressed");
			pSwitch->pwhile("receive_signal | button");
			pSwitch->pperm(userreadperm);
			folder->description("--------------  end of calculation from first_touch, receive any and first_off  -----------------");
			folder->description("#################################################################################################");
			folder->description();

			folder->description();
			folder->description();
			folder->description("-------------------------------------------------------------------------------------------------");
			folder->description("---------------------  begin properties can be changed from client  -----------------------------");
			pValue= folder->getValue("group");
			pValue->description("group of button which hold steps");
			pValue->pmin(1);
			pValue->pmax(remit->second.size(), "actual button count");
			pValue->pdefault(correct_group);
			pValue->action("int | db");
			pValue->pperm(writeperm);
			pValue->description();

			pValue= folder->getValue("steps");
			pValue->description("count of exist steps");
			pValue->pmin(0);
			pValue->pmax(200);
			pValue->pdefault(0);
			createSubroutineLink(file, pValue->getName(), remote, remit->second, "group");
			pValue->action("int | db");
			pValue->pperm(writeperm);
			pValue->description();

			pValue= folder->getValue("transmit_action");
			pValue->description("action for transmitter:");
			pValue->description("    'SEND ONCE'  = 0  # sending only one signal");
			pValue->description("    'SEND'       = 1  # send during length holding button");
			pValue->description("    'WAIT after' = 2  # calibrate time how long an signal should during");
			pValue->description("    'BACK time'  = 3  # calibrate BACK time before make again first touch");
			pValue->pmin(0);
			pValue->pmax(3);
			pValue->action("int | db");
			pValue->pperm(writeperm);
			pValue->description();

			pValue= folder->getValue("steps_action");
			pValue->description("action to count steps in which direction");
			pValue->description("    'UP STOP'   = 0");
			pValue->description("    'DOWN STOP' = 1");
			pValue->description("    'UP LOOP'   = 2");
			pValue->description("    'DOWN LOOP' = 3");
			pValue->pmin(0);
			pValue->pmax(3);
			pValue->action("int | db");
			pValue->pperm(writeperm);
			pValue->description();

			pValue= folder->getValue("display_first");
			pValue->description("how often the interval of subroutine after is to wait for next step");
			pValue->description("('first touch show' on jclient)");
			pValue->flush();
			createSubroutineLink(file, pValue->getName(), remote, remit->second, "group");
			pValue->pmin(0);
			pValue->pmax(1);
			pValue->action("int | db");
			pValue->pperm(writeperm);
			pValue->description();

			pSwitch= folder->getSwitch("double");
			pSwitch->description("steps should wait double time after pressed again");
			pSwitch->flush();
			createSubroutineLink(file, pSwitch->getName(), remote, remit->second, "group");
			pSwitch->action("db");
			pSwitch->pperm(writeperm);
			pSwitch->description();

			pSwitch= folder->getSwitch("calcbutton");
			pSwitch->description("button to calculating 'wait after' time after pressed few steps");
			pSwitch->pperm(writeperm);
			pSwitch->description();

			pSet= folder->getSet("calculate");
			pSet->description("calculate time how long an signal should during");
			pSet->pfrom("pressed / (actual_step + display_first)");
			pSet->pset("after");
			pSet->pwhile("what=0 & calcbutton");
			pSet->description();

			pValue= folder->getValue("calculate_lirc");
			pValue->description("calculate how much signals from receiver inherit one step");
			pValue->pvalue("lirc_high / (actual_step + display_first)");
			pValue->pvalue(pValue->getName());
			pValue->pwhile("what=1 & calcbutton ? 0 : 1");
			createSubroutineLink(file, pValue->getName(), remote, remit->second, "group");
			pValue->pdefault(5);
			pValue->pmin(1);
			pValue->pmax(20);
			pValue->action("int | db");
			pValue->pperm(writeperm);
			pValue->description();

			pTimer= folder->getTimer("after");
			pTimer->description("measure time to wait after signal for next step");
			pTimer->pwhile("transmit_action=2 & button");
			createSubroutineLink(file, pTimer->getName(), remote, remit->second, "group");
			pTimer->pdefault(0.3);
			pTimer->action("db | measure | micro");
			pTimer->pperm(writeperm);
			pTimer->description();

			pTimer= folder->getTimer("back_time");
			pTimer->description("how long the time after last pressed should measured for next step");
			pTimer->pwhile("transmit_action=3 & receive");
			createSubroutineLink(file, pTimer->getName(), remote, remit->second, "group");
			pTimer->pdefault(0.5);
			pTimer->action("db | measure | micro");
			pTimer->pperm(writeperm);
			folder->description("---------------------  end properties can be changed from client  -------------------------------");
			folder->description("#################################################################################################");
			folder->description();

			folder->description();
			folder->description();
			folder->description("-------------------------------------------------------------------------------------------------");
			folder->description("----------------------  begin of sending signal over transmitter  -------------------------------");
			pLirc= folder->getLirc("send_once");
			pLirc->description("send only one signal over transmitter");
			pLirc->premote(org_remote);
			pLirc->pcode(org_code);
			pLirc->pwhile("what=0 & first_touch & (transmit_action=0 | transmit_action=3)");
			pLirc->action("send_once");
			pLirc->description();

			pLirc= folder->getLirc("send_onoff");
			pLirc->description("send signal over transmitter for longer time");
			pLirc->premote(org_remote);
			pLirc->pcode(org_code);
			pLirc->pwhile("button & (transmit_action=1 | transmit_action=2)");
			pLirc->action("send");
			folder->description("----------------------  end of sending signal over transmitter  ---------------------------------");
			folder->description("#################################################################################################");
			folder->description();

			folder->description();
			folder->description();
			folder->description("-------------------------------------------------------------------------------------------------");
			folder->description("-------  begin of time calculation for button (send) and new activating first touch  ------------");
			pTimer= folder->getTimer("wait_after");
			pTimer->description("mesure the time after pressing button or receive signal for set next step");
			pTimer->pmtime("after");
			pTimer->pbegin("transmit_action<=1 & button & first_touch");
			pTimer->pwhile("transmit_action<=1 & button");
			pTimer->action("micro");
			pTimer->description();

			pTimer= folder->getTimer("new_activate");
			pTimer->description("measure time after lost signal over receiver or button");
			pTimer->description("in this time can be pressed again, next step inside this time will be not only for show");
			pTimer->pmtime("back_time");
			pTimer->pbegin("display_first & first_off");
			pTimer->pend("new_activate<=0");
			createSubroutineLink(file, pTimer->getName(), remote, remit->second, "group");
			pTimer->action("micro");
			folder->description("-------  end of time calculation for button (send) and new activating first touch  --------------");
			folder->description("#################################################################################################");
			folder->description();

			folder->description();
			folder->description();
			folder->description("-------------------------------------------------------------------------------------------------");
			folder->description("-------------------------  begin of calculation for next step  ----------------------------------");
			pValue= folder->getValue("first_calc");
			pValue->description("length of first_touch multiplicator");
			pValue->pvalue("display_first");
			pValue->pvalue(0);
			pValue->pvalue("first_calc -1");
			pValue->pvalue("first_calc");
			pValue->pwhile("\"active=0 ? 3 :\n"
								"                receive=0 &\n"
								"                new_activate<=0 ? 0 :\n"
								"                    first_touch &\n"
								"                    first_calc=display_first * 2 ? 1 :\n"
								"                        receive &\n"
								"                        first_calc>0 &\n"
								"                        (    (    what=0 &\n"
								"                                  (    wait_after=0 |\n"
								"                                       (    new_activate > 0 &\n"
								"                                            wait_after=after        )    )    ) |\n"
								"                             (    what=1 &\n"
								"                                  lirc_set &\n"
								"                                  (    first_touch=0 |\n"
								"                                       receive_signal > calculate_lirc |\n"
								"                                       new_activate > 0                    )    )    ) ? 2 : 3\"");
			createSubroutineLink(file, pValue->getName(), remote, remit->second, "group");
			pValue->action("int");
			pValue->pdefault(1);
			pValue->description();

			pValue= folder->getValue("actual_step");
			pValue->description("count of actual step");
			pValue->pmin(0);
			pValue->pmax(500);
			pValue->pdefault(0);
			pValue->pvalue("actual_step");
			pValue->pvalue("actual_step >= steps ? steps : actual_step + 1");
			pValue->pvalue("actual_step <= 0 ? 0 : actual_step - 1");
			pValue->pvalue("actual_step >= steps ? 0 : actual_step + 1");
			pValue->pvalue("actual_step <= 0 ? steps : actual_step - 1");
			pValue->pwhile("\"(    display_first=0 &\n"
							"             first_touch                ) |\n"
							"        (    what=0 &\n"
							"             (    transmit_action=1 &\n"
							"                  receive &\n"
							"                  first_calc=0 &\n"
							"                  (    first_touch |\n"
							"                       wait_after=0 |\n"
							"                       wait_after=after    )    )    ) |\n"
							"        (    what=1 &\n"
							"             (    transmit_action=1 &\n"
							"                  first_calc=0 &\n"
							"                  lirc_set                 )    )                ? steps_action + 1 : 0\"");
			createSubroutineLink(file, pValue->getName(), remote, remit->second, "group");
			pValue->action("int | db");
			pValue->pperm(userwriteperm);
			pValue->description();

			pSet= folder->getSet("again");
			pSet->description("wait double by again pressing");
			pSet->pfrom("display_first * 2");
			pSet->pset("first_calc");
			pSet->pwhile("display_first & double & first_touch & first_calc=0");
			pSet->description();

			pSet= folder->getSet("active_back");
			pSet->description("set active folder back to 0 when end of new_activate is reached");
			pSet->pfrom(0);
			pSet->pset("active");
			pSet->pwhile("receive=0 & new_activate<=0");
			folder->description("-------------------------  end of calculation for next step  ------------------------------------");
			folder->description("#################################################################################################");
			folder->description();
			folder->description();
			folder->description();
			folder->description();
			folder->description();
		}
	}
	return true;
}

void LircSupport::createSubroutineLink(ostream& file, const string& name, const string& remotelink,
		const vector<string>& linkcodes, const string& firstlink) const
{
	unsigned short count(0);
	string code;

	for(vector<string>::const_iterator coit= linkcodes.begin(); coit != linkcodes.end(); ++coit)
	{
		code= *coit;
		if(code != "###NULL::")
		{
			if(code.find(":") != string::npos)
			{// code is an linked subroutine
				vector<string> sp;

				split(sp, code , is_any_of(":"));
				code= sp[1];
			}
			file << "link= " << remotelink << "_" << code << ":" << name << endl;
		}
	}
	file << "lwhile= ";
	if(	firstlink == "" ||
		firstlink == name	)
	{
		for(vector<string>::const_iterator coit= linkcodes.begin(); coit != linkcodes.end(); ++coit)
		{
			code= *coit;
			if(code != "###NULL::")
			{
				++count;
				if(code.find(":") != string::npos)
				{// code is an linked subroutine
					vector<string> sp;

					split(sp, code , is_any_of(":"));
					code= sp[1];
				}
				file << remotelink << "__show:" << code << " ? " << count << " : ";
			}
		}
		file << "1" << endl;
	}else
		file << firstlink << endl;
}












