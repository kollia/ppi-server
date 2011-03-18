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
	string workdir, param, param_id, command;
	string sConfPath(PPICONFIGPATH), fileName;
	Properties oServerProperties;
	MainParams params(argc, argv, /*read path for parent dirs*/1);
	const ICommandStructPattern* commands;

	glob::processName("ppi-mconfig");

	params.setDescription("create measure file for 'measure.conf' and also some layout files\n"
							"specific for defined command.");
	params.version(PPI_MAJOR_RELEASE, PPI_MINOR_RELEASE, PPI_SUBVERSION, PPI_PATCH_LEVEL,
										/*no build*/0, PPI_REVISION_NUMBER, DISTRIBUTION_RELEASE);
	params.help("help", "?");

	param_id= params.command("LIRC", "create configuration for receiver and transmitter if set to fill in 'measure.conf'\n"
										"and also corresponding layout files to copy into ppi-server client directory\n"
										"when the LIRC command be set without any options all remotes defined in lircd.conf will be created for transmit and receive");
	params.option(param_id, "file", "f", true, "lirc configuration file with remote codes (default: -f '/etc/lirc/lircd.conf')");
	params.option(param_id, "show", "s", "show all defined remotes with defined alias to generate in an other step with --remote");
	params.option(param_id, "predefined", "d", "show all namespaces for defined codes in lircd.conf has predefined defaults");
	params.option(param_id, "notransmit", "n", "do not define *.conf file for transmitting");
	params.option(param_id, "remote", "r", true, "create only for this remote control the files .conf and .desktop\n"
													"and do not create or touch lirc.conf");
	params.option(param_id, "vertical", "v", true, "vectical default rows for layout file");
	params.option(param_id, "readperm", "p", true, "permission to read receiving value (default from access.conf 'read')");
	params.option(param_id, "changeperm", "c", true, "permission to send code (default from access.conf 'change')");
	params.option(param_id, "userreadconfwrite", "u", true, "normaly user permission to read and configureator to read and write "
																							"(default from access.conf 'ureadlw')");
	params.option(param_id, "configreadperm", "P", true, "permission to read configuration (default from access.conf 'lconfread')");
	params.option(param_id, "configchangeperm", "C", true, "permission to read configuration (default from access.conf 'lconfchange')");
	params.option(param_id, "learn", "l", "reading pressed buttons from the database for an learning transmitter.\n"
											"This option include option --transmitter (-t) and exclude --receiver (an error occures)");
	//params.option(param_id, "", "", "");

	params.execute();
	commands= params.getCommands();
	workdir= params.getPath();
	command= commands->command();

	sConfPath= URL::addPath(workdir, sConfPath, /*always*/false);
	fileName= URL::addPath(sConfPath, "server.conf");
	if(!oServerProperties.readFile(fileName))
	{
		cout << "### ERROR: cannot read '" << fileName << "'" << endl;
		exit(EXIT_FAILURE);
	}
	oServerProperties.readLine("workdir= " + workdir);


	if(command == "LIRC")
	{
		LircSupport lirc(workdir);

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
