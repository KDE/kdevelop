#include <kdebug.h>
#include <kinstance.h>


#include "$APPNAMELC$_factory.h"
#include "$APPNAMELC$_part.h"


extern "C" 
{

  void *init_libkdev$APPNAMELC$()
    {
      return new $APPNAME$Factory;
    }
    
};


$APPNAME$Factory::$APPNAME$Factory(QObject *parent, const char *name)
  : KDevFactory(parent, name)
{
}


$APPNAME$Factory::~$APPNAME$Factory()
{
  delete s_instance;
  s_instance = 0;
}


KDevPart *$APPNAME$Factory::createPartObject(KDevApi *api, QObject *parent, 
											   const QStringList &/*args*/)
{
  return new $APPNAME$Part(api, parent, "$APPNAMELC$");
}


KInstance *$APPNAME$Factory::s_instance = 0;

KInstance *$APPNAME$Factory::instance()
{
  if (!s_instance)
    s_instance = new KInstance("kdev$APPNAMELC$");

  return s_instance;
}


#include "$APPNAMELC$_factory.moc"
