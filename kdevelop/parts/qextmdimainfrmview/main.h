#ifndef _MDIMAINFRMFACTORY_H_
#define _MDIMAINFRMFACTORY_H_

#include <klibloader.h>


class MdiMainFrmFactory : public KLibFactory
{
    Q_OBJECT

public:
    MdiMainFrmFactory( QObject *parent=0, const char *name=0 );
    ~MdiMainFrmFactory();

    virtual QObject* create( QObject *parent, const char *name,
                             const char *classname, const QStringList &args);
    static KInstance *instance();

private:
    static KInstance *s_instance;
};

#endif
