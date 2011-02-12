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
	params.option("version", "v", "show information of actual version");

	param_id= params.command("LIRC", "create configuration for receiver and transmitter if set to fill in 'measure.conf'\n"
										"and also corresponding layout files to copy into ppi-server client directory");
	params.option(param_id, "file", "f", true, "lirc configuration file with remote codes (default: -f '/etc/lirc/lircd.conf')");
	params.option(param_id, "remote", "R", true, "create only for this remote control the files .conf and .desktop");
	params.option(param_id, "vertical", "v", true, "vectical default rows for layout file");
	params.option(param_id, "readperm", "p", true, "permission to read receiving value (default from access.conf 'read')");
	params.option(param_id, "changeperm", "c", true, "permission to send code (default from access.conf 'change')");
	params.option(param_id, "configreadperm", "P", true, "permission to read configuration (default from access.conf 'confread')");
	params.option(param_id, "configchangeperm", "C", true, "permission to read configuration (default from access.conf 'confchange')");
	params.option(param_id, "receiver", "r", "create folder:subroutine *.conf file for only receiving (default)");
	params.option(param_id, "transmitter", "t", "create *.conf and *.desktop files for receiving and transmitting");
	params.option(param_id, "learn", "l", "reading pressed buttons from the database for an learning transmitter.\n"
											"This option include option --transmitter (-t) and exclude --receiver (an error occures)");
	//params.option(param_id, "", "", "");

	params.execute();
	commands= params.getCommands();
	if(params.hasOption("version"))
	{
		string buildnulls;
		ostringstream buildstream;

		cout << params.getAppName() << " ";
		cout << PPI_MAJOR_RELEASE << ".";
		cout << PPI_MINOR_RELEASE << ".";
		cout << PPI_PATCH_LEVEL << ".";
		buildstream << PPI_BUILD_NUMBER;
		buildnulls.append(5 - buildstream.str().length(), '0');
		cout << buildnulls << PPI_BUILD_NUMBER << endl;

		cout << " version ";
		cout << PPI_MAJOR_RELEASE << ".";
		cout << PPI_MINOR_RELEASE << " ";
		if(PPI_PATCH_LEVEL > 0)
			cout << "patch " << PPI_PATCH_LEVEL << " ";
		cout << " revision ";
		cout << buildnulls << PPI_BUILD_NUMBER << endl;
		return EXIT_SUCCESS;
	}
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
