#ifndef _KDEPROJECTSPACEFACTORY_H_
#define _KDEPROJECTSPACEFACTORY_H_

#include <klibloader.h>


class KDEJavaSDIAppWizardFactory : public KLibFactory
{
    Q_OBJECT

public:
    KDEJavaSDIAppWizardFactory( QObject *parent=0, const char *name=0 );
    ~KDEJavaSDIAppWizardFactory();

    virtual QObject* create( QObject *parent, const char *name,
                             const char *classname, const QStringList &args);
};

#endif
