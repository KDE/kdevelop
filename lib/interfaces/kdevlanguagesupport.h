/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2002 F@lk Brettschneider <falkbr@kdevelop.org>
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>
   Copyright (C) 2003 Amilcar do Carmo Lucas <amilcar@ida.ing.tu-bs.de>

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
/**
 * The interface to programming language specific features
 */

#ifndef _KDEVLANGUAGESUPPORT_H_
#define _KDEVLANGUAGESUPPORT_H_

#include <qstring.h>
#include <qstringlist.h>
#include <kmimetype.h>
#include "kdevplugin.h"
#include "codemodel.h"
#include <designer.h>

class Tag;
class KDevDesignerIntegration;

using namespace KInterfaceDesigner;

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
    virtual QString formatModelItem( const CodeModelItem *item, bool shortDescription=false );

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
    virtual void addMethod( ClassDom klass );
    /**
     * Opens an "Implement Virtual Methods" dialog and adds the
     * configured methods to the sources.
     */
    virtual void implementVirtualMethods( ClassDom klass );
    /**
     * Opens an "Add attribute" dialog and adds the configured
     * method to the sources.
     */
    virtual void addAttribute( ClassDom klass );
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

    /**
     * Returns a pointer to designer integration plugin of given type or 0.
     */
    virtual KDevDesignerIntegration *designer(KInterfaceDesigner::DesignerType type);

public slots:
    /**Adds a function requested by a gui designer.*/
    void addFunction(DesignerType type, const QString &formName, Function function);
    /**Removes a function requested by a gui designer.*/
    void removeFunction(DesignerType type, const QString &formName, Function function);
    /**Edits a function requested by a gui designer.*/
    void editFunction(DesignerType type, const QString &formName, Function oldFunction, Function function);
    
    void openFunction(DesignerType type, const QString &formName, const QString &functionName);
    
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
