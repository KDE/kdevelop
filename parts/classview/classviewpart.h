/*
 *  Copyright (C) 2003 Roberto Raggi (roberto@kdevelop.org)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#ifndef __KDEVPART_CLASSVIEW_H__
#define __KDEVPART_CLASSVIEW_H__

#include "kdevlanguagesupport.h"

#include <qguardedptr.h>
#include <kdevplugin.h>
#include <codemodel.h>

class ClassViewWidget;
class KListViewAction;
class QListViewItem;
class NamespaceItem;
class ClassItem;
class FunctionItem;
class KToolBarPopupAction;

class ClassViewPart : public KDevPlugin
{
    Q_OBJECT
public:
    ClassViewPart(QObject *parent, const char *name, const QStringList &);
    virtual ~ClassViewPart();

private slots:
    void selectNamespace(QListViewItem*);
    void selectClass(QListViewItem*);
    void selectFunction(QListViewItem*);
    void slotProjectOpened();
    void slotProjectClosed();
    void refresh();
    void switchedViewPopup();
    void graphicalClassView();

    void goToFunctionDeclaration();
    void goToFunctionDefinition();
    void goToClassDeclaration();
    void goToNamespaceDeclaration();
    void selectedAddClass();
    void selectedAddMethod();
    void selectedAddAttribute();

    void focusClasses();
    void focusFunctions();
    void unfocusClasses();
    void unfocusFunctions();

private:
    void setupActions();
    bool langHasFeature(KDevLanguageSupport::Features feature);

    QGuardedPtr<ClassViewWidget> m_widget;
    KListViewAction *m_namespaces;
    KListViewAction *m_classes;
    KListViewAction *m_functions;
    KToolBarPopupAction *m_popupAction;
};


#endif
