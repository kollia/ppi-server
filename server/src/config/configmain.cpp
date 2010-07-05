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

#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <vector>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "../util/debug.h"
#include "../util/GlobalStaticMethods.h"
#include "../util/interlacedproperties.h"
#include "../util/URL.h"

using namespace std;
using namespace boost;
using namespace util;

void ussage(const bool full);
int lirc(int argc, char* argv[]);

int main(int argc, char* argv[])
{
	string workdir, param;
	string sConfPath, fileName;
	vector<string> directorys;
	vector<string>::size_type dirlen;
	Properties oServerProperties;

	glob::processName("ppi-mconfig");

	// create working directory
	directorys= split(directorys, argv[0], is_any_of("/"));
	dirlen= directorys.size();
	for(vector<string>::size_type c= 0; c < dirlen; ++c)
	{
		if(c == dirlen-2)
		{// directory is bin, Debug or Release
			if(directorys[c] == ".")
				workdir+= "../";
			break;
		}
		workdir+= directorys[c] + "/";
	}
	sConfPath= URL::addPath(workdir, PPICONFIGPATH, /*always*/false);
	fileName= URL::addPath(sConfPath, "server.conf");
	if(!oServerProperties.readFile(fileName))
	{
		cout << "### ERROR: cannot read '" << fileName << "'" << endl;
		exit(EXIT_FAILURE);
	}
	oServerProperties.readLine("workdir= " + workdir);


	for(short i= 1; i < argc; ++i)
	{
		param= argv[i];
		if(	param == "-?" ||
			param == "--help"	)
		{
			ussage(true);
			return EXIT_SUCCESS;

		}else if(param == "LIRC")
			return lirc(argc, argv);
	}
	ussage(false);
	return EXIT_FAILURE;
}

int lirc(int argc, char* argv[])
{
	bool readfile= false;
	bool receive= false;
	bool transmit= false;
	bool learn= false;
	bool breadperm= false;
	bool bwriteperm= false;
	bool bvertical= false;
	int vertical= 3;
	string param;
	string lircconf("/etc/lirc/lircd.conf");
	string readperm("read");
	string writeperm("change");

	for(short i= 2; i < argc; ++i)
	{
		param= argv[i];
		if(readfile)
		{
			lircconf= param;
			readfile= false;

		}else if(breadperm)
		{
			readperm= param;
			breadperm= false;

		}else if(bwriteperm)
		{
			writeperm= param;
			bwriteperm= false;

		}else if(bvertical)
		{
			istringstream overt(param);

			overt >> vertical;
			bvertical= false;

		}else if(	param == "-f" ||
					param == "--file")
		{
			readfile= true;

		}else if(	param == "-l" ||
					param == "--learn"	)
		{
			learn= true;

		}else if(	param == "-r" ||
					param == "--receiver"	)
		{
			receive= true;

		}else if(	param == "-t" ||
					param == "--transmitter"	)
		{
			transmit= true;

		}else if(	param == "-p" ||
					param == "--readpermission"	)
		{
			breadperm= true;

		}else if(	param == "-w" ||
					param == "--writepermission"	)
		{
			bwriteperm= true;

		}else if(	param == "-v" ||
					param == "--vertical"	)
		{
			bvertical= true;

		}else
		{
			cout << "unknown option '" << param << "' be set" << endl;
			cout << "type -? for help" << endl;
			return EXIT_FAILURE;
		}
	}
	if(	receive &&
		transmit	)
	{
		cout << "do not set option for transmitting and receiving" << endl;
		ussage(false);
		return EXIT_FAILURE;
	}
	if(	receive &&
		learn		)
	{
		cout << "option -l or --learn is only for transmitting" << endl;
		ussage(false);
		return EXIT_FAILURE;
	}
	if(learn)
		transmit= true;
	if(!transmit)
		receive= true;

	if(!learn)
	{
		InterlacedProperties oLirc;

		oLirc.setComment("#");
		//oLirc.setDoc("/*", "*/");
		oLirc.setUncomment("#!");
		oLirc.setDelimiter(" ");
		oLirc.modifier("begin", "remote");
		oLirc.modifier("begin", "codes");
		oLirc.allowLaterModifier(true);
		oLirc.readFile(lircconf);


		// read all permission groups for folder:subroutines in file measure.conf
		typedef vector<IInterlacedPropertyPattern*>::iterator sit;

		unsigned short count;
		string name, newname, code, newcode;
		string org_name, org_code;
		map<string, string> remotealias;
		map<string, map<string, string> > rcodealias;
		map<string, vector<string> > remotes;
		map<string, vector<string> >::iterator foundr;
		vector<string>::iterator foundc;
		vector<IInterlacedPropertyPattern*> rsection, csection;
		vector<string> kill;
		vector<string>::size_type uncommented, uncommentedNull;

		rsection= oLirc.getSections();
		for(sit vfit= rsection.begin(); vfit != rsection.end(); ++vfit)
		{
			map<string, string> codealias;

			csection= (*vfit)->getSections();
			org_name= (*vfit)->getValue("name");
			name= org_name;
			glob::replaceName(name);
			newname= name;
			foundr= remotes.find(newname);
			count= 0;
			while(foundr != remotes.end())
			{
				ostringstream oname;

				++count;
				oname << name << "[" << dec << count << "]";
				newname= oname.str();
				foundr= remotes.find(newname);
			}
			remotealias[newname]= org_name;
			name= newname;
			//cout << "read remote '" << name << "'" << endl;
			for(sit vsit= csection.begin(); vsit != csection.end(); ++vsit)
			{
				uncommented= 0;
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
							if(code == "###LINK")
							{
								string rem, co;
								vector<string> sp;

								//directorys= split(directorys, argv[0], is_any_of("/"));
								code= (*vsit)->getValue(code, uncommented, /*warning*/false);
								split(sp, code , is_any_of(":"));
								++uncommented;
								if(sp.size() == 2)
								{
									rem= sp[0];
									co= sp[1];
									glob::replaceName(rem);
									glob::replaceName(co);
									kill.push_back(rem);
									remotes[name].push_back(rem +":"+ co);
									//cout << "       with code '" << co << "' linked to remote '" << rem << "'" << endl;
								}else
									cerr << "### warning: undefined uncommented code '" << code << "' found in remoute '" << name << "'" << endl;

							}else if(code == "###NULL")
							{
								remotes[name].push_back(code + "::");
								//cout << "       NULL FIELD" << endl;
							}
						}else
						{
							org_code= code;
							glob::replaceName(code);
							newname= code;
							foundc= find(remotes[name].begin(), remotes[name].end(), newname);
							count= 0;
							while(foundc != remotes[name].end())
							{
								ostringstream oname;

								++count;
								oname << code << "[" << dec << count << "]";
								newname= oname.str();
								foundc= find(remotes[name].begin(), remotes[name].end(), newname);
							}
							codealias[newname]= org_code;
							code= newname;
							//cout << "       with code '" << code << "'" << endl;
							remotes[name].push_back(code);
						}
					}// if(readProp)
				}// nextProp()
			}// iterate over codes
			rcodealias[name]= codealias;
		}// iterate over remote

		cout << "### write all codes into lirc.conf for server definition" << endl;
		cout << "    write an file parameter into measure.conf to use this file" << endl;
		ofstream file;
		string org_remote, remote;
		map<string, string>::iterator foundRemouteAlias, foundCodeAlias;
		map<string, map<string, string> >::iterator foundCodeAliasMap, linkCodeAliasMap;
		int actrows= 0;

		file.open("lirc.conf", ios::out);
		if(!file.is_open())
		{
			cerr << "    cannot open lirc.conf to write" << endl;
			return EXIT_FAILURE;
		}

		file << "##################################################################################" << endl;
		file << "#" << endl;
		file << "#      ppi-server config file of measure routines for lirc" << endl;
		file << "#      writen by Alexander Kolli <ppi-server@magnificat.at>" << endl;
		file << "#" << endl;
		file << "#" << endl;
		file << "#  this configuration file lirc.conf have same structure of folder and subroutines" << endl;
		file << "#  like measure.conf. If you use any receiver or transmitter this is an example" << endl;
		file << "#  to receive and or transmit signals over lircd." << endl;
		file << "#  Copy the file lirc.desktop from the same directory" << endl;
		file << "#  into the ppi-client folder, to see the result in the java-client." << endl;
		file << "#  An description to use lirc on command line or better understanding, reading http://www.lirc.org" << endl;
		file << "#" << endl;
		file << "#" << endl;
		file << "#   TYPES:          properties and actions:" << endl;
		file << "#	LIRC		properties: <ID>, <pin>, [priority], [cache], <begin|while|end>" << endl;
		file << "#			actions= <receive|send|send_once>, [db]" << endl;
		file << "#	    [access to an chip with the unique address from maxim on property <id>	]" << endl;
		file << "#" << endl;
		file << "#   PROPERTIES:" << endl;
		file << "#	ID		- name of remote group in /etc/lirc/lircd.conf after begin remote" << endl;
		file << "#	pin		- receiving/transmitting code betwenn begin end codes" << endl;
		file << "#	priority	- priority of writing" << endl;
		file << "#			  all pins are holding in an queue to write in serie." << endl;
		file << "#			  if in the queue more than one chips/pins" << endl;
		file << "#			  higher prioritys will be concerned of writing before the other" << endl;
		file << "#			  (allowed priority 1 (highest) to priority 9999 (lowest))" << endl;
		file << "#	begin		- do writing if begin status occurring" << endl;
		file << "#			  the value can be an defined-value" << endl;
		file << "#	while		- do writing while state be set" << endl;
		file << "#			  the value can be an defined-value" << endl;
		file << "#	end		- set pin to 0 if expression be correct" << endl;
		file << "#			  the value can be an defined-value" << endl;
		file << "#" << endl;
		file << "#    ACTIONS:" << endl;
		file << "#	receive		- receiving code from device" << endl;
		file << "#	send_once	- sending one signal over device" << endl;
		file << "#	send		- sending start and end signal over device" << endl;
		file << "#	db	- writing actual state on database" << endl;
		file << "#" << endl;
		file << "#" << endl;
		file << "###################################################################################" << endl;
		file << endl;
		file << endl;

		file << "# this under comments should be templates to copy in your own measure.conf" << endl;
		file << "# All subroutines ending with _receive you can use in subroutines witch has begin, while or end parameter" << endl;
		if(transmit)
		{
			file << "#     subroutines ending with _send_once use in SET subroutines to send one signal to case or the measuered time in GUI" << endl;
			file << "#     by ending with _send begin to sending signal. Don't forget to SEt this subroutine to 0 for stopping" << endl;
		}
		file << "#" << endl;
		for(map<string, vector<string> >::iterator remit= remotes.begin(); remit != remotes.end(); ++remit)
		{
			remote= remit->first;
			foundRemouteAlias= remotealias.find(remote);
			org_remote= foundRemouteAlias->second;
			file << "# for remote " << org_remote << endl;
			foundCodeAliasMap= rcodealias.find(remote);
			for(vector<string>::iterator coit= remit->second.begin(); coit != remit->second.end(); ++coit)
			{
				code= *coit;
				if(code.find(":") >= code.length())
				{
					foundCodeAlias= foundCodeAliasMap->second.find(code);
					org_code= foundCodeAlias->second;
					file << "#        code " << org_code << endl;
					file << "#                       " << remote << ":" << code << "_receive" << endl;
					if(transmit)
					{
						file << "#                       " << remote << ":" << code << "_send_once" << endl;
						file << "#                       " << remote << ":" << code << "_send" << endl;
					}
				}
			}
			file << "#" << endl;
		}

		file << endl;
		for(map<string, vector<string> >::iterator remit= remotes.begin(); remit != remotes.end(); ++remit)
		{
			remote= remit->first;
			foundRemouteAlias= remotealias.find(remote);
			org_remote= foundRemouteAlias->second;
			foundCodeAliasMap= rcodealias.find(remote);
			file << endl;
			file << endl;
			file << "# remote " << org_remote << endl;
			file << "folder= " << remote << endl;
			file << "#~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
			for(vector<string>::iterator coit= remit->second.begin(); coit != remit->second.end(); ++coit)
			{
				code= *coit;
				if(code.find(":") >= code.length())
				{
					foundCodeAlias= foundCodeAliasMap->second.find(code);
					org_code= foundCodeAlias->second;
					file << "# code " << org_code << endl;
					file << "name= " << code << "_receive" << endl;
					file << "type= LIRC" << endl;
					file << "ID= " << org_remote << endl;
					file << "pin= " << org_code << endl;
					file << "perm= " << readperm << endl;
					file << "action= receive" << endl;
					file << endl;
				}
			}
		}
		file.close();


		for(map<string, vector<string> >::iterator remit= remotes.begin(); remit != remotes.end(); ++remit)
		{
			string filename;

			remote= remit->first;
			if(find(kill.begin(), kill.end(), remote) == kill.end())
			{
				foundRemouteAlias= remotealias.find(remote);
				org_remote= foundRemouteAlias->second;
				foundCodeAliasMap= rcodealias.find(remote);
				filename= remote + ".desktop";
				cout << "### write layout file for remote " << org_remote << " as " << filename << endl;
				file.open(filename.c_str(), ios::out);
				//rem.open("lirc.conf", ios::out);
				file << "<layout>" << endl;
				file << "  <head>" << endl;
				file << "    <title name=\"" << org_remote << "\" />" << endl;
				file << "  </head>" << endl;
				file << "  <body>" << endl;
				file << "    <table border=\"1\">" << endl;
				file << "      <tr>" << endl;
				file << "        <td>" << endl;
				file << "          <table border=\"0\" >" << endl;
				actrows= 0;
				for(vector<string>::iterator coit= remit->second.begin(); coit != remit->second.end(); ++coit)
				{
					code= *coit;
					foundCodeAlias= foundCodeAliasMap->second.find(code);
					if(foundCodeAlias != foundCodeAliasMap->second.end())
						org_code= foundCodeAlias->second;
					else
						org_code= code;
					if(actrows == 0)
						file << "            <tr>" << endl;
					file << "              <td>" << endl;
					cout << "read " << remote << " " << code << endl;
					if(code != "###NULL::")
					{
						file << "                <component type=\"checkbox\" result=\"";
						if(code.find(":") < code.length())
						{// code is an linked subroutine
							vector<string> sp;

							split(sp, code , is_any_of(":"));
							linkCodeAliasMap= rcodealias.find(sp[0]);
							foundCodeAlias= linkCodeAliasMap->second.find(sp[1]);
							org_code= foundCodeAlias->second;
							file << code;
						}else
							file << remote << ":" << code;
						file << "_receive" << "\" readonly=\"readonly\" />" << endl;
						file << "                  " << org_code << endl;

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
				file << "  </body>" << endl;
				file.close();
			}// is inside of kill vector
		}// for loop of remote
	}// if(!learn)
}

void ussage(const bool full)
{
	if(!full)
	{
		cout << "no correct command be set" << endl;
		cout << "type -? for help" << endl;
		return;
	}
	cout << endl;
	cout << "syntax:  ppi-mconfig [options] <commands> [spez. options for command]" << endl;
	cout << endl;
	cout << "       options:" << endl;
	cout << "            -?  --help         - show this help" << endl;
	cout << endl;
	cout << "       commands:" << endl;
	cout << "            LIRC               - configuration for receiver and or transmitter" << endl;
	cout << endl;
	cout << "       spez. options for LIRC:" << endl;
	cout << "            -f  --file             - lirc configuration file with remote codes (default: -f '/etc/lirc/lircd.conf')" << endl;
	cout << "            -l  --learn            - reading pressed buttons from the database for an learning transmitter." << endl;
	cout << "                                     This option include option --transmitter (-t) and exclude --receiver (an error occures)" << endl;
	cout << "            -r  --receiver         - create folder:subroutine *.conf file for only receiving (default)" << endl;
	cout << "            -p  --readpermission   - permission to read receiving value (default from access.conf 'read'" << endl;
	cout << "            -t  --transmitter      - create *.conf and *.desktop files for receiving and transmitting" << endl;
	cout << "            -w  --writepermission  - permission to send code (default from access.conf 'change'" << endl;
	cout << "            -v  --vertical         - vectical rows for layout file" << endl;
	cout << endl;
}
