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

import java.util.LinkedList;

import com.sun.org.apache.regexp.internal.RE;

import at.kolli.layout.INodeCollection;

/**
 * @author kollia
 *
 */
public class NodeContainer implements INodeCollection {

	/**
	 * enum of type of value witch is inherit in container.<br />
	 * <table>
	 *   <tr>
	 *    <td>
	 *      NONE
	 *    </td>
	 *    <td width="10" align="center">
	 *      -
	 *    </td>
	 *    <td>
	 *      container has no value
	 *    </td>
	 *   </tr>
	 *   <tr>
	 *    <td>
	 *      DOUBLE
	 *    </td>
	 *    <td width="10" align="center">
	 *      -
	 *    </td>
	 *    <td>
	 *      container has double value
	 *    </td>
	 *   </tr>
	 *   <tr>
	 *    <td>
	 *      STRING
	 *    </td>
	 *    <td width="10" align="center">
	 *      -
	 *    </td>
	 *    <td>
	 *      container has an character string as value
	 *    </td>
	 *   </tr>
	 * </table>
	 * 
	 */
	public enum type {
		
		NONE,
		DOUBLE,
		STRING
	}
	/**
	 * whether object has an value
	 */
	private type m_eValue= type.NONE;
	/**
	 * double value witch be read
	 */
	private double m_nValue= 0;
	/**
	 * character string witch be read
	 */
	private String m_sValue= null;
	/**
	 * name of folder
	 */
	private String m_sFolder;
	/**
	 * name of subroutine
	 */
	private String m_sSubroutine;
	
	/**
	/* (non-Javadoc)
	 * @see at.kolli.layout.INodeCollection#getDValue()
	 */
	public double getDValue() {
		
		return m_nValue;
	}

	/* (non-Javadoc)
	 * @see at.kolli.layout.INodeCollection#getFolderName()
	 */
	public String getFolderName() {
		
		return m_sFolder;
	}

	/* (non-Javadoc)
	 * @see at.kolli.layout.INodeCollection#getSValue()
	 */
	public String getSValue() {
		
		String sRv= m_sValue;
		
		if(m_eValue.equals(type.DOUBLE))
			sRv= Double.toString(m_nValue);
		return m_sValue;
	}

	/* (non-Javadoc)
	 * @see at.kolli.layout.INodeCollection#getSubroutineName()
	 */
	public String getSubroutineName() {
		
		return m_sSubroutine;
	}

	/* (non-Javadoc)
	 * @see at.kolli.layout.INodeCollection#hasDoubleValue()
	 */
	public boolean hasDoubleValue() {
		
		if(m_eValue.equals(type.DOUBLE))
			return true;
		return false;
	}

	/* (non-Javadoc)
	 * @see at.kolli.layout.INodeCollection#hasStringValue()
	 */
	public boolean hasStringValue() {
		
		if(m_eValue.equals(type.STRING))
			return true;
		return false;
	}

	/* (non-Javadoc)
	 * @see at.kolli.layout.INodeCollection#hasValue()
	 */
	public boolean hasValue() {
		
		if(m_eValue.equals(type.NONE))
			return false;
		return true;
	}

	/* (non-Javadoc)
	 * @see at.kolli.layout.INodeCollection#read(java.lang.String, java.lang.String)
	 */
	public boolean read(String format, String text) {
		
		int c, count;
		StringBuffer fread= new StringBuffer();
		LinkedList<String> fresult= new LinkedList<String>();
		// match 6 formater
		RE restring= new RE("^([^%]*)(%[^%])([^%]*)(%[^%])?([^%]*)(%[^%])?([^%]*)(%[^%])?([^%]*)(%[^%])?([^%]*)$");
		RE newFormat;
		
		if(!restring.match(format))
			return false;
		count= restring.getParenCount();
		//fread.append("^");
		fread.append(restring.getParen(1));
		for(c= 2; c < count; ++c)
		{
			String f= restring.getParen(c);
			
			if(f.equals(""))
				break;
			if(f.substring(0, 1).equals("%"))
			{
				String v= f.substring(1);
				
				if(v.equals("c"))
					m_eValue= type.STRING;
				else if(v.equals("d"))
					m_eValue= type.DOUBLE;
				fresult.addLast(v);
				fread.append("(.+)");
			}else
			{
				int len= f.length();
				char[] af= new char[len];
				
				f.getChars(0, len, af, 0);
				for (char d : af) {
					if(d == ' ')
						fread.append("[ \t]+");
					else
						fread.append(d);
				}
			}
		}
		
		newFormat= new RE(fread.toString());
		if(	text == null
			||
			!newFormat.match(text))
		{
			m_eValue= type.NONE;
			return false;
		}
		c= 1;
		count= newFormat.getParenCount();
		for (String v : fresult) {
			
			if(v.equals("f"))
				m_sFolder= newFormat.getParen(c);
			else if(v.equals("s"))
				m_sSubroutine= newFormat.getParen(c);
			else if(v.equals("d"))
			{
				try{
					m_nValue= Double.parseDouble(newFormat.getParen(c));
				}catch(NullPointerException ex)
				{
					ex.printStackTrace();
					m_nValue= 0;
				}
			}else if(v.equals("c"))
				m_sValue= newFormat.getParen(c);
			++c;
			if(c >= count)
				break;
		}
		return true;
	}

}
