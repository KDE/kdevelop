/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classviewpart.h"

#include <qpopupmenu.h>
#include <qvbox.h>
#include <qwhatsthis.h>
#include <kaction.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kcompletionbox.h>

#include "kdevcore.h"
#include "kdevlanguagesupport.h"
#include "kdevtoplevel.h"
#include "kdevpartcontroller.h"
#include "classstore.h"

#include "classviewwidget.h"
#include "classviewconfigwidget.h"
#include "classactions.h"
#include "classtooldlg.h"
#include "hierarchydlg.h"


K_EXPORT_COMPONENT_FACTORY( libkdevclassview, ClassViewFactory( "kdevclassview" ) );

ClassViewPart::ClassViewPart( QObject *parent, const char *name, const QStringList & )
    : KDevPlugin(parent, name)
{
    setInstance(ClassViewFactory::instance());
    setXMLFile("kdevclassview.rc");
    
    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );

    m_classtree = new ClassViewWidget(this);
    m_classtree->setIcon(SmallIcon("CVclass"));
    m_classtree->setCaption(i18n("Class View"));
    QWhatsThis::add(m_classtree, i18n("Class View\n\n"
                                      "The class viewer shows all classes, methods and variables "
                                      "of the source files and allows switching to declarations "
                                      "and implementations. The right button popup menu allows more specialized "
                                      "functionality."));

    topLevel()->embedSelectView(m_classtree, i18n("Classes"));

    classes_action = new ClassListAction(this, i18n("Classes"), 0,
                                         this, SLOT(selectedClass()),
                                         actionCollection(), "class_combo");
    methods_action = new MethodListAction(this, i18n("Methods"), 0,
                                          this, SLOT(selectedMethod()),
                                          actionCollection(), "method_combo");        
    
    popup_action  = new DelayedPopupAction(i18n("Declaration/Implementation"), "classwiz", 0,
                                           this, SLOT(switchedDeclImpl()),
                                           actionCollection(), "class_wizard");    
    
    setupPopup();
    m_decl_or_impl = false;
    
    if( project() )
        projectOpened();
}


ClassViewPart::~ClassViewPart()
{
    for (QWidget *w=m_widgets.first(); w != 0; w = m_widgets.next())
      topLevel()->removeView(w);
    topLevel()->removeView(m_classtree);

    // In contrast to other widgets, we can have any number of class tool
    // dialogs. That's why we don't use QGuardedPtr here, but instead let
    // the dialogs register() and unregister() themselves.
    m_widgets.setAutoDelete(true);
    delete m_classtree;
}


bool ClassViewPart::langHasFeature(KDevLanguageSupport::Features feature)
{
    bool result = false;
    if (languageSupport())
        result = (feature & languageSupport()->features());
    return result;
}


void ClassViewPart::setupPopup()
{
    QPopupMenu *popup = popup_action->popupMenu();

    popup->clear();

    if (langHasFeature(KDevLanguageSupport::Declarations))
        popup->insertItem(i18n("Go to Declaration"), this, SLOT(selectedGotoDeclaration()));

    popup->insertItem(i18n("Go to Implementation"), this, SLOT(selectedGotoImplementation()));
    popup->insertItem(i18n("Go to Class Declaration"), this, SLOT(selectedGotoClassDeclaration()));
    popup->insertItem(i18n("View Class Hierarchy"), this, SLOT(selectedViewHierarchy()));
    //    popup->insertItem("Dump Class Tree on Console", this, SLOT(dumpTree()));

    bool hasAddMethod = langHasFeature(KDevLanguageSupport::AddMethod);
    bool hasAddAttribute = langHasFeature(KDevLanguageSupport::AddAttribute);
    bool hasNewClass = langHasFeature(KDevLanguageSupport::NewClass);
    if (hasAddMethod || hasAddAttribute || hasNewClass) 
        popup->insertSeparator();
    if (hasNewClass)
        popup->insertItem(SmallIcon("classnew"), i18n("Add Class..."), this, SLOT(selectedAddClass()));
    if (hasAddMethod)
        popup->insertItem(SmallIcon("methodnew"), i18n("Add Method..."), this, SLOT(selectedAddMethod()));
    if (hasAddAttribute)
        popup->insertItem(SmallIcon("variablenew"), i18n("Add Attribute..."), this, SLOT(selectedAddAttribute()));
}


void ClassViewPart::projectConfigWidget(KDialogBase *dlg)
{
    QVBox *vbox = dlg->addVBoxPage(i18n("Class View"));
    ClassViewConfigWidget *w = new ClassViewConfigWidget(this, vbox, "class view config widget");
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}


void ClassViewPart::configChange()
{
    emit setLanguageSupport(languageSupport());
}


void ClassViewPart::projectOpened()
{
    kdDebug(9003) << "ClassViewPart::projectSpaceOpened()" << endl;

    KDevLanguageSupport *ls = languageSupport();
    if (ls)
        connect(ls, SIGNAL(updatedSourceInfo()), this, SLOT(updatedSourceInfo()));
    updatedSourceInfo();
    emit setLanguageSupport(ls);
    setupPopup();
}


void ClassViewPart::projectClosed()
{
    emit setLanguageSupport(0);
    setupPopup();
    updatedSourceInfo();
}


void ClassViewPart::updatedSourceInfo()
{
    classes_action->refresh();
    methods_action->refresh(classes_action->currentClassName());
}


void ClassViewPart::registerClassToolDialog(ClassToolDialog *dlg)
{
    m_widgets.append(dlg);
    topLevel()->embedSelectView(dlg, i18n("Classtools"));
    topLevel()->raiseView(dlg);
}


void ClassViewPart::registerHierarchyDialog(HierarchyDialog *dlg)
{
    m_widgets.append(dlg);
}


void ClassViewPart::unregisterClassToolDialog(ClassToolDialog *dlg)
{
    m_widgets.removeRef(dlg);
}


void ClassViewPart::unregisterHierarchyDialog(HierarchyDialog *dlg)
{
    m_widgets.removeRef(dlg);
}


/**
 * The user selected a class in the toolbar class combo.
 */
void ClassViewPart::selectedClass()
{
    QString className = classes_action->currentClassName();
    
    kdDebug(9003) << "Class selected: " << className << endl;
    methods_action->refresh(className);
}


/**
 * The user selected a method in the toolbar method combo.
 */
void ClassViewPart::selectedMethod()
{
    QString className = classes_action->currentClassName();
    QString methodName = methods_action->currentMethodName();

    kdDebug(9003) << "ClassViewPart::selectedMethod" << endl;
    m_decl_or_impl = true;
    gotoImplementation(className, methodName);
}


/**
 * The user clicked on the class wizard button.
 */
void ClassViewPart::switchedDeclImpl()
{
    QString className = classes_action->currentClassName();
    QString methodName = methods_action->currentMethodName();

    kdDebug(9003) << "ClassViewPart::switchedDeclImpl" << endl;
    if (m_decl_or_impl) {
        m_decl_or_impl = false;
        gotoDeclaration(className, methodName);
    } else {
        m_decl_or_impl = true;
        gotoImplementation(className, methodName);
    }
}


/**
 * The user selected "View class hierarchy" from the delayed class wizard popup.
 */
void ClassViewPart::selectedViewHierarchy()
{
    HierarchyDialog *dlg = new HierarchyDialog(this);
    dlg->show();
}


/**
 * The user selected "Goto declaration" from the delayed class wizard popup.
 */
void ClassViewPart::selectedGotoDeclaration()
{
    QString className = classes_action->currentClassName();
    QString methodName = methods_action->currentMethodName();

    gotoDeclaration(className, methodName);
}


/**
 * The user selected "Goto class declaration" from the delayed class wizard popup.
 */
void ClassViewPart::selectedGotoClassDeclaration()
{
    QString className = classes_action->currentClassName();

    gotoDeclaration(className, QString::null);
}


/**
 * The user selected "Goto implementation" from the delayed class wizard popup.
 */
void ClassViewPart::selectedGotoImplementation()
{
    QString className = classes_action->currentClassName();
    QString methodName = methods_action->currentMethodName();

    gotoImplementation(className, methodName);
}


/**
 * The user selected "New class..." from the delayed class wizard popup.
 */
void ClassViewPart::selectedAddClass()
{
    if (languageSupport())
        languageSupport()->addClass();
}


/**
 * The user selected "Add method..." from the delayed class wizard popup.
 */
void ClassViewPart::selectedAddMethod()
{
    if (languageSupport())
        languageSupport()->addMethod(classes_action->currentClassName());
}


/**
 * The user selected "Add attribute..." from the delayed class wizard popup.
 */
void ClassViewPart::selectedAddAttribute()
{
    if (languageSupport())
        languageSupport()->addAttribute(classes_action->currentClassName());
}


// Only for debugging
void ClassViewPart::dumpTree()
{
    classStore()->out();
}


ParsedClass *ClassViewPart::getClass(const QString &className)
{
    if (className.isEmpty())
        return 0;

    kdDebug(9003) << "ClassViewPart::getClass " << className << endl;
    ParsedClass *pc = classStore()->getClassByName(className);
    if (pc && !pc->isSubClass())
        classes_action->setCurrentClassName(className);
    
    return pc;
}


void ClassViewPart::gotoDeclaration(const QString &className, const QString &methodName)
{
    kdDebug(9003) << "ClassViewPart::gotoDeclaration " << className << "::" << methodName << endl;

    QString toFile;
    int toLine = -1;
    
    if (className.isEmpty()) {
        if (methodName.isEmpty())
            return;

        // Global function
        ParsedMethod *pm = classStore()->globalScope()->getMethodByNameAndArg(methodName);
        if (!pm)
            return;
        
        toFile = pm->declaredInFile();
        toLine = pm->declaredOnLine();
    } else {
        // Either the class itself or a member function
        ParsedClass *pc = getClass(className);
        if (!pc)
            return;
        
        if (methodName.isEmpty()) {
            // Class itself
            toFile = pc->declaredInFile();
            toLine = pc->declaredOnLine();
        } else {
            // Method of the class
            ParsedMethod *pm = pc->getMethodByNameAndArg(methodName);
            if (!pm)
                pm = pc->getSlotByNameAndArg(methodName);
            if (!pm)
                pm = pc->getSignalByNameAndArg(methodName);
            if (!pm)
                return;
            
            toFile = pm->declaredInFile();
            toLine = pm->declaredOnLine();
        }
    }

    if (toLine != -1) {
        kdDebug(9003) << "Classview switching to file " << toFile << "@ line " << toLine << endl;
	partController()->editDocument(toFile, toLine);
    }
}


void ClassViewPart::gotoImplementation(const QString &className, const QString &methodName)
{
    kdDebug(9003) << "ClassViewPart::gotoImplementation " << className << "::" << methodName << endl;

    QString toFile;
    int toLine = -1;
    
    if (className.isEmpty()) {
        if (methodName.isEmpty())
            return;
        
        // Global function
        ParsedMethod *pm = classStore()->globalScope()->getMethodByNameAndArg(methodName);
        if (!pm)
            return;
        
        toFile = pm->definedInFile();
        toLine = pm->definedOnLine();
    } else {
        // Either the class itself or a member function
        ParsedClass *pc = getClass(className);
        if (!pc)
            return;
        
        if (methodName.isEmpty()) {
            // Class itself
            // => does not have an implementation, so go to declaration
            toFile = pc->definedInFile();
            toLine = pc->definedOnLine();
        } else {
            // Method of the class
            ParsedMethod *pm = pc->getMethodByNameAndArg(methodName);
            if (!pm)
                pm = pc->getSlotByNameAndArg(methodName);
            if (!pm)
                pm = pc->getSignalByNameAndArg(methodName);
            if (!pm)
                return;
            
            toFile = pm->definedInFile();
            toLine = pm->definedOnLine();
        }
    }

    if (toLine != -1) {
        kdDebug(9003) << "Classview switching to file " << toFile << "@ line " << toLine << endl;
	partController()->editDocument(toFile, toLine);
    }
}

#include "classviewpart.moc"
