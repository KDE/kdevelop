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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KDEVLANGUAGESUPPORT_H
#define KDEVLANGUAGESUPPORT_H

#include <qstring.h>
#include <qstringlist.h>
#include <kmimetype.h>
#include "kdevplugin.h"
#include "codemodel.h"
#include <designer.h>

/**
@file kdevlanguagesupport.h
Interface to programming language specific features.
*/

class Tag;
class KDevDesignerIntegration;

using namespace KInterfaceDesigner;

/**
KDevelop language support interface - the base class for all programming language support plugins.
Language support is used to load facilities specific to certain programming language.
Language supports are usually loaded among with a project. In this case project file defines
which language support to load.

Language support plugin is a good place for:
- a language parser which fills memory and persistant symbol store
(see @ref CodeModel and @ref KDevCodeRepository);
- code wizards specific to a programming language (like new class wizard);
- GUI designer integration (see @ref KDevLanguageSupport::designer method 
documentation and @ref KDevDesignerIntegration class documentation;
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
    KDevLanguageSupport(const KDevPluginInfo *info, QObject *parent, const char *name);
    /**Destructor.*/
    ~KDevLanguageSupport();

    /**@return The feature set of the language. This is e.g. used
    by the class view to decide which organizer items to display and which not.*/
    virtual Features features();
    
    /**@return A typical mimetype list for the support language, this list 
    should be configurable in the languagesupport dialog.*/
    virtual KMimeType::List mimeTypes();

    /**Formats a Tag as used by the persistant symbol store to the human-readable convention.
    @param tag Tag to format.*/
    virtual QString formatTag(const Tag& tag);
    
    /**Formats a CodeModelItem as used by the CodeModel to the human-readable convention.
    @param item Symbol to format.
    @param shortDescription Show short description of a symbol. For example, when
    formatting functions short description could be a function signature without
    the return type and argument default values.*/
    virtual QString formatModelItem(const CodeModelItem *item, bool shortDescription=false);

    /**Formats a canonicalized class path as used by the symbol store to the 
    human-readable convention. For example, the C++ support part formats the
    string "KParts.Part" into "KParts::Part".
    @param name Class name.*/
    virtual QString formatClassName(const QString &name);
    
    /**The opposite of @ref formatClassName. Reverts formatting.
    @param name Class name.*/
    virtual QString unformatClassName(const QString &name);

    /**Opens a "New class" dialog and adds the configured class to the sources.
    Define NewClass feature if you reimplement this method.*/
    virtual void addClass();
    
    /**Opens an "Add method" dialog and adds the configured method to the sources.
    Define AddMethod feature if you reimplement this method.
    @param klass The class DOM to add a method to.*/
    virtual void addMethod(ClassDom klass);
    
    /**Opens an "Implement Virtual Methods" dialog and adds the configured methods 
    to the sources. Define AddMethod feature if you reimplement this method.
    @param klass The class DOM to add a virtual method to.*/
    virtual void implementVirtualMethods(ClassDom klass);
    
    /**Opens an "Add attribute" dialog and adds the configured attribute to the sources.
    Define AddAttribute feature if you reimplement this method.
    @param klass The class DOM to add an attribute to.*/
    virtual void addAttribute(ClassDom klass);
	
	/**
	 * Opens an "create get/set methods" dialog and adds the configured methods to the sources.
	 * Define CreateAccessMethods feature if you reimplement this method.
	 * @param theClass The class the methods should be added to.
	 * @param theVariable The attribute the access methods should be generated for.
	 */
	virtual void createAccessMethods(ClassDom theClass, VariableDom theVariable);
    
    /**Opens an "Subclass Widget" dialog for given Qt .ui file (formName)
    and propmts to implement it's slots.
    @param formName The name of a form to subclass.
    @return A list of newly created files.*/
    virtual QStringList subclassWidget(const QString& formName);
    
    /**Opens an "Update Widget" dialog for given Qt .ui file (formName)
    and prompts to add missing slot implementations
    in the subclass located in fileName.
    @param formName The name of a form which is being subclassed.
    @param fileName The name of a file with a subclass.
    @return A list of updated files. Can be empty because usually no additional
    actions are required on updated files.*/
    virtual QStringList updateWidget(const QString& formName, const QString& fileName);

    /**Reimplement this method if you want to use integrated GUI designer for the language.
    Implementation could look like (in pseudo code):
    @code
    KDevDesignerIntegration *des = 0;
    switch (type)
    {
        case KInterfaceDesigner::QtDesigner:
            des = getDesignerFromCache(type);
            if (des == 0)
            {
                MyLanguageImplementationWidget *impl = new MyLanguageImplementationWidget(this);
                des = new QtDesignerMyLanguageIntegration(this, impl);
                des->loadSettings(*project()->projectDom(), "kdevmylangsupport/designerintegration");
                saveDesignerToCache(type, des);
            }
            break;
    }
    return des;
    @endcode
    @ref ImplementationWidget and @ref QtDesignerIntegration classes are available
    from designerintegration support library.
    @param type The type of the designer to integrate.
    @return The pointer to designer integration of given type or 0.*/
    virtual KDevDesignerIntegration *designer(KInterfaceDesigner::DesignerType type);

public slots:
    /**Adds a function requested by a GUI designer. No need to reimplement this slot
    unless you want to use specific implementation of KDevDesignerIntegration interface.
    @param type The type of integrated designer.
    @param formName The name of a GUI form.
    @param function The function to implement (add).*/
    void addFunction(DesignerType type, const QString &formName, Function function);
    
    /**Removes a function requested by a GUI designer. No need to reimplement this slot
    unless you want to use specific implementation of KDevDesignerIntegration interface.
    @param type The type of integrated designer.
    @param formName The name of a GUI form.
    @param function The function to remove.*/
    void removeFunction(DesignerType type, const QString &formName, Function function);
    
    /**Edits a function requested by a GUI designer. No need to reimplement this slot
    unless you want to use specific implementation of KDevDesignerIntegration interface.
    @param type The type of integrated designer.
    @param formName The name of a GUI form.
    @param oldFunction The old function signature before editing.
    @param function The new function signature after editing.*/
    void editFunction(DesignerType type, const QString &formName, Function oldFunction, Function function);
    
    /**Opens a function requested by a GUI designer. No need to reimplement this slot
    unless you want to use specific implementation of KDevDesignerIntegration interface.
    @param type The type of integrated designer.
    @param formName The name of a GUI form.
    @param functionName The name of a function to seek in the code for.*/
    void openFunction(DesignerType type, const QString &formName, const QString &functionName);

    /**Opens a form source requested by a GUI designer. No need to reimplement this slot
    unless you want to use specific implementation of KDevDesignerIntegration interface.
    @param type The type of integrated designer.
    @param formName The name of a GUI form.*/
    void openSource(DesignerType type, const QString &formName);
        
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
