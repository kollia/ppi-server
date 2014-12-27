/**
 *   This file 'IDbFillerPattern.h' is part of ppi-server.
 *   Created on: 15.07.2014
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

#ifndef IDBFILLERPATTERN_H_
#define IDBFILLERPATTERN_H_

#include "IErrorHandlingPattern.h"
#include "IPPIDatabasePattern.h"

#include "../server/IClientSendMethods.h"

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

class IDbFillerInformPattern
{
public:
	/**
	 * informing thread to send entries to database
	 */
	virtual void informDatabase()= 0;
	/**
	 * dummy destructor of pattern
	 */
	virtual ~IDbFillerInformPattern()
	{};
};

class IDbFillerPattern : public design_pattern_world::client_pattern::IClientSendMethods,
						 public IDbFillerInformPattern
{
public:
	/**
	 * return thread name of DbFiller
	 */
	virtual string getName()= 0;
	/**
	 * cache running inside an DbFiller
	 * or used as more caches
	 */
	virtual void isRunning()= 0;
	/**
	 * fill double value over an queue into database
	 *
	 * @param folder folder name from the running thread
	 * @param subroutine name of the subroutine in the folder
	 * @param identif identification of which value be set
	 * @param value value which should write into database
	 * @param bNew whether database should actualize value for client default= false
	 */
	virtual void fillValue(const string& folder, const string& subroutine, const string& identif,
					double value, bool bNew= false)= 0;
	/**
	 * fill double value over an queue into database
	 *
	 * @param folder folder name from the running thread
	 * @param subroutine name of the subroutine in the folder
	 * @param identif identification of which value be set
	 * @param dvalues vector of more values which should write into database
	 * @param bNew whether database should actualize value for client default= false
	 */
	virtual void fillValue(const string& folder, const string& subroutine, const string& identif,
					const vector<double>& dvalues, bool bNew= false)= 0;
	/**
	 * fill debug session output from folder working list
	 * into database
	 *
	 * @param folder name of debugging folder
	 * @param subroutine name of debugging subroutine
	 * @param content output string of debug session
	 * @param time on which time subroutine proceed
	 */
	virtual void fillDebugSession(const string& folder, const string& subroutine,
					const string& content, const IPPITimePattern* time)= 0;
	/**
	 * return filled content from cache
	 *
	 * @param dbQueue database queue from cache
	 * @param msgQueue message queue from cache
	 * @param debugQueue debug session output queue from cache
	 */
	virtual void getContent(SHAREDPTR::shared_ptr<map<string, db_t> >& dbQueue,
					SHAREDPTR::shared_ptr<vector<sendingInfo_t> >& msgQueue,
					SHAREDPTR::shared_ptr<map<string, map<pair<ppi_time, string>, string > > >& debugQueue)= 0;
	/**
	 * remove all content from DbFiller
	 * and stop thread when one running
	 *
	 * @return object of error handling
	 */
	virtual EHObj remove()= 0;
	/**
	 * dummy destructor of pattern
	 */
	virtual ~IDbFillerPattern()
	{};
};

#endif /* IDBFILLERPATTERN_H_ */
