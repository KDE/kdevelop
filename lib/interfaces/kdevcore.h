/* This file is part of the KDE project
   Copyright (C) 2001-2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2001 Sandy Meier <smeier@kdevelop.org>
   Copyright (C) 2002 Daniel Engelschalt <daniel.engelschalt@gmx.net>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>
   Copyright (C) 2003 Harald Fernengel <harry@kdevelop.org>
   Copyright (C) 2003 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

/*   The interface to the application core                                 */

#ifndef _KDEVCORE_H_
#define _KDEVCORE_H_

#include <qstringlist.h>
#include <qdict.h>
#include <qobject.h>

#include <kurl.h>

class KDialogBase;
class KDevPlugin;
class KDevLicense;
class KAction;
class CodeModelItem;
class ProjectModelItem;

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
    * Pre-defined context. More may be added so it is possible to add custom
    * contexts.
    * <strong>We reserve enum values until 1000 (yeah, it is one thousand )
    * for kdevelop official context types.</strong>
    */
    enum 
    { 
	EditorContext, 
	DocumentationContext, 
	FileContext,
        ProjectModelItemContext,
	CodeModelItemContext
    };

    /**
    * Implements this in the context so we can provide rtti
    */
    virtual int type() const = 0;

    /**
    * Returns the type of this Context, so clients can discriminate
    * between different file contexts.
    */
    bool hasType( int type ) const;

protected:
    /**
    * Constructor
    */
    Context();

    /**
    * Destructor
    */
    virtual ~Context();
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

    virtual int type() const;

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

    EditorContext( const EditorContext &);
    EditorContext &operator=( const EditorContext &);
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
    * Copy constructor
    */
    DocumentationContext( const DocumentationContext &);
    DocumentationContext &operator=( const DocumentationContext &);

    /**
    * Destructor
    */
    virtual ~DocumentationContext();

    virtual int type() const;

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
    virtual ~FileContext();

    virtual int type() const;

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

    FileContext( const FileContext &);
    FileContext &operator=( const FileContext &);
};

/**
 * A context for the popup menu in class views.
 */
class CodeModelItemContext : public Context
{
public:
    /**
    * Builds the context.
    * @param item
    */
    CodeModelItemContext( const CodeModelItem* item );

    /**
    * Destructor.
    */
    virtual ~CodeModelItemContext();

    virtual int type() const;

    /**
    * Returs the code model item for the selected item.
    */
    const CodeModelItem* item() const;

private:
    class Private;
    Private *d;

    CodeModelItemContext( const CodeModelItemContext &);
    CodeModelItemContext &operator=( const CodeModelItemContext &);
};

/**
 * A context for the popup menu in project views.
 */
class ProjectModelItemContext : public Context
{
public:
    /**
    * Builds the context.
    * @param item
    */
    ProjectModelItemContext( const ProjectModelItem* item );

    /**
    * Destructor.
    */
    virtual ~ProjectModelItemContext();

    virtual int type() const;

    /**
    * Returs the code model item for the selected item.
    */
    const ProjectModelItem* item() const;

private:
    class Private;
    Private *d;

    ProjectModelItemContext( const ProjectModelItemContext &);
    ProjectModelItemContext &operator=( const ProjectModelItemContext &);
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

    /**
     * Returns a list of all available licenses.
     */
    virtual QDict< KDevLicense > licenses() = 0;
    
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
     * The language support part has been changed.
     */
    void languageChanged();

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
