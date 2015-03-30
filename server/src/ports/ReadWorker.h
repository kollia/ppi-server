/**
 *   This file 'ReadWorker.h' is part of ppi-server.
 *   Created on: 07.10.2014
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

#ifndef READWORKER_H_
#define READWORKER_H_

#include "../util/smart_ptr.h"
#include "../util/URL.h"

#include "../util/thread/Thread.h"
#include "../util/thread/CallbackTemplate.h"
#include "../util/stream/ppivalues.h"

#include "../database/logger/lib/logstructures.h"

#include "../pattern/util/LogHolderPattern.h"
#include "../pattern/util/IOutMeasureSet.h"

#include "../server/libs/client/SocketClientConnection.h"

namespace ports
{
	using namespace std;
	using namespace server;

	class ReadWorker : public CallbackTemplate
	{
	public:
		/**
		 * structure of time names
		 * which subroutine should holding
		 * as activation time
		 */
		enum holdTime_e
		{
			/**
			 * before any communication
			 * to server done
			 */
			start= 0,
			/**
			 * after first connection
			 * to server done
			 */
			connect,
			/**
			 * after finish sending address
			 * to server
			 */
			send,
			/**
			 * after read full answer from server
			 */
			end
		};
		/**
		 * Constructor to set thread name
		 *
		 * @param folder name of folder
		 * @param subroutine name of subroutine
		 * @param valueSet subroutine where can set finished value
		 */
		ReadWorker(const string& folder, const string& subroutine, IOutMeasureSet* valueSet)
		: CallbackTemplate(folder+":"+subroutine+"_READ-workerthread"),
		  m_bDebug(false),
		  m_sFolder(folder),
		  m_sSubroutine(subroutine),
		  m_pValueSet(valueSet),
		  m_STARTMUTEX(getMutex("STARTMUTEX")),
		  m_STARTINGCONDITION(getCondition("STARTINGCONDITION"))
		{};
		/**
		 * initialing callback to read from address
		 *
		 * @param address URI address to read
		 * @param holdConnection whether client should hold connection
		 * @param etime which activation time subroutine should holding
		 * @param debugShowContent whether need for debugging hole content to display
		 */
		bool initial(URL address, bool holdConnection, holdTime_e etime, bool debugShowContent);
		/**
		 * set subroutine for output doing actions
		 *
		 * @param whether should write output
		 */
		void setDebug(bool bDebug);
		/**
		 * start behavior to starting subroutine per time
		 *
		 * @param tm time to starting subroutine action
		 * @param from which subroutine starting external run
		 * @return whether starting was successful
		 */
		bool startingBy(const ppi_time& tm, const InformObject& from);
		/**
		 * make http connection with set parameters
		 *
		 * @param curValue current value
		 * @param debug whether reading is set for debugging session
		 * @return measured value with last changing time when not changed by self
		 */
		auto_ptr<IValueHolderPattern> doHttpConnection(const ppi_value& curValue, bool debug);
		/**
		 * write debug string to output
		 */
		void writeDebug(const IValueHolderPattern* value= NULL);
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 * @return object of error handling
		 */
		OVERWRITE EHObj stop(const bool bWait)
		{ return ReadWorker::stop(&bWait); };
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 * @return object of error handling
		 */
		OVERWRITE EHObj stop(const bool *bWait= NULL);
		/**
		 * destructor to destroy
		 * mutex and condition
		 */
		virtual ~ReadWorker()
		{	DESTROYMUTEX(m_STARTMUTEX);
			DESTROYCOND(m_STARTINGCONDITION);	};

	protected:
		/**
		 * working routine of callback class
		 */
		OVERWRITE bool runnable();
		/**
		 * fill string for debug session
		 *
		 * @param str string to fill
		 */
		void fillDebug(const string& str);

	private:
		/**
		 * whether starting routine running
		 * in debug session
		 */
		bool m_bDebug;
		/**
		 * folder name
		 * for which callback thread running
		 */
		const string m_sFolder;
		/**
		 * subroutine name
		 * for which callbavk thread running
		 */
		const string m_sSubroutine;
		/**
		 * READ subroutine to set current value
		 * and do also output for debug session
		 */
		IOutMeasureSet* m_pValueSet;
		/**
		 * address of file or Internet URL
		 */
		URL m_sAddress;
		/**
		 * whether client should hold connection
		 */
		bool m_bHoldConnection;
		/**
		 * connection time
		 * and how often connection was used
		 */
		pair<ppi_time, unsigned short> m_tConnUsed;
		/**
		 * when server set timeout for holding connection.<br />
		 * save timeout / and max using connection
		 */
		pair<unsigned short, unsigned short> m_nTimeoutMax;
		/**
		 * whether need for debugging
		 * hole content to display
		 */
		bool m_bDebugShowContent;
		/**
		 * which activation time subroutine should holding
		 */
		holdTime_e m_eTime;
		/**
		 * socket for Internet client
		 */
		std::auto_ptr<SocketClientConnection> m_oSocket;
		/**
		 * content of reading result
		 */
		vector<string> m_sServerResult;
		/**
		 * time to starting read routine
		 */
		ppi_time m_oStartTime;
		/**
		 * which subroutine starting
		 * external run
		 */
		InformObject m_oExternalStarting;
		/**
		 * output string for debug session
		 */
		string m_sDebugOutput;
		/**
		 * mutex for starting per time
		 */
		pthread_mutex_t* m_STARTMUTEX;
		/**
		 * condition to wait for starting time
		 */
		pthread_cond_t* m_STARTINGCONDITION;
	};

} /* namespace ports */
#endif /* READWORKER_H_ */
