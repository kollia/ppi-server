package at.kolli.layout;

import at.kolli.automation.client.LayoutLoader;
import at.kolli.automation.client.MsgTranslator;
import at.kolli.automation.client.NoStopClientConnector;
import at.kolli.automation.client.NodeContainer;
import at.kolli.automation.client.TreeNodes;
import at.kolli.dialogs.DialogThread;
import at.kolli.dialogs.DisplayAdapter;

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
		NoStopClientConnector client= NoStopClientConnector.instance();
		
		setName("hearingThread");
		if(client.haveSecondConnection())
			cont= new NodeContainer();
		while(m_bRun)
		{	
			try{
				if(cont != null)
				{
					String res= client.hearing();
					
					if(	!cont.read("%f:%s=%d", res)
						&&
						!cont.read("%f:%s %c", res))
					{// result can be an other command
						if(res == null)
						{
							//final DialogThread.states retState= DialogThread.states.OK;
							
							client.closeConnection();
							DisplayAdapter.syncExec(new Runnable() {
								
								public void run() {

									DialogThread.states retState;
									LayoutLoader loader= LayoutLoader.instance();
									DialogThread dialog= DialogThread.instance();//m_oTopLevelShell);
									MsgTranslator trans= MsgTranslator.instance();
									
									synchronized (TreeNodes.m_DISPLAYLOCK)
									{
										loader.start(LayoutLoader.REFRESH);
										dialog.needProgressBar();
										dialog.needUserVerificationFields();
										dialog.show(trans.translate("dialogChangeUser"), trans.translate("dialogUserVerification"));
										retState= dialog.produceDialog(LayoutLoader.REFRESH);
										if(retState == DialogThread.states.CANCEL)
										{
											System.exit(1);
										}else
											loader.start(LayoutLoader.WAIT);
									}
								}
								
							}, "WidgetChecker::run() refresh by connection ending");
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
					synchronized(m_AktTreeNode)
					{
						m_AktTreeNode.listenClient(client, cont);
					}
				}
				if(cont == null)
					sleep(10);
			}catch(InterruptedException ex)
			{
				//anyone iterrupt thread from WidgetChecker,
				//so stoping hole checking
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
