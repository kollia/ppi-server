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

import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;

import com.sun.org.apache.regexp.internal.RE;

import at.kolli.dialogs.DialogThread;
import at.kolli.layout.permission;

/**
 * Overload class of ClientConnector whitch showes an Dialog
 * if the connection is broken
 * 
 * @package at.kolli.automation.client
 * @author Alexander Kolli
 * @version 1.00.00, 30.11.2007
 * @since JDK 1.6
 */
public class NoStopClientConnector extends ClientConnector
{
	/**
	 * object of own class
	 */
	private static NoStopClientConnector _instance= null;
	
	/**
	 * Constructor for ClientConnector
	 * 
	 * @param host where the server is running
	 * @param port where the server is running
	 * @see ClientConnector
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public NoStopClientConnector(String host, int port)
	{
		super(host, port);
	}

	/**
	 * returning instance of own object
	 * 
	 * @return NoStopClientConnector instance
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public static NoStopClientConnector instance()
	{
		return _instance;
	}
	
	/**
	 * creating and returning instance of own object
	 * 
	 * @param host where the server is running
	 * @param port where the server is running
	 * @return NoStopClientConnector instance
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public static NoStopClientConnector instance(String host, int port)
	{
		if(_instance == null)
			_instance= new NoStopClientConnector(host, port);
		return _instance;
	}
	
	/**
	 * beginning connection to server and if no server was found,
	 * function waiting until the server running
	 * 
	 * @param user binding to server with this user
	 * @param password binding to server with this password
	 * @return true if the connection is correct, otherwise by an error false
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public boolean openConnection(String user, String password)
	{
		return openConnection(user, password, true);
	}
	
	/**
	 * beginning connection to server and if no server was found,
	 * function waiting until the server running
	 * 
	 * @param user binding to server with this user
	 * @param password binding to server with this password
	 * @param bWidget an boolean whether the function should wait until an connection
	 * @return true if the connection is correct, otherwise by an error false
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public boolean openConnection(String user, String password, boolean bWidget/*= true*/)
	{
		boolean bRv= super.openConnection(user, password);

		if(!bRv)
		{
			int nMax;
			int nSelected;
			int nCount= 0;
			boolean bOpened;
			RE error= new RE("^ERROR ([0-9]+)( [0-9]+)?$");
			DialogThread dialog= DialogThread.instance();
			MsgTranslator file= MsgTranslator.instance();
			
			if(	error.match(getErrorCode())
				||
				bWidget == false			)
			{
				return false;
			}
			bOpened= dialog.show(file.translate("dialogConnectionTitle"), getErrorMessage());
			while(!bRv)
			{
				nMax= dialog.getMaximum();
				nSelected= dialog.getSelection() + 1;
				if(nSelected > nMax)
					nSelected= 0;
				dialog.setSelection(nSelected);
				try{
					Thread.sleep(10);
					if(nCount == 0)
					{
						nCount= 300;
						System.out.println("Server does not running,");
						System.out.println(" -> found no Server on given port");
						System.out.println();
					}else
						--nCount;
				}catch(InterruptedException ex)
				{
					
				}
				bRv= super.openConnection(user, password);
			}
			if(bOpened)
				dialog.close();
			m_sErrorMsg= "NONE";
			m_sErrorCode= "NONE";
		}
		return bRv;
	}

	/**
	 * ask server which files be set in client sub-directory
	 * @param filter filter files with this extension	 * 
	 * @return array of HashMap of files as key and date as value
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public HashMap<String, Date> getDirectory(String filter)
	{
		HashMap<String, Date> array= super.getDirectory(filter);
		
		if(	!haveSecondConnection()
			&&
			hasError()				)
		{
			RE error= new RE("^ERROR ([0-9]+)( [0-9]+)?$");
			
			if(!error.match(getErrorCode()))
			{
				boolean run= openConnection(TreeNodes.m_sUser, TreeNodes.m_sPwd);
				
				if(run)
				{
					array= getDirectory(filter);
					return array;
				}
			}
		}
		return array;
	}
	/**
	 * set double value on given path.<br />
	 * if the connection breaks, function waiting until found an new connection
	 * 
	 * @param path  of subroutine<br />
	 * 				The path is made up of &lt;folder&gt;:&lt;subroutine&gt;:[subroutine:...]
	 * @param value	witch should be set<br />
	 * 				If the value should be an boolean for an SWITCH-subroutine, set 1 for true and 0 for false
	 * @return true if the server found the path, otherwise an error occurs false
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public boolean setValue(String path, double value)
	{
		Pathholder tStruct= splitPath(path);
		
		if(tStruct == null)
			return false;
		return setValue(tStruct.folder, tStruct.subroutine, value);
	}
	
	/**
	 * set double value on given path.<br />
	 * if the connection breaks, function waiting until found an new connection
	 * 
	 * @param folder the folder which be set on the server
	 * @param subroutine one ore more subroutines, delimited with colons
	 * @param value	witch should be set<br />
	 * 				If the value should be an boolean for an SWITCH-subroutine, set 1 for true and 0 for false
	 * @return true if the server found the path, otherwise an error occurs false
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public boolean setValue(String folder, String subroutine, double value)
	{
		boolean bRv= super.setValue(folder, subroutine, value);
		
		if(	!haveSecondConnection()
			&&
			!bRv					)
		{
			int nMax;
			int nSelected;
			boolean bOpened;
			RE error= new RE("^ERROR ([0-9]+)( [0-9]+)?$");
			DialogThread dialog= DialogThread.instance();
			MsgTranslator file= MsgTranslator.instance();
			
			if(error.match(getErrorCode()))
				return false;
			bOpened= dialog.show(file.translate("dialogConnectionTitle"), getErrorMessage());
			while(!bRv)
			{
				nMax= dialog.getMaximum();
				nSelected= dialog.getSelection() + 1;
				if(nSelected > nMax)
					nSelected= 0;
				dialog.setSelection(nSelected);
				try{
					Thread.sleep(500);
				}catch(InterruptedException ex)
				{
					
				}
				if(openConnection(TreeNodes.m_sUser, TreeNodes.m_sPwd, false))
					bRv= super.setValue(folder, subroutine, value);
			}
			if(bOpened)
				dialog.close();
		}
		return bRv;
	}

	/**
	 * sending request for permission groups
	 * 
	 * @param groups groups of permission, separately with colons
	 * @return writable, readable or None permission.<br />if an error occurred the permission is None and the error can reading in <code>getErrorCode()</code> 
	 */
	public permission permission(String groups)
	{
		permission Rv= super.permission(groups);

		if(	!haveSecondConnection()
			&&
			hasError()				)
		{
			int nMax;
			int nSelected;
			boolean bOpened;
			RE error= new RE("^ERROR ([0-9]+)( [0-9]+)?$");
			DialogThread dialog= DialogThread.instance();
			MsgTranslator file= MsgTranslator.instance();
			
			if(error.match(getErrorCode()))
				return permission.None;
			bOpened= dialog.show(file.translate("dialogConnectionTitle"), getErrorMessage());
			while(hasError())
			{
				nMax= dialog.getMaximum();
				nSelected= dialog.getSelection() + 1;
				if(nSelected > nMax)
					nSelected= 0;
				dialog.setSelection(nSelected);
				try{
					Thread.sleep(500);
				}catch(InterruptedException ex)
				{
					
				}
				if(openConnection(TreeNodes.m_sUser, TreeNodes.m_sPwd, false))
					Rv= super.permission(groups);
			}
			if(bOpened)
				dialog.close();
		}
		return Rv;
		
	}

	/**
	 * change user account for new permission
	 * 
	 * @param user user name for new account
	 * @param password password for user
	 * @return whether changing was successfully
	 */
	public boolean changeUser(String user, String password)
	{
		boolean bRv= super.changeUser(user, password);

		if(	!haveSecondConnection()
			&&
			!bRv					)
		{
			int nMax;
			int nSelected;
			boolean bOpened;
			RE error= new RE("^ERROR ([0-9]+)( [0-9]+)?$");
			DialogThread dialog= DialogThread.instance();
			MsgTranslator file= MsgTranslator.instance();
			
			if(error.match(getErrorCode()))
				return false;
			bOpened= dialog.show(file.translate("dialogConnectionTitle"), getErrorMessage());
			while(!bRv)
			{
				nMax= dialog.getMaximum();
				nSelected= dialog.getSelection() + 1;
				if(nSelected > nMax)
					nSelected= 0;
				dialog.setSelection(nSelected);
				try{
					Thread.sleep(500);
				}catch(InterruptedException ex)
				{
					
				}
				if(openConnection(TreeNodes.m_sUser, TreeNodes.m_sPwd, false))
					bRv= super.changeUser(user, password);
			}
			if(bOpened)
				dialog.close();
			bRv= true;
		}
		return bRv;
	}
	
	/**
	 * get value from given path, which be set currently on server.<br />
	 * if the connection breaks, function waiting until found an new connection
	 * 
	 * @param path folder and subroutines delimited with colons
	 * @return the double value, if an error occurs the return-value is null
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public Double getValue(String path)
	{
		Pathholder tStruct= splitPath(path); 
		
		if(tStruct == null)
			return null;
		return getValue(tStruct.folder, tStruct.subroutine);
	}
	
	/**
	 * get value from given folder and subroutines, which be set currently on server.<br />
	 * if the connection breaks, function waiting until found an new connection
	 * 
	 * @param folder the folder which be set on the server
	 * @param subroutine one ore more subroutines, delimited with colons
	 * @return the double value, if an error occurs the return-value is null
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public Double getValue(String folder, String subroutine)
	{
		Double nRv= super.getValue(folder, subroutine);

		if(	!haveSecondConnection()
			&&
			nRv == null				)
		{
			boolean bOpened;
			RE error= new RE("^ERROR ([0-9]+)( [0-9]+)?$");
			final DialogThread dialog= DialogThread.instance();
			MsgTranslator file= MsgTranslator.instance();
			int nMax;
			int nSelected;
			
			if(error.match(getErrorCode()))
				return null;
			bOpened= dialog.show(file.translate("dialogConnectionTitle"), getErrorMessage());
			while(nRv == null)
			{
				nMax= dialog.getMaximum();
				nSelected= dialog.getSelection() + 1;
				if(nSelected > nMax)
					nSelected= 0;
				dialog.setSelection(nSelected);
				try{
					Thread.sleep(10);
				}catch(InterruptedException ex)
				{
					
				}
				if(openConnection(TreeNodes.m_sUser, TreeNodes.m_sPwd, true))
				{
					nRv= super.getValue(folder, subroutine);
					if(nRv == null)
						dialog.show(file.translate("dialogConnectionTitle"), getErrorMessage());
				}//else
				//	dialog.show(file.translate("dialogConnectionTitle"), getErrorMessage());
			}
			if(bOpened)
				dialog.close();
		}
		return nRv;
	}
	
	public ArrayList<String> getContent(String file)
	{
		ArrayList<String> xmlFile= super.getContent(file);

		if(	!haveSecondConnection()
				&&
				xmlFile == null		)
		{
			boolean bOpened;
			RE error= new RE("^ERROR ([0-9]+)( [0-9]+)?$");
			final DialogThread dialog= DialogThread.instance();
			MsgTranslator trans= MsgTranslator.instance();
			int nMax;
			int nSelected;
			
			if(error.match(getErrorCode()))
				return null;
			bOpened= dialog.show(trans.translate("dialogConnectionTitle"), getErrorMessage());
			while(xmlFile == null)
			{
				nMax= dialog.getMaximum();
				nSelected= dialog.getSelection() + 1;
				if(nSelected > nMax)
					nSelected= 0;
				dialog.setSelection(nSelected);
				try{
					Thread.sleep(10);
				}catch(InterruptedException ex)
				{
					
				}
				if(openConnection(TreeNodes.m_sUser, TreeNodes.m_sPwd, true))
				{
					xmlFile= super.getContent(file);
					if(xmlFile == null)
						dialog.show(trans.translate("dialogConnectionTitle"), getErrorMessage());
				}
			}
			if(bOpened)
				dialog.close();
		}
		return xmlFile;
	}
}
