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


int LircSupport::showRemotes(const remotecodes_t& r) const
{
	map<string, string>::const_iterator f;
	map<string, vector<string> >::const_iterator found;

	if(r.remotealias.size())
	{
		for(map<string, string>::const_iterator n= r.remotealias.begin(); n != r.remotealias.end(); ++n)
		{
			found= r.kill.find(n->first);
			if(found == r.kill.end())
				cout << "    create remote '" << n->second << "'  as  '" << n->first << "'" << endl;
		}
		cout << endl;
		if(r.kill.size())
		{
			for(found= r.kill.begin(); found != r.kill.end(); ++found)
			{
				f= r.remotealias.find(found->first);
				cout << "    do not create remote '" << f->second << "' because it is an link to remote" << endl;
				for(vector<string>::const_iterator c= found->second.begin(); c != found->second.end(); ++c)
					cout << "                               '" << *c << "'" << endl;
			}
			cout << endl;
		}
		return EXIT_SUCCESS;
	}
	cerr << "    cannot find any remote in lircd.conf" << endl << endl;
	return EXIT_FAILURE;
}

int LircSupport::execute(const ICommandStructPattern* params)
{
	bool transmit;
	bool learn(params->hasOption("learn"));
	int vertical= 3;
	string param, opname, remote;
	string lircconf("/etc/lirc/lircd.conf");
	string userreadperm("read"), userwriteperm("change");
	string readperm("lconfread"), writeperm("lconfchange");
	string ureadcw("ureadlw");
	remotecodes_t r;

	if(params->hasOption("predefined"))
	{
		if(params->optioncount() > 1)
		{
			cout << endl;
			cout << "  WARNING: more than one option be set! (except --predefined)" << endl;
			cout << "           show only predefined namespaces" << endl;
		}
		cout << endl;
		cout << "  use this codes inside lircd.conf" << endl;
		cout << "  this make an default predefinition for the transmitter" << endl;
		cout << "  ( as example for KEY_0 to KEY_9 it defines to set only the number inside subroutine actual_step" << endl;
		cout << "    and also for KEY_CHANNELUP to KEY_CHANNELDOWN the subroutine actual_step scrolls higher and lower" << endl;
		cout << "    KEY_0 to KEY_9 is associated with KEY_CHANNELUP to KEY_CHANNELDOWN and have the same actual_step field )" << endl;
		cout << "  it's only for default and changeable by GUI" << endl;
		cout << "  by using 'irrecord -l' on command line you can see all namesapaces to use as code" << endl;
		cout << endl;
		cout << endl;
		cout << "  predefined COUNT codes:" << endl;
		cout << endl;
		cout << "      KEY_0    KEY_NUMMERIC_0    KEY_F      KEY_FN_F                       KEY_KP0    BTN_0" << endl;
		cout << "      KEY_1    KEY_NUMMERIC_1    KEY_F1     KEY_FN_F1     KEY_BRL_DOT1     KEY_KP1    BTN_1" << endl;
		cout << "      KEY_2    KEY_NUMMERIC_2    KEY_F2     KEY_FN_F2     KEY_BRL_DOT2     KEY_KP2    BTN_2" << endl;
		cout << "      KEY_3    KEY_NUMMERIC_3    KEY_F3     KEY_FN_F3     KEY_BRL_DOT3     KEY_KP3    BTN_3" << endl;
		cout << "      KEY_4    KEY_NUMMERIC_4    KEY_F4     KEY_FN_F4     KEY_BRL_DOT4     KEY_KP4    BTN_4" << endl;
		cout << "      KEY_5    KEY_NUMMERIC_5    KEY_F5     KEY_FN_F5     KEY_BRL_DOT5     KEY_KP5    BTN_5" << endl;
		cout << "      KEY_6    KEY_NUMMERIC_6    KEY_F6     KEY_FN_F6     KEY_BRL_DOT6     KEY_KP6    BTN_6" << endl;
		cout << "      KEY_7    KEY_NUMMERIC_7    KEY_F7     KEY_FN_F7     KEY_BRL_DOT7     KEY_KP7    BTN_7" << endl;
		cout << "      KEY_8    KEY_NUMMERIC_8    KEY_F8     KEY_FN_F8     KEY_BRL_DOT8     KEY_KP8    BTN_8" << endl;
		cout << "      KEY_9    KEY_NUMMERIC_9    KEY_F9     KEY_FN_F9     KEY_BRL_DOT9     KEY_KP9    BTN_9" << endl;
		cout << "                                 KEY_F1     KEY_FN_F10    KEY_BRL_DOT10               BTN_A" << endl;
		cout << "                                 KEY_F1                                               BTN_B" << endl;
		cout << "                                 KEY_F12                                              BTN_C" << endl;
		cout << "                                 KEY_F13" << endl;
		cout << "                                 KEY_F14" << endl;
		cout << "                                 KEY_F15" << endl;
		cout << "                                 KEY_F16" << endl;
		cout << "                                 KEY_F17" << endl;
		cout << "                                 KEY_F18" << endl;
		cout << "                                 KEY_F19" << endl;
		cout << "                                 KEY_F20" << endl;
		cout << "                                 KEY_F21" << endl;
		cout << "                                 KEY_F22" << endl;
		cout << "                                 KEY_F23" << endl;
		cout << "                                 KEY_F24" << endl;
		cout << endl;
		cout << endl;
		cout << "  predefined UP and DOWN codes" << endl;
		cout << endl;
		cout << "      KEY_CHANNELUP       KEY_CHANNELDOWN" << endl;
		cout << "      KEY_NEXT            KEY_PREVIOUS" << endl;
		cout << endl;
		cout << "      KEY_PAGEUP          KEY_PAGEDOWN" << endl;
		cout << "      KEY_SCROLLUP        KEY_SCROLLDOWN" << endl;
		cout << "      KEY_VOLUMEUP        KEY_VOLUMEDOWN" << endl;
		cout << "      KEY_BRIGHTNESSUP    KEY_BRIGHTNESSDOWN" << endl;
		cout << "      KEY_KBDILLUMUP      KEY_KBDILLUMDOWN" << endl;
		cout << "      BTN_GEAR_UP         BTN_GEAR_DOWN" << endl;
		cout << endl;
		cout << "      KEY_LEFT            KEY_RIGHT" << endl;
		cout << "      KEY_LEFTSHIFT       KEY_RIGHTSHIFT" << endl;
		cout << "      KEY_LEFTALT         KEY_RIGHTALT" << endl;
		cout << "      KEY_LEFTBRACE       KEY_RIGHTBRACE" << endl;
		cout << "      KEY_LEFTCTRL        KEY_RIGHTCTRL" << endl;
		cout << "      KEY_LEFTMETA        KEY_RIGHTMETA" << endl;
		cout << "      BTN_LEFT            BTN_RIGHT" << endl;
		cout << endl;
		cout << "      KEY_ZOOMIN          KEY_ZOOMOUT" << endl;
		cout << "      KEY_FORWARD         KEY_REWIND" << endl;
		cout << endl;
		cout << endl;
		cout << "  associated predefined UP and DOWN codes with COUNTs" << endl;
		cout << endl;
		cout << "      for KEY_0 - KEY_9" << endl;
		cout << "            KEY_CHANNELUP    KEY_CHANNELDOWN" << endl;
		cout << endl;
		cout << "      for KEY_NUMMERIC_0 - KEY_NUMMERIC_9" << endl;
		cout << "            KEY_NEXT         KEY_PREVIOUS" << endl;
		cout << endl;
		return true;
	}
	transmit= !params->hasOption("notransmit");
	if(params->hasOption("file"))
		lircconf= params->getOptionContent("file");
	if(params->hasOption("show"))
	{
		if(	(	!params->hasOption("file") &&
				params->optioncount() > 1 		) ||
			(	params->hasOption("file") &&
				params->optioncount() > 2 		)	)
		{
			cout << endl;
			cout << "  WARNING: more than one option be set! (except --file)" << endl;
			cout << "           show only remote names with aliases" << endl;
		}
		cout << endl;
		r= readLircd(lircconf);
		return showRemotes(r);
	}
	if(params->hasOption("readperm"))
		userreadperm= params->getOptionContent("readperm");
	if(params->hasOption("changeperm"))
		userwriteperm= params->getOptionContent("changeperm");
	if(params->hasOption("configreadperm"))
		readperm= params->getOptionContent("configreadperm");
	if(params->hasOption("configchangeperm"))
		writeperm= params->getOptionContent("configchangeperm");
	if(params->hasOption("userreadconfwrite"))
		ureadcw= params->getOptionContent("userreadconfwrite");
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
		searchPreDefined(r);
		if(createConfigLayoutFiles(transmit, vertical, userreadperm, userwriteperm, ureadcw, readperm, writeperm, r, remote) == false)
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

						}else if(code == "link")
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
								if(find(r.kill[rem].begin(), r.kill[rem].end(), name) == r.kill[rem].end())
									r.kill[rem].push_back(name);
								r.remotes[name].push_back(rem +":"+ co);
								//cout << "       with code '" << co << "' linked to remote '" << rem << "'" << endl;
							}else
								cout << "### WARNING: undefined uncommented code '" << code << "' found in remoute '" << name << "'" << endl;

						}else if(code == "null")
						{
							r.remotes[name].push_back("###NULL::");
							//cout << "       NULL FIELD" << endl;
						}else if(code == "break")
						{
							r.remotes[name].push_back("###BREAK::");
							//cout << "       NEW ROW" << endl;
						}else
						{
							cout << "### WARNING: found undefined uncommented key value '" << code << "'" << endl;
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
	file << "#       LIRC         properties: <ID>, <pin>, [priority], [cache], [value&(begin|while|end)]" << endl;
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
	file << "#       value      - which value should be writing" << endl;
	file << "#                    the value can be an defined-value" << endl;
	file << "#                    only when set action to send_once or send" << endl;
	file << "#       begin      - do writing if begin status occurring" << endl;
	file << "#                    the value can be an defined-value" << endl;
	file << "#                    only when set action to send_once or send" << endl;
	file << "#       while      - do writing while state be set" << endl;
	file << "#                    the value can be an defined-value" << endl;
	file << "#                    only when set action to send_once or send" << endl;
	file << "#       end        - set pin to 0 if expression be correct" << endl;
	file << "#                    the value can be an defined-value" << endl;
	file << "#                    only when set action to send_once or send" << endl;
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
		const string& userreadperm, const string& userwriteperm, const string& ureadcw,
		const string& readperm, const string& writeperm, const remotecodes_t& r, const string& forremote) const
{
	bool bwritten(false), ballwritten(true);
	Switch* pSwitch;
	auto_ptr<Folder> folder;
	ofstream file;
	map<string, vector<string> >::const_iterator remit;
	map<string, string>::const_iterator foundRemouteAlias;

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

		//  write by transmitting record button
		folder= auto_ptr<Folder>(new Folder(file, "TRANSMIT_RECEIVE_main_settings"));
		pSwitch= folder->getSwitch("record");
		pSwitch->description("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
		pSwitch->description(" record all changes on remote access");
		pSwitch->action("db");
		pSwitch->pperm(writeperm);
		pSwitch->description();
		pSwitch->description();
		pSwitch->description();

		for(remit= r.remotes.begin(); remit != r.remotes.end(); ++remit)
		{
			if(r.kill.find(remit->first) == r.kill.end())
			{
				foundRemouteAlias= r.remotealias.find(remit->first);
				cout << "###" << endl;
				cout << "###   write configuration for remote control '" << foundRemouteAlias->second << "':" << endl;
				if(createRemoteConfFile(remit->first, r, readperm, writeperm, ureadcw, userreadperm, userwriteperm, transmit))
				{

					file << "#" << endl;
					file << "#  configuration file for remote control " <<  foundRemouteAlias->second << endl;
					file << "# ----------------------------------------------------------------------------------------" << endl;
					file << "file= " << remit->first << ".conf" << endl;
					file << endl;
					if(createRemoteDesktopFile(remit->first, r, vertical, writeperm))
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
			foundRemouteAlias= r.remotealias.find(forremote);
			if(foundRemouteAlias != r.remotealias.end())
			{
				cout << "###   write configuration for remote control '" << foundRemouteAlias->second << "':" << endl;
				if(createRemoteConfFile(forremote, r, readperm, writeperm, ureadcw, userreadperm, userwriteperm, transmit))
				{
					if(r.kill.find(forremote) == r.kill.end())
					{
						if(createRemoteDesktopFile(forremote, r, vertical, writeperm))
							bwritten= true;
					}else
						ballwritten= false;
				}else
					ballwritten= false;
			}else
			{
				bwritten= false;
				cerr << "### ERROR: given remote control name do not exist inside lircd.conf" << endl;
				cerr << "           pleas take an other one." << endl;
				cerr << endl;
				showRemotes(r);
			}
		}else
		{
			cerr << "### ERROR: given file '" << forremote << "' is only an linked remote from remote ";
			for(vector<string>::const_iterator it= found->second.begin(); it != found->second.end(); ++it)
				cerr << *it << " ";
			cerr << endl << "           pleas take an other one." << endl;
			ballwritten= false;
		}
	}
	if(bwritten == false)
		return false;
	cout << endl;
	cout << "  Pleas copy now the generated *.conf file(s) into the configuration sub folder (conf) of ppi-server" << endl;
	cout << "  and also the corresponding *desktop files to the client directory of ppi-server." << endl;
	if(forremote == "")
		cout << " Create also an link in the main measure.conf file to lirc.conf (file= lirc.conf)." << endl;
	cout << endl;
	return true;
}

bool LircSupport::createRemoteDesktopFile(const string& remote, const remotecodes_t& r, const int vertical, const string& perm) const
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
	file << "    <meta name=\"permission\" content=\"" << perm << "\" />" << endl;
	file << "  </head>" << endl;
	file << "  <body>" << endl;
	file << "    <table boder=\"1\" width=\"100\">" << endl;
	file << "      <tr>" << endl;
	file << "        <td>" << endl;
	file << "          <input type=\"togglebutton\" value=\"record\" "
				"result=\"TRANSMIT_RECEIVE_main_settings:record\" />" << endl;
	file << "          &#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;" << endl;
	file << "          " << org_remote << endl;
	file << "        </td>" << endl;
	file << "      </tr>" << endl;
	file << "    </table><br />" << endl;
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
		file << "              <td align=\"center\">" << endl;
		//cout << "read " << remote << " " << code << endl;
		if(	code != "###NULL::" &&
			code != "###BREAK::"		)
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
		if(	actrows == vertical ||
			code == "###BREAK::"	)
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
	file << "                <input type=\"spinner\" result=\"" << remote << "__choice:group\" width=\"20\" min=\"1\" max=\"" << remit->second.size() << "\" />&#160;" << endl;
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
		if(	code != "###NULL::" &&
			code != "###BREAK::"		)
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
	file << "              <td align=\"left\">" << endl;
	file << "                wait double<br />" << endl;
	file << "                after pressed again:" << endl;
	file << "              </td>" << endl;
	file << "              <td>" << endl;
	file << "                <input type=\"checkbox\" result=\"" << remote << "__choice:double\" />" << endl;
	file << "              </td>" << endl;
	file << "            </tr>" << endl;

	file << "            <tr>" << endl;
	file << "              <td align=\"right\">" << endl;
	file << "                set only actual step:<br />" << endl;
	file << "                            to value:" << endl;
	file << "              </td>" << endl;
	file << "              <td>" << endl;
	file << "                <input type=\"checkbox\" result=\"" << remote << "__choice:set_steps\" />" << endl;
	file << "                <input type=\"spinner\" result=\"" << remote << "__choice:digits\" width=\"20\" min=\"1\" max=\"5\" />" << endl;
	file << "                &#160;digits<br />" << endl;
	file << "                <input type=\"spinner\" result=\"" << remote << "__choice:to_value\" width=\"50\" min=\"0\" max=\"99999\" />" << endl;
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
	file << "                <input type=\"checkbox\" result=\"" << remote << "__choice:wait_back_time\" readonly=\"readonly\" />" << endl;
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
		const string& readperm, const string& writeperm, const string& ureadcw,
		const string& userreadperm, const string& userwriteperm, const bool transmit) const
{
	bool setDefault;
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
	string code, searchcode, org_code;
	string org_remote;
	default_t tdefault;

	vector<string>::size_type count, linkcount;
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

	file << "#" << endl;
	file << "# This under comments are templates to copy in your own measure.conf or layout file." << endl;
	file << "# The main subroutines are 'count_steps'  - routine to send signal over transmitter" << endl;
	file << "#                                           and count the value actual_step higher or lower" << endl;
	file << "#            and of course 'actual_step'  - where you can read the actual count" << endl;
	file << "#" << endl;
	file << "# 'count_steps' is an subroutine from type 'VALUE' where you can fill in," << endl;
	file << "#               with an subroutine from type SET, how much steps the remote folder should count" << endl;
	file << "#               always in the direction and properties defined before in the GUI (maybe ppi-java-client)." << endl;
	file << "#               This subroutine should be used only in own defined measure.conf files." << endl;
	file << "#               (measure.conf means also self named files xxx.conf linked from main measure.conf file)." << endl;
	file << "#               For layout files use the subroutine 'count' from type SWITCH, when you want to use as button." << endl;
	file << "#               You can also use this 'count' subroutine in an configuration file when you watch" << endl;
	file << "#               the subroutine 'actual_step' and set after right value 'count' to 0." << endl;
	file << "# 'actual_step' if for using in an own subroutine with 'from', 'begin', 'while' or 'end' parameter " << endl;
	file << "#               like subroutines from type SET, SWITCH, VALUE and so on." << endl;
	file << "#" << endl;
	file << "#        example to read 'actual_step':" << endl;
	file << "#                     name= myNewSubroutine" << endl;
	file << "#                     type= SWITCH" << endl;
	file << "#                     begin= " << remote << ":actual_step = 1" << endl;
	file << "#                     end= " << remote << ":actual_step = 0" << endl;
	file << "#        or fill steps into 'count_steps':" << endl;
	file << "#                     name= setCounts" << endl;
	file << "#                     type= SET" << endl;
	file << "#                     from= 3" << endl;
	file << "#                     set= " << remote << ":count_steps" << endl;
	file << "#                     while= anyFolder:subroutine = 1" << endl;
	file << "#" << endl;
	file << "# Important could be also the following subroutines for native access: " << endl;
	file << "#      count          - also to send signal over transmitter, described before" << endl;
	file << "#      receive        - receive signal from receiver or subroutine 'count' (count_steps)" << endl;
	file << "#                       is activated from any client or measure.conf" << endl;
	file << "#      what           - whether subroutine 'count' was as last activated (value 0)" << endl;
	file << "#                       or lirc has received as last a signal (value 1)" << endl;
	file << "#      wait_back_time - value is 1 while folder wait for time to show back the default value" << endl;
	file << "#                       This subroutine will be activated an time after 'receive' was active" << endl;
	file << "#                       when 'first touch show' inside the GUI is checked " << endl;
	file << "#                       or the button folder is defined to 'set only actual step' with more than one digits" << endl;
	file << "#      predef_step    - define value of actual step before when folder button be defined to 'set only actual step'" << endl;
	file << "#                       and 'wait_back_time' is active, elsewhere this subroutine have the value -1" << endl;
	file << "#      receive_signal - get signal number from receiver" << endl;
	file << "#                       (not the same as actual_step but the count length of pressing on transmitter)" << endl;
	file << "#" << endl;
	file << "#" << endl;
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
		if(	code != "###NULL::" &&
			code != "###BREAK::"		)
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
			file << "#            " << remote << "_" << code << ":count_steps" << endl;
			file << "#            " << remote << "_" << code << ":actual_step" << endl;
			file << "#            " << remote << "_" << code << ":count" << endl;
			file << "#            " << remote << "_" << code << ":receive" << endl;
			file << "#            " << remote << "_" << code << ":what" << endl;
			file << "#            " << remote << "_" << code << ":wait_back_time" << endl;
			file << "#            " << remote << "_" << code << ":predef_step" << endl;
			file << "#            " << remote << "_" << code << ":receive_signal" << endl;
		}
	}
	file << "#" << endl;
	file << endl;
	file << endl;
	file << endl;

	/******************************************************************************************************
	 * declaration of all buFttons seen on client to activate
	 */
	folder->description("declaration of all buttons from transmitter seen on client to activate");
	folder->flush();
	folder->description("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	folder->description();
	setDefault= false;
	for(vector<string>::const_iterator coit= remit->second.begin(); coit != remit->second.end(); ++coit)
	{
		code= *coit;
		if(	code != "###NULL::" &&
			code != "###BREAK::"		)
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

	pSwitch= folder->getSwitch("wait_back_time");
	pSwitch->description("whether folder wait for time to show back the default value");
	pSwitch->flush();
	createSubroutineLink(file, pSwitch->getName(), remote, remit->second, "correct_group");
	pSwitch->pperm(userreadperm);
	pSwitch->description();

	pValue= folder->getValue("back_time");
	pValue->description("how long the time after last pressed should measured");
	pValue->flush();
	createSubroutineLink(file, pValue->getName(), remote, remit->second, "correct_group");
	pValue->pperm(writeperm);
	pValue->description();

	pSwitch= folder->getSwitch("set_steps");
	pSwitch->description("whether should set actual step only to an number");
	pSwitch->flush();
	createSubroutineLink(file, pSwitch->getName(), remote, remit->second, "correct_group");
	pSwitch->pperm(writeperm);
	pSwitch->description();

	pValue= folder->getValue("digits");
	pValue->description("how many presses of numbers shuld finish setting");
	pValue->flush();
	createSubroutineLink(file, pValue->getName(), remote, remit->second, "correct_group");
	pValue->action("int");
	pValue->pperm(writeperm);
	pValue->description();

	pValue= folder->getValue("to_value");
	pValue->description("to which value actual step should be set when subroutine set_steps is activated");
	pValue->flush();
	createSubroutineLink(file, pValue->getName(), remote, remit->second, "correct_group");
	pValue->action("int");
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
		if(	code != "###NULL::" &&
			code != "###BREAK::"		)
		{
			if(code.find(":") == string::npos)
			{
				foundRemouteAlias= r.remotealias.find(remote);
				org_remote= foundRemouteAlias->second;
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
				string newremote;
				vector<string> sp;
				map<string, string>::const_iterator remotes;

				split(sp, code , is_any_of(":"));
				code= sp[1];
				remotes= r.remotealias.find(sp[0]);
				if(remotes != r.remotealias.end())
					org_remote= remotes->second;
				else
					org_remote= sp[0];
				foundCodeAliasMap= r.rcodealias.find(sp[0]);
				foundCodeAlias= foundCodeAliasMap->second.find(sp[1]);
				if(foundCodeAlias != foundCodeAliasMap->second.end())
					org_code= foundCodeAlias->second;
				else
					org_code= sp[1];
				displayCodeMap= r.dcodes.find(sp[0]);
				foundCodeAlias= displayCodeMap->second.find(sp[1]);
				if(foundCodeAlias != displayCodeMap->second.end())
					display_name= foundCodeAlias->second;
				else
					display_name= sp[0];
			}
			++correct_group;
			tdefault= getDefaults(remote, org_code);

			// define to which group set link as default
			count= 1;
			linkcount= 0;
			for(vector<string>::const_iterator c= remit->second.begin(); c != remit->second.end(); ++c)
			{
				searchcode= *c;
				if(	searchcode != "###NULL::" &&
					searchcode != "###BREAK::"		)
				{
					if(searchcode.find(":") != string::npos)
					{// code is an linked subroutine
						string newremote;
						vector<string> sp;
						map<string, string>::const_iterator remotes;

						split(sp, searchcode , is_any_of(":"));
						searchcode= sp[1];
					}
					if(searchcode == tdefault.group)
					{
						linkcount= count;
						break;
					}
					++count;
				}
			}
			if(linkcount == 0)
				linkcount= correct_group;

#if 0
			// debug output for which folder will be created
			cout << "  create folder " << remote << "_" << code << endl;
			cout << "                  remote: " << remote << endl;
			cout << "                    code: " << code << endl;
			cout << "         original remote: " << org_remote << endl;
			cout << "         original   code: " << org_code << endl;
			cout << "       display as button: " << display_name << endl;
			cout << "                   with button number " << correct_group << endl;
			cout << "                       link to button " << tdefault.group << " as number " << linkcount << " by default" << endl;
			cout << "                          should have " << tdefault.steps << " steps by default" << endl;
			cout << "            	pre-defined for direction '";
			switch(tdefault.direction)
			{
			case LircSupport::UP_STOP:
				cout << "UP_STOP" << "'" << endl;
				break;
			case LircSupport::DOWN_STOP:
				cout << "DOWN_STOP" << "'" << endl;
				break;
			case LircSupport::UP_LOOP:
				cout << "UP_LOOP" << "'" << endl;
				break;
			case LircSupport::DOWN_LOOP:
				cout << "DOWN_LOOP" << "'" << endl;
				break;
			};
			cout << "                                   to sending for ";
			switch(tdefault.send)
			{
			case LircSupport::ONCE:
				cout << "only one step" << endl;
				break;
			case LircSupport::SEND:
				cout << "longer duration" << endl;
				break;
			}
			if(tdefault.digits)
			{
				cout << "                    set only to value " << tdefault.setto << endl;
				cout << "                                  for " << tdefault.digits << " digits" << endl;
			}
			cout << endl;
#endif

			folder= auto_ptr<Folder>(new Folder(file, remote + "_" + code));
			folder->description("propteries for button '" + display_name + "'");
			folder->flush();
			folder->description("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
			folder->description();

			pValue= folder->getValue("correct_group");
			pValue->description("unique group number of button");
			pValue->pdefault(static_cast<double>(correct_group));
			pValue->pperm(readperm);
			pValue->description();
			pValue->description();
			pValue->description();

			folder->description("-------------------------------------------------------------------------------------------------");
			folder->description("--------------------------  begin of read sending from client  ----------------------------------");
			pValue= folder->getValue("count_steps");
			pValue->description("how much counts the button folder should count");
			pValue->pmin(0);
			pValue->action("int");
			pValue->pperm(userwriteperm);
			pValue->description();

			pValue= folder->getValue("count_steps_do");
			pValue->description("how much counts the button folder should count, added from count_steps");
			pValue->pmin(0);
			pValue->action("int");
			pValue->pperm(userwriteperm);
			pValue->description();

			pSwitch= folder->getSwitch("count");
			pSwitch->description("button for client to send signal over transmitter");
			pSwitch->pperm(userwriteperm);
			pSwitch->description();

			pSwitch= folder->getSwitch("button");
			pSwitch->description("button should be the same for hole folder");
			pSwitch->pwhile("count");
			pSwitch->description();

			pSet= folder->getSet("count_steps_higher");
			pSet->description("add count_steps to count_steps_do when any filled in");
			pSet->description("set count_steps back to 0");
			pSet->description("and set button to 1 by action SEND or varios to 1 or 0 by action SEND_ONCE");
			pSet->pfrom("count_steps_do + count_steps");
			pSet->pfrom(0);
			pSet->pfrom("transmit_action | count=0 ? 1 : 0");
			pSet->pset("count_steps_do");
			pSet->pset("count_steps");
			pSet->pset("button");
			pSet->pwhile("count_steps | count_steps_do");
			pSet->description();

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
			pValue->pperm(userreadperm);
			pValue->description();

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
			pSwitch->description();

			pSwitch= folder->getSwitch("active");
			pSwitch->description("whether folder is active running by get signal from any client or receiver");
			pSwitch->pbegin("receive");
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
			pValue->pdefault(linkcount);
			pValue->action("int | db");
			pValue->pperm(writeperm);
			pValue->description();

			pValue= folder->getValue("steps");
			pValue->description("count of exist steps");
			pValue->pmin(0);
			pValue->pmax(200);
			createSubroutineLink(file, pValue->getName(), remote, remit->second, "group");
			pValue->pdefault(tdefault.steps);
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
			pValue->pdefault(tdefault.send);
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
			pValue->pdefault(tdefault.direction);
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
			pTimer->pdefault(2.5);
			pTimer->action("db | measure | micro");
			pTimer->pperm(writeperm);
			pTimer->description();

			pSwitch= folder->getSwitch("set_steps");
			pSwitch->description("whether should set actual step only to an number");
			if(tdefault.digits)
				pSwitch->pdefault(1);
			else
				pSwitch->pdefault(0);
			pSwitch->action("db");
			pSwitch->pperm(writeperm);
			pSwitch->description();

			pValue= folder->getValue("digits");
			pValue->description("how many presses of numbers should finish setting");
			pValue->flush();
			createSubroutineLink(file, pValue->getName(), remote, remit->second, "group");
			pValue->pmin(1);
			pValue->pmax(5);
			if(tdefault.digits == 0)
				tdefault.digits= 1;
			pValue->pdefault(tdefault.digits);
			pValue->action("int | db");
			pValue->pperm(writeperm);
			pValue->description();

			pValue= folder->getValue("to_value");
			pValue->description("to which value actual step should be set when subroutine set_steps is activated");
			pValue->pdefault(tdefault.setto);
			pValue->action("int | db");
			pValue->pperm(writeperm);
			folder->description("---------------------  end properties can be changed from client  -------------------------------");
			folder->description("#################################################################################################");
			folder->description();

			if(transmit)
			{
				folder->description();
				folder->description();
				folder->description("-------------------------------------------------------------------------------------------------");
				folder->description("----------------------  begin of sending signal over transmitter  -------------------------------");
				pLirc= folder->getLirc("send_once");
				pLirc->description("send only one signal over transmitter");
				pLirc->premote(org_remote);
				pLirc->pcode(org_code);
				pLirc->pvalue(1);
				pLirc->pwhile("what=0 & first_touch & (transmit_action=0 | transmit_action=3)");
				pLirc->action("send_once");
				pLirc->description();

				pLirc= folder->getLirc("send_onoff");
				pLirc->description("send signal over transmitter for longer time");
				pLirc->premote(org_remote);
				pLirc->pcode(org_code);
				pLirc->pvalue("button");
				pLirc->pwhile("((button & send_onoff=0) | (button=0 & send_onoff)) & (transmit_action=1 | transmit_action=2)");
				pLirc->action("send");
				folder->description("----------------------  end of sending signal over transmitter  ---------------------------------");
				folder->description("#################################################################################################");
				folder->description();
			}

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
			pTimer->pbegin("(display_first & first_off) | (set_steps & digits>1 & first_touch)");
			pTimer->pend("new_activate<=0");
			createSubroutineLink(file, pTimer->getName(), remote, remit->second, "group");
			pTimer->action("micro");
			pTimer->description();

			pSwitch= folder->getSwitch("wait_back_time");
			pSwitch->description("whether folder wait for time to show back the default value");
			pSwitch->flush();
			//createSubroutineLink(file, pSwitch->getName(), remote, remit->second, "group");
			pSwitch->pwhile("new_activate>0");
			pSwitch->pperm(userreadperm);
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

			pValue= folder->getValue("predef_step");
			pValue->description("define value of actual step before when folder button be defined to 'set only actual step'");
			pValue->flush();
			createSubroutineLink(file, pValue->getName(), remote, remit->second, "group");
			pValue->pwhile("active & set_steps & first_touch ? ( predef_step=-1 ? to_value : predef_step*10+to_value) : predef_step");
			pValue->action("int");
			pValue->pdefault("-1");
			pValue->pperm(userreadperm);
			pValue->description();

			pValue= folder->getValue("digs_set");
			pValue->description("calculate how much digits are set for current session");
			pValue->flush();
			createSubroutineLink(file, pValue->getName(), remote, remit->second, "group");
			pValue->pwhile("active & set_steps & first_touch ? digs_set + 1 : digs_set");
			pValue->pmin(0);
			pValue->description();

			pValue= folder->getValue("actual_step_before");
			pValue->description("actual step before changing to know whether actual step was changed");
			pValue->pwhile("actual_step");
			pValue->description();

			pValue= folder->getValue("actual_step");
			pValue->description("count of actual step");
			pValue->pmin(0);
			pValue->pmax(500);
			pValue->pdefault(0);
			pValue->pvalue("predef_step");
			pValue->pvalue("actual_step");
			pValue->pvalue("actual_step >= steps ? steps : actual_step + 1");
			pValue->pvalue("actual_step <= 0 ? 0 : actual_step - 1");
			pValue->pvalue("actual_step >= steps ? 0 : actual_step + 1");
			pValue->pvalue("actual_step <= 0 ? steps : actual_step - 1");
			pValue->pwhile("\"set_steps ? ( digs_set=digits | (predef_step!=-1 & new_activate<=0) ? 0 : 1 ) :\n"
							"                     (    display_first=0 &\n"
							"                          first_touch                ) |\n"
							"                     (    what=0 &\n"
							"                          (    transmit_action=1 &\n"
							"                               receive &\n"
							"                               first_calc=0 &\n"
							"                               (    first_touch |\n"
							"                                    wait_after=0 |\n"
							"                                    wait_after=after    )    )    ) |\n"
							"                     (    what=1 &\n"
							"                          (    transmit_action=1 &\n"
							"                               first_calc=0 &\n"
							"                               lirc_set                 )    )         ? steps_action + 2 : 1\"");
			createSubroutineLink(file, pValue->getName(), remote, remit->second, "group");
			pValue->action("int | db");
			pValue->pperm(ureadcw);
			pValue->description();

			pSet= folder->getSet("count_step_done");
			pSet->description("Decrease count steps when one step was counted");
			pSet->pfrom("count_steps -1");
			pSet->pset("count_steps");
			pSet->pwhile("count_steps & ((actual_step != actual_step_before) | (steps=0 & first_touch))");
			pSet->description();

			pSet= folder->getSet("again");
			pSet->description("wait double by again pressing");
			pSet->pfrom("display_first * 2");
			pSet->pset("first_calc");
			pSet->pwhile("display_first & double & first_touch & first_calc=0");
			pSet->description();

			pSet= folder->getSet("predef_back");
			pSet->description("set back predefined steps (predef_step) to -1 when defined in actual_step");
			pSet->pfrom(-1);
			pSet->pfrom(0);
			pSet->pset("predef_step");
			pSet->pset("digs_set");
			pSet->pwhile("active & (digs_set=digits | new_activate<=0)");
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
		if(	code != "###NULL::" &&
			code != "###BREAK::"		)
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
			if(	code != "###NULL::" &&
				code != "###BREAK::"		)
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

LircSupport::t_remote LircSupport::getLinkDefinition(const string& code, const remotecodes_t& r) const
{
	t_remote tRv, nullRv;
	vector<string> sp;
	map<string, string>::const_iterator remotes;
	map<string, string>::const_iterator codes;
	map<string, string>::const_iterator displaycodes;
	map<string, map<string, string> >::const_iterator displayCodeMap;
	map<string, map<string, string> >::const_iterator CodeAliasMap;

	split(sp, code , is_any_of(":"));
	if(sp.size() != 2)
		return nullRv;
	tRv.org_remote= sp[0];
	for(remotes= r.remotealias.begin(); remotes != r.remotealias.end(); ++remotes)
	{
		if(remotes->second == sp[0])
		{
			tRv.alias_remote= remotes->first;
			break;
		}
	}
	if(remotes == r.remotealias.end())
		return nullRv;
	tRv.org_code= sp[1];
	CodeAliasMap= r.rcodealias.find(tRv.alias_remote);
	for(codes= CodeAliasMap->second.begin(); codes != CodeAliasMap->second.end(); ++codes)
	{
		if(codes->second == sp[1])
		{
			tRv.alias_code= codes->first;
			break;
		}
	}
	if(codes == CodeAliasMap->second.end())
		return nullRv;
	displayCodeMap= r.dcodes.find(tRv.alias_remote);
	displaycodes= displayCodeMap->second.find(tRv.alias_code);
	if(displaycodes != displayCodeMap->second.end())
		tRv.display_code= displaycodes->second;
	else
		tRv.display_code= tRv.org_code;
	return tRv;
}

void LircSupport::searchPreDefined(const remotecodes_t& r)
{
	short count;
	string remote, code, org_code;
	vector<string>::const_iterator coit;
	map<string, vector<string> >::const_iterator remit;
	map<string, string>::const_iterator foundCodeAlias;
	map<string, map<string, string> >::const_iterator foundCodeAliasMap;

	for(remit= r.remotes.begin(); remit != r.remotes.end(); ++remit)
	{
		set_t defined;

		remote= remit->first;
		if(r.kill.find(remote) == r.kill.end())
		{
			for(coit= remit->second.begin(); coit != remit->second.end(); ++coit)
			{
				code= *coit;
				if(	code != "###NULL::" &&
					code != "###BREAK::"		)
				{
					if(code.find(":") == string::npos)
					{
						foundCodeAliasMap= r.rcodealias.find(remote);
						foundCodeAlias= foundCodeAliasMap->second.find(code);
						if(foundCodeAlias != foundCodeAliasMap->second.end())
							org_code= foundCodeAlias->second;
						else
							org_code= code;
					}else
					{// code is an linked subroutine
						vector<string> sp;

						split(sp, code , is_any_of(":"));
						foundCodeAliasMap= r.rcodealias.find(sp[0]);
						foundCodeAlias= foundCodeAliasMap->second.find(sp[1]);
						if(foundCodeAlias != foundCodeAliasMap->second.end())
							org_code= foundCodeAlias->second;
						else
							org_code= sp[1];
					}
					if(org_code == "KEY_CHANNELUP")
						defined.KEY_CHANNELUP= true;
					else if(org_code == "KEY_NEXT")
						defined.KEY_NEXT= true;
					else if(org_code == "KEY_PAGEUP")
						defined.KEY_PAGEUP= true;
					else if(org_code == "KEY_SCROLLUP")
						defined.KEY_SCROLLUP= true;
					else if(org_code == "KEY_VOLUMEUP")
						defined.KEY_VOLUMEUP= true;
					else if(org_code == "KEY_BRIGHTNESSUP")
						defined.KEY_BRIGHTNESSUP= true;
					else if(org_code == "KEY_KBDILLUMUP")
						defined.KEY_KBDILLUMUP= true;
					else if(org_code == "BTN_GEAR_UP")
						defined.BTN_GEAR_UP= true;
					else if(org_code == "KEY_LEFT")
						defined.KEY_LEFT= true;
					else if(org_code == "KEY_LEFTSHIFT")
						defined.KEY_LEFTSHIFT= true;
					else if(org_code == "KEY_LEFTALT")
						defined.KEY_LEFTALT= true;
					else if(org_code == "KEY_LEFTBRACE")
						defined.KEY_LEFTBRACE= true;
					else if(org_code == "KEY_LEFTCTRL")
						defined.KEY_LEFTCTRL= true;
					else if(org_code == "KEY_LEFTMETA")
						defined.KEY_LEFTMETA= true;
					else if(org_code == "BTN_LEFT")
						defined.BTN_LEFT= true;
					else if(org_code == "KEY_ZOOMIN")
						defined.KEY_ZOOMIN= true;
					else if(org_code == "KEY_FORWARD")
						defined.KEY_FORWARD= true;
					else
					{
						bool bOk(false);
						size_t digs;

						digs= 4;
						if(!bOk && org_code.substr(0, digs) == "BTN_")
						{
							bool bbOk(true);
							ostringstream is;
							istringstream code(org_code.substr(digs));

							if(org_code == "BTN_A")
								count= 10;
							else if(org_code == "BTN_B")
								count= 11;
							else if(org_code == "BTN_C")
								count= 12;
							else
							{
								code >> count;
								is << org_code.substr(0, digs) << count;
								if(org_code != is.str())
									bbOk= false;
							}
							if(bbOk)
							{
								if(defined.BTN > count)
									defined.BTN= count;
								bOk= true;
							}
						}
						digs= 6;
						if(!bOk && org_code.substr(0, digs) == "KEY_KP")
						{
							ostringstream is;
							istringstream code(org_code.substr(digs));

							code >> count;
							is << org_code.substr(0, digs) << count;
							if(org_code == is.str())
							{
								if(defined.KEY_KP > count)
									defined.KEY_KP= count;
								bOk= true;
							}

						}
						digs= 11;
						if(!bOk && org_code.substr(0, digs) == "KEY_BRL_DOT")
						{
							ostringstream is;
							istringstream code(org_code.substr(digs));

							code >> count;
							is << org_code.substr(0, digs) << count;
							if(org_code == is.str())
							{
								if(defined.KEY_BRL_DOT > count)
									defined.KEY_BRL_DOT= count;
								bOk= true;
							}

						}
						digs= 8;
						if(!bOk && org_code.substr(0, digs) == "KEY_FN_F")
						{
							ostringstream is;
							istringstream code(org_code.substr(digs));

							count= 0;
							code >> count;
							is << org_code.substr(0, digs) << count;
							if(	org_code == "KEY_FN_F" ||
								org_code == is.str()		)
							{
								if(defined.KEY_FN_F > count)
									defined.KEY_FN_F= count;
								bOk= true;
							}

						}
						digs= 5;
						if(!bOk && org_code.substr(0, digs) == "KEY_F")
						{
							ostringstream is;
							istringstream code(org_code.substr(digs));

							count= 0;
							code >> count;
							is << org_code.substr(0, digs) << count;
							if(	org_code == "KEY_F" ||
								org_code == is.str())
							{
								if(defined.KEY_F > count)
									defined.KEY_F= count;
								bOk= true;
							}

						}
						digs= 13;
						if(!bOk && org_code.substr(0, digs) == "KEY_NUMMERIC_")
						{
							ostringstream is;
							istringstream code(org_code.substr(digs));

							code >> count;
							is << org_code.substr(0, digs) << count;
							if(org_code == is.str())
							{
								if(defined.KEY_NUMMERIC > count)
									defined.KEY_NUMMERIC= count;
								bOk= true;
							}

						}
						digs= 4;
						if(!bOk && org_code.substr(0, digs) == "KEY_")
						{
							ostringstream is;
							istringstream code(org_code.substr(digs));

							code >> count;
							is << org_code.substr(0, digs) << count;
							if(org_code == is.str())
							{
								if(defined.KEY > count)
									defined.KEY= count;
								bOk= true;
							}

						}
					}// else single keys
				}// if(code != "###NULL::")
			}// for(remit->second)
		}// if not in r.kill
		m_mtPreDefined[remote]= defined;
	}// for(r.remotes)
}

LircSupport::default_t LircSupport::getDefaults(const string& remote, const string& code) const
{
	map<string, set_t>::const_iterator defined;
	default_t tRv;

	// default settings
	tRv.group= code;
	tRv.steps= 1;
	tRv.direction= UP_LOOP;
	tRv.send= ONCE;
	tRv.digits= 0;
	tRv.setto= 0;
	defined= m_mtPreDefined.find(remote);
	if(defined == m_mtPreDefined.end())
		return tRv;

	if(code == "KEY_CHANNELUP")
	{
		//tRv.group is set to own
		tRv.steps= 99;
		tRv.direction= UP_LOOP;
		tRv.send= SEND;
		if(defined->second.KEY < 20)
			tRv.digits= 2;

	}else if(code == "KEY_CHANNELDOWN")
	{
		if(defined->second.KEY_CHANNELUP)
			tRv.group= "KEY_CHANNELUP";
		tRv.steps= 99;
		tRv.direction= DOWN_LOOP;
		tRv.send= SEND;

	}else if(code == "KEY_NEXT")
	{
		//tRv.group is set to own
		tRv.steps= 9;
		tRv.direction= UP_STOP;
		tRv.send= SEND;
		if(defined->second.KEY_NUMMERIC < 20)
			tRv.digits= 2;

	}else if(code == "KEY_PREVIOUS")
	{
		if(defined->second.KEY_NEXT)
			tRv.group= "KEY_NEXT";
		tRv.steps= 9;
		tRv.direction= DOWN_STOP;
		tRv.send= SEND;

	}else if(code == "KEY_PAGEUP")
	{
		//tRv.group is set to own
		tRv.steps= 10;
		tRv.direction= UP_STOP;
		tRv.send= ONCE;

	}else if(code == "KEY_PAGEDOWN")
	{
		if(defined->second.KEY_PAGEUP)
			tRv.group= "KEY_PAGEUP";
		tRv.steps= 10;
		tRv.direction= DOWN_STOP;
		tRv.send= ONCE;

	}else if(code == "KEY_SCROLLUP")
	{
		//tRv.group is set to own
		tRv.steps= 100;
		tRv.direction= UP_STOP;
		tRv.send= SEND;

	}else if(code == "KEY_SCROLLDOWN")
	{
		if(defined->second.KEY_SCROLLUP)
			tRv.group= "KEY_SCROLLUP";
		tRv.steps= 100;
		tRv.direction= DOWN_STOP;
		tRv.send= SEND;

	}else if(code == "KEY_VOLUMEUP")
	{
		//tRv.group is set to own
		tRv.steps= 30;
		tRv.direction= UP_STOP;
		tRv.send= SEND;

	}else if(code == "KEY_VOLUMEDOWN" )
	{
		if(defined->second.KEY_VOLUMEUP)
			tRv.group= "KEY_VOLUMEUP";
		tRv.steps= 30;
		tRv.direction= DOWN_STOP;
		tRv.send= SEND;

	}else if(code == "KEY_BRIGHTNESSUP")
	{
		//tRv.group is set to own
		tRv.steps= 30;
		tRv.direction= UP_STOP;
		tRv.send= SEND;

	}else if(code == "KEY_BRIGHTNESSDOWN" )
	{
		if(defined->second.KEY_BRIGHTNESSUP)
			tRv.group= "KEY_BRIGHTNESSUP";
		tRv.steps= 30;
		tRv.direction= DOWN_STOP;
		tRv.send= SEND;

	}else if(code == "KEY_KBDILLUMUP")
	{
		//tRv.group is set to own
		tRv.steps= 30;
		tRv.direction= UP_STOP;
		tRv.send= SEND;

	}else if(code == "KEY_KBDILLUMDOWN")
	{
		if(defined->second.KEY_KBDILLUMUP)
			tRv.group= "KEY_KBDILLUMUP";
		tRv.steps= 30;
		tRv.direction= DOWN_STOP;
		tRv.send= SEND;

	}else if(code == "BTN_GEAR_UP")
	{
		//tRv.group is set to own
		tRv.steps= 30;
		tRv.direction= UP_STOP;
		tRv.send= ONCE;

	}else if(code == "BTN_GEAR_DOWN")
	{
		if(defined->second.BTN_GEAR_UP)
			tRv.group= "BTN_GEAR_UP";
		tRv.steps= 30;
		tRv.direction= DOWN_STOP;
		tRv.send= ONCE;

	}else if(code == "KEY_LEFT")
	{
		//tRv.group is set to own
		tRv.steps= 30;
		tRv.direction= DOWN_STOP;
		tRv.send= ONCE;

	}else if(code == "KEY_RIGHT")
	{
		if(defined->second.KEY_LEFT)
			tRv.group= "KEY_LEFT";
		tRv.steps= 30;
		tRv.direction= UP_STOP;
		tRv.send= ONCE;

	}else if(code == "KEY_LEFTSHIFT")
	{
		//tRv.group is set to own
		tRv.steps= 30;
		tRv.direction= DOWN_STOP;
		tRv.send= ONCE;

	}else if(code == "KEY_RIGHTSHIFT")
	{
		if(defined->second.KEY_LEFTSHIFT)
			tRv.group= "KEY_LEFTSHIFT";
		tRv.steps= 30;
		tRv.direction= UP_STOP;
		tRv.send= ONCE;

	}else if(code == "KEY_LEFTALT")
	{
		//tRv.group is set to own
		tRv.steps= 30;
		tRv.direction= DOWN_STOP;
		tRv.send= ONCE;

	}else if(code == "KEY_RIGHTALT")
	{
		if(defined->second.KEY_LEFTALT)
			tRv.group= "KEY_LEFTALT";
		tRv.steps= 30;
		tRv.direction= UP_STOP;
		tRv.send= ONCE;

	}else if(code == "KEY_LEFTBRACE")
	{
		//tRv.group is set to own
		tRv.steps= 30;
		tRv.direction= DOWN_STOP;
		tRv.send= ONCE;

	}else if(code == "KEY_RIGHTBRACE")
	{
		if(defined->second.KEY_LEFTBRACE)
			tRv.group= "KEY_LEFTBRACE";
		tRv.steps= 30;
		tRv.direction= UP_STOP;
		tRv.send= ONCE;

	}else if(code == "KEY_LEFTCTRL")
	{
		//tRv.group is set to own
		tRv.steps= 30;
		tRv.direction= DOWN_STOP;
		tRv.send= ONCE;

	}else if(code == "KEY_RIGHTCTRL")
	{
		if(defined->second.KEY_LEFTCTRL)
			tRv.group= "KEY_LEFTCTRL";
		tRv.steps= 30;
		tRv.direction= UP_STOP;
		tRv.send= ONCE;

	}else if(code == "KEY_LEFTMETA")
	{
		//tRv.group is set to own
		tRv.steps= 30;
		tRv.direction= DOWN_STOP;
		tRv.send= ONCE;

	}else if(code == "KEY_RIGHTMETA")
	{
		if(defined->second.KEY_LEFTMETA)
			tRv.group= "KEY_LEFTMETA";
		tRv.steps= 30;
		tRv.direction= UP_STOP;
		tRv.send= ONCE;

	}else if(code == "BTN_LEFT")
	{
		//tRv.group is set to own
		tRv.steps= 30;
		tRv.direction= DOWN_STOP;
		tRv.send= ONCE;

	}else if(code == "BTN_RIGHT")
	{
		if(defined->second.BTN_LEFT)
			tRv.group= "BTN_LEFT";
		tRv.steps= 30;
		tRv.direction= UP_STOP;
		tRv.send= ONCE;

	}else if(code == "KEY_ZOOMIN")
	{
		//tRv.group is set to own
		tRv.steps= 50;
		tRv.direction= UP_STOP;
		tRv.send= SEND;

	}else if(code == "KEY_ZOOMOUT")
	{
		if(defined->second.KEY_ZOOMIN)
			tRv.group= "KEY_ZOOMIN";
		tRv.steps= 50;
		tRv.direction= DOWN_STOP;
		tRv.send= SEND;

	}else if(code == "KEY_FORWARD")
	{
		//tRv.group is set to own
		tRv.steps= 50;
		tRv.direction= UP_STOP;
		tRv.send= SEND;

	}else if(code == "KEY_REWIND")
	{
		if(defined->second.KEY_FORWARD)
			tRv.group= "KEY_FORWARD";
		tRv.steps= 50;
		tRv.direction= DOWN_STOP;
		tRv.send= SEND;

	}else
	{
		bool bOk(false);
		size_t digs;
		short count;

		digs= 4;
		if(!bOk && code.substr(0, digs) == "BTN_")
		{
			ostringstream is;
			istringstream cstream(code.substr(digs));

			is << code.substr(0, digs);
			if(code == "BTN_A")
			{
				count= 10;
				is << "A";

			}else if(code == "BTN_B")
			{
				count= 11;
				is << "B";

			}else if(code == "BTN_C")
			{
				count= 12;
				is << "C";

			}else
			{
				cstream >> count;
				is << count;
			}
			if(code == is.str())
			{
				ostringstream group;

				group << code.substr(0, digs) << defined->second.BTN;
				tRv.group= group.str();
				tRv.send= ONCE;
				tRv.digits= 1;
				tRv.setto= count;
				bOk= true;
			}
		}
		digs= 6;
		if(!bOk && code.substr(0, digs) == "KEY_KP")
		{
			ostringstream is;
			istringstream cstream(code.substr(digs));

			cstream >> count;
			is << code.substr(0, digs) << count;
			if(code == is.str())
			{
				ostringstream group;

				group << code.substr(0, digs) << defined->second.KEY_KP;
				tRv.group= group.str();
				tRv.send= ONCE;
				tRv.digits= 1;
				tRv.setto= count;
				bOk= true;
			}
		}
		digs= 11;
		if(!bOk && code.substr(0, digs) == "KEY_BRL_DOT")
		{
			ostringstream is;
			istringstream cstream(code.substr(digs));

			cstream >> count;
			is << code.substr(0, digs) << count;
			if(code == is.str())
			{
				ostringstream group;

				group << code.substr(0, digs) << defined->second.KEY_BRL_DOT;
				tRv.group= group.str();
				tRv.send= ONCE;
				tRv.digits= 1;
				tRv.setto= count;
				bOk= true;
			}
		}
		digs= 8;
		if(!bOk && code.substr(0, digs) == "KEY_FN_F")
		{
			ostringstream is;
			istringstream cstream(code.substr(digs));

			is << code.substr(0, digs);
			if(code != "KEY_FN_F")
			{
				cstream >> count;
				is <<  count;
			}else
				count= 0;
			if(code == is.str())
			{
				ostringstream group;

				group << code.substr(0, digs) << defined->second.KEY_FN_F;
				tRv.group= group.str();
				tRv.send= ONCE;
				tRv.digits= 1;
				tRv.setto= count;
				bOk= true;
			}
		}
		digs= 5;
		if(!bOk && code.substr(0, digs) == "KEY_F")
		{
			ostringstream is;
			istringstream cstream(code.substr(digs));

			is << code.substr(0, digs);
			if(code != "KEY_F")
			{
				cstream >> count;
				is <<  count;
			}else
				count= 0;
			if(code == is.str())
			{
				ostringstream group;

				group << code.substr(0, digs) << defined->second.KEY_F;
				tRv.group= group.str();
				tRv.send= ONCE;
				tRv.digits= 1;
				tRv.setto= count;
				bOk= true;
			}
		}
		digs= 13;
		if(!bOk && code.substr(0, digs) == "KEY_NUMMERIC_")
		{
			ostringstream is;
			istringstream cstream(code.substr(digs));

			cstream >> count;
			is << code.substr(0, digs) << count;
			if(code == is.str())
			{
				ostringstream group;

				if(!defined->second.KEY_NEXT)
				{
					group << code.substr(0, digs) << defined->second.KEY_NUMMERIC;
					tRv.group= group.str();

				}else
					tRv.group= "KEY_NEXT";
				tRv.send= ONCE;
				tRv.digits= 2;
				tRv.setto= count;
				bOk= true;
			}
		}
		digs= 4;
		if(!bOk && code.substr(0, digs) == "KEY_")
		{
			ostringstream is;
			istringstream cstream(code.substr(digs));

			cstream >> count;
			is << code.substr(0, digs) << count;
			if(code == is.str())
			{
				ostringstream group;

				if(!defined->second.KEY_CHANNELUP)
				{
					group << code.substr(0, digs) << defined->second.KEY;
					tRv.group= group.str();

				}else
					tRv.group= "KEY_CHANNELUP";
				tRv.send= ONCE;
				tRv.digits= 2;
				tRv.setto= count;
				bOk= true;
			}
		}
	}// else single keys
	return tRv;
}








