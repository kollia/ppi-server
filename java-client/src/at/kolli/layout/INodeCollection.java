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
package at.kolli.layout;

/**
 * @author Alexander Kolli
 *
 */
public interface INodeCollection {

	/**
	 * read an string from server in follow format.<br />
	 * <table>
	 *   <tr>
	 *    <td>
	 *      %f
	 *    </td>
	 *    <td width="10" align="center">
	 *      -
	 *    </td>
	 *    <td>
	 *      position of folder name
	 *    </td>
	 *   </tr>
	 *   <tr>
	 *    <td>
	 *      %s
	 *    </td>
	 *    <td align="center">
	 *      -
	 *    </td>
	 *    <td>
	 *      position of subroutine name
	 *    </td>
	 *   </tr>
	 *   <tr>
	 *    <td>
	 *      %d
	 *    </td>
	 *    <td align="center">
	 *      -
	 *    </td>
	 *    <td>
	 *      position of an double value
	 *    </td>
	 *   </tr>
	 *   <tr>
	 *    <td>
	 *      %c
	 *    </td>
	 *    <td align="center">
	 *      -
	 *    </td>
	 *    <td>
	 *      position of an character string
	 *    </td>
	 *   </tr>
	 * </table>
	 * 
	 * @param format format string to reading
	 * @param text string witch should readed 
	 * @return whether method can read text
	 */
	public boolean read(String format, String text);
	/**
	 * returning name of folder
	 * 
	 * @return name of folder
	 */
	public String getFolderName();
	/**
	 * returning name of subroutine
	 * 
	 * @return name of subroutine
	 */
	public String getSubroutineName();
	/**
	 * returning double value, if not exist <code>null</code>
	 * @return double value
	 */
	public double getDValue();
	/**
	 * returning string value, if not exist <code>null</code>
	 * 
	 * @return string value
	 */
	public String getSValue();
	/**
	 * returning whether any value exists in object
	 * 
	 * @return whether value exists
	 */
	public boolean hasValue();
	/**
	 * returning whether double value exists in object
	 * 
	 * @return whether value exists
	 */
	public boolean hasDoubleValue();
	/**
	 * returning whether string value exists in object
	 * 
	 * @return whether value exists
	 */
	public boolean hasStringValue();
}
