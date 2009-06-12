/**
 * 
 */
package at.kolli.layout;

import org.eclipse.jface.action.Action;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.swt.widgets.Shell;

/**
 * @author kollia
 *
 */
public class ExitAction extends Action 
{
	Shell shell;
	
	public ExitAction(Shell shell)
	{
		this.shell= shell;
	    setText("E&xit@Ctrl+W");
	    setToolTipText("Exit the application");
	}
	
	public void run()
	{
		shell.close();
	}
}
