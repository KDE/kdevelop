#include "main.h"
#include "cprintdlg.h"


extern "C" {

    void *init_libkdevprintplugin()
    {
        return new PrintFactory;
    }
    
};


PrintFactory::PrintFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
}


PrintFactory::~PrintFactory()
{}


QObject *PrintFactory::create(QObject *parent, const char *name,
                              const char *classname, const QStringList &args)
{
    QString filename;
    if (!args.isEmpty())
        filename = *args.begin();

    if (!parent->isWidgetType()) {
        qDebug("Parent of print dialog is not a widget");
        return 0;
    }
    QWidget *parentWidget = (QWidget *) parent;
    
    // Depending on classname, this should method should also
    // be able to create the config dialog
    
    QObject *obj = new CPrintDlg(parentWidget, filename, name, false);
    emit objectCreated(obj);
    return obj;
}
