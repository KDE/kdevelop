/* This file is part of KDevelop
Copyright 2001-2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
Copyright 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
Copyright 2001 Sandy Meier <smeier@kdevelop.org>
Copyright 2002 Daniel Engelschalt <daniel.engelschalt@gmx.net>
Copyright 2002 Simon Hausmann <hausmann@kde.org>
Copyright 2002-2003 Roberto Raggi <roberto@kdevelop.org>
Copyright 2003 Mario Scalas <mario.scalas@libero.it>
Copyright 2003 Harald Fernengel <harry@kdevelop.org>
Copyright 2003,2006 Hamish Rodda <rodda@kde.org>
Copyright 2004 Alexander Dymo <adymo@kdevelop.org>
Copyright 2006 Adam Treat <treat@kde.org>
Copyright 2007 Andreas Pakulat <apaku@gmx.org>

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
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_CONTEXT_H
#define KDEVPLATFORM_CONTEXT_H

#include "interfacesexport.h"

#include <QStringList>
#include <QMimeType>

#include <KUrl>

template <typename T> class QList;

namespace KDevelop
{
class ProjectBaseItem;

/**
Base class for every context.
Think of a Context-based class as "useful information associated with a context menu".

When a context menu with a certain "context" associated appears, the platform's
PluginController requests all plugins to return a list of QActions* they want to add
to the context menu and a QString that should be used as the submenu entry.
For example, a SVN plugin could add "commit" and "update" actions to the context
menu of a document in a submenu called "Subversion".

The plugin that originally gets the contextmenu event shouldn't add its own
actions directly to the menu but instead use the same mechanism.

<b>How to show a context menu from a plugin:</b>
-# Create a QMenu in context menu event handler: @code QMenu menu(this); @endcode
-# Create a context: @code FileContext context(list). @endcode
-# Query for plugins:
@code @code QList<ContextMenuExtension> extensions =
        ICore::self()->pluginController()->queryPluginsForContextMenuExtensions( context ); @endcode
-# Populate the menu:
@code ContextMenuExtension::populateMenu(menu, extensions); @endcode
-# Show the popup menu: @code menu.exec(mapToGlobal(pos)); @endcode

<b>How to fill a context menu from a plugin:</b>
-# Implement the @code contextMenuExtension(Context*) @endcode
   function in your plugin class.
-# Depending on the context fill the returned ContextMenuExtension with actions:\n
@code
ContextMenuExtension ext;
if (context->hasType(Context::EditorContext))
{
    ext.addAction(ContextMenuExtension::EditorGroup, new QAction(...));
}
else if context->hasType(Context::FileContext))
{
    ext.addAction(ContextMenuExtension::FileGroup, new QAction(...));
    ...
}
return ext;
@endcode
 */
class KDEVPLATFORMINTERFACES_EXPORT Context
{
public:
    /**Pre-defined context types. More may be added so it is possible to add custom
        contexts. <strong>We reserve enum values until 1000 (yeah, it is one thousand )
        for kdevplatform official context types.</strong>*/
    enum Type
    {
        FileContext,                 /**<File menu.*/
        CodeContext,                 /**<Code context menu(DeclarationContext or DUContextContext)*/
        EditorContext,               /**<Editor menu.*/
        ProjectItemContext,          /**<ProjectItem context menu.*/
        OpenWithContext              /**<Open With context menu.*/
    };

    /**Implement this in the context so we can provide rtti.*/
    virtual int type() const = 0;

    /**@return The type of this Context, so clients can discriminate
        between different file contexts.*/
    bool hasType( int type ) const;

protected:
    /**Constructor.*/
    Context();

    /**Destructor.*/
    virtual ~Context();
private:
    class ContextPrivate* const d;
};

/**
A context for the a list of selected urls.
 */
class KDEVPLATFORMINTERFACES_EXPORT FileContext : public Context
{
public:
    /**Builds the file context using a @ref KUrl::List
        @param urls The list of selected url.*/
    FileContext( const QList<QUrl> &urls );

    /**Destructor.*/
    virtual ~FileContext();

    virtual int type() const;

    /**@return A reference to the selected URLs.*/
    QList<QUrl> urls() const;

private:
    class FileContextPrivate* const d;

    FileContext( const FileContext & );
    FileContext &operator=( const FileContext & );
};

/**
A context for ProjectItem's.
 */
class KDEVPLATFORMINTERFACES_EXPORT ProjectItemContext : public Context
{
public:
    /**Builds the context.
        @param items The items to build the context from.*/
    ProjectItemContext( const QList<ProjectBaseItem*> &items );

    /**Destructor.*/
    virtual ~ProjectItemContext();

    virtual int type() const;

    /**@return The project model item for the selected item.*/
    QList<ProjectBaseItem*> items() const;

private:
    class ProjectItemContextPrivate* const d;

    ProjectItemContext( const ProjectItemContext & );
    ProjectItemContext &operator=( const ProjectItemContext & );
};

/**
 * Context menu to open files with custom applications.
 */
class KDEVPLATFORMINTERFACES_EXPORT OpenWithContext : public Context
{
public:
    /**
     * @p url The files to open.
     * @p mimeType The mime type of said file.
     */
    OpenWithContext(const KUrl::List& urls, const QMimeType& mimeType);

    /**
     * @return Context::OpenWithContext
     */
    virtual int type() const;

    /**
     * @return The files to open.
     */
    KUrl::List urls() const;

    /**
     * @return The mimetype of the url to open.
     */
    QMimeType mimeType() const;

private:
    class OpenWithContextPrivate* const d;
    Q_DISABLE_COPY(OpenWithContext)
};

}
#endif

