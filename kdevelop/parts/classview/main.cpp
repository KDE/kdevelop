#include <kdebug.h>
#include <kinstance.h>
#include "main.h"
#include "classview.h"


extern "C" {

    void *init_libkdevclassview()
    {
        return new ClassFactory;
    }
    
};


ClassFactory::ClassFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
    instance();
}


ClassFactory::~ClassFactory()
{
    delete s_instance;
    s_instance = 0;
}


QObject *ClassFactory::create(QObject *parent, const char *name,
                             const char *classname, const QStringList &args)
{
    if (parent && !parent->isWidgetType()) {
        kdDebug(9001) << "Parent of class view is not a widget" << endl;
        return 0;
    }
    QWidget *parentWidget = (QWidget *) parent;
    
    // Depending on classname, this should method should also
    // be able to create the config dialog

    kdDebug(9001) << "Building ClassView" << endl;
    
    QObject *obj = new ClassView(parentWidget, name);
    emit objectCreated(obj);
    return obj;
}


KInstance *ClassFactory::s_instance = 0;
KInstance *ClassFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevclassview");

    return s_instance;
}
