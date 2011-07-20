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
package at.kolli.automation.client;

/**
 * Holder of double value to call by reference
 * 
 * @author Alexander Kolli
 *
 */
public class DoubleHolder {
	
	private double value= 0.0;
	
	/**
	 * set value into object
	 * 
	 * @param value double value to set
	 */
	public void set(double value)
	{
		this.value= value;
	}
	
	/**
	 * get value content
	 * 
	 * @return value inside object
	 */
	public double get()
	{
		return value;
	}

}
