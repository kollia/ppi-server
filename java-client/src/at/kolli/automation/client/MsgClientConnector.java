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

import java.io.IOException;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;

import org.apache.regexp.RE;

import at.kolli.dialogs.DialogThread;
import at.kolli.layout.HtmTags;
import at.kolli.layout.permission;

/**
 * Overload class of ClientConnector which fill the error message in an Dialog
 * if the connection is broken
 * 
 * @package at.kolli.automation.client
 * @author Alexander Kolli
 * @version 1.00.00, 30.11.2007
 * @since JDK 1.6
 */
public class MsgClientConnector extends ClientConnector
{
	/**
	 * object of own class
	 */
	private static MsgClientConnector _instance= null;

	/**
	 * object from MsgTranslator to translate error-codes
	 */
	protected MsgTranslator m_oTrans;
	/**
	 * regular user which can login as first
	 */
	private String m_sRegUser;
	/**
	 * regular password for regular user
	 */
	private String m_sRegPassword;
	/**
	 * current user logged in
	 */
	private String m_sCurUser;
	/**
	 * password of current user
	 */
	private String m_sCurPassword;
	/**
	 * contains the error-code if the socket throw an exception,
	 * or the error-code getting from the server
	 */
	protected String m_sErrorCode;
	/**
	 * translated error-string from error-code
	 * @see MsgTranslator
	 */
	protected String m_sErrorMsg;
	
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
	public MsgClientConnector(String host, int port)
	{
		super(host, port);
		m_oTrans= MsgTranslator.instance();
		m_sErrorCode= "NONE";
		m_sErrorMsg= "NONE";
	}

	/**
	 * returning instance of own object
	 * 
	 * @return NoStopClientConnector instance
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public static MsgClientConnector instance()
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
	public static MsgClientConnector instance(String host, int port)
	{
		if(_instance == null)
			_instance= new MsgClientConnector(host, port);
		return _instance;
	}

	/**
	 * open an second connection to server for hearing on changes
	 * 
	 * @return true if the connection is correct, otherwise by an error false
	 */
	synchronized public boolean secondConnection()
	{
		boolean bRv= false;
		String res;
		
		if(haveSecondConnection())
			return true;
		
		try{
			res= secondConnection(m_sCurUser, m_sCurPassword);
			if(generateServerError(res) == null)
				bRv= true;
			
		}catch(IOException ex)
		{
			generateServerError(ex.getMessage());
			bRv= false;
		}
		return bRv;
		
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
	public synchronized void openNewConnection(String user, String password) throws IOException
	{
		boolean bReadPercent;
		boolean bExcept= false;
		int nMax= 0;
		int nSelected;
		String sRv= "NONE";
		String sOldStartingLevel= "", sOldErrorCode= "";
		Integer nOldStartPercent= 100;
		DialogThread dialog;

		bReadPercent= true;
		if(	m_sRegUser != null &&
			!m_sRegUser.equals("")	)
		{
			m_sCurUser= m_sRegUser;
			m_sCurPassword= m_sRegPassword;
		}else
		{
			m_sCurUser= user;
			m_sCurPassword= password;
		}
		dialog= DialogThread.instance();		
		nMax= dialog.getMaximum();
		dialog.setSelection(0);
		nSelected= 0;
		do{
			try{
				bExcept= false;
				sRv= "NONE";
				sRv= super.openConnection(m_sCurUser, m_sCurPassword);
				sRv= generateServerError(sRv);
				if(sOldErrorCode.substring(0, 14).equals("PORTSERVERBUSY"))
					dialog.setSelection(100);
				if(sRv == null)
				{
					m_sRegUser= m_sCurUser;
					m_sRegPassword= m_sCurPassword;
					break;
					
				}		
			}catch(IOException ex)
			{
				long aktSec;
				String errMsg;
				
				bExcept= true;
				errMsg= ex.getMessage();
				generateServerError(errMsg);
				if(!errMsg.substring(0, 14).equals("PORTSERVERBUSY"))
				{
					sOldErrorCode= m_sErrorCode;
					dialog.show(m_oTrans.translate("dialogConnectionTitle"), m_sErrorMsg);
					aktSec= System.currentTimeMillis();
					do{	
						if(nSelected < nMax)
							++nSelected;
						else
							nSelected= 0;
						dialog.setSelection(nSelected);
						if(HtmTags.debug)
							System.out.println("Wait for connection per " + nSelected + " %");
						try{
							Thread.sleep(100);
							
						}catch(InterruptedException interupt)
						{
							System.out.println("cannont sleep thread on file MsgClientConnector.java line 226");
							break;
						}
					}while((aktSec + 3000) > System.currentTimeMillis());
					
				}else
				{
					Integer nPercent;
					
					nPercent= getServerStartingPercent();
					if(	!sOldErrorCode.equals(m_sErrorCode) &&
						nOldStartPercent != 100				)
					{
						generateServerError(sOldStartingLevel);
						nPercent= 100;
					}
					sOldStartingLevel= errMsg;
					sOldErrorCode= m_sErrorCode;
					nOldStartPercent= nPercent;
					dialog.show(m_oTrans.translate("dialogConnectionTitle"), m_sErrorMsg);
					if(nPercent == -1)
					{
						if(bReadPercent)
						{
							nSelected= 0;
							bReadPercent= false;
						}
						if(nSelected < nMax)
							++nSelected;
						else
							nSelected= 0;
						dialog.setSelection(nSelected);
					}else
					{
						bReadPercent= true;
						if(nPercent < 0)
							nPercent= 0;
						else if(nPercent > 100)
							nPercent= 100;
						dialog.setSelection(nPercent);
					}
					try{
						Thread.sleep(100);
						
					}catch(InterruptedException interupt)
					{
						System.out.println("cannont sleep thread on file MsgClientConnector.java line 239");
					}
				}
				if(HtmTags.debug)
				{
					System.out.println(m_sErrorMsg);
					System.out.println("Server does not running,");
					System.out.println(" -> found no Server on given port");
					System.out.println();
				}
			}
		}while(	bExcept &&
				dialog.isOpen()	);
	}

	/**
	 * ask server which files be set in client sub-directory
	 * @param filter filter files with this extension
	 * @param bthrow whether method should throw an exception
	 * @return array of HashMap of files as key and date as value
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public HashMap<String, Date> getDirectory(String filter, boolean bthrow) throws IOException
	{
		HashMap<String, Date> array= null;

		m_sErrorCode= "NONE";
		m_sErrorMsg= "NONE";
		try{
			array= super.getDirectory(filter);
		}catch(IOException ex)
		{
			m_sErrorCode= ex.getMessage();
			m_sErrorMsg= m_oTrans.translate(m_sErrorCode);
			if(bthrow)
				throw ex;
		}
		if( array != null &&
			array.get("ERROR") != null	)
		{
			String num= Integer.toString(array.get("ERROR").getDate());
			String res= "";
			
			for(int o= 3-num.length(); o >= 0; --o)
				res+= "0";
			res+= num;
			generateServerError("ERROR " + res);
			array= null;
		}
		return array;
	}
	/**
	 * generate server error when given result was an error
	 * 
	 * @param res result from server
	 * @return error number when generated, otherwise null
	 */
	protected String generateServerError(final String res)
	{
		return generateServerError(res, "", "");
	}
	/**
	 * generate server error when given result was an error
	 * 
	 * @param res result from server
	 * @param folder name of folder when sending command need
	 * @param subroutine name of subroutine when sending command need
	 * @return error number when generated, otherwise null
	 */
	synchronized protected String generateServerError(final String res, final String folder, final String subroutine)
	{
		String number;
		RE error= new RE("^ERROR ([0-9]+)( [0-9]+)?$");

		m_sErrorCode= "NONE";
		m_sErrorMsg= "NONE";
		if(res.equals("OK"))
			return null;
		if(!error.match(res))
		{
			if(res.substring(0, 14).equals("PORTSERVERBUSY"))
			{
				Integer nPercent;
				String sProcess;
				String[] split;

				split= res.substring(15).split(" ");
				if(split.length < 2)
				{
					sProcess= "starting";
					nPercent= -1;
				}else
				{
					sProcess= split[0];
					try{
						nPercent= Integer.valueOf(split[1]);
					}catch(Exception ex)
					{
						nPercent= -1;
					}
				}
				m_sErrorCode= "PORTSERVERBUSY_" + sProcess;
				m_sErrorMsg= m_oTrans.translate(m_sErrorCode);
			}else
			{
				m_sErrorCode= res;
				if(	m_sErrorCode.equals("UNKNOWNHOSTEXCEPTION") ||
					m_sErrorCode.equals("UNDEFINEDSERVER")			)
				{
					m_sErrorMsg= m_oTrans.translate(m_sErrorCode, m_sHost, Long.toString(m_nPort));
				}else
					m_sErrorMsg= m_oTrans.translate(m_sErrorCode);
			}
			return m_sErrorCode;
		}
		number= error.getParen(1);
		m_sErrorCode= "PORTSERVERERROR" + number;
		if(	number.equals("001") ||
			number.equals("002")	)
		{
			m_sErrorMsg= m_oTrans.translate(m_sErrorCode);
			
		}else if(number.equals("003"))
			m_sErrorMsg= m_oTrans.translate(m_sErrorCode, error.getParen(2));
		else if(number.equals("004"))
			m_sErrorMsg= m_oTrans.translate(m_sErrorCode, folder);
		else if(number.equals("005"))
			m_sErrorMsg= m_oTrans.translate(m_sErrorCode, subroutine, folder);
		else if(number.equals("006"))
			m_sErrorMsg= m_oTrans.translate(m_sErrorCode, subroutine);
		else if(	number.equals("011") ||
					number.equals("012") ||
					number.equals("015")	)
		{
			m_sErrorMsg= m_oTrans.translate(m_sErrorCode, m_sCurUser);
			if(number != "012")
			{
				m_sCurUser= "";
				m_sRegUser= "";
			}			
		}else
			m_sErrorMsg= m_oTrans.translate(m_sErrorCode);
		return number;
	}
	/**
	 * set double value on given path.<br />
	 * if the connection breaks, function waiting until found an new connection
	 * 
	 * @param path  of subroutine<br />
	 * 				The path is made up of &lt;folder&gt;:&lt;subroutine&gt;:[subroutine:...]
	 * @param value	witch should be set<br />
	 * 				If the value should be an boolean for an SWITCH-subroutine, set 1 for true and 0 for false
	 * @param bthrow whether method should throw an exception
	 * @return true if the server found the path, otherwise an error occurs false
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public boolean setValue(String path, double value, boolean bthrow) throws IOException
	{
		Pathholder tStruct= splitPath(path);
		
		if(tStruct == null)
			return false;
		return setValue(tStruct.folder, tStruct.subroutine, value, bthrow);
	}
	
	/**
	 * set double value on given path.<br />
	 * if the connection breaks, function waiting until found an new connection
	 * 
	 * @param folder the folder which be set on the server
	 * @param subroutine one ore more subroutines, delimited with colons
	 * @param value	witch should be set<br />
	 * 				If the value should be an boolean for an SWITCH-subroutine, set 1 for true and 0 for false
	 * @param bthrow whether method should throw an exception
	 * @return true if the server found the path, otherwise an error occurs false
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public boolean setValue(String folder, String subroutine, double value, boolean bthrow) throws IOException
	{
		boolean bRv= false;
		String res;

		try{
			res= super.setValue(folder, subroutine, value);
			if(generateServerError(res, folder, subroutine) == null)
				bRv= true;
			
		}catch(IOException ex)
		{
			generateServerError(ex.getMessage(), folder, subroutine);
			if(bthrow)
				throw ex;
		}
		return bRv;
	}

	/**
	 * sending request for permission groups
	 * 
	 * @param groups groups of permission, separately with colons
	 * @param bthrow whether method should throw an exception
	 * @return writable, readable or None permission.<br />if an error occurred the permission is null and the error can reading in <code>getErrorCode()</code> 
	 */
	public permission permission(String groups, boolean bthrow) throws IOException
	{
		permission Rv= permission.None;
		String res;
		
		try{
			res= super.permission(groups);
			if(res.equals("write"))
			{
				Rv= permission.writeable;
				res= "OK";
				
			}else if(res.equals("read"))
			{
				Rv= permission.readable;
				res= "OK";
				
			}else if(res.equals("none"))
			{
				Rv= permission.None;
				res= "OK";
			}
			if(generateServerError(res) != null)
				return null;
			
		}catch(IOException ex)
		{
			generateServerError(ex.getMessage());
			if(bthrow)
				throw ex;
			return null;
		}
		return Rv;
		
	}

	/**
	 * change user account for new permission
	 * 
	 * @param user user name for new account
	 * @param password password for user
	 * @param bthrow whether method should throw an exception
	 * @return whether changing was successfully
	 */
	public boolean changeUser(String user, String password, boolean bthrow) throws IOException
	{
		boolean bRv= false;
		String res;
		
		try{
			res= super.changeUser(user, password);
			if(generateServerError(res) == null)
			{
				bRv= true;
				m_sCurUser= user;
			}
			
		}catch(IOException ex)
		{
			generateServerError(ex.getMessage());
			if(bthrow)
				throw ex;
		}
		return bRv;
	}
	
	/**
	 * get value from given path, which be set currently on server.<br />
	 * if the connection breaks, function waiting until found an new connection
	 * 
	 * @param path folder and subroutines delimited with colons
	 * @param bthrow whether method should throw an exception
	 * @return the double value, if an error occurs the return-value is null
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public Double getValue(String path, boolean bthrow) throws IOException
	{
		Pathholder tStruct= splitPath(path); 
		
		if(tStruct == null)
			return null;
		return getValue(tStruct.folder, tStruct.subroutine, bthrow);
	}
	
	/**
	 * get value from given folder and subroutines, which be set currently on server.<br />
	 * if the connection breaks, function waiting until found an new connection
	 * 
	 * @param folder the folder which be set on the server
	 * @param subroutine one ore more subroutines, delimited with colons
	 * @param bthrow whether method should throw an exception
	 * @return the double value, if an error occurs the return-value is null
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public Double getValue(String folder, String subroutine, boolean bthrow) throws IOException
	{
		String res;
		Double value= null;
		DoubleHolder val= new DoubleHolder();
		
		try{
			res= getValue(folder, subroutine, val);
			if(generateServerError(res, folder, subroutine) == null)
				value= val.get();
			
		}catch(IOException ex)
		{
			generateServerError(ex.getMessage(), folder, subroutine);
			if(bthrow)
				throw ex;
			value= null;
		}
		return value;
	}
	/**
	 * get side content from server for given file name
	 * 
	 * @param file name of file from server side
	 * @param bthrow whether method should throw an exception
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public ArrayList<String> getContent(String file, boolean bthrow) throws IOException
	{
		RE error= new RE("<[ \t]*error[ \t]*.*[ \t]*number[ \t]*=[ \t]*('|\")([ 0-9]+)('|\")[ \t]*/[ \t]*>");
		ArrayList<String> xmlFile= null;
		String res;
		
		try{
			xmlFile= getContent(file);
			if(error.match(xmlFile.get(xmlFile.size()-1)))
			{
				// toDo: test error handling from server
				res= "ERROR " + error.getParen(2);
				generateServerError(res);
			}
			
		}catch(IOException ex)
		{
			generateServerError(ex.getMessage());
			if(bthrow)
				throw ex;
		}
		return xmlFile;
	}
	/**
	 * clear hearing on server for the request was sending before
	 * 
	 * @param bthrow whether method should throw an exception
	 * @return whether the clearing was done
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public boolean clearHearing(boolean bthrow) throws IOException
	{
		String res;
		
		try{
			res= clearHearing();
			if(res.equals("false"))
				return false;
			if(generateServerError(res) != null)
				return false;
			
		}catch(IOException ex)
		{
			generateServerError(ex.getMessage());
			if(bthrow)
				throw ex;
			return false;
		}
		return true;
	}

	/**
	 * sending request for subroutines in folder to hearing for changes.<br />
	 * this command should only send if open an second connection
	 * to server where you can send an hearing request with the command <code>request()</code>
	 * 
	 * @param folder name of folder
	 * @param subroutine name of subroutine
	 * @return OK when hear command was requested correctly, or an error code from server
	 */
	public boolean hear(String folder, String subroutine, boolean bthrow) throws IOException
	{
		String res;

		try{
			res= hear(folder, subroutine);
			if(generateServerError(res) != null)
				return false;
			
		}catch(IOException ex)
		{
			generateServerError(ex.getMessage());
			if(bthrow)
				throw ex;
			return false;
		}
		return true;
	}
	/**
	 * hear on server for changing subroutines
	 * 
	 * @return folder and subroutine with changed value, or the error from server
	 * @throws IOException
	 */
	public String hearing(boolean bthrow) throws IOException
	{
		RE error= new RE("^ERROR ([0-9]+)( [0-9]+)?$");
		String res;

		try{
			res= hearing();
			if(error.match(res))
			{
				generateServerError(res);
				return null;
			}
			
		}catch(IOException ex)
		{
			generateServerError(ex.getMessage());
			if(bthrow)
				throw ex;
			return null;
		}
		return res;
	}

	/**
	 * sending request for subroutines in folder to hearing for changes.<br />
	 * this command should only send if open an second connection
	 * to server where you can send an hearing request with the command <code>request()</code>
	 * 
	 * @param result name of folder and subroutine, seperated with an colon
	 * @return OK when hear command was requested correctly, or an error code from server
	 */
	public boolean hear(String result, boolean bthrow) throws IOException
	{
		Pathholder tStruct= splitPath(result);
		
		return hear(tStruct.folder, tStruct.subroutine, bthrow);
	}
	
	/**
	 * Error-code if the connection is refused, an given path cannot find from server,
	 * or permission denied
	 * 
	 * @return error-code
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public String getErrorCode()
	{
		return m_sErrorCode;
	}
	
	/**
	 * translated error-code
	 * 
	 * @return error-message
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public String getErrorMessage()
	{
		if(m_sErrorMsg.equals("NONE"))
			return m_oTrans.translate("NONE");
		return m_sErrorMsg;
	}
	
	/**
	 * wheter an error has occured
	 * 
	 * @return true if an error is orginated
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public boolean hasError()
	{
		if(m_sErrorMsg.equals("NONE"))
			return false;
		return true;
	}
	
}
