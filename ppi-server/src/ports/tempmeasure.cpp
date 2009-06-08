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
#include <iostream>
#include <math.h>

#include "tempmeasure.h"

bool TempMeasure::measure()
{
	float nTemperature= getTemperature();
	ResistanceMeasure::setValue((double)nTemperature);
	if(isDebug())
		cout << "Temperature:" << m_nTemperature << endl;
	return true;
}

float TempMeasure::getTemperature()
{
	int res;
	double resistor;
	double T, temp;

	resistor= getResistance();
	if(resistor < 1)
	{
		printf("unknown ERROR: reading resistor is %fl Ohm\n", resistor);
		printf("               maybe to low resistor for calculating natural logarithm\n");
		printf("               set to minimum 1 Ohm.\n\n");
		resistor= 1;
	}
	T= (double)resistor;
	//printf("set to %f\n", (T/10000));
	temp= log( (T/10000) );
	temp= 1 / (temp / 4300 + 1 / 298) - 273;
	res= (int)(temp*10);
	temp= (double)res;
	temp= ((double)res) / 1000;
	return (float)temp;
}