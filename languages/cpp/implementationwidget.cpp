/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@mksat.net                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
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
#include <qlineedit.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <klistview.h>

#include <kdevproject.h>
#include <domutil.h>
#include <filetemplate.h>

#include "cppsupportpart.h"

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

ImplementationWidget::ImplementationWidget(CppSupportPart *part, const QString &formName, QWidget* parent, const char* name, bool modal)
    :CreateImplemenationWidgetBase(parent, name, modal), m_part(part), m_formName(formName)
{
    QDomDocument doc;
    DomUtil::openDOMFile(doc, m_formName);
    m_baseClassName = DomUtil::elementByPathExt(doc, "class").text();
    setCaption(i18n("Create or Select Implementation Class for: %1").arg(m_baseClassName));
    
    KListViewItem *item = new KListViewItem(classView, i18n("Namespaces and Classes"));
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
    QString template_h = "#include \"$BASEINCLUDE$\"\n\nclass $CLASSNAME$: public $BASECLASSNAME$ {\nQ_OBJECT\npublic:\n    $CLASSNAME$(QWidget *parent = 0, const char *name = 0);\n};\n";
    QString template_cpp = "#include \"$CLASSINCLUDE$\"\n\n$CLASSNAME$::$CLASSNAME$(QWidget *parent, const char *name)\n    :$BASECLASSNAME$(parent, name)\n{\n}\n";
    if (m_part->project()->options() == KDevProject::UsesAutotoolsBuildSystem)
        template_cpp += "\n#include \"$MOCINCLUDE$\"\n";
    
    QFileInfo formInfo(m_formName);
    template_h.replace(QRegExp("\\$BASEINCLUDE\\$"), formInfo.baseName()+".h");
    template_h.replace(QRegExp("\\$CLASSNAME\\$"), classNameEdit->text());
    template_h.replace(QRegExp("\\$BASECLASSNAME\\$"), m_baseClassName);
    
    template_cpp.replace(QRegExp("\\$CLASSINCLUDE\\$"), fileNameEdit->text() + ".h");
    template_cpp.replace(QRegExp("\\$CLASSNAME\\$"), classNameEdit->text());
    template_cpp.replace(QRegExp("\\$BASECLASSNAME\\$"), m_baseClassName);
    template_cpp.replace(QRegExp("\\$MOCINCLUDE\\$"), fileNameEdit->text() + ".moc");
    
    template_h = FileTemplate::read(m_part, "h") + template_h;
    template_cpp = FileTemplate::read(m_part, "cpp") + template_cpp;
    
    QString file_h = fileNameEdit->text() + ".h";
    QString file_cpp = fileNameEdit->text() + ".cpp";
    if (!m_part->project()->activeDirectory().isEmpty())
    {
        file_h = m_part->project()->activeDirectory() + "/" + file_h;
        file_cpp = m_part->project()->activeDirectory() + "/" + file_cpp;
    }

    QFile ifile(QDir::cleanDirPath(m_part->project()->projectDirectory() + "/" + file_cpp));
    if (!ifile.open(IO_WriteOnly)) {
        KMessageBox::error(0, i18n("Cannot write to implementation file"));
        return false;
    }
    QTextStream istream(&ifile);
    istream << template_cpp;
    ifile.close();
  
    QFile hfile(QDir::cleanDirPath(m_part->project()->projectDirectory() + "/" + file_h));
    if (!hfile.open(IO_WriteOnly)) {
        KMessageBox::error(0, i18n("Cannot write to header file"));
        return false;
    }
    QTextStream hstream(&hfile);
    hstream << template_h;
    hfile.close();
    
    QStringList fileList;
    fileList.append(file_h);
    fileList.append(file_cpp);
    m_part->project()->addFiles(fileList);
}

#include "implementationwidget.moc"

