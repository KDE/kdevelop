/* This file is part of the KDE project
   Copyright (C) 2000-2001 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>

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
#ifndef KDEVAPI_H
#define KDEVAPI_H

#include <QObject>
#include <QtGlobal>
#include "kdevexport.h"

class QStringList;
class QDomDocument;
class KDevCore;
class KDevProject;
class KDevLanguageSupport;
class KDevBackgroundParser;
class KDevDocumentController;
class KDevMainWindow;
class KDevPlugin;
class KDevPluginController;
class KDevEnv;

/**
@file kdevapi.h
KDevelop API interface.
*/

/**
The interface to KDevelop's core components.
Needs to be implemented in a shell. Developers do not need to use this
class because @ref KDevPlugin already provides API convenience methods.
*/
class KDEVINTERFACES_EXPORT KDevApi
{
public:
    /**
     * Get an instance of KDevApi
     *
     * Gets the preexisting instance of KDevApi. If KDevApi doesn't exist, it
     * will be created. If @p parent is supplied and the object will need to be
     * created, then @p parent will be used as the object's parent. Otherwise,
     * @p parent will be ignored.
     * @param parent the parent of this object
     */
    static KDevApi* self();

    virtual ~KDevApi();

    /** Set the main window to be used by the API */
    void setMainWindow( KDevMainWindow* );

    /** @return A reference to the toplevel widget. */
    KDevMainWindow *mainWindow() const;

    /** Set the document controller to be used by the API */
    void setDocumentController( KDevDocumentController* );

    /**
     * @return A reference to the document controller which is used to
     * manipulate loaded KParts.
     */
    KDevDocumentController *documentController() const;

    /** Set the document controller to be used by the API */
    void setPluginController( KDevPluginController* );

    /**
     * @return A reference to the plugin controller which is used to
     * manipulate loaded plugin.
     */
    KDevPluginController *pluginController() const;

    /** Set the application core for the API to use */
    void setCore( KDevCore* );

    /**
     * @return A reference to the application core - an object which provides
     * basic functionalities for inter-parts communications / cooperation.
     */
    KDevCore *core() const;

    void setEnvironment( KDevEnv * );
    KDevEnv *environment() const;

    /** @return the current project component or 0 if no project is loaded. */
    KDevProject *project() const;

    /**
     * Sets the current project.
     * @param project The project plugin which becames the current project.
     */
    void setProject(KDevProject *project);

    /**
     * Get the language support currently in use.
     * @return the language support component or 0 if no support available.
     */
    KDevLanguageSupport *languageSupport() const;

    /**
     * Sets the object charged of handling the source files written in a
     * particular language (languages support component).
     * @param languageSupport The language support plugin.
     */
    void setLanguageSupport(KDevLanguageSupport *languageSupport);

    //FIXME make this a singleton in kdev-its-gonna-be-great and document it
    KDevBackgroundParser *backgroundParser() const;
    void setBackgroundParser(KDevBackgroundParser *backgroundParser);

protected:
    KDevApi(); /* private constructor. we're a singleton */
    Q_DISABLE_COPY(KDevApi) //disable copying

private:
    static KDevApi* s_self;

    class KDevApiPrivate* const d;
};

#endif
//kate: indent-mode cstyle; indent-width 4; replace-tabs on; auto-insert-doxygen on;
