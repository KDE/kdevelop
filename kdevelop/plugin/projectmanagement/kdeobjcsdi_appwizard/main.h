#ifndef _KDEPROJECTSPACEFACTORY_H_
#define _KDEPROJECTSPACEFACTORY_H_

#include <klibloader.h>


class KDEObjcSDIAppWizardFactory : public KLibFactory
{
    Q_OBJECT

public:
    KDEObjcSDIAppWizardFactory( QObject *parent=0, const char *name=0 );
    ~KDEObjcSDIAppWizardFactory();

    virtual QObject* create( QObject *parent, const char *name,
                             const char *classname, const QStringList &args);
};

#endif
