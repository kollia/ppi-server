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

import org.eclipse.jface.dialogs.IDialogSettings;
import org.eclipse.jface.dialogs.TitleAreaDialog;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.DisposeEvent;
import org.eclipse.swt.events.DisposeListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.VerifyEvent;
import org.eclipse.swt.events.VerifyListener;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.ProgressBar;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Text;

import at.kolli.automation.client.MsgTranslator;
import at.kolli.automation.client.TreeNodes;
import at.kolli.dialogs.DialogThread.states;

/**
 * Dialog to display an progress bar and shows messages for during workflow
 * 
 * @package at.kolli.dialogs
 * @author Alexander Kolli
 * @version 1.00.00, 04.12.2007
 * @since JDK 1.6
 */
public class ConnectionDialog extends TitleAreaDialog
{
	/**
	 * whether dialog is open
	 */
	private volatile Boolean m_bOpen= false;
	/**
	 * type of dialog
	 */
	private short m_nType;
	/**
	 * ok button to set enable or disable
	 */
	Button m_okButton;
	/**
	 * cancelButton to get bounds of button
	 */
	Button m_cancelButton;
	/**
	 * field for ok button to change bounds
	 */
	Composite m_okButtonComposite;
	/**
	 * whether progress bar should pass continuously
	 */
	private boolean m_nIndeterminate= false;
	/**
	 * whether Connection needs an progress bar
	 */
	private Boolean m_bProgress= false;
	/**
	 * whether ConnectionDialog needs an user verification
	 */
	private Boolean m_bVerification= false;
	/**
	 * whether user is verified
	 */
	private boolean m_bVerified= false;
	/**
	 * instnace of progress bar
	 */
	private ProgressBar m_oProgressBar;
	/**
	 * Combo object of user entry
	 */
	private Combo m_oUser= null;
	/**
	 * Text object of password entry
	 */
	private Text m_oPwd= null;
	/**
	 * password for user after input
	 */
	private String m_sPwd;
	
	/**
	 * constructor to create dialog shell
	 * 
	 * @param shell Shell of actual window
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public ConnectionDialog(Shell shell)
	{
		super(shell);
	}
	
	/**
	 * create dialog
	 * 
	 * @param dialogType type from static LayoutLoader
	 */
	public void create(short dialogType)
	{
		m_nType= dialogType;
		create();
	}
	/**
	 * Overridden adds buttons to this dialog's button bar.
	 * 
	 * @param parent the button bar composite
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	protected void createButtonsForButtonBar(Composite parent)
	{		
		MsgTranslator trans= MsgTranslator.instance();
		
		if(m_bVerification)
		{
			FillLayout fillLayout= new FillLayout();
			
			m_okButtonComposite= new Composite(parent, 0);
			m_okButtonComposite.setBackground(parent.getBackground());
			m_okButtonComposite.setForeground(parent.getForeground());
			m_okButtonComposite.setFont(parent.getFont());
			m_okButtonComposite.setLayout(fillLayout);
			//m_okButton= createButton(parent, OK, trans.translate("okButton"), false);
			m_okButton= new Button(parent, SWT.PUSH);
			m_okButton.setText(trans.translate("okButton"));
			m_okButton.setEnabled(false);
			m_okButton.addSelectionListener(new SelectionAdapter()
			{		
				@Override
				public void widgetSelected(SelectionEvent e)
				{
					synchronized (TreeNodes.m_sUser)
					{
						TreeNodes.m_sUser= m_oUser.getText();
						TreeNodes.m_sPwd= m_sPwd;
					}
					m_bVerified= true;
				}	
			});
		}
		m_cancelButton= createButton(parent, CANCEL, trans.translate("cancelButton"), !m_bVerification);
		m_cancelButton.addSelectionListener(new SelectionAdapter()
		{		
			@Override
			public void widgetSelected(SelectionEvent e)
			{
				super.widgetSelected(e);
				close();
			}
		
		});
		/*if(m_bVerification)
		{
			int cancelWidth= cancel.get;
			
			if(cancelWidth.width > okWidth.width)
				m_okButton.setBounds(cancelWidth);
			else
				cancel.setBounds(okWidth);
		}*/
	}
	
	/**
	 * enable or disable verification fields and ok button
	 *  
	 * @param set whether fields are enabled or disabled
	 */
	private void setEnable(Boolean set)
	{
		if(m_bOpen == false)
			return;
		if(	!m_bVerification
			||
			!m_okButton.isVisible()	)
		{
			return;
		}
		if(set)
		{
			Button cancel;
			Rectangle cancelBounds, okBounds, userBounds, pwdBounds;
		
			cancel= getButton(CANCEL);
			cancelBounds= cancel.getBounds();
			okBounds= m_okButton.getBounds();
			if(cancelBounds.width > okBounds.width)
			{
				okBounds.width= cancelBounds.width;
				m_okButton.setBounds(okBounds);
			}else
			{
				cancelBounds.width= okBounds.width;
				cancel.setBounds(cancelBounds);
			}
			userBounds= m_oUser.getBounds();
			pwdBounds= m_oPwd.getBounds();
			pwdBounds.width= userBounds.width;
			m_oPwd.setBounds(pwdBounds);
		}
		m_oUser.setEnabled(set);
		m_oPwd.setEnabled(set);
		m_okButton.setEnabled(set);
		
	}
	/**
	 * wait for verify user and password
	 * 
	 * @param user caller get user name
	 * @param password caller get password
	 * @return whether dialog box is allocated for verify user
	 */
	public states verifyUser(String user, String password, final String error)
	{
		IDialogSettings login= TreeNodes.m_Settings.getSection("loginDialog");
		int fnIndex= -1;
		final int nIndex;
		Boolean bVerified= false;
		String[] split;
		String names;
		
		if(!m_bVerification)
			return states.ERROR;

		if(login != null)
		{
			names= login.get("loginnames");
			split= names.split(":");
			fnIndex= 0;
			for (String name : split)
			{
				if(name.equals(user))
					break;
				++fnIndex;
			}
		}
		nIndex= fnIndex;
		
		DisplayAdapter.syncExec(new Runnable()
		{				
			//@Override
			public void run() 
			{
				setEnable(true);
				m_oPwd.setText("");
				if(nIndex >= 0)
				{
					m_oUser.select(nIndex);
					if(error.equals(""))
						m_oPwd.setFocus();
				}else if(error.equals(""))
					m_oUser.setFocus();
				if(	error.equals("ERROR 011")
					||
					error.equals("ERROR 015"))
				{
					m_oUser.setFocus();
				}else if(!error.equals(""))
					m_oPwd.setFocus();
			}
		});
		m_bVerified= false;
		while(!bVerified)
		{
			synchronized (TreeNodes.m_sUser)
			{
				user= TreeNodes.m_sUser;
				password= TreeNodes.m_sPwd;
			}
			if(m_bVerified)
				break;
			if(!m_bOpen)
				return states.CANCEL;
			try{
				Thread.sleep(500);
			}catch(InterruptedException ex)
			{
				ex.printStackTrace();
			}
			synchronized (TreeNodes.m_sUser)
			{
				user= TreeNodes.m_sUser;
				password= TreeNodes.m_sPwd;
			}
		}
		DisplayAdapter.syncExec(new Runnable()
		{				
			//@Override
			public void run() 
			{
				setEnable(false);
			}
		});
		return states.OK;
	}
	
	/**
	 * Overridden creates and returns the contents of the upper part of this dialog (above the button bar).
	 * 
	 * @param parent The parent composite to contain the dialog area
	 * @return the dialog area control
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	protected Control createDialogArea(Composite parent)
	{
		int style= m_nIndeterminate ? SWT.INDETERMINATE : SWT.NULL;
		final Composite area= new Composite(parent, SWT.NULL);
		final GridLayout layout= new GridLayout();
		final GridData data= new GridData();
		final GridData progressData= new GridData();
		
		m_bOpen= true;
		if(m_bProgress)
		{
			data.verticalAlignment= GridData.CENTER;
			data.horizontalAlignment= GridData.CENTER;
			data.grabExcessHorizontalSpace= true;
			data.grabExcessVerticalSpace= true;
			area.setLayout(layout);
			area.setLayoutData(data);
			progressData.widthHint= 300;
			m_oProgressBar= new ProgressBar(area, style);	
			m_oProgressBar.setLayoutData(progressData);
		}
		if(m_bVerification)
		{
			IDialogSettings login= TreeNodes.m_Settings.getSection("loginDialog");
			MsgTranslator trans= MsgTranslator.instance();
			Label oUser;
			Label oPwd;
			
			new Label(area, SWT.NONE);
			oUser= new Label(area, SWT.NONE);
			m_oUser= new Combo(area, SWT.DROP_DOWN);
			if(login != null)
			{
				String snames[];
				String names= login.get("loginnames");
				
				if(names != null)
				{
					snames= names.split(":");
					for (String name : snames)
					{
						m_oUser.add(name);
					}
				}
			}
			m_oUser.setEnabled(false);
			oPwd= new Label(area, SWT.NONE);
			m_oPwd= new Text(area, SWT.SINGLE);
			m_oPwd.setEnabled(false);
			oUser.setText(trans.translate("W_USER") +":");
			oPwd.setText(trans.translate("W_PASSWORD") +":");
			m_sPwd= "";
			m_oUser.addSelectionListener(new SelectionAdapter()
			{
				public void widgetDefaultSelected(SelectionEvent ev) 
				{
					synchronized (TreeNodes.m_sUser)
					{
						TreeNodes.m_sUser= m_oUser.getText();
						TreeNodes.m_sPwd= m_sPwd;
					}
					m_bVerified= true;
				}
			});
			m_oPwd.addSelectionListener(new SelectionAdapter()
			{
				public void widgetDefaultSelected(SelectionEvent ev)
				{
					synchronized (TreeNodes.m_sUser)
					{
						TreeNodes.m_sUser= m_oUser.getText();
						TreeNodes.m_sPwd= m_sPwd;
					}
					m_bVerified= true;
				}
			});
			m_oPwd.addVerifyListener(new VerifyListener()
			{
				public void verifyText(VerifyEvent arg0)
				{
					String buf;
					int len= m_sPwd.length();
					int pos= arg0.start;
					
					buf= m_sPwd.substring(0, pos);
					buf+= arg0.text;
					if(pos+1 < len)
						buf+= m_sPwd.substring(pos + 1);
					m_sPwd= buf;
					if(arg0.text.compareTo("") != 0)
						arg0.text= "*";
				}
			});
		}
		area.addDisposeListener(new DisposeListener() {
		
			public void widgetDisposed(DisposeEvent arg0) {
				
				m_bOpen= false;
			}
		
		});
		return area;
	}

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
	 * Gets current selection of progress bar
	 * 
	 * @return selection value
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public int getSelection()
	{
		return m_oProgressBar.getSelection();
	}
	
	/**
	 * Sets selection on progressbar
	 * 
	 * @param value value witch should be set
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public void setSelection(int value)
	{
		m_oProgressBar.setSelection(value);
	}
	
	/**
	 * Gets maximum value of progressbar
	 * 
	 * @return maximum value
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public int getMaximum()
	{
		return m_oProgressBar.getMaximum();
	}
	
	/**
	 * Sets maximum value on progressbar
	 * 
	 * @param value value which should be set
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public void setMaximum(int value)
	{
		m_oProgressBar.setMaximum(value);
	}
	
	/**
	 * Gets minimum value of progress bar
	 * 
	 * @return minimum value
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public int getMinimum()
	{
		return m_oProgressBar.getMinimum();
	}
	
	/**
	 * Sets minimum value on progress bar
	 * 
	 * @param value value which should be set
	 * @serial
	 * @see
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public void setMinimum(int value)
	{
		m_oProgressBar.setMinimum(value);
	}
}
