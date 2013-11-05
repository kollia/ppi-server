/**
 *   This file 'NoAnswerSender.h' is part of ppi-server.
 *   Created on: 02.11.2013
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

#ifndef NOANSWERSENDER_H_
#define NOANSWERSENDER_H_

#include <string>

#include "../../../util/thread/Thread.h"

#include "../../../pattern/server/IExternClientInput.h"

using namespace std;
using namespace design_pattern_world::client_pattern;

namespace util
{

	/**
	 * sending all questions which reach ExternClientInputTemplate
	 * and need no really answer
	 */
	class NoAnswerSender : public Thread
	{
	public:
		/**
		 * structure for an sending question which need no answer.<br />
		 * this will be done from an seperate thread
		 */
		struct sendingInfo_t
		{
			/**
			 * to which process the question should be send
			 */
			string toProcess;
			/**
			 * question method object with all parameters
			 */
			string method;
			/**
			 * when answer (which not needed, but its possible)
			 * have more than one strings, there should be defined
			 * and end message string
			 */
			string done;
		};
		/**
		 * constructor of object
		 *
		 * @param threadName name of thread
		 * @param objCaller class object of ExternClientInputTemplate which need this sender thread
		 */
		NoAnswerSender(const string& threadName, IExternClientInput* objCaller)
		: Thread("NoAnswerThread_for_" + threadName, false),
		  m_pExternClient(objCaller),
		  m_SENDQUEUELOCK(getMutex("STENDQUEUELOCK")),
		  m_SENDQUEUECONDITION(getCondition("SENDQUEUECONDITION"))
		{};
		/**
		 * send message to given server in constructor
		 * or write into queue when no answer be needed
		 *
		 * @param toProcess for which process the method should be
		 * @param method object of method which is sending to server
		 * @param done on which getting string the answer should ending. Ending also when an ERROR or warning occurs
		 * @return backward send return string vector from server if answer is true, elsewhere returning vector with no size
		 */
		vector<string> sendMethod(const string& toProcess, const OMethodStringStream& method, const string& done);
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		virtual int stop(const bool bWait)
		{ return NoAnswerSender::stop(&bWait); };
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		virtual int stop(const bool *bWait= NULL);
		/**
		 * calculate the error code given back from server as string.<br />
		 * the return error codes from server should be ERROR or WARNING.
		 * If the returned string was an warning, the number will be multiplied with -1 (become negative)
		 * Elsewhere the number is 0
		 *
		 * @param input the returned string from server
		 * @return error number
		 */
		int error(const string& input);
		/**
		 * destructor of object
		 */
		virtual ~NoAnswerSender();

	protected:
		/**
		 * dummy initialization of thread
		 *
		 * @param args user defined parameter value or array,<br />
		 * 				comming as void pointer from the external call
		 * 				method start(void *args).
		 * @return defined error code from extended class
		 */
		virtual int init(void *args)
		{ return 0; };
		/**
		 * abstract method to running thread
		 * in the extended class.<br />
		 * This method starting again when ending without an sleeptime
		 * if the method stop() isn't call.
		 *
		 * @return defined error code from extended class
		 */
		virtual int execute();
		/**
		 * dummy method to ending the thread
		 */
		virtual void ending()
		{};

	private:
		/**
		 * ExternClientInputTemplate which trigger new sending questions
		 * which do not have to wait for answer
		 */
		IExternClientInput* m_pExternClient;
		/**
		 * queue of question methods which need no answer
		 */
		vector<sendingInfo_t> m_vsSendingQueue;
		/**
		 * last answer from sending question
		 * which need no answer.<br />
		 * could be an error/warning message
		 */
		string m_sNoWaitError;
		/**
		 * mutex lock for write sending messages
		 * into an queue which are no answer needed
		 */
		pthread_mutex_t* m_SENDQUEUELOCK;
		/**
		 * condition to wait for new sending messages
		 */
		pthread_cond_t* m_SENDQUEUECONDITION;
	};

} /* namespace ppi_database */
#endif /* NOANSWERSENDER_H_ */
