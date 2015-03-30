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

#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <string>

#include "../pattern/util/LogHolderPattern.h"

#include "../util/URL.h"
#include "../util/exception.h"
#include "../util/thread/Thread.h"

#include "../database/lib/DbInterface.h"

#include "ShellWriter.h"

using namespace std;
using namespace ppi_database;
using namespace design_pattern_world;

namespace ports
{

	ShellWriter::ShellWriter(long ID, const string& user)
	{
		m_nID= ID;
		m_sUser= user;
		m_bDebug= false;
		m_DEBUGINFO= Thread::getMutex("MAXIMDEBUGINFO");
		m_WRITTENVALUES= Thread::getMutex("WRITTENVALUES");
	}

	bool ShellWriter::init(const IPropertyPattern* properties)
	{
		PPIConfigFiles configFiles;

		configFiles= PPIConfigFileStructure::instance();
		if(!configFiles->readMeasureConfig())
		{
			string err("cannot read measure.conf for SHELL owreader and account ");

			err+= m_sUser;
			cerr << "### ERROR: " << err << endl;
			cerr << "           " << "so do not start this owreader" << endl;
			err+= "\n so do not start this owreader";
			LOG(LOG_ERROR, err);
			return false;
		}
		return true;
	}

	void ShellWriter::usePropActions(const IActionPropertyPattern* properties) const
	{
	}

	short ShellWriter::useChip(const IActionPropertyMsgPattern* prop, string& id, unsigned short& kernelmode)
	{
		return 1;
	}

	vector<string> ShellWriter::getUnusedIDs() const
	{
		vector<string> unused;
		return unused;
	}

	bool ShellWriter::connect()
	{
		return true;
	}

	void ShellWriter::disconnect()
	{
	}

	string ShellWriter::getChipType(string ID)
	{
		return m_sUser + " shell";
	}

	//string ShellWriter::getConstChipTypeID(const string ID) const
	string ShellWriter::getChipTypeID(const string pin)
	{
		ostringstream id;

		id << m_nID;
		return id.str();
	}

	vector<string> ShellWriter::getChipIDs() const
	{
		vector<string> ids;
		return ids;
	}

	bool ShellWriter::existID(const string type, string ID) const
	{
		if(type != "SHELL")
			return false;
		if(ID == m_sUser)
			return true;
		return false;
	}

	bool ShellWriter::isDebug()
	{
		bool debug;

		Thread::LOCK(m_DEBUGINFO);
		debug= m_bDebug;
		Thread::UNLOCK(m_DEBUGINFO);
		return debug;
	}

	void ShellWriter::setDebug(const bool debug)
	{
		Thread::LOCK(m_DEBUGINFO);
		m_bDebug= debug;
		Thread::UNLOCK(m_DEBUGINFO);
	}

	short ShellWriter::write(const string id, const double value, const string& addinfo)
	{
		// Shell writer get no writing counts from subroutine SHELL
		return 0;
	}

	short ShellWriter::read(const string id, double &value)
	{
		// Shell writer do not have to read any counts
		return 0;
	}

	int ShellWriter::command_exec(const string& command, vector<string>& result, bool& more)
	{
		short nCommand(0);
		int nRv;
		bool bchangedVec(false), bInfo;
		bool wait(false), block(false), debug(false), bLogError;
		bool bStarting(false);
		string sline;
		string execute;
		string folder, subroutine, foldsub;
		SHAREDPTR::shared_ptr<IActionPropertyPattern> pSub;
		DbInterface* db;
		istringstream get(command);
		map<string, bool>::iterator itFoundBlock;
		SHAREDPTR::shared_ptr<CommandExec> thread;
		ppi_time tmStarting;
		typedef vector<SHAREDPTR::shared_ptr<CommandExec> >::iterator thIt;
		typedef map<string, SHAREDPTR::shared_ptr<CommandExec> >::iterator blIt;
		PPIConfigFiles configFiles;
		InformObject oExternalStarting;

		configFiles= PPIConfigFileStructure::instance();
		get >> folder;
		get >> subroutine;
		foldsub= folder + ":" + subroutine;
		pSub= configFiles->getSubroutineProperties(folder, subroutine);
		bLogError= !pSub->haveAction("noerrorlog");
		bInfo= !pSub->haveAction("noinfo");
		get >> sline;
		if(	sline != "read" &&
			sline != "info"		)
		{
			if(	sline == "starting")
			{
				string time;

				bStarting= true;
				nCommand= 2;
				/*
				 * reading time string
				 * by 3 parts
				 */
				get >> time;
				get >> sline;
				time+= " " + sline;
				get >> sline;
				time+= " " + sline;
				if(!tmStarting.read(time, "%d.%m.%Y %H:%M:%S %N"))
				{
					ostringstream msg;

					msg << "ERROR: by trying to start CommandExec thread "
									"inside SchellWriter by command '" << execute << "'\n"
									"for subroutine object of " << folder << ":" << subroutine << endl;
					msg << "       " << tmStarting.errorStr();
					cerr << msg.str() << endl;
					LOG(LOG_ALERT, msg.str());
					return -3;
				}
				/*
				 * creating InformObject
				 * from rest of string
				 */
				oExternalStarting.readDefString(get.str());
				sline= "command";

			}else if(sline == "begincommand")
				nCommand= 1;
			else if(sline == "whilecommand")
				nCommand= 2;
			else if(sline == "endcommand")
				nCommand= 3;
			execute= pSub->getValue(sline);
			execute= configFiles->createCommand(folder, subroutine, sline, execute);

		}else
			execute= sline;
		while(!get.eof())
		{
			get >> sline;
			if(sline == "wait")
				wait= true;
			else if(sline == "block")
				block= true;
			else if(sline == "debug")
				debug= true;
			else if(sline == "nodebug")
				debug= false;
		}
		// incoming more is for set subroutine to 0 when
		// no shell command be starting (no ERROR)
		if(!wait)
			more= !pSub->haveAction("last");
		else
			more= false;
		if(block == false)
		{
			for(thIt it= m_vCommandThreads.begin(); it != m_vCommandThreads.end(); ++it)
			{
				if(	!(*it)->stopping() &&
					(*it)->wait()			)
				{
					thread= *it;
					break;
				}
			}
			if(thread == NULL)
			{
				EHObj res;

				db= DbInterface::instance();
				thread= SHAREDPTR::shared_ptr<CommandExec>(new CommandExec(db, bLogError, bInfo));
				thread->setFor(folder, subroutine);
				res= thread->start();
				if(res->fail())
				{
					int log;
					string msg;

					res->addMessage("ShellWriter", "CommandExecStartInit",
									folder + "@" + subroutine);
					msg= res->getDescription();
					if(res->hasError())
					{
						log= LOG_ERROR;
						cerr << glob::addPrefix("### ERROR: ", msg) << endl;
					}else
					{
						log= LOG_WARNING;
						cout << glob::addPrefix("### WARNING: ", msg) << endl;
						m_vCommandThreads.push_back(thread);
					}
					LOG(log, msg);
					return -2;
				}else
					m_vCommandThreads.push_back(thread);
			}
		}else
		{// when subroutine defined with action property block
		 // variable m_vCommandThread should have only one thread inside
			blIt pfound;

			pfound= m_msoBlockThreads.find(foldsub);
			if(pfound == m_msoBlockThreads.end())
			{
				EHObj res;

				if(execute == "info")
					return 0;
				db= DbInterface::instance();
				thread= SHAREDPTR::shared_ptr<CommandExec>(new CommandExec(db, bLogError, bInfo));
				thread->setFor(folder, subroutine);
				res= thread->start();
				if(res->fail())
				{
					int log;
					string msg;

					res->addMessage("ShellWriter", "CommandExecStart",
									execute + "@" + folder + "@" + subroutine);
					msg= res->getDescription();
					if(res->hasError())
					{
						log= LOG_ERROR;
						cerr << glob::addPrefix("### ERROR: ", msg) << endl;
					}else
					{
						log= LOG_WARNING;
						cout << glob::addPrefix("### WARNING: ", msg) << endl;
						m_vCommandThreads.push_back(thread);
					}
					LOG(log, msg);
					return -2;
				}else
					m_msoBlockThreads[foldsub]= thread;

			}else
				thread= pfound->second;
			if(!thread->running())
			{
				LOCK(m_WRITTENVALUES);
				m_msdWritten.clear();
				UNLOCK(m_WRITTENVALUES);
			}
		}
		thread->setWritten(&m_msdWritten, m_WRITTENVALUES, nCommand);
		if(bStarting)
		{
			nRv= 0;
			if(!thread->startingBy(tmStarting, execute, oExternalStarting))
				nRv= -4;
		}else
			nRv= CommandExec::command_exec(thread, execute, result, more, wait, block, debug);
		try{
			do{// remove all not needed threads from vector
				unsigned short count= 1;

				bchangedVec= false;
				for(thIt it= m_vCommandThreads.begin(); it != m_vCommandThreads.end(); ++it)
				{
					if(	!(*it)->running()			)
					{
						try{
							m_vCommandThreads.erase(it);

						}catch(SignalException& ex)
						{
							ostringstream msg;

							msg << "try to erase " << count << ". thread ";
							msg << "of " << m_vCommandThreads.size() << " exist";
							ex.addMessage(msg.str());
							throw ex;
						}
						bchangedVec= true;
						break;
					}
					++count;
				}
			}while(bchangedVec);
		}catch(SignalException& ex)
		{
			string err;

			ex.addMessage("remove all not needed CommandExec threads");
			err= ex.getTraceString();
			cout << endl << err << endl;
			LOG(LOG_ERROR, err);
		}
		return nRv;
	}

	void ShellWriter::range(const string id, double& min, double& max, bool &bfloat)
	{
		min= 0;
		max= -1;
		bfloat= false;
	}

	void ShellWriter::endOfCacheReading(const double cachetime)
	{

	}

	bool ShellWriter::reachAllChips()
	{
		return true;
	}

	void ShellWriter::endOfLoop()
	{
		// nothing to do
	}

	ShellWriter::~ShellWriter()
	{
		DESTROYMUTEX(m_DEBUGINFO);
		DESTROYMUTEX(m_WRITTENVALUES);
	}

}
