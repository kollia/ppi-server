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
#ifndef LIGHTMEASURE_H_
#define LIGHTMEASURE_H_

#include <vector>
#include <sys/time.h>
#include <errno.h>

#include "../util/structures.h"

#include "../util/properties/configpropertycasher.h"

#include "portbaseclass.h"

class TimeMeasure : public portBase
{
	protected:
		vector<ohm> getNearestOhm(unsigned long measuredTime, vector<ohm> vOhm, bool bCheckOhm= false);

		Pins m_tNegative;
		Pins m_tOut;
		Pins m_tIn;
		unsigned long m_maxMeasuredTime;
		double m_dCorrection;

		/**
		 * set min and max parameter to the range which can be set for this subroutine.<br />
		 * If the subroutine is set from 0 to 1 and float false, the set method sending only 0 and 1 to the database.
		 * Also if the values defined in an bit value 010 (dec:2) set 0 and for 011 (dec:3) set 1 in db.
		 * Otherwise the range is only for calibrate the max and min value if set from client outher range.
		 * If pointer of min and max parameter are NULL, the full range of the double value can be used
		 *
		 * @param bfloat whether the values can be float variables
		 * @param min the minimal value
		 * @param max the maximal value
		 * @return whether the range is defined or can set all
		 */
		virtual bool range(bool& bfloat, double* min, double* max);

	private:
		vector<correction_t> getNearestMeasure(unsigned long measuredTime);

		unsigned short m_nMeasuredness;
		vector<correction_t> m_vCorrection;

	public:
		/**
		 * set microtime to 0 in an length from defined ITIMERSTARTSEC
		 */
		static void setMikrotime(void)
		{
			int res;
			struct itimerval time;

			time.it_interval.tv_sec= ITIMERSTARTSEC;
			time.it_interval.tv_usec= 0;
			time.it_value.tv_sec= ITIMERSTARTSEC;
			time.it_value.tv_usec= 0;
			res= setitimer(ITIMERTYPE, &time, NULL);
			if(res==-1)
			{
				printf("ERROR %d ", errno);
				perror("setitimer");
				printf("\n");
				return;
			}
		};
		/**
		 * reading microtime from beginning count of setMicrotime
		 */
		static inline unsigned long getMikrotime(void)
		{
			int res;
			long sec, lRv;
			struct itimerval timerv;
			struct itimerval *ptimer;

			//usleep(1);
			ptimer= &timerv;
			res= getitimer(ITIMERTYPE, ptimer);
			if(res==-1)
			{
				printf("result:%d\n", res);
				printf("ERROR: %d\n", errno);
				perror("getitimer");
				if(res==EFAULT)
					printf("      value oder ovalue sind keine gültigen Pointer.\n");
				else if(res==EINVAL)
					printf("      which ist weder ITIMER_REAL noch ITIMER_VIRTUAL noch ITIMER_PROF.\n");
				return -1;

			}

			sec= ITIMERSTARTSEC - timerv.it_value.tv_sec;
			lRv= 1000000 - timerv.it_value.tv_usec;
			if(sec > 1)
			{
				lRv+= 1000000 * sec;
			}
			return lRv;
		};
		/**
		 * create object of class TimeMeasure.<br />
		 * Constructor for an extendet object
		 *
		 * @param folder in which folder the routine running
		 * @param subroutine name of the routine
		 */
		TimeMeasure(string folderName, string subroutineName)
		: portBase("TIMEMEASURE", folderName, subroutineName) { };
		/**
		 * create object of class TimeMeasure.<br />
		 * Constructor for an extendet object
		 *
		 * @param type type of object from extendet class
		 * @param folder in which folder the routine running
		 * @param subroutine name of the routine
		 */
		TimeMeasure(string type, string folderName, string subroutineName)
		: portBase(type, folderName, subroutineName) { };
		bool init(ConfigPropertyCasher &properties);
		void init(	Pins tOut, Pins tIn, Pins tNegative,
					unsigned short measuredness, vector<correction_t> *elkoCorrection);
		unsigned long getMeasuredTime();
		unsigned long getNewMikroseconds(vector<ohm> *elkoCorrection);
		correction_t getNewCorrection(correction_t tCorrection, vector<ohm> vOhm, unsigned short nSleep);
		short setNewMeasuredness(unsigned short measureCount, unsigned short sleeptime);
		void setGradients(vector<correction_t> correction);
		/**
		 * measure new value for subroutine
		 *
		 * @param actValue current value
		 * @return return measured value
		 */
		virtual double measure(const double actValue);
		virtual ~TimeMeasure();
};

#endif /*LIGHTMEASURE_H_*/
