/**
 *   This file 'ErrorHandling.cpp' is part of ppi-server.
 *   Created on: 14.10.2014
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

#include "ErrorHandling.h"

namespace util
{
	void ErrorHandling::read()
	{
		setDescription("en", "ClientTransaction", "client_send",
						"by trying to send command to @1:@2");
		setDescription("en", "ClientTransaction", "tcgetattr",
						"cannot read terminal interface for password");
		setDescription("en", "ClientTransaction", "tcsetattr",
						"cannot set terminal interface for blind reading password");
		setDescription("en", "ClientTransaction", "tcsetattr_back",
						"cannot set back terminal interface to see on command line typing letters");
		setDescription("en", "ClientTransaction", "get_result",
						"client get follow answer from server");
		setDescription("en", "ClientTransaction", "get_result",
						"get no result from server");
		setDescription("en", "ClientTransaction", "undefined_server",
						"undefined server running on @1:@2\ngetting back as result '@3'");

		setDescription("en", "OwServerQuestions", "get_question",
						"by reading questions for external port server @1");
		setDescription("en", "OwServerQuestions", "get_wrong_question",
						"external port server @1 get undefined question of '@2'");

		setDescription("en", "owserver", "init_chip",
						"can not initial erxternal server @1");
		setDescription("en", "owserver", "init_exception",
						"exception occurred by initialing external server @1");

		setDescription("en", "Client", "getClientErrorString",
						"Error descriptions from internet server now not implemented");
		setDescription("en", "Client", "getNoErrorString",
						"given error string '@1' is no defined error or warning");
		setDescription("en", "Client", "noRun",
						"ppi-server does not running");

		setDescription("en", "ProcessChecker", "getWrongQuestion",
						"ProcessChecker get unknown question '@1'");
		setDescription("en", "ProcessChecker", "waitForQuestion",
						"by ProcessChecker waiting for new questions");

		setDescription("en", "Starter", "startDatabase",
						"while trying to start database");
		setDescription("en", "Starter", "checkDatabase",
						"while checking database for be alive");
		setDescription("en", "Starter", "startInternet",
						"while trying to start internet server to communicate");
		setDescription("en", "Starter", "checkInternet",
						"while checking internet server for be alive");
		setDescription("en", "Starter", "startShell",
						"while trying to start command line shell for user @1");
		setDescription("en", "Starter", "checkShell",
						"while checking command line shell for user @1 to be alive");
		setDescription("en", "Starter", "startPort",
						"while trying to start external port server @1 for reading");
		setDescription("en", "Starter", "checkPort",
						"while checking external port server @1 for reading to be alive");
		setDescription("en", "Starter", "startPortMeasure",
						"while trying to start external port server @1 for measuring");
		setDescription("en", "Starter", "checkPortMeasure",
						"while checking external port server @1 for measuring to be alive");
		setDescription("en", "Starter", "startLirc",
						"while trying to start server to reading/writing IR transactions (LIRC)");
		setDescription("en", "Starter", "checkLirc",
						"while checking server to reading/writing IR transactions (LIRC) to be alive");
		setDescription("en", "Starter", "startk8055",
						"while trying to start server reading/writing on velleman k8055 board");
		setDescription("en", "Starter", "checkk8055",
						"while checking server to reading/writing on velleman k8055 board to be alive");
		setDescription("en", "Starter", "startk8055",
						"while trying to start server reading/writing on Maxim/Dallas semiconductors");
		setDescription("en", "Starter", "checkk8055",
						"while checking server to reading/writing on Maxim/Dallas semiconductors to be alive");
		setDescription("en", "Starter", "readStatus",
						"while reading actual status from server");

		setDescription("en", "DbFiller", "sendDatabase",
						"by sending method @1 from working list to database");
		setDescription("en", "DbFiller", "sendToDatabase",
						"by sending method @1 for @2:@3 from working list to database");

		setDescription("en", "DbFillerFactory", "create_object",
						"can not create object of DbFillerFactory\n"
						"so fill database DIRECT which maybe cost performance");
		setDescription("en", "DbFillerFactory", "start",
						"by try to starting database filler factory");
		setDescription("en", "DbFiller", "start",
						"by try to starting database filler thread for folder @1");

		setDescription("en", "MeasureTread", "startInformer",
						"by try to starting informer thread @1 for folder @2");
		setDescription("en", "MeasureTread", "startInformerErr",
						"by try to starting informer thread @1 for folder @2\n"
						" - folder list inform now directly other folders, which has more bad performance");

		setDescription("en", "DbInterface", "sendCommand",
						"by sending commend '@1' to database server");

		setDescription("en", "Shell", "CommandExecStartInit",
						"by trying to start CommandExec thread for working\n"
						"inside subroutine object of @1:@2");
		setDescription("en", "Shell", "CommandExecStart",
						"by trying to start CommandExec thread for command '@1'\n"
						"inside subroutine object of @2:@3");
		setDescription("en", "ShellWriter", "CommandExecStartInit",
						"by trying to start CommandExec thread "
						"inside SchellWriter for command '@1'\n"
						"and subroutine object of @1:@2");
		setDescription("en", "ShellWriter", "CommandExecStart",
						"by trying to start CommandExec thread "
						"inside SchellWriter for command '@1'\n"
						"and subroutine object of @2:@3");

		setDescription("en", "Read", "wrong_start",
						"by starting external worker thread to start reading"
						" per defined time inside @1:@2"					);
		setDescription("en", "Read", "set_policy",
						"inside folder:subroutine @1:@2"				);

		setDescription("en", "owreadermain", "run",
						"by running external server @1");
		setDescription("en", "owreadermain", "stop",
						"by stopping external server @1");

		setDescription("en", "internetservermain", "run",
						"by running internet server process");

		setDescription("en", "databasemain", "run",
						"by running database process");
	}

} /* namespace util */
