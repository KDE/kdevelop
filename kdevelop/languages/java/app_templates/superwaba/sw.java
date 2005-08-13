
/** An example that shows the new user interface gadgets for grayscale */

import waba.fx.*;
import waba.sys.*;
import waba.ui.*;

public class %{APPNAME} extends MainWindow
{
	MenuBar mbar;
	Button pushB;

public %{APPNAME}()
{
	super( "%{APPNAME}", TAB_ONLY_BORDER );

	setDoubleBuffer( true );
	// use native style?
	if ( waba.sys.Settings.platform.equals( "PalmOS" ) )
	{
		waba.sys.Settings.setPalmOSStyle( true );
	}
	// if we are a color device then we can use a nice color
	// otherwise WHITE is the most readable
	if ( !waba.sys.Settings.isColor )
	{
		Color.defaultBackColor = Color.WHITE;
		waba.ui.MainWindow.getMainWindow().setBackColor( Color.WHITE );
	}
	else
	{
		Color.defaultBackColor = new Color( 213, 210, 205 );
		waba.ui.MainWindow.getMainWindow().setBackColor( new Color( 213, 210, 205 ) );
	}
}

public void onStart()
{

	initGUI();
	Settings.appSecretKey = "installed";
}

// Called by the system to pass events to the application.
public void onEvent( Event event )
{
	if ( event.type == ControlEvent.WINDOW_CLOSED )
	{
		if ( event.target == mbar )
		{
			switch ( mbar.getSelectedMenuItem() )
			{
					case 1:
					quitApp();
					break;
					case 101:
					showAbout();
					break;
					default :
					break;
			}
		}
	}
	else if ( event.type == ControlEvent.PRESSED )
	{
		if ( event.target == pushB )
		{
			showAbout();
		}
	}
}

private void showAbout( )
{
	MessageBox mb = new MessageBox( "%{APPNAME}", "This is a small test app." );
	mb.setDoubleBuffer( true );
	popupBlockingModal( mb );
}

private void quitApp()
{
	exit( 0 );
}


private void initGUI()
{
  String col0[] = { "File","Exit..."};
  String col1[] = { "Help","About" };

  pushB = new Button( "Push me" );
  add(pushB, CENTER, CENTER);
  setMenuBar( mbar = new MenuBar( new String[][]{ col0, col1 }) );

}

}
