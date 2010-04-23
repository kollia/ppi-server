/*
 * ICommunicationPattern.h
 *
 *  Created on: 29.06.2009
 *      Author: root
 */

#ifndef ICOMMUNICATIONPATTERN_H_
#define ICOMMUNICATIONPATTERN_H_

#include "../util/ithreadpattern.h"

#include "IClientPattern.h"
#include "IFileDescriptorPattern.h"

namespace design_pattern_world {

	namespace server_pattern {

		class ICommunicationPattern :	virtual public util_pattern::IThreadPattern,
										virtual public IClientPattern
		{
		public:
			/**
			 * commit an connection to an client
			 *
			 * @param access file descriptor whitch get from  IServerConnectArtPattern by listen
			 */
			virtual void connection(SHAREDPTR::shared_ptr<IFileDescriptorPattern>& access)= 0;
			/**
			 * return actual file descriptor
			 *
			 * @return descriptor
			 */
			virtual const SHAREDPTR::shared_ptr<IFileDescriptorPattern> getDescriptor() const= 0;
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
			 * returning name of transaction
			 *
			 * @return name
			 */
			virtual string getTransactionName() const= 0;
			/**
			 * whether an client is connected
			 *
			 * @return true if an client is conected
			 */
			virtual bool hasClient() const= 0;
			/**
			 * set next communication object
			 *
			 * @param nextcomm new communication object
			 */
			virtual void setNextComm(ICommunicationPattern* nextcomm)= 0;
			/**
			 * get next communication object
			 *
			 * @return next communication object
			 */
			virtual ICommunicationPattern* getNextComm() const= 0;
			/**
			 * dummy destructor of pattern
			 */
			virtual ~ICommunicationPattern() {};
		};

	}

}

#endif /* ICOMMUNICATIONPATTERN_H_ */
