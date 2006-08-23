/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2002 F@lk Brettschneider <falkbr@kdevelop.org>
   Copyright (C) 2003 Amilcar do Carmo Lucas <amilcar@ida.ing.tu-bs.de>
   Copyright (C) 2003-2004 Alexander Dymo <adymo@kdevelop.org>
   Copyright (C) 2006 Adam Treat <treat@kde.org>

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
#ifndef KDEVLANGUAGESUPPORT_H
#define KDEVLANGUAGESUPPORT_H

#include "kdevplugin.h"

#include <iostream> //Needed to serialize the AST
#include <fstream> //Needed to serialize the AST

#include <QString>
#include <qstringlist.h>

#include <kurl.h>
#include <kmimetype.h>

#include <kdevcodemodel.h>
#include <kdevcodeproxy.h>
#include <kdevcodedelegate.h>
#include <kdevcoderepository.h>

class KDevAST;
class KDevDocument;
class KDevParseJob;
class KDevCodeHighlighting;

/**
@file kdevlanguagesupport.h
Interface to programming language specific features.
*/

/**
KDevelop language support interface - the base class for all programming
language support plugins. Language support is used to load facilities specific
to certain programming language. Language supports are usually loaded among with
a project. In this case project file defines which language support to load.

Language support plugin is a good place for:
- a language parser which fills memory and persistant symbol store
(see @ref CodeModel and @ref KDevCodeRepository);
- code wizards specific to a programming language (like new class wizard);
- symbol (class, function, etc.) name formatting to a human-readable convention
(pretty ormatted name).
.
*/
class KDEVINTERFACES_EXPORT KDevLanguageSupport: public KDevPlugin
{
    Q_OBJECT

public:
    /**Constructs a language support plugin.
    @param info Important information about the plugin - plugin internal and
    generic (GUI) name, description, a list of authors, etc. That information is
    used to show plugin information in various places like "about application"
    dialog, plugin selector dialog, etc. Plugin does not take ownership on info
    object, also its lifetime should be equal to the lifetime of the plugin.
    @param parent The parent object for the plugin.
    */
    KDevLanguageSupport(KInstance *instance, QObject *parent);

    /**Destructor.*/
    ~KDevLanguageSupport();

    /**@return A codemodel for the specified url or the currently active document.*/
    virtual KDevCodeModel *codeModel( const KUrl &url = KUrl() ) const = 0;

    /**@return A proxy model that can be used for sorting/filtering on the
    aggregate of all codemodel's.*/
    virtual KDevCodeProxy *codeProxy() const = 0;

    /**@return A codemodel delegate for painting the model according to the
    languages format.*/
    virtual KDevCodeDelegate *codeDelegate() const = 0;

    /**@return A code repository (accessor to persistant symbol stores) for
    codemodel's.*/
    virtual KDevCodeRepository *codeRepository() const = 0;

    /**@return A code highlighting object for highlighting text editor views, or null
    if there is not one available.*/
    virtual KDevCodeHighlighting *codeHighlighting() const;

    virtual KDevParseJob *createParseJob( const KUrl &url ) = 0;
    virtual KDevParseJob *createParseJob( KDevDocument *document ) = 0;

    //FIXME Make these pure to force language parts to provide this.
    //Right now the cpp language part can't persist the AST...
    virtual void read( KDevAST *ast, std::ifstream &in );
    virtual void write( KDevAST *ast, std::ofstream &out );

    // FIXME make pure
    virtual void releaseAST( KDevAST *ast);

    /**@return A typical mimetype list for the support language, this list
    should be configurable in the languagesupport dialog.*/
    virtual QStringList mimeTypes() const = 0;

    /**@return Whether the given document is supported by the language part.*/
    virtual bool supportsDocument( KDevDocument *document );

    /**@return Whether the given url is supported by the language part.*/
    virtual bool supportsDocument( const KUrl &url );

    /**
     * The mutex for the specified \a thread must be held when doing any background parsing.
     */
    QMutex* parseMutex(QThread* thread) const;

    /**
     * Lock all background parser mutexes.
     */
    void lockAllParseMutexes();

    /**
     * Unlock all background parser mutexes.
     */
    void unlockAllParseMutexes();

public Q_SLOTS:
    void threadFinished();

private:
    mutable QHash<QThread*, QMutex*> m_parseMutexes;
    QMutex* m_mutexMutex;
};

#endif
