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
#include <stdlib.h>
#include <sys/io.h>
#include <sys/time.h>
#include <errno.h>
#include <limits.h>

#include <iostream>
#include <list>

#include "../util/structures.h"
#include "../util/thread/Terminal.h"

#include "../pattern/util/LogHolderPattern.h"

#include "portbaseclass.h"
#include "timemeasure.h"

bool TimeMeasure::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
{
#if 0
	unsigned short measuredness= 1;
	string error("");
	string sOut= properties->needValue("out");
	string sIn= properties->needValue("in");
	string sNeg= properties->getValue("neg", /*warning*/false);
	string sMeasuredness= properties->getValue("measuredness", /*warning*/false);
	Pins tOut= getPinsStruct(sOut);
	Pins tIn= getPinsStruct(sIn);
	Pins tNegative= getPinsStruct(sNeg);
	vector<correction_t> vCorrection;

	if(sMeasuredness != "")
		measuredness= atoi(&sMeasuredness[0]);
	if(tOut.ePin == NONE)
		error= "'out' ";
	if(tIn.ePin == NONE)
	{
		if(error != "")
			error+= "and ";
		error+= "'in'";
	}
	if(error != "")
	{
		string errorOut("### ERROR: in folder ");

		errorOut+= getFolderName();
		errorOut+= " and subroutine ";
		errorOut+= getSubroutineName();
		errorOut+= "\n           parameter ";
		errorOut+= error;
		errorOut+= " does not be set correctly";
		LOG(LOG_ERROR, errorOut);
		out() << errorOut << endl;
		return false;
	}
	m_tOut= tOut;
	m_tIn= tIn;
	m_tNegative= tNegative;
	portBase::setValue(0, "i:"+getFolderName()+":"+getSubroutineName());
	m_nMeasuredness= measuredness;
	m_vCorrection= *elkoCorrection;
	m_maxMeasuredTime= 1500000;
	noAfterContactPublication();
	//setCorrection(elkoCorrection);
	return true;
#endif

	properties->notAllowedAction("binary");
	if(!portBase::init(properties, pStartFolder))
		return false;
	return false;
}

bool TimeMeasure::range(bool& bfloat, double* min, double* max)
{
	bfloat= false;
	*min= 0;
	*max= (double)LONG_MAX;
	return true;
}

/*void TimeMeasure::setCorrection(vector<ohm> *elkoCorrection)
{
	unsigned int nMuch= elkoCorrection->size();
	unsigned int c= 0;

	m_dCorrection= 1;
	if(nMuch > 0)
	{
		while(c < nMuch)
		{
			if((*elkoCorrection)[c].bSetTime)
			{
				m_dCorrection= (*elkoCorrection)[c].nMikrosec;
				++c;
				break;
			}else
				++c;
		}
		for(c= c; c<nMuch; ++c)
		{
			if((*elkoCorrection)[c].bSetTime)
			{
				m_dCorrection+= (*elkoCorrection)[c].nMikrosec;
				m_dCorrection/= 2;
			}
		}
	}
}*/

/*void TimeMeasure::setGradients(vector<correction_t> correction)
{
	m_vCorrection= correction;
}*/

TimeMeasure::~TimeMeasure()
{
	//setPin(m_tOut, false);
}

IValueHolderPattern& TimeMeasure::measure(const ppi_value& actValue)
{
	char buf[150];
	string msg;
	unsigned long nLightValue= getMeasuredTime();

	if(isDebug())
	{
		sprintf(buf, "%lu", nLightValue);
		msg= "measured time:";
		msg+= buf;
		msg+=" us";
		TIMELOG(LOG_INFO, getFolderName(), msg);
		out() << msg << endl;
	}
	m_oMeasureValue.value= (double)nLightValue;
	return m_oMeasureValue;
}

unsigned long TimeMeasure::getMeasuredTime()
{
	portpin_address_t tSet;//= getPortPinAddress(m_tOut, true);
	portpin_address_t tGet;//= getPortPinAddress(m_tIn, false);
	unsigned long nSetPort= tSet.nPort;
	unsigned long nGetPort= tGet.nPort;
	int nSetPin= tSet.nPin;
	unsigned nGetPin= tGet.nPin;
	unsigned long mikroSleepTime= 0, msleeptime= 0;
	struct itimerval time;
	vector<correction_t> vCorrection;

	//setPin(m_tNegative, false);
	//setPin(m_tOut, false);

	unsigned res;

	time.it_interval.tv_sec= ITIMERSTARTSEC;
	time.it_interval.tv_usec= 0;
	time.it_value.tv_sec= ITIMERSTARTSEC;
	time.it_value.tv_usec= 0;
	if(isDebug())
	{
		out() << "maximal calculating for " << m_maxMeasuredTime << " mikroseconds" << endl;
		out() << "beginning status: ";
		res= inb(nGetPort);
		//nPrintPin= (int)res;
		//printBin(&nPrintPin, nGetPort);
		out() << "wait for          ";
		//nPrintPin= (int)nGetPin;
		//printBin(&nPrintPin, nGetPort);
	}
	lockApplication(true);
	if(setitimer(ITIMERTYPE, &time, NULL)==-1)
	{
		char cError[100];
		string msg("timer ERROR ");

		sprintf(cError, "%d: ", errno);
		msg+= cError;
		msg+= "cannot read correctly time";
		LOG(LOG_ERROR, msg);
		return m_maxMeasuredTime;
	}else
	{
		outb(inb(nSetPort) | nSetPin, nSetPort);
		//out() << flush; // flush after outb() -> maybe an bug
		while(	!(res= inb(nGetPort) & nGetPin)
				&&
				(unsigned long)getMikrotime() < m_maxMeasuredTime	);
		{
			usleep(1);
		}
		mikroSleepTime= getMikrotime();
		//setPin(m_tOut, false);
	}
	lockApplication(false);
	if(isDebug())
	{
		out() << "result:           ";
		//nPrintPin= (int)res;
		//printBin(&nPrintPin, nGetPort);
	}
	if(mikroSleepTime >= m_maxMeasuredTime)
	{
		char cError[100];
		string msg("overflow of measured given time ");

		sprintf(cError, "%lu", m_maxMeasuredTime);
		msg+= cError;
		LOG(LOG_ERROR, msg);
		mikroSleepTime= m_maxMeasuredTime;
		msleeptime= m_maxMeasuredTime;
	}
	return mikroSleepTime;
	/*else
	{
		// calculating mikroSleepTime inside measuredness
		msleeptime= mikroSleepTime / (unsigned long)m_nMeasuredness * (unsigned long)m_nMeasuredness;

		if(msleeptime < mikroSleepTime)
			msleeptime= msleeptime + (unsigned long)m_nMeasuredness;
	}*/
#ifdef DEBUG
	if(isDebug())
	{
		m_count++;
		printf("%lu. meschuring %lu= %lu mikroseconds\n", m_count, mikroSleepTime, msleeptime);
	}
#endif // DEBUG

	vCorrection= getNearestMeasure(msleeptime);
	if(vCorrection.size() == 0)
		return msleeptime;
	if(vCorrection.size() == 1)
	{
		printf("correction:%.60lf\n", vCorrection[0].correction);
		return (unsigned long)((double)msleeptime * vCorrection[0].correction);
	}
	//correction= vCorrection[0].correction + (vCorrection[1].correction - vCorrection[0].correction) *
	//			(msleeptime - vCorrection[0].nMikrosec) / (vCorrection[1].nMikrosec - vCorrection[0].nMikrosec);
#ifdef DEBUG
	if(isDebug())
	{
		out() << "found nearest given mikrosecounds:" << endl;
		out() << vCorrection[0].correction << " correction is " << vCorrection[0].nMikrosec << " mikroseconds" << endl;
		out() << vCorrection[1].correction << " correction is " << vCorrection[1].nMikrosec << " mikroseconds" << endl;
		printf("is correction:%.60lf\n", vCorrection[0].correction);
	}
#endif // DEBUG

	return msleeptime;
	//return (unsigned long)((double)msleeptime * correction);
}

vector<correction_t> TimeMeasure::getNearestMeasure(unsigned long measuredTime)
{
	unsigned int n;
	unsigned int nCount= m_vCorrection.size();
	vector<correction_t> vRv;

	if(nCount <= 2)
		return m_vCorrection;
	for(n= 0; n<nCount; n++)
	{
		if(m_vCorrection[n].nMikrosec >= measuredTime)
		{
			if(n > 0)
				vRv.push_back(m_vCorrection[n-1]);
			vRv.push_back(m_vCorrection[n]);
			if(	n == 0
				&&
				nCount > 1	)
			{
				vRv.push_back(m_vCorrection[1]);
			}
			break;
		}
	}
	if(	vRv.size() == 0
		&&
		nCount > 0		)
	{
		if(nCount > 1)
			vRv.push_back(m_vCorrection[nCount-2]);
		vRv.push_back(m_vCorrection[nCount-1]);
	}
	return vRv;
}

unsigned long TimeMeasure::getNewMikroseconds(vector<ohm> *elkoCorrection)
{
	char res[150];
	unsigned long time;
	unsigned int nMuch= elkoCorrection->size();
	unsigned int nPos= 0;
	string logString, logString2;

	while(nPos < nMuch)
	{
		if(!(*elkoCorrection)[nPos].bSetTime)
			break;
		++nPos;
	}
	sprintf(res, "%.0f", (*elkoCorrection)[nPos].be);
	logString= "### check mikroseconds for an resistance with ";
	logString+= res;
	logString+= " OHM ";
#ifndef DEBUG
	out() << logString << endl;
	out() << "." << flush;
#endif

	time= getMeasuredTime();
	for(short i= 0; i<1; ++i)
	{
#ifndef DEBUG
		out() << "." << flush;
#endif // DEBUG
		time+= getMeasuredTime();
		time/= 2;
		sleep(3);
	}
	sprintf(res, "%lu", time);
	logString2= "    measured time: ";
	logString2+= res;
#ifndef DEBUG
	out() << endl;
	out() << "measured time:" << time << endl;
#endif // DEBUG
	logString+= "\n";
	logString+= logString2;
	LOGEX(LOG_INFO, logString, getRunningThread()->getExternSendDevice());
	return time;
}

vector<ohm> TimeMeasure::getNearestOhm(unsigned long measuredTime, vector<ohm> vOhm, bool bCheckOhm/*=false*/)
{
	unsigned int n;
	unsigned int nCount= vOhm.size();
	vector<ohm> vRv;

	for(n= 0; n<nCount; n++)
	{
		if(	(	bCheckOhm
				&&
				vOhm[n].be >= measuredTime	)
			||
			(	!bCheckOhm
				&&
				vOhm[n].nMikrosec >= measuredTime	)	)
		{
			if(n > 0)
				vRv.push_back(vOhm[n-1]);
			vRv.push_back(vOhm[n]);
			if(	n == 0
				&&
				nCount > 1	)
			{
				vRv.push_back(vOhm[1]);
			}
			break;
		}
	}
	if(	vRv.size() == 0
		&&
		nCount > 0		)
	{
		if(nCount >1)
			vRv.push_back(vOhm[nCount-2]);
		vRv.push_back(vOhm[nCount-1]);
	}
	return vRv;
}

correction_t TimeMeasure::getNewCorrection(correction_t tCorrection, vector<ohm> vOhm, unsigned short nSleep)
{
	char res[150];
	unsigned long time= 0, newtime;
	double correction, resistance;
	string logString, logString2;
	vector<ohm> vNearest;
	correction_t tRv;

	sprintf(res, "%.0lf", tCorrection.be);
	logString= "### check correction for an resistance with ";
	logString+= res;
	logString+= " OHM ";
#ifndef DEBUG
	out() << logString << endl;
#endif

	unsigned long time2;
	for(short i= 0; i<1; ++i)
	{
		time2= getMeasuredTime();
		if(time)
			time= (time + time2) / 2;
		else
			time= time2;
		out() << "." << flush;
		sleep(nSleep);
	}
	out() << endl << "measured time:" << time << endl;

	vNearest= getNearestOhm(time, vOhm);
#ifdef DEBUG
	out() << "found nearest given mikrosecounds:" << endl;
	out() << vNearest[0].be << " ohm is " << vNearest[0].nMikrosec << " mikroseconds" << endl;
	out() << vNearest[1].be << " ohm is " << vNearest[1].nMikrosec << " mikroseconds" << endl;
	out() << vNearest[0].be << " + (" << (vNearest[1].be - vNearest[0].be);
	out() << ") * (" << time << " - " << vNearest[0].nMikrosec << ") / (";
	out() << vNearest[1].nMikrosec << " - " << vNearest[0].nMikrosec << ")" << endl;
#endif // DEBUG

	resistance = vNearest[0].be + (vNearest[1].be - vNearest[0].be) *
				 (double)(time - vNearest[0].nMikrosec) /
				 (double)(vNearest[1].nMikrosec - vNearest[0].nMikrosec);
	out() << "calculated OHM are " << resistance << endl;
	out() << "but should be " << tCorrection.be << " Ohm" << endl;

	vNearest= getNearestOhm((unsigned long)tCorrection.be, vOhm, /*bCheckOhm*/true);
#ifdef DEBUG
	out() << "found nearest given OHM:" << endl;
	out() << vNearest[0].be << " ohm is " << vNearest[0].nMikrosec << " mikroseconds" << endl;
	out() << vNearest[1].be << " ohm is " << vNearest[1].nMikrosec << " mikroseconds" << endl;
#endif

	newtime= (unsigned long)(((tCorrection.be - vNearest[0].be) *
			 (double)(vNearest[1].nMikrosec - vNearest[0].nMikrosec)) /
			 (vNearest[1].be - vNearest[0].be) + (double)vNearest[0].nMikrosec);
	out() << "old time was " << time << " should be now " << newtime << endl;
	correction= tCorrection.be / resistance;
	logString2= "fill correction in configfile with ";
	sprintf(res, "%.60lf", correction);
	logString2+= res;
#ifndef DEBUG
	out() << logString2 << endl;
#else // DEBUG

	vNearest= getNearestOhm(newtime, vOhm);
	resistance = vNearest[0].be + (vNearest[1].be - vNearest[0].be) *
					 (double)(newtime - vNearest[0].nMikrosec) /
					 (double)(vNearest[1].nMikrosec - vNearest[0].nMikrosec);
	out() << "found nearest given mikrosecounds:" << endl;
	out() << vNearest[0].be << " ohm is " << vNearest[0].nMikrosec << " mikroseconds" << endl;
	out() << vNearest[1].be << " ohm is " << vNearest[1].nMikrosec << " mikroseconds" << endl;
	out() << vNearest[0].be << " + (" << (vNearest[1].be - vNearest[0].be);
	out() << ") * (" << time << " - " << vNearest[0].nMikrosec << ") / (";
	out() << vNearest[1].nMikrosec << " - " << vNearest[0].nMikrosec << ")" << endl;
	out() << "new time with correction is " << (unsigned long)newtime << endl;
	out() << "calculated OHM are " << resistance << endl;
#endif // DEBUG

	logString+= "\n";
	logString+= logString2;
	LOGEX(LOG_INFO, logString, getRunningThread()->getExternSendDevice());
	tRv.be= tCorrection.be;
	tRv.nMikrosec= time;
	tRv.correction= correction;
	return tRv;
}

short TimeMeasure::setNewMeasuredness(unsigned short measureCount, unsigned short sleeptime)
{
	unsigned short wait= 5;
	unsigned long time;
	list<timemap_t> vUnsorted;
	unsigned long maxtime= 0, mintime= ULONG_MAX, diff, olddiff= 0;
	vector<struct timemap_t> vSorted;
	char res[150];
	list<timemap_t>::iterator iterator;
	unsigned short n;
	string logString("### check for measuredness on port ");
	string endlog;

	//logString+= getPortName(m_tIn.nPort);
#ifndef DEBUG
	out() << logString << endl;
#endif // DEBUUG
	LOGEX(LOG_INFO, logString, getRunningThread()->getExternSendDevice());

	m_nMeasuredness= 1;
	maxtime= 0;
	for(n= 0; n<measureCount; ++n)
	{
		time= getMeasuredTime();
#ifndef DEBUG
		out() << "." << flush;
#else
		if(n< wait)
			out() << "." << flush;
#endif // DEBUG
		if(n > wait)
		{
			if(time>maxtime)
				maxtime= time;
			if(time<mintime)
				mintime= time;
			diff= maxtime-mintime;
			sprintf(res, "%i", n);
			logString= res;
			logString+= ". measure ";
			sprintf(res, "%lu", time);
			logString+= res;
			logString+= "us is measuredness of ";
			sprintf(res, "%lu", diff);
			logString+= res;

#ifndef DEBUG
			if(	diff != olddiff
				&&
				n > 5			)
			{
				out() << endl << logString << endl;
			}
#else // DEBUG
			out() << logString << endl << endl;
#endif // DEBUG
			endlog+= ".";
			if(	diff != olddiff
				&&
				n > 5			)
			{
				endlog+= "\n";
				endlog+= logString;
				endlog+= "\n";
			}

			if(n>5) // no measure for first result
			{
				timemap_t map;

				map.time= time;
				vUnsorted.push_back(map);
			}
			olddiff= diff;
		}
		sleep(sleeptime);
	}
	LOGEX(LOG_INFO, endlog, getRunningThread()->getExternSendDevice());
#ifdef DEBUG
	out() << endlog << endl << endl;
#endif // DEBUG
	out() << endl;
	maxtime= 0;
	mintime= ULONG_MAX;

	for(iterator= vUnsorted.begin(); iterator!=vUnsorted.end(); iterator++)
	{// calculating max and min
		if((*iterator).time>maxtime)
			maxtime= (*iterator).time;
		if( (*iterator).time<mintime)
		{
			mintime= (*iterator).time;
		}
	}

	logString= "measure in ";
	sprintf(res, "%d", measureCount);
	logString+= res;
	logString+= " times:\n";
	logString+= "maxtime was ";
	sprintf(res, "%lu", maxtime);
	logString+= res;
	logString+= " and mintime was ";
	sprintf(res, "%lu", mintime);
	logString+= res;
	logString+= " -> is an difference from ";
	sprintf(res, "%lu", (maxtime-mintime));
	logString+= res;
	logString+= ".\n";
	LOGEX(LOG_INFO, logString, getRunningThread()->getExternSendDevice());
#ifndef DEBUG
	out() << logString << endl;
#endif //DEBUG

	maxtime= maxtime-mintime;
	mintime= maxtime / 200 * 200;
	if(maxtime > mintime)
		mintime+= 200;
	if(mintime < 1)
		mintime= 1;
	m_nMeasuredness= (unsigned short)mintime;
	return (short)m_nMeasuredness;
}
