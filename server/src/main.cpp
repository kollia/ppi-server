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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <vector>
#include <map>

#include "util/MainParams.h"
#include "util/debug.h"
#include "util/properties/measureStructures.h"
#include "util/thread/Terminal.h"

#include "starter.h"

using namespace std;
using namespace util;

// only used method for some debugging tests
void tests(const string& workdir, int argc, char* argv[]);

int main(int argc, char* argv[])
{
	bool result;
	string command;
	Starter server;
	MainParams params(argc, argv, /*read path for parent dirs*/1);
	const ICommandStructPattern* commands;

	// method has only content by debugging when need
	tests(params.getPath(), argc, argv);

	params.setDescription("start or stop ppi-server to reading external sensors and devices");

	params.option("configure", "c", "display which folder configure by starting\n"
							"(can be set by longer starting time to know what server is doing)\n"
							"and show after configuration all first values of defined ports from external readers");
	params.option("folderstart", "f", "show all folder on command line which are defining and starting");
	params.option("observers", "o", "show by starting all defined observers for every folder:subroutine");
	params.option("subroutines", "s", "for option --configure (-s) or --folderstart (-f) show also subroutines\n"
					"(when none of this option be set, also folder configuration will be displayed)");
	params.option("nodbbegintime", "n", "do not read beginning time from database for subroutines with type TIME\n"
					"which normally saved as:\n"
					" - \"|folder|<folder>|runlength|\"      for 7/8 longest folder run time\n"
					" - \"|folder|<folder>|maxcount|\"       measure 'runnlength' always by this maximal count\n"
					"                                      to write into database\n"
					" - \"|<folder>|<subroutine>|reachend\"  for middle length of reaching late finish-position\n"
			//		" - \"|<folder>|<subroutine>|maxcount\"  measure 'reachlate' always by this maximal count\n"
			//		"                                      to write into database\n"
					"and set this values inside database to 0");
	params.option("timerdblog", "t", "logging inside database currently reach info for subroutines with type TIME\n"
					" - \"|<folder>|<subroutine>|runpercent|\"    by which cpu percent taking 'runlength' time\n"
					" - \"|<folder>|<subroutine>|reachpercent|\"  cpu percent taking estimated finish time 'reachend'\n"
					" - \"|<folder>|<subroutine>|wanttime|\"      which time want to measure\n"
					" - \"|<folder>|<subroutine>|informlate|\"    when subroutine was informed to calculate exact stopping time\n"
					" - \"|<folder>|<subroutine>|startlate|\"     starting TIMER subroutine as second time to late\n"
					"                                             for stopping on exact time\n"
					" - \"|<folder>|<subroutine>|reachlate\"      reach currently late finish-position\n"
					" - \"|<folder>|<subroutine>|wrongreach\"     wrong difference to reach finish-position dependent to 'reachend'");
	params.option("folderdebug", "d", true, "debugging from begin the following <folder>[:<subroutine>]\n"
					               "also more subroutines can be given separated with an comma inside quotes\n"
					               "('<folder>[:<subroutine>], <folder>[:<subroutine>]')");
	params.option("debug", "D", "show logging messages, deep defined inside server.conf, on screen\n"
									"(only usable for stop command)");
	params.version(PPI_MAJOR_RELEASE, PPI_MINOR_RELEASE, PPI_SUBVERSION, PPI_PATCH_LEVEL,
										/*no build*/0, PPI_REVISION_NUMBER, DISTRIBUTION_RELEASE);
	params.help("help", "?");

	params.command("start", "starting server");
	params.command("stop", "stopping server");
	params.command("restart", "restarting the hole server");
//	params.command("status", "get feedback whether server is running");

	params.execute();
	commands= params.getCommands();
	command= commands->command();
	server.setWorkingDirectory(params.getPath());

	try
	{
		if(	command == "start" ||
			command == "stop" ||
			command == "restart"	)
		{
			if(getuid() != 0)
			{
				cerr << "process has to start as root" << endl;
				return EXIT_FAILURE;
			}
		}

		if(command == "start")
		{
			if(params.hasOption("debug"))
			{
				cerr << "debug option only allowed by stopping command" << endl;
				cerr << "   for more description read help manual (ppi-server --help)" << endl << endl;
				return EXIT_FAILURE;
			}
			pthread_mutex_init(&g_READMUTEX, NULL);
			result= server.execute(&params);
			cout << "### ppi-server was stopped ";
			if(!result)
			{
				cout << "with an ERROR" << endl;
				Terminal::deleteObj();
				return EXIT_FAILURE;
			}
			cout << "successfully" << endl;
			Terminal::deleteObj();
			return EXIT_SUCCESS;

		}else if(command == "status")
		{
			result= server.status();

		}else if(command == "stop")
		{
			vector<string>::size_type nCount;

			nCount= params.optioncount();
			if(	nCount == 0 ||
				(	nCount == 1 &&
					params.hasOption("debug")	)	)
			{
				result= server.stop(params.hasOption("debug"));
			}else
			{
				cout << "by stopping ppi-server only debug option be allowed" << endl;
				cout << "   for more description read help manual (ppi-server --help)" << endl << endl;
				return EXIT_FAILURE;
			}

		}else if(command == "restart")
		{
			server.stop(params.hasOption("configue"));
			cout << "### restart ppi-server" << endl;
			result= server.execute(commands);
		}else
		{
			cout << "no correct command be set" << endl;
			cout << "  type -? for help" << endl;
			return EXIT_FAILURE;
		}
		//delete server;
	}catch(...)
	{
		cout << "unknown exception" << endl;
		return EXIT_FAILURE;
	}


	if(command == "start")
		cout << "LogServer" << endl;
	if(command == "stop")
	{
		if(result)
		{
			cout << "server was stopped" << endl;
			return EXIT_SUCCESS;
		}
		return EXIT_FAILURE;

	}else if(command == "start")
	{
		if(result)
		{
			cout << endl << "#### server was stopping correctly" << endl;
			//system("ps -eLf | grep ppi-server");
			return EXIT_SUCCESS;
		}else
		{
			cerr << endl << "#### server stopping with ERROR" << endl;
			return EXIT_FAILURE;
		}
	}
	return EXIT_FAILURE;
}


//#define __MAKE_CALCULATER_TESTS
//#define __CHECK_THREAD_CREATION
//#define __CHECK_WORKING_INTERLACEDPROPERTIES
//#define __SIMPLE_SERVER_CLIENT_CONNECTION
//#define __PARAMETER_METHOD_STRINGSTREAM
//#define __CALENDAR_DEFINITIONS
//#define __EXCEPTION_HANDLING

#ifdef __MAKE_CALCULATER_TESTS
#define __MAKE_TESTS
#endif // __MAKE_CALCULATER_TESTS
#ifdef __CHECK_THREAD_CREATION
#define __MAKE_TESTS
#endif // __CHECK_THREAD_CREATION
#ifdef __CHECK_WORKING_INTERLACEDPROPERTIES
#define __MAKE_TESTS
#endif //__CHECK_WORKING_INTERLACEDPROPERTIES
#ifdef __SIMPLE_SERVER_CLIENT_CONNECTION
#define __MAKE_TESTS
#endif // __SIMPLE_SERVER_CLIENT_CONNECTION
#ifdef  __PARAMETER_METHOD_STRINGSTREAM
#define __MAKE_TESTS
#endif
#ifdef  __CALENDAR_DEFINITIONS
#define __MAKE_TESTS
#endif
#ifdef __EXCEPTION_HANDLING
#define __MAKE_TESTS
#endif // __EXCEPTION_HANDLING

#ifdef __MAKE_TESTS
// some includes needed for tests method
#include "util/URL.h"
// only for calculator test
#include "util/CalculatorContainer.h"
// only for simple server client communication
#include "server/libs/client/SocketClientConnection.h"
#include "server/libs/server/TcpServerConnection.h"
#include "util/smart_ptr.h"
// for testing on streams
#include "util/properties/interlacedproperties.h"
// for check thread creation
#include "util/thread/CallbackTemplate.h"
// for MethodStringStream tests
#include "util/stream/IMethodStringStream.h"
#include "util/stream/OMethodStringStream.h"
// for checking exception handling
#include "util/structures.h"
#include "util/exception.h"
#endif // __MAKE_TESTS

#ifdef __CHECK_THREAD_CREATION
	class TestCallback : public CallbackTemplate
	{
	protected:
		static int m_count;
		/**
		 * abstract method running in thread.<br />
		 * This method starting again when method ending with return 0
		 * and stopping by all other values.<br />
		 * By calling external method finished()
		 * method gives back the return code.<br />
		 * In the most case the should be 1 for finished correctly, -1 finished with warnings
		 * or -2 with errors.
		 *
		 * @return defined error code from extended class
		 */
		virtual short runnable()
		{
			++m_count;
			cout << "running thread " << m_count << " [" << Thread::gettid() << "]" << endl;
			return 1;
		};
	};
	int TestCallback::m_count= 0;
#endif //__CHECK_THREAD_CREATION

void tests(const string& workdir, int argc, char* argv[])
{
#ifdef __MAKE_TESTS
	// define configure path
	string sConfPath;

	sConfPath= URL::addPath(workdir, PPICONFIGPATH, /*always*/false);
#endif // __MAKE_TESTS

#ifdef __MAKE_CALCULATER_TESTS

	double result;
	CalculatorContainer calc;

	calc.doOutput(true);
	calc.statement("23/0");
	calc.render();
	calc.calculate(result);
	cout << "result is " << result << endl;

#endif // __MAKE_CALCULATER_TESTS

#ifdef __CHECK_THREAD_CREATION
	// check thread creation
	CallbackTemplate *callback;

	while(1)
	{
		callback= new TestCallback();
		callback->initialStarting();
		cout << "thread finish with " << callback->finished(true) << endl;
		//usleep(500);
		delete callback;
	}
#endif //__CHECK_THREAD_CREATION

#ifdef __CHECK_WORKING_INTERLACEDPROPERTIES
	// check working of InterlacedProperties
	typedef vector<IInterlacedPropertyPattern*>::iterator secIt;
	string fileName;
	InterlacedActionProperties mainprop(/*check after*/true);
	vector<IInterlacedPropertyPattern*> folderSections;
	vector<IInterlacedPropertyPattern*> subSections;

	fileName= URL::addPath(sConfPath, "measure.conf");
	mainprop.action("action");
	mainprop.modifier("folder");
	mainprop.setMsgParameter("folder");
	mainprop.modifier("name");
	mainprop.setMsgParameter("name", "subroutine");
	mainprop.valueLocalization("\"", "\"", /*remove*/true);
	mainprop.readFile(fileName);
	folderSections= mainprop.getSections();
	for(secIt fit= folderSections.begin(); fit != folderSections.end(); ++fit)
	{
		cout << (*fit)->getMsgHead(false) << endl;
		subSections= (*fit)->getSections();
		for(secIt sit= subSections.begin(); sit != subSections.end(); ++sit)
		{
			cout << (*sit)->getMsgHead(false) << endl;
		}
	}
	exit(EXIT_SUCCESS);
#endif //__CHECK_WORKING_INTERLACEDPROPERTIES

	// simple server client communication
#ifdef __SIMPLE_SERVER_CLIENT_CONNECTION
		string command;
		string value;
	    SHAREDPTR::shared_ptr<IFileDescriptorPattern> descriptor;

	    if (argc == 2)
	    {
	    	command= argv[1];
	    	if(command == "client")
	    	{
	    		server::SocketClientConnection client(	SOCK_STREAM,
														"127.0.0.1",
														20000,
														10			);

	    		//connection.newTranfer(new ClientTransaction());
	    		if(!client.init())
	    		{
	    			descriptor= client.getDescriptor();
	    			cout << "client send: \"Hallo Du!\"" << endl;
	    			(*descriptor) << "Hallo Du!\nWie geht es Dir\nden";
	    			(*descriptor) << " heute?";
	    			descriptor->endl();
	    			descriptor->flush();
	    			if(!descriptor->eof())
	    			{
						(*descriptor) >> value;
						cout << value << endl;
						descriptor->closeConnection();
	    			}else
	    				cout << "server close to early connection" << endl;
	    		}

	    	}else if(command == "server")
	    	{
	    		server::TcpServerConnection connection(	"127.0.0.1",
														20000,
														10,
														NULL	);

	    		if(!connection.init())
	    		{
	    			if(!connection.accept())
	    			{
	    				short count(1), maxcount(3);

	    				descriptor= connection.getDescriptor();
	    				while(	!descriptor->eof() &&
	    						count < maxcount		)
	    				{
	    					(*descriptor) >> value;
	    					cout << "Server get message " << value << endl;
	    					++count;
	    				}
	    				if(!descriptor->eof())
	    				{
							cout << "server send back: Server got message" << endl;
							(*descriptor) << "Server got message";
							descriptor->endl();
							descriptor->unlock();
							descriptor->closeConnection();
	    				}else
	    					cout << "client close to early connection" << endl;
	    			}
	    		}
	    	}else
	    		fprintf(stderr,"usage %s [server|client]\n", argv[0]);
	    }else
	    	fprintf(stderr,"usage %s [server|client]\n", argv[0]);

#endif //__SIMPLE_SERVER_CLIENT_CONNECTION

#ifdef __PARAMETER_METHOD_STRINGSTREAM
		IParameterStringStream reader("truego");
		bool val;
		IMethodStringStream first("setValue \"power_switch\" \"Raffstore\" 5 ");
		cout << first.str(true) << endl;
		first.createSyncID();
		cout << first.str(true) << endl;
		IMethodStringStream second("getValue syncID 1 \"irgendwas\"");
		cout << second.str(true) << endl;
		second= first;
		cout << second.str(true) << endl;
		OMethodStringStream third("3 45 \"neu\"");
		cout << third.str(true) << endl;
		third.createSyncID();
		cout << third.str(true) << endl;
		OMethodStringStream fourth("\"ich\"");
		cout << fourth.str(true) << endl;
		fourth.createSyncID();
		cout << fourth.str(true) << endl;
		OMethodStringStream fivethd("syncID 2 23 \"neu\"");
		cout << fivethd.str() << endl;
		fivethd.createSyncID();
		cout << fivethd.str(true) << endl;
		OMethodStringStream sixed("");
		cout << sixed.str(true) << endl;
		sixed.createSyncID();
		cout << sixed.str(true) << endl;
		OMethodStringStream seven("syncID syncID 6 23 \"neu\"");
		cout << seven.str() << endl;
		seven.createSyncID();
		cout << seven.str(true) << endl;

		cout << "IMethodStringStream: "<< first.str(true) << endl;
		first.createSyncID();
		cout << "                     "<< first.str(true) << endl;
		cout << endl;
		cout << "OMethodStringStream: " << second.str(true) << endl;
		second.createSyncID();
		cout << "                     " << second.str(true) << endl;
		cout << "IMethodStringStream answer: " << third.str(true) << endl;
		third.createSyncID();
		cout << "                            " << third.str(true) << endl;
		cout << "OMethodStringStream answer: " << fourth.str(true) << endl;
		fourth.createSyncID();
		cout << "                            " << fourth.str(true) << endl;
		exit(1);

		reader >> val;
		cout << "value is " << val << endl;
		if(reader.fail())
			cout << "an error is occured" << endl;
		if(reader.empty())
			cout << "reading of string is finished" << endl;
#endif // __PARAMETER_METHOD_STRINGSTREAM

#ifdef __CALENDAR_DEFINITIONS
		bool newer= false;
		time_t t;
		time(&t);
		Calendar::calcDate(newer, t, 1, 'h');
		Calendar::calcDate(newer, t, 3, 'h');
		Calendar::calcDate(newer, t, 1, 'D');
		Calendar::calcDate(newer, t, 5, 'D');
		Calendar::calcDate(newer, t, 1, 'W');
		Calendar::calcDate(newer, t, 4, 'W');
		Calendar::calcDate(newer, t, 1, 'M');
		Calendar::calcDate(newer, t, 4, 'M');
		Calendar::calcDate(newer, t, 1, 'Y');
		Calendar::calcDate(newer, t, 2, 'Y');
#endif // __CALENDAR_DEFINITIONS

#ifdef __EXCEPTION_HANDLING

		convert_t* var;

		//abort();
		try{
			cout << flush;
			cout << "time be set " << boolalpha << var->bSetTime << endl;
		}catch(SignalException& ex)
		{
			ex.printTrace();
			try{
				int i= 3;
				cout << "second time be set " << boolalpha << var->bSetTime << endl;
			}catch(SignalException& ex)
			{
				ex.printTrace();
			}catch(...)
			{
				cout << "get uncaught Exception" << endl;
			}
		}catch(...)
		{
			cout << "get uncaught Exception" << endl;
		}

#endif // __EXCEPTION_HANDLING
#ifdef __MAKE_TESTS
		exit(0);
#endif // __MAKE_TESTS
}
