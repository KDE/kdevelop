/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlcdnumber.h>
#include <qpopupmenu.h>
#include <qwhatsthis.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>

#include "classview.h"
#include "classactions.h"
#include "classtreewidget.h"
#include "classtooldlg.h"
//#include "hierarchydlg.h"
#include "main.h"
#include "classstore.h"
#include "kdevlanguagesupport.h"


ClassView::ClassView(QObject *parent, const char *name)
    : KDevComponent(parent, name)
{
    setInstance(ClassFactory::instance());
    setXMLFile("kdevclassview.rc");
    
    m_decl_or_impl = false;
    m_langsupport = 0;
    m_store = 0;

    // In contrast to other widgets, we can have any number of class tool
    // dialogs. That's why we don't use QGuardedPtr here, but instead let
    // the dialogs register() and unregister() themselves.
    m_widgets.setAutoDelete(true);
}


ClassView::~ClassView()
{
    delete m_classtree;
}


void ClassView::setupGUI()
{
    kdDebug(9003) << "Building ClassTreeWidget" << endl;

    m_classtree = new ClassTreeWidget(this);
    //    w->setIcon()
    m_classtree->setCaption(i18n("Class view"));
    QWhatsThis::add(m_classtree, i18n("Class View\n\n"
                                      "The class viewer shows all classes, methods and variables "
                                      "of the source files and allows switching to declarations "
                                      "and implementations. The right button popup menu allows more specialized "
                                      "functionality."));

    embedWidget(m_classtree, SelectView, i18n("CV"), i18n("class tree view"));

    classes_action = new ClassListAction(i18n("Classes"), 0, this, SLOT(selectedClass()),
                                         actionCollection(), "class_combo");
    methods_action = new MethodListAction(i18n("Methods"), 0, this, SLOT(selectedMethod()),
                                          actionCollection(), "method_combo");
    popup_action  = new DelayedPopupAction(i18n("Declaration/Implementation"), "classwiz", 0, this, SLOT(switchedDeclImpl()),
                                           actionCollection(), "class_wizard");
    setupPopup();
}


void ClassView::setupPopup()
{
    QPopupMenu *popup = popup_action->popupMenu();

    popup->clear();
    popup->insertItem(i18n("Goto declaration"), this, SLOT(selectedGotoDeclaration()));
    popup->insertItem(i18n("Goto implementation"), this, SLOT(selectedGotoImplementation()));
    popup->insertItem(i18n("Goto class declaration"), this, SLOT(selectedGotoClassDeclaration()));
//    popup->insertItem(i18n("View class hierarchy"), this, SLOT(selectedViewHierarchy()));
    popup->insertItem("Dump class tree on console", this, SLOT(dumpTree()));

    if (m_langsupport) {
        bool hasAddMethod = m_langsupport->hasFeature(KDevLanguageSupport::AddMethod);
        bool hasAddAttribute = m_langsupport->hasFeature(KDevLanguageSupport::AddAttribute);
        bool hasNewClass =  m_langsupport->hasFeature(KDevLanguageSupport::NewClass);
        if (hasAddMethod || hasAddAttribute || hasNewClass) 
            popup->insertSeparator();
        if (hasNewClass)
            popup->insertItem(i18n("New class..."), this, SLOT(selectedNewClass()));
        if (hasAddMethod)
            popup->insertItem(i18n("Add method..."), this, SLOT(selectedAddMethod()));
        if (hasAddAttribute)
            popup->insertItem(i18n("Add attribute..."), this, SLOT(selectedAddAttribute()));
    }
}


void ClassView::languageSupportOpened(KDevLanguageSupport *ls)
{
    m_langsupport = ls;
    connect(ls, SIGNAL(updateSourceInfo()), this, SLOT(refresh()));
    emit setLanguageSupport(ls);

    setupPopup();
}


void ClassView::languageSupportClosed()
{
    m_langsupport = 0;
    emit setLanguageSupport(0);
    
    setupPopup();
}


void ClassView::classStoreOpened(ClassStore *store)
{
    kdDebug(9003) << "ClassView::classStoreOpened()" << endl;
    m_store = store;
    emit setClassStore(store);
    classes_action->setClassStore(store);
    classes_action->refresh();
    methods_action->setClassStore(store);
    methods_action->refresh(classes_action->currentText());
}


void ClassView::classStoreClosed()
{
    kdDebug(9003) << "ClassView::classStoreClosed()" << endl;
    m_store = 0;
    emit setClassStore(0);
    classes_action->setClassStore(0);
    classes_action->refresh();
    methods_action->setClassStore(0);
    methods_action->refresh(classes_action->currentText());
}


void ClassView::refresh()
{
    classes_action->refresh();
    methods_action->refresh(classes_action->currentText());
}


void ClassView::registerClassToolDialog(ClassToolDialog *dlg)
{
    m_widgets.append(dlg);
    emit embedWidget(dlg, SelectView, i18n("CT"), i18n("class tool"));
}


#if 0
void ClassView::registerHierarchyDialog(HierarchyDialog *dlg)
{
    m_widgets.append(dlg);
}
#endif


void ClassView::unregisterClassToolDialog(ClassToolDialog *dlg)
{
    m_widgets.removeRef(dlg);
}


#if 0
void ClassView::unregisterHierarchyDialog(HierarchyDialog *dlg)
{
    m_widgets.removeRef(dlg);
}
#endif


/**
 * The user selected a class in the toolbar class combo.
 */
void ClassView::selectedClass()
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
void ClassView::selectedMethod()
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
void ClassView::switchedDeclImpl()
{
    QString className = classes_action->currentText();
    QString methodName = methods_action->currentText();

    kdDebug(9003) << "ClassView::switchedDeclImpl" << endl;
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


#if 0
/**
 * The user selected "View class hierarchy" from the delayed class wizard popup.
 */
void ClassView::selectedViewHierarchy()
{
    HierarchyDialog *dlg = new HierarchyDialog(this);
    dlg->setClassStore(m_store);
    dlg->setLanguageSupport(m_langsupport);
    dlg->show();
}
#endif


/**
 * The user selected "Goto declaration" from the delayed class wizard popup.
 */
void ClassView::selectedGotoDeclaration()
{
    QString className = classes_action->currentText();
    QString methodName = methods_action->currentText();
    
    gotoDeclaration(className, methodName,
                    methodName.isEmpty()? Class : PublicMethod);
}


/**
 * The user selected "Goto class declaration" from the delayed class wizard popup.
 */
void ClassView::selectedGotoClassDeclaration()
{
    QString className = classes_action->currentText();
    
    gotoDeclaration(className, QString::null, Class);
}


/**
 * The user selected "Goto implementation" from the delayed class wizard popup.
 */
void ClassView::selectedGotoImplementation()
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
void ClassView::selectedNewClass()
{
    if (m_langsupport)
        m_langsupport->newClassRequested();
}


/**
 * The user selected "Add method..." from the delayed class wizard popup.
 */
void ClassView::selectedAddMethod()
{
    if (m_langsupport)
        m_langsupport->addMethodRequested(classes_action->currentText());
}


/**
 * The user selected "Add attribute..." from the delayed class wizard popup.
 */
void ClassView::selectedAddAttribute()
{
    if (m_langsupport)
        m_langsupport->addAttributeRequested(classes_action->currentText());
}


// Only for debugging
void ClassView::dumpTree()
{
    m_store->out();
}


ParsedClass *ClassView::getClass(const QString &className)
{
    if (className.isEmpty())
        return 0;

    kdDebug(9003) << "ClassView::getClass " << className << endl;
    ParsedClass *pc = m_store->getClassByName(className);
    if (pc && !pc->isSubClass)
        classes_action->setCurrentItem(className);
    
    return pc;
}


void ClassView::gotoDeclaration(const QString &className,
                                const QString &memberName,
                                ItemType type)
{
    kdDebug(9003) << "ClassView::gotoDeclaration " << className << "::" << memberName << endl;
    
    QString toFile;
    int toLine = -1;
    
    ParsedClass *pc = getClass(className);
    ParsedStruct *ps = 0;
    ParsedAttribute *pa = 0;
    
    switch(type) {
    case Class:
        if (pc) {
            toFile = pc->declaredInFile;
            toLine = pc->declaredOnLine;
        }
        break;
    case Struct:
        if (pc)
            ps = pc->getStructByName(memberName);
        else
            ps = m_store->globalContainer.getStructByName(memberName);
        toFile = ps->declaredInFile;
        toLine = ps->declaredOnLine;
        break;
    case PublicAttr:
    case ProtectedAttr:
    case PrivateAttr:
        if (pc)
            pa = pc->getAttributeByName(memberName);
        else {
            ps = m_store->globalContainer.getStructByName(className);
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
        pa = m_store->globalContainer.getMethodByNameAndArg(memberName);
      break;
    case GlobalVariable:
        pa = m_store->globalContainer.getAttributeByName(memberName);
        break;
    default:
        kdDebug(9003) << "Unknown type " << (int)type << " in ClassView::gotoDeclaration." << endl;
        break;
    }
    
    // Fetch the line and file from the attribute if the value is set.
    if (pa) {
        toFile = pa->declaredInFile;
        toLine = pa->declaredOnLine;
    }
    
    if (toLine != -1) {
        kdDebug(9003) << "Classview switching to file " << toFile << "@ line " << toLine << endl;
        emit gotoSourceFile(toFile, toLine);
    }
}


void ClassView::gotoImplementation(const QString &className,
                                   const QString &memberName,
                                   ItemType type)
{
    kdDebug(9003) << "ClassView::gotoImplementation " << className << "::" << memberName << endl;

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
        pm = m_store->globalContainer.getMethodByNameAndArg(memberName);
        break;
    default:
        kdDebug(9003) << "Unknown type " << (int)type << "in ClassView::gotoImplementation." << endl;
    }
    
    if (pm)
        emit gotoSourceFile(pm->definedInFile, pm->definedOnLine);
}
#include "classview.moc"
