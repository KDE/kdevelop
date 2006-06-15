/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "implementationwidget.h"

#include <qfileinfo.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qdir.h>
#include <qregexp.h>
#include <qdom.h>
#include <qradiobutton.h>

#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <klistview.h>

#include <kdevproject.h>
#include <domutil.h>
#include <filetemplate.h>
#include <kdevlanguagesupport.h>

namespace ImplUtils{
class ClassItem: public KListViewItem{
public:
    ClassItem(KListViewItem *parent, ClassDom dom)
        :KListViewItem(parent, dom->name(), dom->fileName()), m_dom(dom) { setOpen(true); }
    ClassItem(KListView *parent, ClassDom dom)
        :KListViewItem(parent, dom->name(), dom->fileName()), m_dom(dom) { setOpen(true); }
    ClassDom dom() const { return m_dom; }
private:
    ClassDom m_dom;
};

class NamespaceItem: public KListViewItem{
public:
    NamespaceItem(KListViewItem *parent, NamespaceDom dom)
        :KListViewItem(parent, dom->name(), dom->fileName()), m_dom(dom) { setOpen(true); }
    NamespaceItem(KListView *parent, NamespaceDom dom)
        :KListViewItem(parent, dom->name(), dom->fileName()), m_dom(dom) { setOpen(true); }
    NamespaceDom dom() const { return m_dom; }
private:
    NamespaceDom m_dom;
};
}

ImplementationWidget::ImplementationWidget(KDevLanguageSupport *part, QWidget* parent, const char* name, bool modal)
    :CreateImplemenationWidgetBase(parent, name, modal), m_part(part)
{
}

void ImplementationWidget::init(const QString &formName)
{
    m_formName = formName;
    
    classView->clear();
    fileNameEdit->clear();
    classNameEdit->clear();
    
    QDomDocument doc;
    DomUtil::openDOMFile(doc, m_formName);
    m_baseClassName = DomUtil::elementByPathExt(doc, "class").text();
    setCaption(i18n("Create or Select Implementation Class for: %1").arg(m_baseClassName));
    
    KListViewItem *item = new KListViewItem(classView, i18n("Namespaces &amp;&amp; Classes"));
    item->setOpen(true);
    processNamespaces(m_part->codeModel()->globalNamespace(), item);
}

void ImplementationWidget::processNamespaces(NamespaceDom dom, KListViewItem *parent)
{
    const NamespaceList nslist = dom->namespaceList();
    for (NamespaceList::const_iterator it = nslist.begin(); it != nslist.end(); ++it)
        processNamespaces(*it, new ImplUtils::NamespaceItem(parent, *it));
    const ClassList cllist = dom->classList();
    for (ClassList::ConstIterator it = cllist.begin(); it != cllist.end(); ++it)
        processClasses(*it, new ImplUtils::ClassItem(parent, *it));    
}

void ImplementationWidget::processClasses(ClassDom dom, KListViewItem *parent)
{
    const ClassList cllist = dom->classList();
    for (ClassList::ConstIterator it = cllist.begin(); it != cllist.end(); ++it)
        processClasses(*it, new ImplUtils::ClassItem(parent, *it));    
}

ImplementationWidget::~ImplementationWidget()
{
}

/*$SPECIALIZATION$*/
void ImplementationWidget::classNameChanged(const QString &text)
{
    fileNameEdit->setText(text.lower());
}

void ImplementationWidget::accept()
{
    if (createButton->isOn())
    {
        if (classNameEdit->text().isEmpty())
            return;
        if (!createClass())
            return;
        ClassList cllist = m_part->codeModel()->globalNamespace()->classByName(classNameEdit->text());
        if (cllist.count() > 0)
            m_selectedClass = cllist.first();
        else
            KMessageBox::error(0, i18n("Class was created but not found in class store."));
    }
    else if (useButton->isOn())
    {
        if (!classView->currentItem())
            return;
        ImplUtils::ClassItem *item = dynamic_cast<ImplUtils::ClassItem*>(classView->currentItem());
        if (!item)
            return;
        m_selectedClass = item->dom();
    }
    QDialog::accept();
}

ClassDom ImplementationWidget::selectedClass()
{
    return m_selectedClass;
}

bool ImplementationWidget::createClass()
{   
    m_part->project()->addFiles(createClassFiles());
    return true;
}

int ImplementationWidget::exec(const QString &formName)
{
    init(formName);
    return QDialog::exec();
}

#include "implementationwidget.moc"

