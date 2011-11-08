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
#include "../util/thread/Thread.h"

#include "ShellWriter.h"

using namespace std;
using namespace design_pattern_world;

namespace ports
{

	ShellWriter::ShellWriter(long ID, const string& user)
	{
		m_nID= ID;
		m_sUser= user;
		m_bDebug= false;
		m_DEBUGINFO= Thread::getMutex("MAXIMDEBUGINFO");
	}

	bool ShellWriter::init(const IPropertyPattern* properties)
	{
		m_sConfPath= properties->getValue("confpath");
		m_oMeasure.modifier("folder");
		m_oMeasure.modifier("name");
		if(!m_oMeasure.readFile(URL::addPath(m_sConfPath, "measure.conf")))
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
		int nRv;
		bool bchangedVec(false);
		static map<string, bool> mwait;
		static map<string, bool> mblock;
		static map<string, bool> minsert;
		bool wait, block, binsert;
		string sline;
		string execute;
		string folder, subroutine, foldsub;
		const IInterlacedPropertyPattern* pSub;
		istringstream get(command);
		SHAREDPTR::shared_ptr<CommandExec> thread;
		typedef vector<SHAREDPTR::shared_ptr<CommandExec> >::iterator thIt;


		get >> folder;
		get >> subroutine;
		foldsub= folder + ":" + subroutine;
		if(mwait.find(foldsub) != mwait.end())
		{
			wait= mwait[foldsub];
			block= mblock[foldsub];
			binsert= minsert[foldsub];
		}else
		{
			wait= false;
			block= false;
			binsert= false;
		}
		if(block == false)
		{
			wait= false;
			//thread= SHAREDPTR::shared_ptr<CommandExec>(new CommandExec());
			thread= SHAREDPTR::shared_ptr<CommandExec>(new CommandExec());
			pSub= m_oMeasure.getSection("folder", folder);
			pSub= pSub->getSection("name", subroutine);
			get >> sline;
			execute= pSub->getValue(sline);
			get >> sline;
			if(sline == "wait")
			{
				wait= true;
				get >> sline;
				if(sline == "block")
					block= true;
			}
		}else
		{// when subroutine defined with action property block
		 // variable m_vCommandThread have only one thread inside
			thread= m_vCommandThreads[0];
		}
		nRv= CommandExec::command_exec(thread, execute, result, more, wait, block);
		do{// remove all not needed threads from vector
			bchangedVec= false;
			for(thIt it= m_vCommandThreads.begin(); it != m_vCommandThreads.end(); ++it)
			{
				if(!(*it)->running())
				{
					m_vCommandThreads.erase(it);
					bchangedVec= true;
					break;
				}
			}
		}while(bchangedVec);
		if(	wait == false ||
			block == true ||
			thread->running()	)
		{// give CommandExec inside queue and delete object by next pass
		 // when he was stopping between
			if(binsert == false)
			{
				m_vCommandThreads.push_back(thread);
				binsert= true;
			}
		}
		// set variable for next pass
		if(more == false)
		{
			mwait[foldsub]= false;
			mblock[foldsub]= false;
			minsert[foldsub]= false;

		}else
		{
			mwait[foldsub]= wait;
			mblock[foldsub]= block;
			minsert[foldsub]= binsert;
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
	}

}
