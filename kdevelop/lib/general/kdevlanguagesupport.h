/**
 * The interface to programming language specific features
 */
#ifndef _KDEVLANGUAGESUPPORT_H_
#define _KDEVLANGUAGESUPPORT_H_

#include <qstring.h>
#include "kdevcomponent.h"


class KDevLanguageSupport : public KDevComponent
{
    Q_OBJECT
    
public:

    enum Features { NewClass, AddMethod, AddAttribute };
    
    KDevLanguageSupport( QObject *parent=0, const char *name=0 );
    ~KDevLanguageSupport();

    /**
     * Returns whether this component supports a given feature.
     * Other parts can use this information to build appropriate
     * menus.
     */
    virtual bool hasFeature(Features feature);
    /**
     * Opens a "New class" dialog and adds the configured
     * class to the sources.
     */
    virtual void newClassRequested();
    /**
     * Opens an "Add method" dialog and adds the configured
     * method to the sources.
     */
    virtual void addMethodRequested(const QString &className);
    /**
     * Opens an "Add attribute" dialog and adds the configured
     * method to the sources.
     */
    virtual void addAttributeRequested(const QString &className);

signals:
    /**
     * Emitted when the content of the classtore has been
     * modified
     */
    void updateSourceInfo();
};


#endif
