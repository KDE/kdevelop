#ifndef _JAVAPROJECTSPACEFACTORY_H_
#define _JAVAPROJECTSPACEFACTORY_H_

#include <klibloader.h>


class JavaProjectSpaceFactory : public KLibFactory
{
    Q_OBJECT

public:
    JavaProjectSpaceFactory( QObject *parent=0, const char *name=0 );
    ~JavaProjectSpaceFactory();

    virtual QObject* create( QObject *parent, const char *name,
                             const char *classname, const QStringList &args);
};

#endif
