#include <kdebug.h>
#include <kinstance.h>
#include "main.h"
#include "grepview.h"


extern "C" {

    void *init_libkdevgrepview()
    {
        return new GrepFactory;
    }
    
};


GrepFactory::GrepFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
    instance();
}


GrepFactory::~GrepFactory()
{
    delete s_instance;
    s_instance = 0;
}


QObject *GrepFactory::create(QObject *parent, const char *name,
                             const char *classname, const QStringList &args)
{
    if (parent && !parent->isWidgetType()) {
        kdDebug(9001) << "Parent of print dialog is not a widget" << endl;
        return 0;
    }
    QWidget *parentWidget = (QWidget *) parent;
    
    // Depending on classname, this should method should also
    // be able to create the config dialog

    kdDebug(9001) << "Building GrepView" << endl;
    
    QObject *obj = new GrepView(parentWidget, name);
    emit objectCreated(obj);
    return obj;
}


KInstance *GrepFactory::s_instance = 0;
KInstance *GrepFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevgrepview");

    return s_instance;
}
