/**
 * The interface to programming language specific features
 */

#ifndef _KDEVLANGUAGESUPPORT_H_
#define _KDEVLANGUAGESUPPORT_H_

#include <qstring.h>
#include <qstringlist.h>
#include "kdevpart.h"


class KDevLanguageSupport : public KDevPart
{
    Q_OBJECT
    
public:

    enum Features {
        NewClass, AddMethod, AddAttribute, /* features of the language support part */
        Signals, Slots, Namespaces         /* features of the language itself       */
    };
    
    KDevLanguageSupport( KDevApi *api, QObject *parent=0, const char *name=0 );
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
     * TODO: Use KDevNode stuff for this
     */
    virtual void addClass();
    /**
     * Opens an "Add method" dialog and adds the configured
     * method to the sources.
     */
    virtual void addMethod(const QString &className);
    /**
     * Opens an "Add attribute" dialog and adds the configured
     * method to the sources.
     */
    virtual void addAttribute(const QString &className);

    /** 
     * returns a typical filterlist  for the support language
     * should be configurable in the languagesupport dialog
     * example "*.cpp;*.h;*.c";
     * used in grepview, adding files
     */
    virtual QStringList fileFilters();

signals:
    /**
     * Emitted when the content of the classtore has been
     * modified
     */
    void updatedSourceInfo();
};


#endif
