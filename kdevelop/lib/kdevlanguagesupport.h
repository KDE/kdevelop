/**
 * The interface to programming language specific features
 */
#ifndef _KDEVLANGUAGESUPPORT_H_
#define _KDEVLANGUAGESUPPORT_H_

#include <qstring.h>
#include "kdevcomponent.h"


class KDevLanguageSupport : public KDevComponent
{
public:

    KDevLanguageSupport( QObject *parent=0, const char *name=0 );
    ~KDevLanguageSupport();

    virtual void addMethodRequested(const QString &className);
    virtual void addAttributeRequested(const QString &className);
};


#endif
