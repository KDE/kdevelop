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

#ifndef _CLASSVIEWPART_H_
#define _CLASSVIEWPART_H_

#include <qguardedptr.h>
#include "kdevpart.h"
#include "kdevlanguagesupport.h"


class ClassTreeWidget;
class ClassToolDialog;
class HierarchyDialog;
class ClassStore;
class ClassListAction;
class MethodListAction;
class DelayedPopupAction;
class ParsedClass;


class ClassViewPart : public KDevPart
{
    Q_OBJECT

public:
    ClassViewPart( KDevApi *api, QObject *parent=0, const char *name=0 );
    ~ClassViewPart();

    enum ItemType { Scope, Class, Struct,
                    PublicAttr, ProtectedAttr, PrivateAttr, PackageAttr,
                    PublicMethod, ProtectedMethod, PrivateMethod, PackageMethod,
                    PublicSlot, ProtectedSlot, PrivateSlot,
                    Signal, GlobalFunction, GlobalVariable };

    void registerClassToolDialog(ClassToolDialog *dlg);
    void registerHierarchyDialog(HierarchyDialog *dlg);
    void unregisterClassToolDialog(ClassToolDialog *dlg);
    void unregisterHierarchyDialog(HierarchyDialog *dlg);
    
signals:
    void setLanguageSupport(KDevLanguageSupport *ls);
    
private slots:
    // Connected with core
    void projectOpened();
    void projectClosed();
    // Connected with languageSupport
    void updatedSourceInfo();
    // Internal
    void selectedClass();
    void selectedMethod();
    void switchedDeclImpl();
    void selectedViewHierarchy();
    void selectedGotoDeclaration();
    void selectedGotoImplementation();
    void selectedGotoClassDeclaration();
    void selectedAddClass();
    void selectedAddMethod();
    void selectedAddAttribute();
    void dumpTree();
    
private:
    QGuardedPtr<QWidget> m_classtree;
    QList<QWidget> m_widgets;
    friend class ClassTreeBase;
    friend class HierarchyDialog;

    bool langHasFeature(KDevLanguageSupport::Features feature);
    void setupPopup();
    ParsedClass *getClass(const QString &className);
    void gotoDeclaration(const QString &className, const QString &memberName, ItemType type);
    void gotoImplementation(const QString &className, const QString &memberName, ItemType type);

    ClassListAction *classes_action;
    MethodListAction *methods_action;
    DelayedPopupAction *popup_action;
    bool m_decl_or_impl;
};

#endif
