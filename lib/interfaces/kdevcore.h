/***************************************************************************
 *   Copyright (C) 2001-03 by The KDevelop Authors                         *
 *                                                                         *
 *   The interface to the application core                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KDEVCORE_H_
#define _KDEVCORE_H_

#include <qstringlist.h>
#include <qobject.h>

#include <kurl.h>

class KDialogBase;
class KDevPlugin;
class KAction;

namespace KParts
{
    class Part;
}

class QStatusBar;
class QPopupMenu;

/**
 * Base class for every context. Think of a Context-based class as "useful
 * info associated to a context menu". Several context menu can be defined,
 * each defining different information: because of these context menus being
 * used in many modules, they are defined here.
 */
class Context
{
public:
    /**
    * Builds a context of the specified @p type.
    */
    Context( const QCString &type );
    /**
    * Destructor
    */
    virtual ~Context();

    /**
    * Returns the type of this Context, so clients can discriminate
    * between different file contexts.
    */
    bool hasType( const QCString &type ) const;

private:
    class Private;
    Private *d;
};

/**
 * A context for the popup menu in the editor.
 */
class EditorContext : public Context
{
public:
    /**
    * Builds a context for an editor part.
    * @param url
    * @param line the line number where the cursor is
    * @param col the column number where the cursor is
    * @param linestr the content of the line where the cursor is
    * @param wordstr
    */
    EditorContext( const KURL &url, int line, int col,
        const QString &linestr, const QString &wordstr );
    /**
    * Destructor
    */
    virtual ~EditorContext();

    /**
    * Returns the url for the file which this context was invoked for.
    */
    const KURL &url() const;

    /**
    * Returns the line number for the cursor position.
    */
    int line() const;

    /**
    * Returns the column number for the cursor position.
    */
    int col() const;

    /**
    * Returns a QString with the content of the line which this context was
    * invoked for.
    */
    QString currentLine() const;

    /**
    * Returns a QString containing the word near to the cursor when this
    * context object was created.
    */
    QString currentWord() const;

private:
    class Private;
    Private *d;
};


/**
 * A context for the popup menu in the html widget.
 */
class DocumentationContext : public Context
{
public:
    /**
    * Builds context.
    * @param url
    * @param selection selected text
    */
    DocumentationContext( const QString &url, const QString &selection );

    /**
    * Destructor
    */
    virtual ~DocumentationContext();

    /**
    * Returns the url of the document this context was invoked for.
    */
    QString url() const;

    /**
    * Returns the selected text in the document.
    */
    QString selection() const;

private:
    class Private;
    Private *d;
};

/**
 * A context for the popup menu in file views (as the FileTreeView or the
 * AutoMake Managers' file view).
 * It can works in two modes:
 * - <b>compatibility mode</b>: for old modules still relaying on single
 *   file selection
 * - <b>standard mode</b>: which should be used by new modules and which
 *   consists in adding a bunch of KURLs to the context
 */
class FileContext : public Context
{
public:
    /**
    * <b>Compatibility mode</b> Build context using a single filename
    * @param fileName absolute pathname of the file to be used for this file
    *        context
    * @param isDirectory
    */
    FileContext( const QString &fileName, bool isDirectory );
    /**
    * Builds the file context using a @see KURL::List
    * @param someURLs
    */
    FileContext( const KURL::List &someURLs );
    /**
    * Destructor
    */
    ~FileContext();

    /**
    * <b>Compatibility mode</b>
    * Returns the absolute name of the selected pathname (file or directory).
    * If the context has been built using the standard costructor than this
    * method will refer to the first path of the embedded KURL list.
    */
    QString fileName() const;
    /**
    * <b>Compatibility mode</b>
    * Returns true if currently stored file is a directory
    * If the context has been built using the standard costructor than this
    * method will refer to the first path of the embedded KURL list.
    */
    bool isDirectory() const;

    /*
    * Returns a reference to the bunch of URLs
    */
    const KURL::List &urls() const;

private:
    class Private;
    Private *d;
};

/**
 * A context for the popup menu in class views.
 */
class ClassContext : public Context
{
public:
    /**
    * Builds the context.
    * @param classname
    */
    ClassContext( const QString &classname );

    /**
    * Destructor.
    */
    virtual ~ClassContext();

    /**
    * Returs the classname for the selected item. including its scope
    * (i.e. namespace).
    * (currently not used)
    */
    QString classname() const;

private:
    class Private;
    Private *d;
};

/**
* A KDevCore class defines an object which is used to realized the cooperation
* between the various plug-in which compose KDevelop: it defines virtual
* function, signals that can be captured for menu customization, GUI utility
* functions for adding new KActions, ...
*/
class KDevCore : public QObject
{
    Q_OBJECT

public:
    /**
     * An enumeration that specifies the behaviour of the
     * methods gotoSourceFile() and gotoDocumentationFile().
     */
    enum Embedding { Replace, SplitHorizontal, SplitVertical };

    /**
    * Constructor
    * @param parent
    * @param name
    */
    KDevCore( QObject *parent=0, const char *name=0 );

    /**
    * Destructor
    */
    virtual ~KDevCore();

    /**
    * @param action
    */
    virtual void insertNewAction( KAction* action ) = 0;

    /**
     * This method should be called by a part that wants to show a
     * context menu. The parameter @p context should be filled with
     * information about the context in which this happens (see
     * EditorContext, DocumentationContext, ClassContext, ...).
     * Essentially, this method emits the signal contextMenu()
     * which other parts can use to hook in.
     */
    virtual void fillContextMenu(QPopupMenu *popup, const Context *context) = 0;

    /**
       close the current project and opens the new one
     */
    virtual void openProject( const QString& projectFileName ) = 0;

    /**
     * Marks the component as running (or not running). As long as at least one
     * component is running, the stop button is enabled. When it is pressed,
     * all components get a stopButtonClicked().
     */
    virtual void running( KDevPlugin *which, bool runs ) = 0;

signals:

    /**
     * Emitted after the core has done all initializations and
     * the main window has been shown.
     */
    void coreInitialized();
    /**
     * A project has been opened
     */
    void projectOpened();
    /**
     * The project is about to be closed.
     */
    void projectClosed();

    /**
     * The user has clicked the stop button.
     * @param which is 0 if all actions should be cancelled.
     */
    void stopButtonClicked( KDevPlugin *which );
    /**
     * A context menu has been requested somewhere. Components
     * may hook some entries into it. More information on the
     * context can be obtained by looking for the type of
     * @p context and casting it accordingly.
     *
     */
    void contextMenu( QPopupMenu *popupMenu, const Context *context );
    /**
     * Expects that a configuration page for use in the
     * KDevelop settings dialog is created by the component.
     * @param dlg the dialog which the configuration widget should be added to
     */
    void configWidget( KDialogBase *dlg );
    /**
     * Expects that a configuration page for use in the
     * Project settings dialog is created by the component.
     * @param dlg the dialog which the configuration widget should be added to
     */
    void projectConfigWidget( KDialogBase *dlg );
};

#endif
