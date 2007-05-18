/* This file is part of KDevelop
Copyright (C) 2001-2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
Copyright (C) 2001 Sandy Meier <smeier@kdevelop.org>
Copyright (C) 2002 Daniel Engelschalt <daniel.engelschalt@gmx.net>
Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>
Copyright (C) 2003 Harald Fernengel <harry@kdevelop.org>
Copyright (C) 2003,2006 Hamish Rodda <rodda@kde.org>
Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>
Copyright (C) 2006 Adam Treat <treat@kde.org>
Copyright (C) 2007 Andreas Pakulat <apaku@gmx.org>

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

#ifndef CONTEXT_H
#define CONTEXT_H

#include "kdevexport.h"

#include <QStringList>

#include <kurl.h>
#include <ktexteditor/cursor.h>

namespace KDevelop
{
// class CodeItem;
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
-# Create a KMenu in context menu event handler: @code KMenu menu(this); @endcode
-# Create a context: @code FileContext context(list). @endcode
-# Ask PluginController to fill the menu:
@code core()->pluginController()->buildContextMenu(&menu, context); @endcode
-# Show the popup menu: @code menu.exec(mapToGlobal(pos)); @endcode

<b>How to fill a context menu from a plugin:</b>
-# Implement the @code requestContextMenuActions(const Context &) @endcode
   function in your plugin class.
-# Depending on the context return a pair of a submenu title and a list of
   actions that should appear in the menu:\n
@code
QList<QAction*> actionlist;
if (context->hasType(Context::EditorContext))
{
    actionlist << new QAction(...);
}
else if context->hasType(Context::FileContext))
{
    actionlist << new QAction(...);
    ...
}
return qMakePair("Subversion", actionlist);
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
        EditorContext,               /**<Editor menu.*/
        FileContext,                 /**<File menu.*/
        CodeItemContext,             /**<CodeItem context menu.*/
        ProjectItemContext           /**<ProjectItem context menu.*/
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

/**A context for the KTextEditor.*/
class KDEVPLATFORMINTERFACES_EXPORT EditorContext: public Context
{
public:
    /**Builds a context for a KTextEditor part.
        @param url The url of a file in the editor.
        @param position The position where the cursor is.
        @param linestr The content of the line where the cursor is.
        @param wordstr The current word under the cursor.*/
    EditorContext( const KUrl &url, const KTextEditor::Cursor& position,
                   const QString &linestr, const QString &wordstr );

    /**Destructor.*/
    virtual ~EditorContext();

    virtual int type() const;

    /**@return The url for the file which this context was invoked for.*/
    KUrl url() const;

    /**@return The cursor position.*/
    KTextEditor::Cursor position() const;

    /**@return A QString with the content of the line which this context was
        invoked for.*/
    QString currentLine() const;

    /**@return A QString containing the word near to the cursor when this
        context object was created.*/
    QString currentWord() const;

private:
    class EditorContextPrivate* const d;

    EditorContext( const EditorContext & );
    EditorContext &operator=( const EditorContext & );
};

/**
A context for the a list of selected urls.
 */
class KDEVPLATFORMINTERFACES_EXPORT FileContext : public Context
{
public:
    /**Builds the file context using a @ref KUrl::List
        @param urls The list of selected url.*/
    FileContext( const KUrl::List &urls );

    /**Destructor.*/
    virtual ~FileContext();

    virtual int type() const;

    /**@return A reference to the selected URLs.*/
    KUrl::List urls() const;

private:
    class FileContextPrivate* const d;

    FileContext( const FileContext & );
    FileContext &operator=( const FileContext & );
};

/**
A context for CodeItem's.
 */
// Disabled until we have a code-model or duchain-model
//  class KDEVPLATFORMINTERFACES_EXPORT CodeItemContext: public Context
// {
// public:
//     /**Builds the context.
//     @param item The item to build the context from.*/
//     CodeItemContext( const CodeItem* item );
//
//     /**Destructor.*/
//     virtual ~CodeItemContext();
//
//     virtual int type() const;
//
//     /**@return The code model item for the selected item.*/
//     const CodeItem* item() const;
//
// private:
//     class Private;
//     Private *d;
//
//     CodeItemContext( const CodeItemContext & );
//     CodeItemContext &operator=( const CodeItemContext & );
// };

/**
A context for ProjectItem's.
 */
class KDEVPLATFORMINTERFACES_EXPORT ProjectItemContext : public Context
{
public:
    /**Builds the context.
        @param item The item to build the context from.*/
    ProjectItemContext( ProjectBaseItem* item );

    /**Destructor.*/
    virtual ~ProjectItemContext();

    virtual int type() const;

    /**@return The project model item for the selected item.*/
    ProjectBaseItem* item() const;

private:
    class ProjectItemContextPrivate* const d;

    ProjectItemContext( const ProjectItemContext & );
    ProjectItemContext &operator=( const ProjectItemContext & );
};

}
#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
