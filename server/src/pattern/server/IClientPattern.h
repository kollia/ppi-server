/*
 * ICommunicationPattern.h
 *
 *  Created on: 29.06.2009
 *      Author: root
 */

#ifndef ICLIENTPATTERN_H_
#define ICLIENTPATTERN_H_

#include <string>

using namespace std;

namespace design_pattern_world {

	namespace server_pattern {

		class IClientPattern
		{
		public:
			/**
			 * returning the default communication ID
			 * wich needet by starting an new connection.
			 * Set by creating
			 *
			 * @return default client ID
			 */
			virtual unsigned int getDefaultID() const= 0;
			/**
			 * returning the actual communication ID of the thread
			 *
			 * @return communication ID
			 */
			virtual unsigned int getConnectionID() const= 0;
			/**
			 * whether an client is connected
			 *
			 * @return true if an client is conected
			 */
			virtual bool hasClient() const= 0;
			/**
			 * search whether client with given defined name
			 * is the correct one
			 *
			 * @param definition defined name to find client
			 * @return whether client is correct with given definition
			 */
			virtual bool isClient(const string& definition) const= 0;
			/**
			 * send string to actual <code>ITransferPattern</code>
			 *
			 * @param str string which shold send to client
			 * @param wait whether method should wait for an answer
			 * @param endString if sending client want an array, this is the last string for ending
			 * @return answer from client
			 */
			virtual string sendString(const string& str, const bool& wait, const string& endString)= 0;
			/**
			 * dummy destructor of pattern
			 */
			virtual ~IClientPattern() {};
		};

	}

}

#endif /* ICLIENTPATTERN_H_ */
