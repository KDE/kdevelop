/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2002 F@lk Brettschneider <falkbr@kdevelop.org>
   Copyright (C) 2003 Amilcar do Carmo Lucas <amilcar@ida.ing.tu-bs.de>
   Copyright (C) 2003-2004 Alexander Dymo <adymo@kdevelop.org>

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
#ifndef KDEVLANGUAGESUPPORT_H
#define KDEVLANGUAGESUPPORT_H

#include "kdevplugin.h"

#include <qstring.h>
#include <qstringlist.h>
#include <kmimetype.h>

/**
@file kdevlanguagesupport.h
Interface to programming language specific features.
*/

class Tag;
class KDevCodeItem;
class KDevCodeClassItem;
class KDevCodeVariableItem;

/**
KDevelop language support interface - the base class for all programming language support plugins.
Language support is used to load facilities specific to certain programming language.
Language supports are usually loaded among with a project. In this case project file defines
which language support to load.

Language support plugin is a good place for:
- a language parser which fills memory and persistant symbol store
(see @ref CodeModel and @ref KDevCodeRepository);
- code wizards specific to a programming language (like new class wizard);
- symbol (class, function, etc.) name formatting to a human-readable convention (pretty
formatted name).
.
*/
class KDevLanguageSupport: public KDevPlugin
{
    Q_OBJECT

public:

    /**Features supported by this language support.*/
    enum Features {
        /*features of the language itself*/
        Classes=1        /**<Language has classes (or class-like packages).*/,
        Structs=2        /**<Language has structures or records.*/,
        Functions=4      /**<Language has functions.*/,
        Variables=8      /**<Language has variables.*/,

        Namespaces=16    /**<Language has namespaces.*/,
        Signals=32       /**<Language has signals (or qt library bindings are available).*/,
        Slots=64         /**<Language has slots (or qt library bindings are available).*/,
        Declarations=128 /**<Language has function declarations (like c, c++ and pascal).*/,

        /*features of the language support part*/
        NewClass=512        /**<Language support can create classes.*/,
        AddMethod=1024      /**<Language support can create methods.*/,
        AddAttribute=2048   /**<Language support can create class attributes.*/,
        CreateAccessMethods=4096  /**<Language support can create get/set methods for attributes.*/
    };

    /**Constructs a language support plugin.
    @param info Important information about the plugin - plugin internal and generic
    (GUI) name, description, a list of authors, etc. That information is used to show
    plugin information in various places like "about application" dialog, plugin selector
    dialog, etc. Plugin does not take ownership on info object, also its lifetime should
    be equal to the lifetime of the plugin.
    @param parent The parent object for the plugin. Parent object must implement @ref KDevApi
    interface. Otherwise the plugin will not be constructed.
    @param name The internal name which identifies the plugin.*/
    KDevLanguageSupport(const KDevPluginInfo *info, QObject *parent);
    /**Destructor.*/
    ~KDevLanguageSupport();

    /**@return The feature set of the language. This is e.g. used
    by the class view to decide which organizer items to display and which not.*/
    virtual int features() const = 0;

    /**@return A typical mimetype list for the support language, this list
    should be configurable in the languagesupport dialog.*/
    virtual QStringList mimeTypes() const = 0;

    /**Formats a Tag as used by the persistant symbol store to the human-readable convention.
    @param tag Tag to format.*/
    virtual QString formatTag(const Tag& tag) const = 0;

    /**Formats a KDevCodeItem as used by the CodeModel to the human-readable convention.
    @param item Symbol to format.
    @param shortDescription Show short description of a symbol. For example, when
    formatting functions short description could be a function signature without
    the return type and argument default values.*/
    virtual QString formatModelItem(const KDevCodeItem *item, bool shortDescription=false) const = 0;

    /**Formats a canonicalized class path as used by the symbol store to the
    human-readable convention. For example, the C++ support part formats the
    string "KParts.Part" into "KParts::Part".
    @param name Class name.*/
    virtual QString formatClassName(const QString &name) const = 0;

    /**The opposite of @ref formatClassName. Reverts formatting.
    @param name Class name.*/
    virtual QString unformatClassName(const QString &name) const = 0;

signals:
    /**Emitted when the content of the memory symbol store has been modified.*/
    void updatedSourceInfo();

    /**Emitted before removing the file from the memory symbol store.*/
    void aboutToRemoveSourceInfo(const QString& fileName);

    /**Emitted when a file has been removed from the memory symbol store.*/
    void removedSourceInfo(const QString& fileName);

    /**Emitted when a file has been added to the memory symbol store.*/
    void addedSourceInfo( const QString& fileName );
};

#endif
