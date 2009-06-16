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
#ifndef TEMPPORT_H_
#define TEMPPORT_H_

//#include <sys/io.h>
#include "../util/structures.h"

#include "resistancemeasure.h"

using namespace ports;

class TempMeasure : public ResistanceMeasure
{
	private:
		unsigned int m_nBValue; // temperature-constant from producer in kelvin
								// in most time 2000 - 5000 kelvin
		float m_nTemperature;

	public:
		TempMeasure(string folderName, string subroutineName)
		: ResistanceMeasure(folderName, subroutineName) { };
		/*TempMeasure(Pins tOut, Pins tIn, Pins tNegative, unsigned short measuredness, vector<ohm> *elkoCorrection, vector<unsigned short> ohmVector) :
			ResistanceMeasure(tOut, tIn, tNegative, measuredness, elkoCorrection, ohmVector)
			{ };*/
		virtual bool measure();

	protected:
		float getTemperature();
};

#endif /*TEMPPORT_H_*/
