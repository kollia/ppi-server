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

#include <cstdlib>

#include <iostream>
#include <sstream>

#include "../util/debug.h"
#include "../util/URL.h"
#include "../util/GlobalStaticMethods.h"
#include "../util/MainParams.h"

#include "../util/properties/properties.h"

#include "LircSupport.h"

using namespace std;
using namespace util;

void ussage(const bool full);

int main(int argc, char* argv[])
{
	bool bOk;
	string workdir, commandname;
	string sConfPath(PPICONFIGPATH), fileName;
	vector<string> names;
	Properties oServerProperties;
	MainParams params(argc, argv, /*read path for parent dirs*/1);
	ParamCommand* command;
	const ICommandStructPattern* commands;

	glob::processName("ppi-mconfig");

	params.setDescription("create measure file for 'measure.conf' and also some layout files\n"
							"specific for defined command.");
	params.version(PPI_MAJOR_RELEASE, PPI_MINOR_RELEASE, PPI_SUBVERSION, PPI_PATCH_LEVEL,
										/*no build*/0, PPI_REVISION_NUMBER, DISTRIBUTION_RELEASE);
	params.help("help", "?");

	command= params.command("LIRC", "create configuration for receiver and transmitter if set to fill in 'measure.conf'\n"
									"and also corresponding layout files to copy into ppi-server client directory\n"
									"when the LIRC command be set without any options all remotes defined in lircd.conf will be created for transmit and receive");
	command->option("file", "f", true, "lirc configuration file with remote codes (default: -f '/etc/lirc/lircd.conf')");
	command->option("show", "s", "show all defined remotes with defined alias to generate in an other step with --remote");
	command->option("predefined", "d", "show all namespaces for defined codes in lircd.conf has predefined defaults");
	command->option("vertical", "v", true, "vectical default rows for transmitter layout file (default:3)");
	command->spaceline("");
	command->option("notransmit", "n", "do not define *.conf file for transmitting");
	command->option("remote", "r", true, "create only for this remote control the files .conf and .desktop\n"
													"and do not create or touch lirc.conf");
	command->spaceline("");
	command->option("learn", "l", true, "reading pressed buttons from the database for an learning transmitter.\n"
											"inside time pressing record button");
	command->option("info", "i", "show more debugging information by writing output.\n"
									"this option is only for setting option --learn");
	command->spaceline("permission:");
	command->option("readperm", "p", true, "to read receiving value (default from access.conf 'read')");
	command->option("changeperm", "c", true, "to send code (default from access.conf 'change')");
	command->option("userreadconfwrite", "u", true, "normaly user can read and configureator read and write "
																							"(default from access.conf 'ureadlw')");
	command->option("configreadperm", "P", true, "to read configuration (default from access.conf 'lconfread')");
	command->option("configchangeperm", "C", true, "to read configuration (default from access.conf 'lconfchange')");

	bOk= params.execute(/*stop by error*/false);
	commands= params.getCommands();
	if(	bOk == false ||
		commands->hasOption("learn")					)
	{
		bool blfound(false);
		vector<pair<pair<string, string>, string> >* errors;

		errors= params.getParmeterContent();
		for(vector<pair<pair<string, string>, string > >::iterator it= errors->begin(); it != errors->end(); ++it)
		{
			//cerr << it->first.first << " | " << it->first.second << " | " << it->second << endl;
			if(	blfound &&
				(	it->first.second == "" ||
					it->first.second == "-learn"	)	)
			{
				names.push_back(it->first.first);
				it->second= "";
			}
			if(it->first.second == "--learn")
				blfound= true;
		}
		if(params.error())
			return EXIT_FAILURE;
	}
	if(params.hasOption("help"))
	{
		params.usage();
		return EXIT_SUCCESS;
	}
	if(params.hasOption("version"))
	{
		cout << params.getVersion();
		return EXIT_SUCCESS;
	}
	workdir= params.getPath();
	commandname= commands->command();

	sConfPath= URL::addPath(workdir, sConfPath, /*always*/false);
	fileName= URL::addPath(sConfPath, "server.conf");
	if(!oServerProperties.readFile(fileName))
	{
		cout << "### ERROR: cannot read '" << fileName << "'" << endl;
		exit(EXIT_FAILURE);
	}
	oServerProperties.readLine("workdir= " + workdir);


	if(commandname == "LIRC")
	{
		LircSupport lirc(workdir);

		lirc.setLearnNames(names);
		return lirc.execute(commands);
	}
	cout << "no correct command be set" << endl;
	cout << "  type -? for help" << endl;
	return EXIT_FAILURE;
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
	cout << "            -f  --file             - " << endl;
	cout << "            -l  --learn            - " << endl;
	cout << "                                     " << endl;
	cout << "            -r  --receiver         - create folder:subroutine *.conf file for only receiving (default)" << endl;
	cout << "            -p  --readpermission   - permission to read receiving value (default from access.conf 'read'" << endl;
	cout << "            -t  --transmitter      - create *.conf and *.desktop files for receiving and transmitting" << endl;
	cout << "            -w  --writepermission  - permission to send code (default from access.conf 'change'" << endl;
	cout << "            -v  --vertical         - vectical rows for layout file" << endl;
	cout << endl;
}
