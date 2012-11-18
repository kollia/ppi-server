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
package at.kolli.dialogs;

import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import org.eclipse.jface.dialogs.Dialog;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;

import at.kolli.layout.HtmTags;

/**
 * class running as thread and waiting to display an dialog
 * if the connection is out of reach
 * ore application work durring a long time
 * 
 * @package at.kolli.dialogs
 * @author Alexander Kolli
 * @version 1.00.00, 02.12.2007
 * @since JDK 1.6
 */
public class DialogThread // extends Thread
{
	/**
	 * enum of dialog states
	 */
	public enum states {
		
			NONE,
			RUN,
			OK,
			CANCEL,
			ERROR
	}
	/**
	 * steps of progress bar per side
	 */
	public static int m_nProgressSteps;
	/**
	 * instance of own thread-object
	 */
	private static DialogThread _instance= null;
	/**
	 * Instances representing the "windows" which the desktop or "window manager" is managing
	 */
	private Shell m_oShell;
	/**
	 * whether thread is running want to display dialog on screen
	 */
	private volatile boolean m_bRunning= true;
	/**
	 * boolean value whether the dialog is displayed on screen or not
	 */
	private volatile boolean m_bOpen= false;//new Boolean(false);
	/**
	 * lock object to create conditions
	 */
	private final Lock lock= new ReentrantLock();
	/**
	 * condition to wait if more threads want to open the dialog box
	 */
	private final Condition closeDialog= lock.newCondition();
	/**
	 * whether Connection needs an progress bar
	 */
	private Boolean m_bProgress= false;
	/**
	 * whether ConnectionDialog needs an user verification
	 */
	private Boolean m_bVerification= false;
	/**
	 * dialog thread should ending
	 */
	private volatile boolean m_bStop= false;
	/**
	 * instance of Dialog
	 */
	private volatile ConnectionDialog m_oDialog= null;
	/**
	 * title for dialog
	 */
	private String m_sTitle;
	/**
	 * message for dialog
	 */
	private String m_sMessage;
	/**
	 * result of user interaction
	 */
	private states m_eState= states.NONE;
	/**
	 * selected with in progress bar
	 */
	private Integer m_nSelected= 0;
	/**
	 * minimum of progress bar
	 */
	private Integer m_nMinimum= 0;
	/**
	 * maximum of progress bar
	 */
	private Integer m_nMaximum= 0;
	
	/**
	 * constructor to create instance of DialogThread object
	 * 
	 * @param shell Shell of actual window
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	private DialogThread(Shell shell)
	{
		m_oShell= shell;//new Shell(shell, SWT.APPLICATION_MODAL);
	}
	
	/**
	 * returning instance of own DialogThread object
	 * 
	 * @return single instance of DialogThread object
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public static DialogThread instance()
	{
		return _instance;
	}

	/**
	 * wait for verify user and password
	 * 
	 * @param user caller get user name
	 * @param password caller get password
	 * @return whether dialog box is allocated for verify user
	 */
	public states verifyUser(String user, String password, String error)
	{
		if(!m_bVerification)
			return states.ERROR;
		while(m_oDialog == null)
		{
			if(HtmTags.debug)
				System.out.println("WARNING: DialogThread.verifyUser() no Dialog defined for verifying, wait until dialog starts");
			try{
				Thread.sleep(500);
			}catch(InterruptedException ex)
			{}
		}
		return m_oDialog.verifyUser(user, password, error);
	}
	/**
	 * creating instance of DialogThread object if it does not exists
	 * and returning it
	 * 
	 * @param shell Shell of actual window instance
	 * @return single instance of DialogThread object
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public static DialogThread instance(Shell shell)
	{
		if(_instance == null)
			_instance= new DialogThread(shell);
		return _instance;
	}
	
	/**
	 * running workflow of DialogThread
	 * 
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	/*public void run()
	{
		setName("DialogThread");
		while(!m_bStop)
		{
			m_bRunning= true;
			DisplayAdapter.syncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					produceDialog(LayoutLoader.CREATE);
				}
			});

			try{
				System.out.println("want to open bOpen on DialogThread::run");
				synchronized(m_bOpen)
				{
					m_bRunning= false;
					m_bOpen.wait();
					m_bRunning= true;
				}
				System.out.println("give bOpen free");
			}catch(InterruptedException ex)
			{
				//anyone iterrupt thread from DialogThread,
				//to stop or show Dialog
			}
		}
	}*/
	
	/**
	 * whether ConnectionDialog needs an progress bar
	 */
	public void needProgressBar()
	{
		m_bProgress= true;
	}
	
	/**
	 * whether ConnectionDialog needs an user verification
	 */
	public void needUserVerificationFields()
	{
		m_bVerification= true;
	}
	
	/**
	 * starting dialog on screen
	 * 
	 * @return integer result of user action
	 */
	public states produceDialog(short layoutType)
	{
			if(!m_bOpen)
			{
				final short ltype= layoutType;

				m_bOpen= true;
				synchronized (m_eState)
				{
					m_eState= states.RUN;
				}
				m_oDialog= new ConnectionDialog(m_oShell);
				if(m_bProgress)
					m_oDialog.needProgressBar();
				if(m_bVerification)
					m_oDialog.needUserVerificationFields();	
				DisplayAdapter.syncExec(new Runnable() {
				
					public void run() {

						int result;
						
						m_oDialog.create(ltype);
						m_oDialog.setTitle(m_sTitle);
						m_oDialog.setMessage(m_sMessage);
						result= m_oDialog.open();
						Thread t= Thread.currentThread();
						System.out.println("dialog box ending");
						System.out.println(t.getName()+" want to lock state variable");
						synchronized (m_eState) 
						{						
							System.out.println(t.getName()+" lock state variable");
							if(result == Dialog.OK)
								m_eState= states.OK;
							else if(result == Dialog.CANCEL)
								m_eState= states.CANCEL;
							else
								m_eState= states.ERROR;	
							System.out.println(t.getName()+" give state lock free");
						}
						m_oDialog.close();			
					}
				}, "DialogThread::produceDialog() create dialog");
			
				m_oDialog= null;
				m_bOpen= false;
				m_bProgress= false;
				m_bVerification= false;
				lock.lock();
				closeDialog.signalAll();
				lock.unlock();
				
			}else
			{
				lock.lock();
				try{
				// wait until dialog was closed by other thread
					closeDialog.await();
				}catch(InterruptedException ex)
				{}
				finally
				{
					lock.unlock();
				}					
			}
		return m_eState;
	}
	
	/**
	 * have thread dialog open on screen
	 * 
	 * @return true if dialog is displayed on screen, otherwise false
	 * @author Alexander Kolli
	 * @version 1.00.00, 23.12.2007
	 * @since JDK 1.6
	 */
	synchronized public boolean isOpen()
	{
		return m_bOpen;
	}
	/**
	 * set state of open
	 * 
	 * @param open whether dialog is open
	 * @author Alexander Kolli
	 * @version 0.02.00, 16.07.2011
	 * @since JDK 1.6
	 */
	synchronized protected void setOpen(boolean open)
	{
		m_bOpen= open;
	}
	
	/**
	 * returning the state of current dialog
	 * 
	 * @return user interaction
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public states dialogState()
	{ 
		states eRv= states.NONE;
		
		synchronized (m_eState)
		{
			eRv= m_eState;
		}
		return eRv;
	}
	
	/**
	 * showing whether dialog is displayed on screen or not
	 * 
	 * @return true if dialog is displayed, otherwise false
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public boolean running()
	{
		return m_bRunning;
	}
	
	/**
	 * show message in dialog thread
	 * and start to display if not done before
	 * 
	 * @param message text which should be shown
	 * @return true if the dialog must be disblay, otherwise false if only must show message in actual dialog
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public boolean show(String message)
	{
		return this.show("", message);
	}
	
	/**
	 * show title and message in dialog thread
	 * and start to display if not done before
	 * 
	 * @param title text which should be shown as title
	 * @param message text which should be shown as message
	 * @return true if the dialog must be disblay, otherwise false if only must show title and message in actual dialog
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public boolean show(String title, String message)
	{
		boolean bRv;
		
		if(!title.equals(""))
			m_sTitle= title;
		m_sMessage= message;
		if(!m_bRunning)
		{
			bRv= true;
		}else
		{
			if(m_bOpen)
			{
				Display.getDefault().asyncExec(new Runnable()
				{				
					//@Override
					public void run() 
					{
						m_oDialog.setTitle(m_sTitle);
						m_oDialog.setMessage(m_sMessage);
					}
				});
			}
			bRv= false;
		}
		return bRv;
	}
	
	/**
	 * sets the current value in the progress bar
	 * 
	 * @param value current value
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public void setSelection(int value)
	{
		synchronized(m_nSelected)
		{
			m_nSelected= value;

			if(!m_bOpen)
				return;
			DisplayAdapter.asyncExec(new Runnable()
			{				
				//@Override
				public void run()
				{
					synchronized (m_eState)
					{
						if(m_eState.equals(states.RUN))
						{
							if(HtmTags.debug)
								System.out.println("dialog progress bar be set to " + m_nSelected + "%");
							m_oDialog.setSelection(m_nSelected);
						}
					}
				}				
			});
		}
	}
	
	/**
	 * returning the current value of progress bar
	 * 
	 * @return current value
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public int getSelection()
	{
		int nRv;
		
		synchronized(m_nSelected)
		{
			nRv= m_nSelected;
		}
/*			Thread t= Thread.currentThread();
			
			if(dialogState().equals(DialogThread.states.CANCEL))
				return 0;
			System.out.println(t.getName()+" want to lock state variable to get progressBar.selection");
			synchronized (m_eState)
			{					
				System.out.println(t.getName()+" lock state variable");
				if(m_eState != states.RUN)
					return 0;
				m_nSelected= 0;
				DisplayAdapter.syncExec(new Runnable()
				{				
					//@Override
					public void run()
					{
						m_nSelected= m_oDialog.getSelection();
					}				
				});
				System.out.println(t.getName()+" give state lock free");
			}*/
		return nRv; //m_nSelected;
	//	}
	}
	
	/**
	 * maximum value of progress bar
	 * 
	 * @return maximum value
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public int getMaximum()
	{
		synchronized(m_nMaximum)
		{
			if(!m_bOpen)
				return 0;
			DisplayAdapter.syncExec(new Runnable()
			{				
				//@Override
				public void run()
				{
					m_nMaximum= m_oDialog.getMaximum();
				}				
			});
			return m_nMaximum;
		}
	}
	
	/**
	 * minimum value of progress bar
	 * 
	 * @return minimum value
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public int getMinimum()
	{
		synchronized(m_nMaximum)
		{
			if(!m_bOpen)
				return 0;
			DisplayAdapter.syncExec(new Runnable()
			{				
				//@Override
				public void run()
				{
					m_nMinimum= m_oDialog.getMinimum();
				}				
			});
			return m_nMinimum;
		}
	}
	
	/**
	 * close actual Dialog
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public void close()
	{
		m_bStop= true;
		//synchronized (TreeNodes.m_DISPLAYLOCK)
		{
			Display.getDefault().asyncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{				
					if(m_bOpen)
						m_oDialog.close();
				}
			});
		}
		if(HtmTags.debug)
			System.out.println("Closing dialog box");
	}
}
