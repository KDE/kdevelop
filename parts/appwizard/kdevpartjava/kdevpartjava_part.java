/*
 *  Copyright (C) $YEAR$ $AUTHOR$ <$EMAIL$>   
 */
 
import org.kde.qt.*;
import org.kde.koala.*;

class $APPNAME$Part extends KDevPart
{


  private $APPNAME$Widget m_widget;

public $APPNAME$Part(KDevApi api, QObject parent, String name)
{
  super(api, parent, name);
  m_widget = new $APPNAME$Widget(this);
  
  QWhatsThis.add(m_widget, "WHAT DOES THIS PART DO?");
  
  core().embedWidget(m_widget, KDevCore.OutputView, "$APPNAME$");
}

}

