/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "hierarchydlg.h"

#include <kdialog.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <kfile.h>
#include <kfiledialog.h>
#include <kurlrequesterdlg.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <qlayout.h>
#include <qfileinfo.h>
#include <qlistview.h>
#include <qapplication.h>
#include <qsplitter.h>

#include "kdevlanguagesupport.h"
#include "kcomboview.h"

#include "classviewpart.h"
//#include "classtoolwidget.h"
#include "digraphview.h"
#include "viewcombos.h"


HierarchyDialog::HierarchyDialog( ClassViewPart *part )
    : QDialog(0, "hierarchy dialog", false)
{
    class_combo = new KComboView(true, 150, this);
    class_combo->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
//    class_combo->setMinimumWidth(150);
    namespace_combo = new KComboView(true, 150, this);
    namespace_combo->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
//    namespace_combo->setMinimumWidth(150);

    QPushButton *close_button = new KPushButton(KStdGuiItem::close(), this);
    QPushButton *save_button = new KPushButton(KStdGuiItem::save(), this);
    QPushButton *refresh_button = new KPushButton(i18n("Refresh"), this);

    QSplitter *splitter = new QSplitter(Vertical, this);
    digraph = new DigraphView(splitter, "digraph view");
//    member_tree = new ClassToolWidget(part, splitter);

    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    QBoxLayout *combo_layout = new QHBoxLayout();
    layout->addLayout(combo_layout);
    combo_layout->addWidget(namespace_combo);
    combo_layout->addWidget(class_combo);
    combo_layout->addSpacing(60);
    combo_layout->addWidget(refresh_button);
    combo_layout->addWidget(save_button);
    combo_layout->addWidget(close_button);
    layout->addWidget(splitter);

    connect( namespace_combo, SIGNAL(activated(QListViewItem*)),
             this, SLOT(slotNamespaceComboChoice(QListViewItem*)) );
    connect( class_combo, SIGNAL(activated(QListViewItem*)),
             this, SLOT(slotClassComboChoice(QListViewItem*)) );
    connect( namespace_combo, SIGNAL(textChanged(const QString&)),
             this, SLOT(slotNamespaceComboChoice(const QString&)) );
    connect( class_combo, SIGNAL(textChanged(const QString&)),
             this, SLOT(slotClassComboChoice(const QString&)) );
    connect( close_button, SIGNAL(clicked()),
             this, SLOT(hide()) );
    connect( save_button, SIGNAL(clicked()),
             this, SLOT(save()) );
    connect( refresh_button, SIGNAL(clicked()),
             this, SLOT(refresh()) );
    connect( digraph, SIGNAL(selected(const QString&)),
             this, SLOT(classSelected(const QString&)) );

    m_part = part;
//    m_part->registerHierarchyDialog(this);
    refresh();
}


HierarchyDialog::~HierarchyDialog()
{
//    m_part->unregisterHierarchyDialog(this);
}

void HierarchyDialog::save()
{
    KURLRequesterDlg dlg(QString::null, this, "save_inheritance");
    dlg.fileDialog()->setFilter("image/png image/jpeg image/bmp image/svg+xml");
    dlg.fileDialog()->setOperationMode( KFileDialog::Saving );
    dlg.fileDialog()->setMode( KFile::File | KFile::LocalOnly );
    dlg.urlRequester()->setMode( KFile::File | KFile::LocalOnly );
    if(dlg.exec() && dlg.selectedURL().isLocalFile())
    {
	QFileInfo fi(dlg.selectedURL().pathOrURL());
        QApplication::setOverrideCursor( Qt::waitCursor );
        KDevLanguageSupport *ls = m_part->languageSupport();
        for (QMap<QString, ClassDom>::const_iterator it = classes.begin(); it != classes.end(); ++it)
        {
            kdDebug(9003) << "Adding class to graph: " << it.key() << endl;
            QString formattedName = ls->formatClassName(it.key());
            QStringList baseClasses = it.data()->baseClassList();
            for (QStringList::const_iterator bit = baseClasses.begin(); bit != baseClasses.end(); ++bit)
            {
                QMap<QString, QString>::const_iterator baseIt = uclasses.find(*bit);
                if (baseIt != uclasses.end())
                {
                    QString formattedParentName = ls->formatClassName(baseIt.data());
                    digraph->addEdge(formattedParentName, formattedName);
                }
            }
        }
        digraph->process(fi.absFilePath(), fi.extension());
        QApplication::restoreOverrideCursor();
    }
}

void HierarchyDialog::refresh()
{
    digraph->clear();
    classes.clear();
    uclasses.clear();
    ViewCombosOp::refreshNamespaces(m_part, namespace_combo);
    processNamespace("", m_part->codeModel()->globalNamespace());

    KDevLanguageSupport *ls = m_part->languageSupport();

    for (QMap<QString, ClassDom>::const_iterator it = classes.begin(); it != classes.end(); ++it)
    {
        kdDebug(9003) << "Adding class to graph: " << it.key() << endl;
        QString formattedName = ls->formatClassName(it.key());
        QStringList baseClasses = it.data()->baseClassList();
        for (QStringList::const_iterator bit = baseClasses.begin(); bit != baseClasses.end(); ++bit)
        {
            QMap<QString, QString>::const_iterator baseIt = uclasses.find(*bit);
            if (baseIt != uclasses.end())
            {
                QString formattedParentName = ls->formatClassName(baseIt.data());
                digraph->addEdge(formattedParentName, formattedName);
            }
        }
    }
    digraph->process();
}

void HierarchyDialog::setLanguageSupport(KDevLanguageSupport *ls)
{
    if (ls)
        connect(ls, SIGNAL(updatedSourceInfo()), this, SLOT(refresh()));
    else
        refresh();
}


void HierarchyDialog::slotClassComboChoice(QListViewItem * item)
{
    ClassItem *ci = dynamic_cast<ClassItem*>(item);
    if (!ci)
        return;

    KDevLanguageSupport *ls = m_part->languageSupport();

    QString className = ls->formatClassName(uclasses[item->text(0)]);
    digraph->setSelected(className);
    digraph->ensureVisible(className);
    classSelected(className);
}

void HierarchyDialog::slotClassComboChoice( const QString& itemText )
{
    QListViewItem* item = class_combo->listView()->firstChild();
    while( item )
    {
        if( item->text(0) == itemText )
	{
            ClassItem *ci = dynamic_cast<ClassItem*>(item);
            if (!ci)
                return;
        
            KDevLanguageSupport *ls = m_part->languageSupport();
        
            QString className = ls->formatClassName(uclasses[item->text(0)]);
            digraph->setSelected(className);
            digraph->ensureVisible(className);
            classSelected(className);
      	    return;
	}
	item = item->nextSibling();
    }
}

void HierarchyDialog::classSelected(const QString &/*className*/)
{
/*    ParsedClass *currentClass = m_part->classStore()->getClassByName(className);
    member_tree->clear();
    if (currentClass) {
        KDevLanguageSupport::Features features = m_part->languageSupport()->features();
        if (features & KDevLanguageSupport::Functions)
            member_tree->insertAllClassMethods(currentClass, (PIAccess)-1);
        if (features & KDevLanguageSupport::Variables)
            member_tree->insertAllClassAttributes(currentClass, (PIAccess)-1);
    }*/
}

void HierarchyDialog::slotNamespaceComboChoice( QListViewItem * item )
{
    NamespaceItem *ni = dynamic_cast<NamespaceItem*>(item);
    if (!ni)
        return;
    ViewCombosOp::refreshClasses(m_part, class_combo, ni->dom()->name());
}

void HierarchyDialog::slotNamespaceComboChoice( const QString& itemText )
{
    QListViewItem* item = namespace_combo->listView()->firstChild();
    while( item )
    {
        if( item->text(0) == itemText )
	{
            NamespaceItem *ni = dynamic_cast<NamespaceItem*>(item);
            if (!ni)
                return;
            ViewCombosOp::refreshClasses(m_part, class_combo, ni->dom()->name());
	    return;
	}
	item = item->nextSibling();
    }
}

void HierarchyDialog::processNamespace( QString prefix, NamespaceDom dom )
{
    qWarning("processNamespace: prefix %s", prefix.latin1());
    QString prefixInc = prefix.isEmpty() ? "" : ".";
//    QString nsprefix = dom->name().isEmpty() ? QString("") : prefixInc + dom->name();

    NamespaceList namespaceList = dom->namespaceList();
    for (NamespaceList::const_iterator it = namespaceList.begin(); it != namespaceList.end(); ++it)
    {
        qWarning("about to processNamespace: prefix %s", (prefixInc + (*it)->name()).latin1());
        processNamespace(prefixInc + (*it)->name(), *it);
    }

    ClassList classList = dom->classList();
    for (ClassList::const_iterator it = classList.begin(); it != classList.end(); ++it)
    {
        processClass(prefix, *it);
    }
}

void HierarchyDialog::processClass( QString prefix, ClassDom dom )
{
    qWarning("processClass: prefix %s class %s", prefix.latin1(), dom->name().latin1());

    QString prefixInc = prefix.isEmpty() ? "" : ".";
    classes[prefix + prefixInc + dom->name()] = dom;
    uclasses[dom->name()] = prefix + prefixInc + dom->name();

    ClassList classList = dom->classList();
    for (ClassList::const_iterator it = classList.begin(); it != classList.end(); ++it)
    {
        processClass(prefix + prefixInc + dom->name(), *it);
    }
}

#include "hierarchydlg.moc"
