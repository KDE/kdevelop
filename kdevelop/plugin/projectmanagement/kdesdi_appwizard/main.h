#ifndef _KDEPROJECTSPACEFACTORY_H_
#define _KDEPROJECTSPACEFACTORY_H_

#include <klibloader.h>


class KDESDIAppWizardFactory : public KLibFactory
{
    Q_OBJECT

public:
    KDESDIAppWizardFactory( QObject *parent=0, const char *name=0 );
    ~KDESDIAppWizardFactory();

    virtual QObject* create( QObject *parent, const char *name,
                             const char *classname, const QStringList &args);
};

#endif
