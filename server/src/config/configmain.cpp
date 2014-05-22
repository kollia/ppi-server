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

#include "../pattern/util/LogHolderPattern.h"

#include "DbTimeChecker.h"
#include "LircSupport.h"
#include "OwfsSupport.h"

using namespace std;
using namespace util;

void ussage(const bool full);

int main(int argc, char* argv[])
{
	bool bOk;
	string workdir, commandname;
	string sConfPath(PPICONFIGPATH), fileName;
	vector<string> names;
	InterlacedProperties oServerProperties;
	MainParams params(argc, argv, /*read path for parent dirs*/1);
	ParamCommand* command;
	const ICommandStructPattern* commands;

	glob::processName("ppi-mconfig");

	params.setDescription("create measure file or examples for 'measure.conf' and also some layout files\n"
							"specific for defined command.");
	params.version(PPI_MAJOR_RELEASE, PPI_MINOR_RELEASE, PPI_SUBVERSION, PPI_PATCH_LEVEL,
										/*no build*/0, PPI_REVISION_NUMBER, DISTRIBUTION_RELEASE);
	params.help("help", "?");
	params.option("info", "i", "show more information by writing output.");
	params.option("debug", "d", "show debugging information by writing output.\n"
									"also the information for option --info");

	command= params.command("TIMER", "check database file, created with ppi-server option --timerdblog,\n"
					                   "whether reached precise time or different");
	command->option("list", "l", "list all defined times inside database");
	command->option("exactstop", "E", "make also list of times,\n"
					"but filter list by time needed after exact stopping\n"
					"(only usable with option --list)");
	command->option("estimated", "e", "make also list of times,\n"
					"but filter list by fault estimated times"
					"(only usable with option --list)");
	command->option("reachend", "r", "make also list of times,\n"
					"but filter list by new estimated reaching end time\n"
					"(only usable with option --list)");
	command->option("foldersort", "f", "sort output by folders with subroutines\n"
					"otherwise sorting will be done by running folder ID");
	command->option("exacttimesort", "T", "sort output by exact stopping time\n"
					"otherwise sort by time written into database\n"
					"(not usable with option --estimatetimesort)");
	command->option("estimatetimesort", "t", "sort output by wrong estimation time\n"
					"otherwise sort by time written into database\n"
					"(not usable with option --exacttimesort)");
	command->option("startingtime", "s", "show starting and ending times of server");
	command->option("begin", "B", true, "begin calculation of statistic by given time\n"
					"(can't be used with option --starting");
	command->option("stop", "S", true, "stop calculation of statistic by this given time\n"
					"(can't be used with option --starting");
	command->spaceline("");

	command= params.command("LIRC", "create configuration for receiver and transmitter if set to fill in 'measure.conf'\n"
									"and also corresponding layout files to copy into ppi-server client directory\n"
									"when the LIRC command be set without any options all remotes defined in lircd.conf\n"
									"will be created for transmit and receive");
	command->option("file", "f", true, "lirc configuration file with remote codes (default: '/etc/lirc/lircd.conf')");
	command->option("show", "s", "show all defined remotes with defined alias to generate in an other step with --remote");
	command->option("predefined", "d", "show all namespaces for defined codes in lircd.conf has predefined defaults");
	command->option("vertical", "v", true, "vectical default rows for transmitter layout file (default:3)");
	command->spaceline("");
	command->option("notransmit", "n", "do not define *.conf file for transmitting");
	command->option("remote", "r", true, "create only for this remote control the files .conf and .desktop\n"
													"and do not create or touch lirc.conf");
	command->spaceline("");
	command->option("learn", "l", true, "reading pressed buttons from the database for an learning transmitter.\n"
											"Inside time pressing record button and leaf, should be defined one name.\n"
											"More than one names can be defined calculating to the last unit");
	command->spaceline("permission:");
	command->option("readperm", "p", true, "to read receiving value (default from access.conf 'read')");
	command->option("changeperm", "c", true, "to send code (default from access.conf 'change')");
	command->option("userreadconfwrite", "u", true, "normaly user can read and configureator read and write "
																							"(default from access.conf 'ureadlw')");
	command->option("configreadperm", "P", true, "to read configuration (default from access.conf 'lconfread')");
	command->option("configchangeperm", "C", true, "to read configuration (default from access.conf 'lconfchange')");


#ifdef _OWFSLIBRARY
	command= params.command("OWFS", "creating example config file for all maxim/dallas chips\n"
									"use this first to know which chip ID's can be used\n"
									"and which pin's an chip has\n"
									"( do not change this file !!!\n"
									"  because for the next call of ppi-mconfig with OWFS command\n"
									"  the application know which chips are configured\n"
									"  and do not ask again for the configured chips before        )");
	command->option("file", "f", true,	"file in which should writing example\n"
										"(default is 'maxim_examples.conf' inside current directory)");
#endif //_OWFSLIBRARY

	bOk= params.execute(/*stop by error*/false);
	commands= params.getCommands();

	if(params.hasOption("debug"))
		LogHolderPattern::init(LOG_DEBUG);
	else
		if(params.hasOption("info"))
			LogHolderPattern::init(LOG_INFO);
		else
			LogHolderPattern::init(LOG_WARNING);

	commandname= commands->command();
	if(	commandname == "LIRC" &&
		(	bOk == false ||
			commands->hasOption("learn")	)	)
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
	}else if(params.error())
		return EXIT_FAILURE;

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

	sConfPath= URL::addPath(workdir, sConfPath, /*always*/false);
	fileName= URL::addPath(sConfPath, "server.conf");
	oServerProperties.setDelimiter("owreader", "[", "]");
	oServerProperties.modifier("owreader");
	oServerProperties.readLine("workdir= " + workdir);
	if(!oServerProperties.readFile(fileName))
	{
		cout << "### ERROR: cannot read '" << fileName << "'" << endl;
		exit(EXIT_FAILURE);
	}

	if(commandname == "TIMER")
	{
		DbTimeChecker checker(workdir);

		return checker.execute(commands, &oServerProperties);

	}else if(commandname == "LIRC")
	{
		LircSupport lirc(workdir);

		lirc.setLearnNames(names);
		return lirc.execute(commands);
	}
#ifdef _OWFSLIBRARY
	else if(commandname == "OWFS")
	{
		OwfsSupport owfs(workdir);

		return owfs.execute(commands, &oServerProperties);
	}
#endif /* _OWFSLIBRARY */
	cout << "no correct command be set" << endl;
	cout << "  type -? for help" << endl;
	return EXIT_FAILURE;
}


