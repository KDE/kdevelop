
import org.kde.qt.*;
import org.kde.koala.*;
import java.util.*;
import java.lang.Error;

public class $APPNAME$Factory extends KDevFactory  {

	private $APPNAME$Factory(Class dummy) {
		super((Class) null);
	}

	private static KInstance s_instance = null;

	public static KInstance instance()
	{
	  if (s_instance == null)
	    s_instance = new KInstance("kdev$APPNAMELC$");

	  return s_instance;
	}

}
