/**
 * The interface to programming language specific features
 */

#ifndef _KDEVLANGUAGESUPPORT_H_
#define _KDEVLANGUAGESUPPORT_H_

#include <qstring.h>
#include <qstringlist.h>
#include "kdevplugin.h"


class KDevLanguageSupport : public KDevPlugin
{
    Q_OBJECT

public:

    enum Features {
        Classes=1, Structs=2, Functions=4, Variables=8,
        Namespaces=16, Signals=32, Slots=64, Declarations=128,   /* features of the language itself       */
        NewClass=512, AddMethod=1024, AddAttribute=2048          /* features of the language support part */
    };

    KDevLanguageSupport( const QString& pluginName, const QString& icon, QObject *parent, const char *name );
    ~KDevLanguageSupport();

    /**
     * Returns the feature set of the language. This is e.g. used
     * by the class view to decide which organizer items to display
     * and which not.
     */
    virtual Features features();
    /**
     * Returns a typical filter list for the support language
     * should be configurable in the languagesupport dialog
     * example "*.cpp,*.h,*.c";
     * used in grepview?, adding files
     */
    virtual QStringList fileFilters();

    /**
     * Formats a canonicalized class path as used by the class store
     * to the human-readable convention. For example, the C++ support
     * part formats the string "KParts.Part" into "KParts::Part".
     */
    virtual QString formatClassName(const QString &name);
    /**
     * The opposite of formatClassName().
     */
    virtual QString unformatClassName(const QString &name);
    
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
     * Opens an "Implement Virtual Methods" dialog and adds the
     * configured methods to the sources.
     */
    virtual void implementVirtualMethods(const QString &className);
    /**
     * Opens an "Add attribute" dialog and adds the configured
     * method to the sources.
     */
    virtual void addAttribute(const QString &className);
    /**
     * Opens an "Subclass Widget" dialog for given Qt .ui file (formName)
     * and propmts to implement it's slots.
     * Returns a list of newly created files.
    */
    virtual QStringList subclassWidget(const QString& formName);
    /**
     * Opens and "Update Widget" dialog for given Qt .ui file (formName)
     * and prompts to add missing slot implementations
     * in the subclass located in fileName.
     * Returns a list of newly created files.
    */
    virtual QStringList updateWidget(const QString& formName, const QString& fileName);


signals:
    /**
     * Emitted when the content of the classtore has been
     * modified
     */
    void updatedSourceInfo();
};


#endif
