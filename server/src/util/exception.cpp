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

//#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <execinfo.h>
#include <errno.h>
#include <string.h>

#include <cstdlib>
//#include <cstring>

#include <exception>
#include <fstream>
#include <sstream>
#include <iostream>

#include "exception.h"
#include "GlobalStaticMethods.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

using namespace boost;

SignalException::SignalException() throw()
{
	m_vStackTrace= trace(5);
	m_nThreadID= syscall(SYS_gettid);
}

SignalException::SignalException(const string& errortxt) throw()
{
	m_sErrorText= errortxt;
	m_vStackTrace= trace(5);
	m_nThreadID= syscall(SYS_gettid);
}

SignalException::~SignalException() throw()
{
	//signal(getSignal(), SignalTranslator<SignalException>::SingleTonTranslator::SignalHandler);
}

vector<string> SignalException::getFirstTraceVector() const
{
	string errortxt;
	vector<string> vRTrace;
	sigset_t signal_set;

	sigemptyset(&signal_set);
	if(sigaddset(&signal_set, getSignal()) != 0)
		cout << "cannot add signal " << getSignalName() << " from class " << getClassName() << endl;
	if(sigprocmask(SIG_UNBLOCK, &signal_set, NULL) != 0)
		cout << "cannot unblock signal" << getSignalName() << endl;

	if(m_sErrorText == "")
		errortxt= getMainErrorMessage();
	else
		errortxt= m_sErrorText;
	vRTrace.push_back("__Exception " + errortxt);
	vRTrace.insert(vRTrace.end(), m_vStackTrace.begin(), m_vStackTrace.end());
	m_bInit= true;
	return vRTrace;
}

string SignalException::getMainErrorMessage() const
{
	ostringstream msg;

	if(m_sErrorText != "")
		return m_sErrorText;
	msg << "from class " << getClassName() << "() ";
	if(getSignalName() != "")
		msg << "for signal " << getSignalName() << " ";
	msg << " inside process \"" << glob::getProcessName() << "\" " << getpid() << ":" << getThreadID();
	return msg.str();
}

vector<string> SignalException::trace(const unsigned short del) const
{
	vector<string> vRv;
    void * array[TRACELINES];
    int nSize = backtrace(array, TRACELINES);
    char ** symbols = backtrace_symbols(array, nSize);

    for (int i = 0; i < nSize; i++)
    {
    	ostringstream line;

    	//if(del==0)
    	//	cout << symbols[i] << endl;
        line << symbols[i];
        if(i > (del - 1))
        	vRv.push_back(line.str());
    }
    if(nSize == TRACELINES)
    	vRv.push_back("[...]");
    //if(del==0)
    //	cout << endl << endl;
    free(symbols);
	return vRv;
}

void SignalException::addMessage(const string& message)
{
	typedef vector<string>::iterator it;
	bool bIns(false);
	string line, traceline;
	vector<string> spl, vTrace, newTrace;
	it spIt;

	if(!m_bInit)
		m_vStackTrace= getFirstTraceVector();
	split(spl, message, is_any_of("\n"));
	vTrace= trace(2);
	traceline= *vTrace.begin();
	traceline= traceline.substr(0, traceline.find("+") + 1);
	//cout << "___________search about first line " << traceline << endl;
	for(it mit= m_vStackTrace.begin(); mit != m_vStackTrace.end(); ++mit)
	{
		line= *mit;
		line= line.substr(0, line.find("+") + 1);
		if(	!bIns &&
			line == traceline	)
		{
			bIns= true;
			spIt= spl.begin();
			line= *spIt;
			trim(line);
			line= "__caused by " + line;
			newTrace.push_back(line);
			++spIt;
			for(it sp2It= spIt; sp2It != spl.end(); ++sp2It)
			{
				line= *sp2It;
				trim(line);
				line= "__          " + line;
				newTrace.push_back(line);
			}
		}
		newTrace.push_back(*mit);
	}
	if(!bIns)
	{
		newTrace.push_back("");
		spIt= spl.begin();
		line= *spIt;
		trim(line);
		line= "__caused by " + line;
		newTrace.push_back(line);
		++spIt;
		for(it sp2It= spIt; sp2It != spl.end(); ++sp2It)
		{
			line= *sp2It;
			trim(line);
			line= "__          " + line;
			newTrace.push_back(line);
		}
	}
	m_vStackTrace= newTrace;
}

const char* SignalException::what() const throw()
{
	m_sErrorText= "uncaught " + getTraceString();
	return m_sErrorText.c_str();
}

vector<string> SignalException::getTraceVector() const
{
	vector<string> vRv;
	vector<string> vTrace;

	if(!m_bInit)
		vTrace= getFirstTraceVector();
	else
		vTrace= m_vStackTrace;
	for(vector<string>::const_iterator it= vTrace.begin(); it != vTrace.end(); ++it)
	{
		if(it->substr(0, 2) == "__")
			vRv.push_back(it->substr(2));
		else
			vRv.push_back(SPACELEFT + *it);
	}
	return vRv;
}

string SignalException::getTraceString() const
{
	string sRv;
	vector<string> vTrace;

	if(!m_bInit)
		vTrace= getFirstTraceVector();
	else
		vTrace= m_vStackTrace;
	for(vector<string>::const_iterator it= vTrace.begin(); it != vTrace.end(); ++it)
	{
		if(it->substr(0, 2) == "__")
			sRv+= it->substr(2) + "\n";
		else
			sRv+= SPACELEFT + *it + "\n";
	}
	return sRv;
}

void SignalException::printTrace() const
{
	vector<string> vTrace;

	if(!m_bInit)
		vTrace= getFirstTraceVector();
	else
		vTrace= m_vStackTrace;
	cout << endl;
	for(vector<string>::const_iterator it= vTrace.begin(); it != vTrace.end(); ++it)
	{
		if(it->substr(0, 2) == "__")
			cout << it->substr(2) << endl;
		else
			cout << SPACELEFT << *it << endl;
	}
}

SignalTranslator<SegmentationFault> g_objSegmentationFaultTranslator;
SignalTranslator<FloatingPointException> g_objFloatingPointExceptionTranslator;


