package at.kolli.dialogs;

import org.eclipse.swt.widgets.Display;

import at.kolli.layout.HtmTags;

/**
 * adapter pattern for synchronize SWT syncExec and asyncExec
 * for debugging messages
 * 
 * @author Alexander Kolli
 * @version 1.0
 */
public class DisplayAdapter {

	
	public static void syncExec(Runnable runnable)
	{
		syncExec(runnable, "");
	}
	public static void syncExec(Runnable runnable, String def)
	{
		/*if(HtmTags.debug)
		{
			System.out.println("synchronize display for SWT by " + runnable);
			if(!def.equals(""))
				System.out.println("   by defined position of " + def);
		}*/
		Display.getDefault().syncExec(runnable);
		/*if(HtmTags.debug)
		{
			System.out.println("has synchronized by " + runnable);
			if(!def.equals(""))
				System.out.println("   by defined position of " + def);
		}*/
	}
	public static void asyncExec(Runnable runnable)
	{
		syncExec(runnable, "");
	}
	public static void asyncExec(Runnable runnable, String def)
	{
		/*if(HtmTags.debug)
		{
			System.out.println("set asynchron display for SWT by " + runnable);
			if(!def.equals(""))
				System.out.println("   by defined position of " + def);
		}*/
		Display.getDefault().syncExec(runnable);
		/*if(HtmTags.debug)
		{
			System.out.println("has synchronized by " + runnable);
			if(!def.equals(""))
				System.out.println("   by defined position of " + def);
		}*/
	}
}
