#include <kdebug.h>
#include <kinstance.h>
#include "dbgfactory.h"
#include "dbgmanager.h"

extern "C" {

    void *init_libkdevelopdbg()
    {
        return new DbgFactory;
    }
    
};


DbgFactory::DbgFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
    instance();
}


DbgFactory::~DbgFactory()
{
    delete s_instance;
    s_instance = 0;
}


QObject *DbgFactory::create(QObject *parent, const char *name,
                             const char *classname, const QStringList &args)
{
  QObject *obj = new DbgManager(parent, name);
  emit objectCreated(obj);
  return obj;
}


KInstance *DbgFactory::s_instance = 0;
KInstance *DbgFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevelopdbg");

    return s_instance;
}
