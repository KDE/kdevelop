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

#include <qpopupmenu.h>
#include <qwhatsthis.h>
#include <kaction.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>

#include "kdevcore.h"
#include "kdevlanguagesupport.h"
#include "kdevtoplevel.h"
#include "kdevpartcontroller.h"
#include "classstore.h"

#include "classviewpart.h"
#include "classviewwidget.h"
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

    m_classtree = new ClassViewWidget(this);
    m_classtree->setIcon(SmallIcon("CVclass"));
    m_classtree->setCaption(i18n("Class View"));
    QWhatsThis::add(m_classtree, i18n("Class View\n\n"
                                      "The class viewer shows all classes, methods and variables "
                                      "of the source files and allows switching to declarations "
                                      "and implementations. The right button popup menu allows more specialized "
                                      "functionality."));

    topLevel()->embedSelectView(m_classtree, i18n("Classes"));

    classes_action = new ClassListAction(classStore(), i18n("Classes"), 0,
                                         this, SLOT(selectedClass()),
                                         actionCollection(), "class_combo");
    methods_action = new MethodListAction(classStore(), i18n("Methods"), 0,
                                          this, SLOT(selectedMethod()),
                                          actionCollection(), "method_combo");
    popup_action  = new DelayedPopupAction(i18n("Declaration/Implementation"), "classwiz", 0,
                                           this, SLOT(switchedDeclImpl()),
                                           actionCollection(), "class_wizard");
    setupPopup();
    m_decl_or_impl = false;
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
    popup->insertItem("Dump Class Tree on Console", this, SLOT(dumpTree()));

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


void ClassViewPart::projectOpened()
{
    kdDebug(9003) << "ClassViewPart::projectSpaceOpened()" << endl;

    KDevLanguageSupport *ls = languageSupport();
    if (ls)
        connect(ls, SIGNAL(updatedSourceInfo()), this, SLOT(updatedSourceInfo()));
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
    methods_action->refresh(classes_action->currentText());
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
    QString className = classes_action->currentText();
    if (className.isEmpty())
        return;
    
    kdDebug(9003) << "Class selected: " << className << endl;
    methods_action->refresh(className);
}


/**
 * The user selected a method in the toolbar method combo.
 */
void ClassViewPart::selectedMethod()
{
    QString className = classes_action->currentText();
    QString methodName = methods_action->currentText();
    if (className.isEmpty() || methodName.isEmpty())
        return;

    kdDebug(9003) << "Method selected: "
                  << className << "::" << methodName << endl;
    m_decl_or_impl = true;
    gotoImplementation(className, methodName, PublicMethod);
}


/**
 * The user clicked on the class wizard button.
 */
void ClassViewPart::switchedDeclImpl()
{
    QString className = classes_action->currentText();
    QString methodName = methods_action->currentText();

    kdDebug(9003) << "ClassViewPart::switchedDeclImpl" << endl;
    if (m_decl_or_impl) {
        m_decl_or_impl = false;
        gotoDeclaration(className, methodName,
                        methodName.isEmpty()? Class : PublicMethod);
    } else {
        m_decl_or_impl = true;
        if (methodName.isEmpty())
            gotoDeclaration(className, QString::null, Class);
        else
            gotoImplementation(className, methodName, PublicMethod);
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
    QString className = classes_action->currentText();
    QString methodName = methods_action->currentText();
    
    gotoDeclaration(className, methodName,
                    methodName.isEmpty()? Class : PublicMethod);
}


/**
 * The user selected "Goto class declaration" from the delayed class wizard popup.
 */
void ClassViewPart::selectedGotoClassDeclaration()
{
    QString className = classes_action->currentText();
    
    gotoDeclaration(className, QString::null, Class);
}


/**
 * The user selected "Goto implementation" from the delayed class wizard popup.
 */
void ClassViewPart::selectedGotoImplementation()
{
    QString className = classes_action->currentText();
    QString methodName = methods_action->currentText();

    if (methodName.isEmpty())
        gotoDeclaration(className, QString::null, Class);
    else
        gotoImplementation(className, methodName, PublicMethod);
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
        languageSupport()->addMethod(classes_action->currentText());
}


/**
 * The user selected "Add attribute..." from the delayed class wizard popup.
 */
void ClassViewPart::selectedAddAttribute()
{
    if (languageSupport())
        languageSupport()->addAttribute(classes_action->currentText());
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
        classes_action->setCurrentItem(className);
    
    return pc;
}


void ClassViewPart::gotoDeclaration(const QString &className,
                                const QString &memberName,
                                ItemType type)
{
    kdDebug(9003) << "ClassViewPart::gotoDeclaration " << className << "::" << memberName << endl;
    
    QString toFile;
    int toLine = -1;
    
    ParsedClass *pc = getClass(className);
    ParsedStruct *ps = 0;
    ParsedAttribute *pa = 0;
    
    switch(type) {
    case Class:
        if (pc) {
            toFile = pc->declaredInFile();
            toLine = pc->declaredOnLine();
        }
        break;
    case Struct:
        if (pc)
            ps = pc->getStructByName(memberName);
        else
            ps = classStore()->globalContainer.getStructByName(memberName);
        toFile = ps->declaredInFile();
        toLine = ps->declaredOnLine();
        break;
    case PublicAttr:
    case ProtectedAttr:
    case PrivateAttr:
    case PackageAttr:
        if (pc)
            pa = pc->getAttributeByName(memberName);
        else {
            ps = classStore()->globalContainer.getStructByName(className);
            if (ps)
                pa = ps->getAttributeByName(memberName);
        }
        break;
    case PublicMethod:
    case ProtectedMethod:
    case PrivateMethod:
        if (pc) {
            pa = pc->getMethodByNameAndArg(memberName);
            // If at first we don't succeed...
            if (!pa)
                pa = pc->getSlotByNameAndArg(memberName);
        }
        break;
    case PublicSlot:
    case ProtectedSlot:
    case PrivateSlot:
        if (pc)
            pa = pc->getSlotByNameAndArg(memberName);
      break;
    case Signal:
        if (pc)
            pa = pc->getSignalByNameAndArg(memberName);
      break;
    case GlobalFunction:
        pa = classStore()->globalContainer.getMethodByNameAndArg(memberName);
      break;
    case GlobalVariable:
        pa = classStore()->globalContainer.getAttributeByName(memberName);
        break;
    default:
        kdDebug(9003) << "Unknown type " << (int)type << " in ClassViewPart::gotoDeclaration." << endl;
        break;
    }
    
    // Fetch the line and file from the attribute if the value is set.
    if (pa) {
        toFile = pa->declaredInFile();
        toLine = pa->declaredOnLine();
    }
    
    if (toLine != -1) {
        kdDebug(9003) << "Classview switching to file " << toFile << "@ line " << toLine << endl;
	partController()->editDocument(toFile, toLine);
    }
}


void ClassViewPart::gotoImplementation(const QString &className,
                                   const QString &memberName,
                                   ItemType type)
{
    kdDebug(9003) << "ClassViewPart::gotoImplementation " << className << "::" << memberName << endl;

    ParsedClass *pc = getClass(className);
    ParsedMethod *pm = 0;
    
    switch(type) {
    case PublicSlot:
    case ProtectedSlot:
    case PrivateSlot:
        if (pc)
            pm = pc->getSlotByNameAndArg(memberName);
        break;
    case PublicMethod:
    case ProtectedMethod:
    case PrivateMethod:
        if (pc) {
            pm = pc->getMethodByNameAndArg(memberName);
            // If at first we don't succeed...
            if (!pm)
                pm = pc->getSlotByNameAndArg(memberName); 
        }
        break;
    case GlobalFunction:
        pm = classStore()->globalContainer.getMethodByNameAndArg(memberName);
        break;
    default:
        kdDebug(9003) << "Unknown type " << (int)type << "in ClassViewPart::gotoImplementation." << endl;
    }
    
    if (pm)
	partController()->editDocument(pm->definedInFile(), pm->definedOnLine());
}

#include "classviewpart.moc"
