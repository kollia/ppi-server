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

import java.io.IOException;
import java.util.Map;

import org.eclipse.swt.custom.StackLayout;
import org.eclipse.swt.widgets.Composite;

import at.kolli.automation.client.NodeContainer;

public interface IComponentListener 
{
	/**
	 * add listeners if the component have an correct result attribute.<br />
	 * This method is to listen on activity if the component is in an {@link Composite}
	 * witch is be on the top of the {@link StackLayout}
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	public void addListeners() throws IOException;

	/**
	 * method listen on server whether value of component is changed
	 * 
	 * @param results map of result attributes with actual values
	 * @param cont container with new value on which result of folder and subroutine
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	public void serverListener(final Map<String, Double> results, NodeContainer cont) throws IOException;

	/**
	 * remove listeners if the component have an correct result attribute.<br />
	 * This method is to remove all listeners which set bevore with addListeners()
	 * if the component is in an {@link Composite} witch is not on the top of the {@link StackLayout}
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	public void removeListeners();
}
