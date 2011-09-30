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

import at.kolli.automation.client.LayoutLoader;
import at.kolli.automation.client.MsgTranslator;
import at.kolli.automation.client.MsgClientConnector;
import at.kolli.automation.client.NodeContainer;
import at.kolli.automation.client.TreeNodes;
import at.kolli.dialogs.DialogThread;

/**
 * Thread to read continually the state for the components on the server
 * 
 * @package at.kolli.layout
 * @author Alexander Kolli
 * @version 1.00.00, 04.12.2007
 * @since JDK 1.6
 */
public class WidgetChecker extends Thread
{
	/**
	 * single instance of WidgetChecker
	 */
	private static WidgetChecker _instance= null;
	/**
	 * whether tread should be running
	 */
	private volatile Boolean m_bRun= true;
	/**
	 * array of all TreeNodes
	 */
	private TreeNodes m_AktTreeNode;
	
	/**
	 * constructor to create thread with all TreeNodes
	 * 
	 * @param aTreeNodes all TreeNodes which can listen on server
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	private WidgetChecker(TreeNodes aTreeNodes)
	{
		// start class
		m_AktTreeNode= aTreeNodes;
	}
	
	/**
	 * creating if not exist and returning single instance of WidgetChecker
	 * 
	 * @param aTreeNodes all TreeNodes which can listen on server
	 * @return instance if exist, otherwise null
	 * @serial
	 * @see
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public static WidgetChecker instance(TreeNodes aTreeNode)
	{
		if(_instance == null)
			_instance= new WidgetChecker(aTreeNode);
		return _instance;
	}
	
	/**
	 * returning single instance of WidgetChecker
	 * 
	 * @return instance if exist, otherwise null
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public static WidgetChecker instance()
	{
		return _instance;
	}
	
	/**
	 * sets an new array of TreeNodes to listen on server
	 * 
	 * @param aTreeNodes new array of TreeNodes
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public void setTreeNode(TreeNodes oTreeNode)
	{
		synchronized (m_AktTreeNode)
		{	
			m_AktTreeNode= oTreeNode;
		}
	}
	
	/**
	 * running instance of thread
	 * 
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public void run()
	{
		NodeContainer cont= null;
		MsgClientConnector client= MsgClientConnector.instance();
		LayoutLoader loader= LayoutLoader.instance();
		
		setName("hearingThread");
		if(client.haveSecondConnection())
			cont= new NodeContainer();
		while(m_bRun)
		{	
			try{
				if(cont != null)
				{
					String res= client.hearing(/*bthrow*/false);
					
					if(	!cont.read("%f:%s=%d", res)
						&&
						!cont.read("%f:%s %c", res))
					{// result can be an other command
						if(res == null)
						{ 
							//final DialogThread.states retState= DialogThread.states.OK;
							DialogThread.states retState;
							DialogThread dialog= DialogThread.instance();//m_oTopLevelShell);
							MsgTranslator trans= MsgTranslator.instance();
							
							client.closeConnection();
							loader.setState(LayoutLoader.BROKEN);
							dialog.needProgressBar();
							dialog.needUserVerificationFields();
							dialog.show(trans.translate("dialogChangeUser"), trans.translate("dialogUserVerification"));
							retState= dialog.produceDialog(LayoutLoader.REFRESH);
							if(retState == DialogThread.states.CANCEL)
							{
								return;
							}else
								loader.setState(LayoutLoader.WAIT);
							client.secondConnection();
							continue;
						}
						if(res.equals("stopclient"))
						{
							m_bRun= false;
							break;
						}
						sleep(10);
					}
				}
				synchronized (TreeNodes.m_DISPLAYLOCK)
				{
					String node;
					
					node= loader.checkNewSide(cont);
					if(!node.equals(""))
					{// set new side active
						loader.m_sAktFolder= node;
						loader.setActSideVisible(/*inform server by no body*/false);
					}else
					{// fill values into actual side
						synchronized(m_AktTreeNode)
						{
							m_AktTreeNode.listenClient(client, cont);
						}
					}
				}
				if(cont == null)
					sleep(10);
			}catch(InterruptedException ex)
			{
				//anyone interrupt thread from WidgetChecker,
				//so stopping hole checking
			}catch(IOException ex)
			{
				if(HtmTags.debug)
				{
					System.out.println(MsgTranslator.instance().translate(ex.getMessage()));
					ex.printStackTrace();
				}
			}
		}
	}
	
	/**
	 * thread stopping immediately
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public void stopping()
	{
		m_bRun= false;
		interrupt();
	}
}
