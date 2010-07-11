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
/**
 * written by Alexander Kolli
 * 
 */
package at.kolli.automation.client;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;

import at.kolli.layout.permission;

import org.apache.regexp.RE;

/**
 * connection interface to the ppi-server
 * 
 * @package at.kolli.automation.client
 * @author Alexander Kolli
 * @version 1.00.00, 30.11.2007
 * @since JDK 1.6
 */
public class ClientConnector 
{
	/**
	 * host where the ppi-server running
	 */
	private String m_sHost;
	/**
	 * port where the ppi-server running
	 */
	private int m_nPort;
	/**
	 * created socket to the ppi-server
	 */
	private Socket m_oSock;
	/**
	 * created second socket to the ppi-server
	 */
	private Socket m_oSecSock= null;
	/**
	 * connection ID given from Server
	 */
	private long m_nConnectionID;
	/**
	 * whether have an second connection to server
	 */
	private boolean m_bSecond= false;
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
	 * instance of own object
	 */
	private static ClientConnector _instance;
	/**
	 * writer on socket
	 */
	protected PrintWriter m_oPut;
	/**
	 * reader from socket
	 */
	protected BufferedReader m_oGet;
	/**
	 * second reader from socket
	 */
	protected BufferedReader m_oSecGet= null;
	/**
	 * object from MsgTranslator to translate error-codes
	 */
	protected MsgTranslator m_oTrans;

	/**
	 * constructor
	 * 
	 * @param host where the ppi-server running
	 * @param port where the ppi-serer running
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	protected ClientConnector(String host, int port)
	{
		m_sHost= host;
		m_nPort= port;
		m_sErrorMsg= "NONE";
		m_sErrorCode= "NONE";
		m_oTrans= MsgTranslator.instance();
	}
	
	/**
	 * returning instance of own object
	 * 
	 * @return ClientConnector instance
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public static ClientConnector instance()
	{
		return _instance;
	}
	
	/**
	 * creating and returning instance of own object
	 * 
	 * @param host where the ppi-server running
	 * @param port where the ppi-server running
	 * @return ClientConnector instance
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public static ClientConnector instance(String host, int port)
	{
		if(_instance == null)
			_instance= new ClientConnector(host, port);
		return _instance;
	}
	
	/**
	 * beginning connection to server
	 * 
	 * @param user binding to server with this user
	 * @param password binding to server with this password
	 * @return boolean true/false whether connection is created
	 */
	public synchronized boolean openConnection(String user, String password)
	{
		String res;

		m_sErrorMsg= "NONE";
		m_sErrorCode= "NONE";
		try{
			m_oSock= new Socket(m_sHost, m_nPort);
		}catch(UnknownHostException ex)
		{
			String sPort= "" +m_nPort;
			
			m_sErrorMsg= m_oTrans.translate("UNKNOWNHOSTEXCEPTION", m_sHost, sPort);
			m_sErrorCode= "UNKNOWNHOSTEXCEPTION";
			return false;
		}catch(IOException ex)
		{
			m_sErrorMsg= m_oTrans.translate("SOCKETIOEXCEPTION");
			m_sErrorCode= "SEOCKETIOEXCEPTION";
			return false;
		}
		
		try{
			m_oPut= new PrintWriter(m_oSock.getOutputStream());
		}catch(IOException ex)
		{
			m_sErrorMsg= m_oTrans.translate("WRITERIOEXCEPTION");
			m_sErrorCode= "WRITERIOEXCEPTION";
			try{
				m_oSock.close();
			}catch(IOException iex)
			{
				// ???
			}
			return false;
		}
		
		try{
			m_oGet= new BufferedReader(new InputStreamReader(m_oSock.getInputStream()));
		}catch(IOException ex)
		{
			m_sErrorMsg= m_oTrans.translate("READERIOEXCEPTION");
			m_sErrorCode= "READERIOEXCEPTION";
			try{
				m_oSock.close();
				m_oPut.close();
			}catch(IOException iex)
			{
				// ???
			}
			return false;
		}
		
		String put= "GET wait";
		
		m_oPut.println(put);
		m_oPut.flush();
		try{
			String id;
			RE answer= new RE("^port-server:([0-9]+)$");
			boolean match;
			
			res= m_oGet.readLine();
			if(res == null)
			{
				m_sErrorMsg= m_oTrans.translate("READERIOEXCEPTION");
				m_sErrorCode= "READERIOEXCEPTION";
				return false;
			}
			match= answer.match(res);			 
			if(!match)
			{
				String sPort= ""+m_nPort;
				
				m_sErrorMsg= m_oTrans.translate("UNDEFINEDSERVER", m_sHost, sPort);
				m_sErrorCode= "UNDEFINEDSERVER";
				try{
					m_oSock.close();
				}catch(IOException iex)
				{
					// ???
				}
				m_oPut.close();
				return false;
			}
			id= answer.getParen(1);
			m_nConnectionID= Long.decode(id);
			m_oPut.println("U:" + user + ":" + password);
			m_oPut.flush();
			res= m_oGet.readLine();
			if(res == null)
			{
				m_sErrorMsg= m_oTrans.translate("READERIOEXCEPTION");
				m_sErrorCode= "READERIOEXCEPTION";
				return false;
			}
			if(!res.equals("OK"))
			{
				m_sErrorCode= res; 
				m_sErrorMsg= m_oTrans.translatePortServerError(res, TreeNodes.m_sUser, null);
				return false;
			}
		}catch(IOException ex)
		{
			m_sErrorMsg= m_oTrans.translate("READERIOEXCEPTION");
			m_sErrorCode= "READERIOEXCEPTION";
			try{
				m_oSock.close();
			}catch(IOException iex)
			{
				// ???
			}
			m_oPut.close();
			return false;
		}
		return true;
	}
	
	/**
	 * open an second connection to server for hearing on changes
	 * 
	 * @return boolean true/false whether connection is created
	 */
	public synchronized boolean secondConnection()
	{
		String res;
		PrintWriter oPut;

		m_sErrorMsg= "NONE";
		m_sErrorCode= "NONE";
		try{
			m_oSecSock= new Socket(m_sHost, m_nPort);
		}catch(UnknownHostException ex)
		{
			String sPort= "" +m_nPort;
			
			m_sErrorMsg= m_oTrans.translate("UNKNOWNHOSTEXCEPTION", m_sHost, sPort);
			m_sErrorCode= "UNKNOWNHOSTEXCEPTION";
			m_oSecSock= null;
			return false;
		}catch(IOException ex)
		{
			m_sErrorMsg= m_oTrans.translate("SOCKETIOEXCEPTION");
			m_sErrorCode= "SEOCKETIOEXCEPTION";
			m_oSecSock= null;
			return false;
		}
		
		try{
			oPut= new PrintWriter(m_oSecSock.getOutputStream());
		}catch(IOException ex)
		{
			m_sErrorMsg= m_oTrans.translate("WRITERIOEXCEPTION");
			m_sErrorCode= "WRITERIOEXCEPTION";
			try{
				m_oSecSock.close();
			}catch(IOException iex)
			{
				// ???
			}
			m_oSecSock= null;
			return false;
		}
		
		try{
			m_oSecGet= new BufferedReader(new InputStreamReader(m_oSecSock.getInputStream()));
		}catch(IOException ex)
		{
			m_sErrorMsg= m_oTrans.translate("READERIOEXCEPTION");
			m_sErrorCode= "READERIOEXCEPTION";
			try{
				m_oSecSock.close();
				oPut.close();
			}catch(IOException iex)
			{
				// ???
			}
			m_oSecSock= null;
			m_oSecGet= null;
			return false;
		}
		
		String put= "GET wait ID:" + Long.toString(m_nConnectionID);
		
		oPut.println(put);
		oPut.flush();
		try{
			String id;
			String buf;
			RE answer= new RE("^port-server:([0-9]+)$");
			boolean match= false;
			 
			buf= m_oSecGet.readLine();
			if(buf != null)
				match= answer.match(buf);
			if(!match)
			{
				String sPort= ""+m_nPort;
				
				m_sErrorMsg= m_oTrans.translate("UNDEFINEDSERVER", m_sHost, sPort);
				m_sErrorCode= "UNDEFINEDSERVER";
				try{
					m_oSecGet.close();
					m_oSecSock.close();
				}catch(IOException iex)
				{
					// ???
				}
				oPut.close();
				m_oSecSock= null;
				m_oSecGet= null;
				return false;
			}
			id= answer.getParen(1);
			if(m_nConnectionID != Long.decode(id))
			{
				m_sErrorCode= "FAILDSECONDCONNECTIONID";
				m_sErrorMsg= m_oTrans.translate(m_sErrorCode);
				try{
					m_oSecGet.close();
					m_oSecSock.close();
				}catch(IOException iex)
				{
					// ???
				}
				oPut.close();
				m_oSecSock= null;
				m_oSecGet= null;
				return false;
			}
			oPut.println("U:" + TreeNodes.m_sUser + ":" + TreeNodes.m_sPwd);
			oPut.flush();
			res= m_oSecGet.readLine();
			if(!res.equals("OK"))
			{
				m_sErrorCode= res; 
				m_sErrorMsg= m_oTrans.translatePortServerError(res, TreeNodes.m_sUser, null);
				try{
					m_oSecGet.close();
					m_oSecSock.close();
				}catch(IOException iex)
				{
					// ???
				}
				oPut.close();
				m_oSecSock= null;
				m_oSecGet= null;
				return false;
			}
		}catch(IOException ex)
		{
			m_sErrorMsg= m_oTrans.translate("READERIOEXCEPTION");
			m_sErrorCode= "READERIOEXCEPTION";
			try{
				m_oSecSock.close();
			}catch(IOException iex)
			{
				// ???
			}
			oPut.close();
			m_oSecSock= null;
			m_oSecGet= null;
			return false;
		}
		//oPut.close();
		//synchronized (m_bSecond) {
			
			m_bSecond= true;
		//}
		return true;
	}
	
	/**
	 * whether have client an second connection to server for hearing
	 * 
	 * @return true if second connection is established
	 */
	public boolean haveSecondConnection()
	{
		boolean bRv= false;
		
		//synchronized (m_bSecond) {
		
			bRv= m_bSecond;
		//}
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
		String getString;
		Calendar cal= Calendar.getInstance();
		HashMap<String, Date> array= new HashMap<String, Date>();
		RE descriptor= new RE("^/(.*) -> ([0-9]{1,2})/([0-9]{1,2})/([0-9]{2}) ([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})");
		
		m_sErrorMsg= "NONE";
		m_sErrorCode= "NONE";
		
		synchronized (this)
		{
			m_oPut.println("DIR " + filter);
			m_oPut.flush();
			try{
				getString= m_oGet.readLine();
				while(getString.compareTo("done") != 0)
				{
					if(descriptor.match(getString))
					{
						int Year= Integer.parseInt("20" +descriptor.getParen(4));
						int Month= Integer.parseInt(descriptor.getParen(2));
						int Day= Integer.parseInt(descriptor.getParen(3));
						int Houer= Integer.parseInt(descriptor.getParen(5));
						int Min= Integer.parseInt(descriptor.getParen(6));
						int Sec= Integer.parseInt(descriptor.getParen(7));
						
						cal.set(Year, (Month - 1), Day, Houer, Min, Sec);
						array.put(descriptor.getParen(1), cal.getTime());
					}
					getString= m_oGet.readLine();
				}
			}catch(IOException ex)
			{
				m_sErrorMsg= m_oTrans.translate("READERIOEXCEPTION");
				m_sErrorCode= "READERIOEXCEPTION";
			}catch(NullPointerException ex)
			{
				m_sErrorMsg= m_oTrans.translate("ABORTEDCONNECTION");
				m_sErrorCode= "ABORTEDCONNECTION";
			}
		}
		
		return array;
	}

	/**
	 * set double value on given path
	 * 
	 * @param path  of subroutine<br />
	 * 				The path is made up of &lt;folder&gt;:&lt;subroutine&gt;:[subroutine:...]
	 * @param value	witch should be set<br />
	 * 				If the value should be an boolean for an SWITCH-subroutine, set 1 for true and 0 for false
	 * @return true if the server found the path, otherwise an error occurs false
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
	 * set double value on given path
	 * 
	 * @param folder the folder which be set on the server
	 * @param subroutine name of subroutine 
	 * @param value	witch should be set<br />
	 * 				If the value should be an boolean for an SWITCH-subroutine, set 1 for true and 0 for false
	 * @return true if the server found the path, otherwise an error occurs false
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public boolean setValue(String folder, String subroutine, double value)
	{
		boolean bRv= true;
		String getString;
		RE error= new RE("^ERROR ([0-9]+)( [0-9]+)?$");
		
		m_sErrorMsg= "NONE";
		m_sErrorCode= "NONE";

		synchronized (this)
		{	
			m_oPut.println("SET " + folder + ":" + subroutine + " " + value);
			m_oPut.flush();
			try{
				getString= m_oGet.readLine();
				if(getString == null)
				{
					m_sErrorMsg= m_oTrans.translatePortServerError("ABORTEDCONNECTION", folder, subroutine);
					m_sErrorCode= "ABORTEDCONNECTION";
					return false;
				}
				if(error.match(getString))
				{
					m_sErrorMsg= m_oTrans.translatePortServerError(getString, folder, subroutine);
					m_sErrorCode= getString;
					bRv= false;
				}
			}catch(IOException ex)
			{
				m_sErrorMsg= m_oTrans.translate("READERIOEXCEPTION");
				m_sErrorCode= "READERIOEXCEPTION";
				bRv= false;
			}
		}
		return bRv;
	}

	/**
	 * split an path from folder and subroutines, which be delimited with colons
	 * 
	 * @param path folder and subroutines delimited with colons
	 * @return an object from PathHolder which contains separately folder and subroutines
	 * @see PathHolder
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public Pathholder splitPath(String path)
	{
		Pathholder tStruct= new Pathholder();
		String split[]= path.split(":");

		m_sErrorMsg= "NONE";
		m_sErrorCode= "NONE";
		if(	split == null
			||
			split.length < 2	)
		{
			m_sErrorMsg= m_oTrans.translate("UNDEFINEDPATH", path);
			m_sErrorCode= "UNDEFINEDPATH";
			return null;
		}
		for(int o= 0; o<split.length-1; ++o)
			tStruct.folder+= split[o] + ":";
		tStruct.folder= tStruct.folder.substring(0, tStruct.folder.length()-1);
		tStruct.subroutine= split[split.length-1];
		return tStruct;
	}

	/**
	 * get value from given path, which be set currently on server
	 * 
	 * @param path folder and subroutines delimited with colons
	 * @return the double value, if an error occurs the return-value is null
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

	/*public double getProberty(String folder, String subroutine)
	{
		String getString;
		RE error= new RE("^ERROR ([0-9]+)( [0-9]+)?$");
		
		m_sErrorMsg= "NONE";
		m_sErrorCode= "NONE";
		if(!m_bPermConnect)
		{
			if(!connect())
				return 0;
		}

		synchronized (this)
		{	
			m_oPut.println("PROP " + folder + ":" + subroutine);
			m_oPut.flush();
			try{
				getString= m_oGet.readLine();
				if(error.match(getString))
				{
					m_sErrorMsg= m_oTrans.translatePortServerError(getString, folder, subroutine);
					m_sErrorCode= getString;
					return 0;
				}			
			}catch(IOException ex)
			{
				m_sErrorMsg= m_oTrans.translate("READERIOEXCEPTION");
				m_sErrorCode= "READERIOEXCEPTION";
				return 0;
			}
		}
		if(!m_bPermConnect)
			closeConnection();
		return Double.parseDouble(getString);
	}*/
	
	/**
	 * read XML-file from server
	 * 
	 * @param file string of filename
	 * @return ArrayList with strings from the layout xml-file
	 */
	public ArrayList<String> getContent(String file)
	{
		ArrayList<String> xmlFile= new ArrayList<String>();
		RE endParser= new RE("<[ \t]*/[ \t]*layout[ \t]*>");
		RE error= new RE("<[ \t]*error[ \t]*.*[ \t]*number[ \t]*=[ \t]*('|\")([ 0-9]+)('|\")[ \t]*/[ \t]*>");
		String getString= null;

		m_sErrorMsg= "NONE";
		m_sErrorCode= "NONE";
		synchronized (this)
		{
			m_oPut.println("CONTENT " + file);
			m_oPut.flush();
			
			try{
				do{
					getString= m_oGet.readLine();
					if(getString == null)
					{
						m_sErrorMsg= m_oTrans.translate("READERIOEXCEPTION");
						m_sErrorCode= "READERIOEXCEPTION";
						return null;
					}
					if(error.match(getString))
					{
						getString= "ERROR " + error.getParen(2);
						m_sErrorMsg= m_oTrans.translatePortServerError(getString, file, "");
						m_sErrorCode= getString;
						return null;
					}
					xmlFile.add(getString);
					
				}while(!endParser.match(getString));
				
			}catch(IOException ex)
			{
				m_sErrorMsg= m_oTrans.translate("READERIOEXCEPTION");
				m_sErrorCode= "READERIOEXCEPTION";
				return null;
			}catch(NullPointerException ex)
			{
				System.out.println("getString: "+getString);
				ex.printStackTrace();
				m_sErrorMsg= m_oTrans.translate("READERIOEXCEPTION");
				m_sErrorCode= "READERIOEXCEPTION";
				return null;
			}
		}
		return xmlFile;
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
		String res;
		
		res= command("CHANGE " + user + ":" + password, user, "");
		if(res == null)
			return false;
		if(!res.equals("OK"))
		{
			m_sErrorCode= "UNKNOWNERROR";
			m_sErrorMsg= m_oTrans.translate(m_sErrorCode);
			return false;
		}
		return true;
	}
	
	/**
	 * sending request for permission groups
	 * 
	 * @param groups groups of permission, separately with colons
	 * @return writable, readable or None permission.<br />if an error occurred the permission is None and the error can reading in <code>getErrorCode()</code> 
	 */
	public permission permission(String groups)
	{
		String res;
		
		res= command("PERMISSION " + groups, TreeNodes.m_sUser, "");
		if(res == null)
			return permission.None;
		if(res.equals("write"))
			return permission.writeable;
		else if(res.equals("read"))
			return permission.readable;		
		return permission.None;
	}
	
	/**
	 * sending request for subroutines in folder to hearing for changes.<br />
	 * this command should only send if open an second connection
	 * to server where you can send an hearing request with the command <code>request()</code>
	 * 
	 * @param folder name of folder
	 * @param subroutine name of subroutine
	 * @return whether hear command was requested correctly
	 */
	public boolean hear(String folder, String subroutine)
	{
		String res;
		
		res= command("HEAR " + folder + ":" + subroutine, folder, subroutine);
		if(res == null)
			return false;
		if(!res.equals("done"))
		{
			m_sErrorCode= "UNKNOWNERROR";
			m_sErrorMsg= m_oTrans.translate(m_sErrorCode);
			return false;
		}
		return true;
	}
	
	public boolean hear(String result)
	{
		RE f= new RE("^([^:]+):(.+)$");
		
		if(!f.match(result))
		{
			m_sErrorCode= "NOCORRECTFOLDERRESULT";
			m_sErrorMsg= m_oTrans.translate(m_sErrorCode);
			return false;
		}
		return hear(f.getParen(1), f.getParen(2));
	}
	
	
	public String hearing()
	{
		String res= null;
		RE error= new RE("^ERROR ([0-9]+)( [0-9]+)?$");
		
		m_sErrorMsg= "NONE";
		m_sErrorCode= "NONE";

		synchronized (m_oSecGet)
		{
			try{
				res= m_oSecGet.readLine();
				if(res == null)
				{
					m_sErrorMsg= m_oTrans.translate("ABORTEDCONNECTION");
					m_sErrorCode= "ABORTEDCONNECTION";
					return null;
				}
				if(error.match(res))
				{
					m_sErrorMsg= m_oTrans.translatePortServerError(res, "", "");
					m_sErrorCode= res;
					return null;
				}			
			}catch(IOException ex)
			{
				ex.printStackTrace();
				m_sErrorMsg= m_oTrans.translate("READERIOEXCEPTION");
				m_sErrorCode= "READERIOEXCEPTION";
				return null;
			}catch(NullPointerException ex)
			{
				System.out.println("getString: "+res);
				ex.printStackTrace();
				m_sErrorMsg= m_oTrans.translate("READERIOEXCEPTION");
				m_sErrorCode= "READERIOEXCEPTION";
				return null;
			}
		}
		return res;
	}
	/**
	 * clear hearing on server for the request was sending before
	 * 
	 * @return whether clearing was ok
	 */
	public boolean clearHearing()
	{

		String res;
		
		if(!haveSecondConnection())
			return false;
		res= command("NEWENTRYS", "", "");
		if(res == null)
			return false;
		if(!res.equals("done"))
		{
			m_sErrorCode= "UNKNOWNERROR";
			m_sErrorMsg= m_oTrans.translate(m_sErrorCode);
			return false;
		}
		return true;
	}
	
	/**
	 * sending any command to server and waiting for answer
	 * 
	 * @param str command which should sending to server
	 * @return answer if the request was successfully, otherwise null
	 */
	protected String command(String str, String err1, String err2)
	{
		String res= null;
		RE error= new RE("^ERROR ([0-9]+)( [0-9]+)?$");
		
		m_sErrorMsg= "NONE";
		m_sErrorCode= "NONE";

		synchronized (this)
		{	
			m_oPut.println(str);
			m_oPut.flush();
			try{
				res= m_oGet.readLine();
				if(res == null)
				{
					m_sErrorMsg= m_oTrans.translate("ABORTEDCONNECTION");
					m_sErrorCode= "ABORTEDCONNECTION";
					return null;
				}
				if(error.match(res))
				{
					m_sErrorMsg= m_oTrans.translatePortServerError(res, err1, err2);
					m_sErrorCode= res;
					return null;
				}			
			}catch(IOException ex)
			{
				m_sErrorMsg= m_oTrans.translate("READERIOEXCEPTION");
				m_sErrorCode= "READERIOEXCEPTION";
				return null;
			}catch(NullPointerException ex)
			{
				System.out.println("getString: "+res);
				ex.printStackTrace();
				m_sErrorMsg= m_oTrans.translate("READERIOEXCEPTION");
				m_sErrorCode= "READERIOEXCEPTION";
				return null;
			}
		}
		return res;
	}
	/**
	 * get value from given folder and subroutines, which be set currently on server
	 * 
	 * @param folder the folder which be set on the server
	 * @param subroutine one ore more subroutines, delimited with colons
	 * @return the double value, if an error occurs the return-value is null
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public Double getValue(String folder, String subroutine)
	{
		double nRv;
		String getString= null;
		RE error= new RE("^ERROR ([0-9]+)( [0-9]+)?$");
		
		m_sErrorMsg= "NONE";
		m_sErrorCode= "NONE";

		synchronized (this)
		{	
			m_oPut.println("GET " + folder + ":" + subroutine);
			m_oPut.flush();
			try{
				getString= m_oGet.readLine();
				if(getString == null)
				{
					m_sErrorMsg= m_oTrans.translate("ABORTEDCONNECTION");
					m_sErrorCode= "ABORTEDCONNECTION";
					return null;
				}
				if(error.match(getString))
				{
					m_sErrorMsg= m_oTrans.translatePortServerError(getString, folder, subroutine);
					m_sErrorCode= getString;
					return null;
				}			
			}catch(IOException ex)
			{
				m_sErrorMsg= m_oTrans.translate("READERIOEXCEPTION");
				m_sErrorCode= "READERIOEXCEPTION";
				return null;
			}catch(NullPointerException ex)
			{
				System.out.println("getString: "+getString);
				ex.printStackTrace();
				m_sErrorMsg= m_oTrans.translate("READERIOEXCEPTION");
				m_sErrorCode= "READERIOEXCEPTION";
				return null;
			}
		}
		try{
			nRv= Double.parseDouble(getString);
		}catch(NumberFormatException ex)
		{
			nRv= 0;
		}
		return nRv;
	}

	/**
	 * close connection to server
	 * 
	 * @return true if connection is correctly closed, otherwise false
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public boolean closeConnection()
	{
		m_sErrorMsg= "NONE";
		m_sErrorCode= "NONE";
		m_oPut.println("ending");
		m_oPut.flush();

		//synchronized (m_bSecond) {
		
			m_bSecond= false;
		//}
		synchronized (this)
		{	
			try{
				m_oGet.close();
				if(m_oSecGet != null)
					m_oSecGet.close();
				m_oPut.close();
				m_oSock.close();
				if(m_oSecSock != null)
					m_oSecSock.close();
			}catch(IOException ex)
			{
				m_sErrorMsg= m_oTrans.translate("IOCLOSEERROR");
				m_sErrorCode= "IOCLOSEERROR";
				return false;
			}
		}
		return true;
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
	
	/*public double getProberty(String folder, String subroutine)
	{
		String getString;
		RE error= new RE("^ERROR ([0-9]+)( [0-9]+)?$");
		
		m_sErrorMsg= "NONE";
		m_sErrorCode= "NONE";
		if(!m_bPermConnect)
		{
			if(!connect())
				return 0;
		}
	
		synchronized (this)
		{	
			m_oPut.println("PROP " + folder + ":" + subroutine);
			m_oPut.flush();
			try{
				getString= m_oGet.readLine();
				if(error.match(getString))
				{
					m_sErrorMsg= m_oTrans.translatePortServerError(getString, folder, subroutine);
					m_sErrorCode= getString;
					return 0;
				}			
			}catch(IOException ex)
			{
				m_sErrorMsg= m_oTrans.translate("READERIOEXCEPTION");
				m_sErrorCode= "READERIOEXCEPTION";
				return 0;
			}
		}
		if(!m_bPermConnect)
			closeConnection();
		return Double.parseDouble(getString);
	}*/
	
}
