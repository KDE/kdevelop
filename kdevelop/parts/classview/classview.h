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

#ifndef _CLASSVIEW_H_
#define _CLASSVIEW_H_

#include <qguardedptr.h>
#include "kdevcomponent.h"


class ClassTreeWidget;
class ClassToolDialog;
class ClassStore;
class ClassListAction;
class MethodListAction;
class DelayedPopupAction;
class ParsedClass;


class ClassView : public KDevComponent
{
    Q_OBJECT

public:
    ClassView( QObject *parent=0, const char *name=0 );
    ~ClassView();

    enum ItemType { Scope, Class, Struct,
                    PublicAttr, ProtectedAttr, PrivateAttr,
                    PublicMethod, ProtectedMethod, PrivateMethod,
                    PublicSlot, ProtectedSlot, PrivateSlot,
                    Signal, GlobalFunction, GlobalVariable };

    void registerClassTool(ClassToolDialog *dlg);
    void unregisterClassTool(ClassToolDialog *dlg);
    
signals:
    void setLanguageSupport(KDevLanguageSupport *ls);
    void setClassStore(ClassStore *store);
    
protected:
    virtual void setupGUI();
    virtual void languageSupportOpened(KDevLanguageSupport *ls);
    virtual void languageSupportClosed();
    virtual void classStoreOpened(ClassStore *store);
    virtual void classStoreClosed();

private slots:
    void refresh();
    void selectedClass();
    void selectedMethod();
    void switchedDeclImpl();
    void selectedGotoDeclaration();
    void selectedGotoImplementation();
    void selectedGotoClassDeclaration();
    void selectedNewClass();
    void selectedAddMethod();
    void selectedAddAttribute();
    void dumpTree();
    
private:
    QGuardedPtr<QWidget> m_classtree;
    QList<QWidget> m_classtools;
    friend class ClassTreeBase;

    void setupPopup();
    ParsedClass *getClass(const QString &className);
    void gotoDeclaration(const QString &className, const QString &memberName, ItemType type);
    void gotoImplementation(const QString &className, const QString &memberName, ItemType type);

    ClassListAction *classes_action;
    MethodListAction *methods_action;
    DelayedPopupAction *popup_action;
    ClassStore *m_store;
    KDevLanguageSupport *m_langsupport;
    bool m_decl_or_impl;
};

#endif
