/**
 * The interface to programming language specific features
 */

#ifndef _KDEVLANGUAGESUPPORT_H_
#define _KDEVLANGUAGESUPPORT_H_

#include <qstring.h>
#include <qstringlist.h>
#include <kmimetype.h>
#include "kdevplugin.h"

class Tag;
class CodeModelItem;

class KDevLanguageSupport : public KDevPlugin
{
    Q_OBJECT

public:

    enum Features {
        Classes=1, Structs=2, Functions=4, Variables=8,
        Namespaces=16, Signals=32, Slots=64, Declarations=128,   /* features of the language itself       */
        NewClass=512, AddMethod=1024, AddAttribute=2048,         /* features of the language support part */
        Scripts=4096, NewScript=8192                             /* features for scripting support (perl) */
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
     * Returns a typical mimetype list for the support language
     * should be configurable in the languagesupport dialog.
     */
    virtual KMimeType::List mimeTypes();

    /**
     * Formats a Tag as used by the class store to the human-readable convention.
     */
    virtual QString formatTag( const Tag& tag );
    /**
     * Formats a CodeModelItem as used by the CodeModel to the human-readable convention.
     */
    virtual QString formatModelItem( const CodeModelItem *item );

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
     * @todo Use KDevNode stuff for this
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

    /**
     * Emitted before remove the file from the classstore
     */
    void aboutToRemoveSourceInfo( const QString& fileName );

    /**
     * Emitted when a file has been removed from the classstore
     */
    void removedSourceInfo( const QString& fileName );

    /**
     * Emitted when a file has been added Emitted when a file has been removed from the classstorefrom the classstore
     */
    void addedSourceInfo( const QString& fileName );
};


#endif
