#include <kdebug.h>
#include <kinstance.h>
#include "main.h"
#include "outputviews.h"


extern "C" {

    void *init_libkdevoutputviews()
    {
        return new OutputFactory;
    }
    
};


OutputFactory::OutputFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
    instance();
}


OutputFactory::~OutputFactory()
{
    delete s_instance;
    s_instance = 0;
}


QObject *OutputFactory::create(QObject *parent, const char *name,
                             const char *classname, const QStringList &args)
{
    if (parent && !parent->isWidgetType()) {
        kdDebug(9004) << "Parent of output view is not a widget" << endl;
        return 0;
    }
    QWidget *parentWidget = (QWidget *) parent;
    
    // Depending on classname, this should method should also
    // be able to create the config dialog

    QObject *obj;
    if (args.count() > 0 && qstrcmp(args[0].latin1(), "AppOutputView") == 0) {
        kdDebug(9004) << "Building MakeView" << endl;
        obj = new AppOutputView(parentWidget, name);
    } else if (args.count() > 0 && qstrcmp(args[0].latin1(), "MakeView") == 0) {
        kdDebug(9004) << "Building AppOutputView" << endl;
        obj = new MakeView(parentWidget, name);
    } else {
        kdDebug(9004) << "Wrong args for kdevoutputviews library" << endl;
        if (args.count() > 0)
            kdDebug(9004) << args[0] << endl;
        return 0;
    }

    emit objectCreated(obj);
    return obj;
}


KInstance *OutputFactory::s_instance = 0;
KInstance *OutputFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevoutputviews");

    return s_instance;
}
