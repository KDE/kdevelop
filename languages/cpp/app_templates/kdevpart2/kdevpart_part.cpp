%{CPP_TEMPLATE}

#include <qwhatsthis.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>

#include <kdevelop/kdevcore.h>

#include "%{APPNAMELC}_widget.h"
#include "%{APPNAMELC}_part.h"

typedef KGenericFactory<%{APPNAME}Part> %{APPNAME}Factory;
K_EXPORT_COMPONENT_FACTORY( libkdev%{APPNAMELC}, %{APPNAME}Factory( "kdev%{APPNAMELC}" ) );

%{APPNAME}Part::%{APPNAME}Part(QObject *parent, const char *name, const QStringList& )
  : KDevPlugin("%{APPNAME}", "%{APPNAMELC}", parent, name ? name : "%{APPNAME}Part" )
{
  setInstance(%{APPNAME}Factory::instance());
  setXMLFile("kdevpart_%{APPNAMELC}.rc");

  m_widget = new %{APPNAME}Widget(this);

  QWhatsThis::add(m_widget, i18n("WHAT DOES THIS PART DO?"));

  // now you decide what should happen to the widget. Take a look at kdevcore.h
  // or at other plugins how to embed it.

  // if you want to embed your widget as an outputview, simply uncomment
  // the following line.

  // mainWindow()->embedOutputView( m_widget, "name that should appear", "enter a tooltip" );

}


%{APPNAME}Part::~%{APPNAME}Part()
{
// if you embed a widget, you need to tell the mainwindow when you remove it
//  if ( m_widget )
//  {
//    mainWindow()->removeView( m_widget );
//  }
  delete m_widget;
}


#include "%{APPNAMELC}_part.moc"
