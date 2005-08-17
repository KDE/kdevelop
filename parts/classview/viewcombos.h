/*
 *  Copyright (C) 2003 Alexander Dymo (cloudtemple@mksat.net)
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
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */
#ifndef VIEWCOMBOS_H
#define VIEWCOMBOS_H

#include <klocale.h>

#include <q3listview.h>

#include "codemodel.h"

class ClassViewPart;
class KComboView;

const QString EmptyClasses = i18n("(Classes)");
const QString EmptyFunctions = i18n("(Functions)");

class NamespaceItem: public Q3ListViewItem{
public:
    NamespaceItem(ClassViewPart *part, Q3ListView *parent, QString name, NamespaceDom dom);
    NamespaceItem(ClassViewPart *part, Q3ListViewItem *parent, QString name, NamespaceDom dom);
    ~NamespaceItem();
    NamespaceDom dom() const;
    virtual void setup();
private:
    NamespaceDom m_dom;
    ClassViewPart *m_part;
};

class ClassItem: public Q3ListViewItem{
public:
    ClassItem(ClassViewPart *part, Q3ListView *parent, QString name, ClassDom dom);
    ClassItem(ClassViewPart *part, Q3ListViewItem *parent, QString name, ClassDom dom);
    ~ClassItem();
    ClassDom dom() const;
    virtual void setup();
private:
    ClassDom m_dom;
    ClassViewPart *m_part;
};

class FunctionItem: public Q3ListViewItem{
public:
    FunctionItem(ClassViewPart *part, Q3ListView *parent, QString name, FunctionDom dom);
    FunctionItem(ClassViewPart *part, Q3ListViewItem *parent, QString name, FunctionDom dom);
    ~FunctionItem();
    FunctionDom dom() const;
    virtual void setup();
private:
    FunctionDom m_dom;
    ClassViewPart *m_part;
};

namespace ViewCombosOp{

enum ProcessType {Refresh, Reload};

void refreshNamespaces(ClassViewPart *part, KComboView *view);
void refreshClasses(ClassViewPart *part, KComboView *view, const QString &dom);
void refreshFunctions(ClassViewPart *part, KComboView *view, const ClassDom & dom);
void refreshFunctions(ClassViewPart *part, KComboView *view, const QString & dom);

NamespaceDom namespaceByName(NamespaceDom dom, QString name);

}

#endif
