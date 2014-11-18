/**
 *   This file 'ReadWorker.cpp' is part of ppi-server.
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

#include <boost/algorithm/string/trim.hpp>

#include "ReadWorker.h"

namespace ports
{
	using namespace boost;

	bool ReadWorker::initial(URL address, bool holdConnection, holdTime_e etime, bool debugShowContent)
	{
		m_sAddress= address;
		if(m_sAddress.getProtocol() == http)
		{
			m_sAddress.encode();
			m_oSocket= auto_ptr<SocketClientConnection>(
							new SocketClientConnection(	SOCK_STREAM,
														m_sAddress.getHost(),
														m_sAddress.getPort(),
														10			)	);
		}else
		{
			string err;

			err=  "ERROR: for READ subroutine " + m_sFolder + ":" + m_sSubroutine + "\n";
			err+= "       can only read on protocol http://\n";
			err+= "       but inside src parameter is defined:\n";
			err+= "                     " + m_sAddress.getBaseUri();
			m_pValueSet->out() << err << endl;
			LOG(LOG_ERROR, err);
			return false;
		}
		m_bHoldConnection= holdConnection;
		m_eTime= etime;
		m_bDebugShowContent= debugShowContent;
		return true;
	}

	bool ReadWorker::startingBy(const ppi_time& tm)
	{
		LOCK(m_STARTMUTEX);
		m_oStartTime= tm;
		AROUSE(m_STARTINGCONDITION);
		UNLOCK(m_STARTMUTEX);
		return true;
	}

	bool ReadWorker::runnable()
	{
		LOCK(m_STARTMUTEX);
		while(!stopping())
		{
			if(!m_oStartTime.isSet())
				CONDITION(m_STARTINGCONDITION, m_STARTMUTEX);
			if(m_oStartTime.isSet())
			{
				timespec tmtime;

				tmtime.tv_sec= m_oStartTime.tv_sec;
				tmtime.tv_nsec= (m_oStartTime.tv_usec * 1000);
				if(TIMECONDITION(m_STARTINGCONDITION, m_STARTMUTEX, &tmtime) == ETIMEDOUT)
				{
					auto_ptr<IValueHolderPattern> oValue;

					oValue= doHttpConnection(0, m_bDebug);
					m_pValueSet->setValue(m_sFolder, m_sSubroutine, *(oValue.get()),
									"i:"+m_sFolder+":"+m_sSubroutine);
				}
			}
		}// while(!stopping())
		UNLOCK(m_STARTMUTEX);
		return false;
	}

	void ReadWorker::setDebug(bool bDebug)
	{
		LOCK(m_STARTMUTEX);
		m_bDebug= bDebug;
		UNLOCK(m_STARTMUTEX);
	}

	auto_ptr<IValueHolderPattern> ReadWorker::doHttpConnection(const ppi_value& curValue, bool debug)
	{
		bool bHoldConnection;
		int connResult(0);
		EHObj errHandle;
		string result;
		ostringstream oSendRequest;
		ppi_time currentTime;
		ppi_time connectTime[4];
		auto_ptr<IValueHolderPattern> oValue(new ValueHolder);
		SHAREDPTR::shared_ptr<IFileDescriptorPattern> descriptor;

		if(m_eTime == start)
		{
			if(!currentTime.setActTime())
			{
				string err("cannot set starting time for READ subroutine ");

				err+= m_sFolder + ":" + m_sSubroutine + "\n";
				err+= currentTime.errorStr() + "\n";
				err+= "so take current subroutine changing time";
				if(debug)
					m_pValueSet->out() << err << endl;
				TIMELOG(LOG_WARNING, "start-time-set", err);
			}
			oValue->setTime(currentTime);
		}
		if(debug)
		{// start connection (start)
			connectTime[0].setActTime();
		}
		bHoldConnection= m_bHoldConnection;
		while(1)
		{
			bool bNewConnect;
			/*
			 * BLOCK to start connection
			 *       and can continue by bHoldConnected is true
			 *       when first get no answer
			 *       connection maybe was closed from server
			 */
			if(!bHoldConnection)
				bNewConnect= true;
			else if(m_oSocket->connected())
			{// check first whether connection timeout be reached, when exist
				bool bClose(false);
				ostringstream debugOut;

				if(debug)
				{
					debugOut << endl;
					debugOut << "connection timeout set before from server" << endl;
					debugOut << "   Keep-Alive: timeout=" << m_nTimeoutMax.first
									<< ", max=" << m_nTimeoutMax.second << endl;
				}
				if(m_nTimeoutMax.first > 0)
				{
					ppi_time cur, last(m_tConnUsed.first);

					if(cur.setActTime())
					{
						last.tv_sec+= static_cast<__time_t>(m_nTimeoutMax.first);
						bClose= cur > last ? true : false;
						if(	debug &&
							bClose	)
						{
							debugOut << "first connection " << m_tConnUsed.first.toString(/*date*/true)
											<< " is older then " << m_nTimeoutMax.first << " seconds" << endl;
						}
					}else
					{
						string err("cannot set current time for READ subroutine ");

						err+= m_sFolder + ":" + m_sSubroutine;
						err+= " to measure how long connection is open\n";
						err+= cur.errorStr();
						debugOut << err << endl;
						err+= "so close connection and reinitialize";
						TIMELOG(LOG_INFO, "hold-connection-measure", err);
						bClose= true;
					}

				}
				if(	!bClose &&
					m_nTimeoutMax.second > 0 &&
					m_tConnUsed.second >= m_nTimeoutMax.second	)
				{
					bClose= true;
					if(debug)
						debugOut << "connection was used more than " << m_nTimeoutMax.second << " times" << endl;
				}
				if(bClose)
				{
					bNewConnect= true;
					m_oSocket->close();
					if(debug)
					{
						debugOut << "so close connection and reinitialize";
						m_pValueSet->out() << debugOut.str() << endl;
					}
				}else
					bNewConnect= false;
			}else
				bNewConnect= true;
			if(bNewConnect)
			{
				if(!m_tConnUsed.first.setActTime() &&
					m_bHoldConnection					)
				{
					string err("cannot set connection beginning time for READ subroutine ");

					err+= m_sFolder + ":" + m_sSubroutine + "\n";
					err+= m_tConnUsed.first.errorStr() + "\n";
					err+= "so close connection by every new activating of subroutine";
					if(debug)
						m_pValueSet->out() << err << endl;
					TIMELOG(LOG_WARNING, "set-connection-begin-time", err);
				}
				m_tConnUsed.second= 0;
				m_nTimeoutMax.first= 0;
				m_nTimeoutMax.second= 0;
				if(debug)
				{
					string outstr;

					if(	m_bHoldConnection == false ||
						bHoldConnection == true			)
					{
						outstr= "\nInitialize ";
					}else
						outstr= "\nReinitialize ";
					outstr+= "connection to server ";
					outstr+= m_sAddress.getHost();
					m_pValueSet->out() << outstr << endl;
				}
				errHandle= m_oSocket->init();
				if(errHandle->hasError())
					connResult= -1;
			}else
			{
				if(debug)
					m_pValueSet->out() << "Try to take existing connection to server" << endl;
//				connResult= m_oSocket->reconnect();
//				if(connResult == 28/*EISCONN*/)	// The socket is already connected.
//					connResult= 0;				// but do not read correctly (don't know why)
												// try to reconnect because otherwise by closing
												// throw exception by freeaddrinfo()
			}
			if(connResult == 0)
			{
				bool berror;
				int nRetCode;
				string res;
				istringstream output;
				string request, debugOutStr;
				ostringstream str;

				if(	m_eTime == connect ||
					m_eTime == send			)
				{
					/*
					 * when m_eTime is send
					 * calculate the half time
					 * between connected and
					 * get back the first package
					 * maybe in this time the server
					 * get the request
					 */
					if(!currentTime.setActTime())
					{
						string err("cannot set connection time for READ subroutine ");

						err+= m_sFolder + ":" + m_sSubroutine + "\n";
						err+= currentTime.errorStr() + "\n";
						err+= "so take current subroutine changing time";
						if(debug)
							m_pValueSet->out() << err << endl;
						TIMELOG(LOG_WARNING, "connect-time-set", err);
					}
					oValue->setTime(currentTime);
				}
				if(debug)
				{// is connected (connect)
					connectTime[1].setActTime();
				}
				descriptor= m_oSocket->getDescriptor();
				request= m_sAddress.getAbsolutePathQuery();
				oSendRequest << "GET " << request << " HTTP/1.1" << endl;
				str << "User-Agent: ppi-server/";
				str << PPI_MAJOR_RELEASE << ".";
				str << PPI_MINOR_RELEASE << ".";
				str << PPI_SUBVERSION;
				oSendRequest << str.str() << endl;
				if(debug)
				{
					debugOutStr+= "send request of '" + request + "'\n";
					debugOutStr+= "            for '" + str.str() + "'\n";
				}
				request= "Accept: text/html,application/xhtml+xml";
				oSendRequest << request << endl;
				if(debug)
				{
					debugOutStr+= "                '" + request + "'\n";
					if(m_bHoldConnection)
						debugOutStr+= "                 Keep Connection Alive\n";
					else
						debugOutStr+= "                 close Connection\n";
				}
				oSendRequest << "Connection: ";
				if(m_bHoldConnection)
					oSendRequest << "Keep-Alive";
				else
					oSendRequest << "close";
				oSendRequest << endl;
				oSendRequest << "Host: " << m_sAddress.getHost() << ":" << m_sAddress.getPort();
				oSendRequest << endl << endl;
				berror= false;
				try{
					(*descriptor) << oSendRequest.str();
					descriptor->flush();
				}catch(...)
				{
					berror= true;
				}
				// end of HEADER sending --------------------------------------------------------
				// ------------------------------------------------------------------------------

				if(!berror)
				{
					// reading first answer of server
					try{
						(*descriptor) >> result;
					}catch(...)
					{
						berror= true;
					}
					if(debug)
					{// has send request (send)
						connectTime[2].setActTime();
					}
					if(m_eTime == send)
					{
						if(!currentTime.setActTime())
						{
							string err("cannot set after sending time for READ subroutine ");

							err+= m_sFolder + ":" + m_sSubroutine + "\n";
							err+= currentTime.errorStr() + "\n";
							err+= "so take current subroutine changing time";
							if(debug)
								m_pValueSet->out() << err << endl;
							TIMELOG(LOG_WARNING, "send-time-set", err);

						}else if(oValue->getTime().isSet())
						{
							/*
							 * get the first package
							 * take now the half time
							 * of connected and current time
							 */
							currentTime-= oValue->getTime();
							currentTime.tv_sec/= 2;
							currentTime.tv_usec/= 2;
							currentTime+= oValue->getTime();
						}
						oValue->setTime(currentTime);
					}
				}
				trim(result);
				if(	berror ||
					descriptor->error() ||
					result == ""			)
				{
					string errStr;

					m_oSocket->close();
					if(bHoldConnection)
					{// when by first trying read from server
					 // result is an empty string
					 // and m_bHoldConnection is set
					 // reconnect client to server
						if(debug)
						{
							m_pValueSet->out() << debugOutStr << endl;
							m_pValueSet->out() << "ERROR by reading answer, get no content -> so close connection to "
									<< m_sAddress.getHost() << endl;
							m_pValueSet->out() << "try again" << endl;
						}
						bHoldConnection= false;
						continue;
					}
					currentTime.clear();
					oValue->setTime(currentTime);
					oValue->setValue(-1);
					errStr= "ERROR by reading answer, get no content -> so close connection to ";
					errStr+= m_sAddress.getHost();
					if(debug)
					{
						m_pValueSet->out() << debugOutStr << endl;
						m_pValueSet->out() << errStr << endl;
					}
					TIMELOG(LOG_ERROR, "null-content-reading", errStr);
					if(debug)
						m_pValueSet->out() << debugOutStr << endl;
					connResult= -1;
					break;
				}
				if(debug)
					m_pValueSet->out() << debugOutStr << endl;

				output.str(result);
				output >> res;
				if(res.substr(0, 5) != "HTTP/")
				{
					string errStr;

					errStr=  "ERROR: get answer from no HTTP server, close connection\n";
					errStr+= "       server answered with '" + result + "'";
					if(debug)
						m_pValueSet->out() << errStr << endl;
					errStr+= "inside subroutine " + m_sFolder + ":" + m_sSubroutine + "\n";
					errStr+= "for request of '" + m_sAddress.getBaseUri() + "'";
					TIMELOG(LOG_ERROR, "wrong-server-answer", errStr);
					m_oSocket->close();
					connResult= -1;
					break;
				}
				output >> nRetCode;
				output >> res;
				oValue->setValue(static_cast<ppi_value>(nRetCode));
				if(debug)
				{
					m_pValueSet->out() << "subroutine get answer from server" << endl;
					m_pValueSet->out() << "HEADER:" << endl;
					m_pValueSet->out() << result << endl;
				}
			}// if(connResult == 0)
			break;
		}// while(1)
		if(connResult == 0)
		{
			bool bHeadEnd(false);
			unsigned int nDebugCount(0);
			string debugOutStr;

			bHoldConnection= m_bHoldConnection;
			m_sServerResult.clear();
			do{
				bool berror(false);

				try{
					(*descriptor) >> result;

				}catch(...)
				{
					berror= true;
				}
				if(	berror ||
					descriptor->error())
				{
					string errStr;

					m_oSocket->close();
					currentTime.clear();
					oValue->setTime(currentTime);
					oValue->setValue(-1);
					errStr= "ERROR by reading answer, get fault content -> so close connection to ";
					errStr+= m_sAddress.getHost() + "\n";
					if(debug)
					{
						m_pValueSet->out() << debugOutStr << endl;
						m_pValueSet->out() << errStr;
					}
					errStr+= "inside subroutine " + m_sFolder + ":" + m_sSubroutine + "\n";
					errStr+= "for request of '" + m_sAddress.getBaseUri() + "'";
					LOG(LOG_ERROR, errStr);
					break;
				}
				if(!bHeadEnd)
				{
					if(debug)
						debugOutStr+= result;
					trim(result);// need trim also to define end of HEADER
					if(bHoldConnection)
					{
						string res;
						istringstream output;

						output.str(result);
						output >> res;
						if(res == "Connection:")
						{
							output >> res;
							if(res == "close")
							{
								bHoldConnection= false;
								if(debug)
									debugOutStr+= " >> SERVER close connection\n";
							}
						}else if(res == "Keep-Alive:")
						{// server set timeout for holding connection
							istringstream oNum;
							unsigned short nNum;
							bool bTimeout(false), bMax(false);

							while(	!output.eof()	&&
									!output.fail()		)
							{
								output >> res;
								if(res.substr(0, 8) == "timeout=")
								{
									oNum.str(res.substr(8));
									oNum >> nNum;
									if(!oNum.fail())
									{
										m_nTimeoutMax.first= nNum;
										bTimeout= true;
									}

								}else if(res.substr(0, 4) == "max=")
								{
									oNum.str(res.substr(4));
									oNum >> nNum;
									if(!oNum.fail())
									{
										m_nTimeoutMax.second= nNum;
										bMax= true;
									}
								}
							}// while(!output.eof())
							if(!bTimeout)
							{
								string err("by getting Keep-Alive answer from " + m_sAddress.getHost() + "\n");

								if(!bTimeout)
									err+= "cannot read correctly timeout result\n";
								if(!bMax)
									err+= "cannot read correctly max using connection\n";
								err+= "by anser string '" + result + "'";
								if(debug)
									debugOutStr+= err + "\n";
								TIMELOG(LOG_ERROR, "Keep-Alive", err);
							}
						}// if(res == "Keep-Alive:")
					}
					if(result == "")
					{
						bHeadEnd= true;
						if(m_nTimeoutMax.second > 0)
							++m_tConnUsed.second;
						if(	!m_bHoldConnection &&
							m_eTime != end &&
							(	!m_bDebugShowContent ||
								!debug					)	)
						{
							if(debug)
							{
								if(!descriptor->eof())
									(*descriptor) >> result;
								if(!descriptor->eof())
								{
									debugOutStr= debugOutStr.substr(0, debugOutStr.length() - 2);
									debugOutStr+= " ( end reading by HEADER, "
													"because do not need content )\n";
								}
							}
							break;
						}
					}
				}else
				{
					if(debug)
					{
						if(nDebugCount == 0)
							debugOutStr+= "BODY:\n";
						if(	nDebugCount < 5 ||
							m_bDebugShowContent	)
						{
							debugOutStr+= "  > " + result;
							++nDebugCount;

						}else if(nDebugCount == 5)
						{
							debugOutStr+= "  > .....\n";
							++nDebugCount;
						}
					}
					m_sServerResult.push_back(result);
				}

			}while(!descriptor->eof());
			if(	debug &&
				debugOutStr != ""	)
			{
				m_pValueSet->out() << debugOutStr << endl;
			}

			if(bHeadEnd)
			{
				if(m_eTime == end)
				{
					if(!currentTime.setActTime())
					{
						string err("cannot set ending time for READ subroutine ");

						err+= m_sFolder + ":" + m_sSubroutine + "\n";
						err+= currentTime.errorStr() + "\n";
						err+= "so take current subroutine changing time";
						if(debug)
							m_pValueSet->out() << err << endl;
						TIMELOG(LOG_WARNING, "end-time-set", err);
					}
					oValue->setTime(currentTime);
				}
				if(debug)
				{// get all answer (end)
					ppi_time seconds[3];

					connectTime[3].setActTime();
					// calculate length of need time
					if(	!connectTime[0].error() &&
						!connectTime[1].error()		)
					{
						seconds[0]= connectTime[1] - connectTime[0];
					}
					if(	!connectTime[1].error() &&
						!connectTime[2].error()		)
					{
						/*
						 * get the first package
						 * take now the half time
						 * of connected and current time
						 */
						seconds[1]= connectTime[2] - connectTime[1];
						seconds[1].tv_sec/= 2;
						seconds[1].tv_usec/=2;
						connectTime[2]-= seconds[1];
					}
					if(	!connectTime[2].error() &&
						!connectTime[3].error()		)
					{
						seconds[2]= connectTime[3] - connectTime[2];
						seconds[2]+= seconds[1];
					}
					string outstr("\n");
					// output for debugging connection times
					outstr+= "need for connection ";
					if(connectTime[0].error())
						outstr+= connectTime[0].errorStr();
					else if(connectTime[1].error())
						outstr+= connectTime[1].errorStr();
					else
						outstr+= seconds[0].toString(/*as date*/false) + " seconds by "
												+ connectTime[1].toString(/*as date*/true);
					outstr+= "\n    sending request ";
					if(connectTime[1].error())
						outstr+= connectTime[1].errorStr();
					else if(connectTime[2].error())
						outstr+= connectTime[2].errorStr();
					else
						outstr+= seconds[1].toString(/*as date*/false) + " seconds by "
												+ connectTime[2].toString(/*as date*/true);
					outstr+= "\n get answer content ";
					if(connectTime[2].error())
						outstr+= connectTime[2].errorStr();
					else if(connectTime[3].error())
						outstr+= connectTime[3].errorStr();
					else
						outstr+= seconds[2].toString(/*as date*/false) + " seconds by "
												+ connectTime[3].toString(/*as date*/true);
					outstr+="\n";
					m_pValueSet->out() << outstr << endl;
				}
				if(!bHoldConnection)
					m_oSocket->close();
			}else
			{
				string warn;

				warn= "connection to '" + m_sAddress.getBaseUri();
				warn+= "' get no correct answer";
				if(debug)
					m_pValueSet->out() << warn << endl;
				LOG(LOG_WARNING, warn);
				currentTime.clear();
				oValue->setTime(currentTime);
				oValue->setValue(-2);
				m_oSocket->close();
			}
		}else
		{
			string err, sDecoded, sEncoded(m_sAddress.getBaseUri());
			URL decoded(m_sAddress);

			decoded.decode();
			sDecoded= decoded.getBaseUri();
			err= "can't open connection to address '" + sDecoded + "'";
			if(sDecoded != sEncoded)
				err+= "\nwas send as encoded URL '" + sEncoded + "'";
			if(	errHandle &&
				errHandle->fail()	)
			{
				err+= "\n" + errHandle->getDescription();
			}
			if(debug)
				m_pValueSet->out() << err << endl;
			LOG(LOG_ERROR, err);
			currentTime.clear();
			oValue->setTime(currentTime);
			oValue->setValue(-2);
			m_oSocket->close();
		}
		return oValue;
	}

	EHObj ReadWorker::stop(const bool *bWait/*= NULL*/)
	{
		m_pError= CallbackTemplate::stop(/*wait*/false);
		AROUSEALL(m_STARTINGCONDITION);
		if(	bWait &&
			*bWait == true &&
			!m_pError->hasError()	)
		{
			(*m_pError)= CallbackTemplate::stop(bWait);
		}
		return m_pError;
	}
} /* namespace ports */
