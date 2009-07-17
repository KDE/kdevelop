/* Copyright 2008 Aleix Pol <aleixpol@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "applychangeswidget.h"
#include <ktexteditor/document.h>
// #include <ktexteditor/editor.h>
// #include <ktexteditor/editorchooser.h>
#include <ktexteditor/view.h>

#include <kparts/part.h>
//#include <kompare/kompareinterface.h>
#include <KTabWidget>
#include <KMimeType>
#include <KMimeTypeTrader>
#include <QLayout>
#include <QPushButton>
#include <QSplitter>
#include <QLabel>
#include <QStandardItemModel>
#include <QTreeView>
#include <QDebug>

namespace KDevelop
{
    
struct ApplyChangesWidgetPrivate
{
    ApplyChangesWidgetPrivate(ApplyChangesWidget * p)
        : parent(p), m_index(0) {}
    
    void addItem(QStandardItemModel* mit, KTextEditor::Document *document, const KTextEditor::Range &range, const QString& type);
    void jump( const QModelIndex & idx);
    void createEditPart(const KUrl & url, const QString & info);

    
    ApplyChangesWidget * const parent;
    unsigned int m_index;
    QList<KParts::ReadWritePart*> m_editParts;
    KTabWidget * m_documentTabs;
    
    QList<QStandardItemModel*> m_changes;
};

ApplyChangesWidget::ApplyChangesWidget(const QString& info, const KUrl& url, QWidget* parent)
    : KDialog(parent), d(new ApplyChangesWidgetPrivate(this))
{
    setSizeGripEnabled(true);
    setInitialSize(QSize(800, 400));
    
    d->m_documentTabs = new KTabWidget(this);
    QWidget * w = new QWidget;
    d->m_documentTabs->addTab(w, url.fileName());
   
    
    QPushButton * switchButton = new QPushButton("Edit Document", this);
    switchButton->setEnabled(false);
    switchButton->hide();
    
    d->createEditPart(url, info);
    
    connect(switchButton, SIGNAL(released()),
            this, SLOT(switchEditView()));
    connect(d->m_documentTabs, SIGNAL(currentChanged(int)),
            this, SLOT(indexChanged(int)));
    
    setMainWidget(d->m_documentTabs);
}

ApplyChangesWidget::~ApplyChangesWidget()
{
    delete d;
}

KTextEditor::Document* ApplyChangesWidget::document() const
{
    return qobject_cast<KTextEditor::Document*>(d->m_editParts[d->m_index]);
}

}

Q_DECLARE_METATYPE(KTextEditor::Range)

namespace KDevelop
{

void ApplyChangesWidgetPrivate::addItem(QStandardItemModel* mit, KTextEditor::Document *document, const KTextEditor::Range &range, const QString& type)
{
    bool isFirst=mit->rowCount()==0;
    QStringList edition=document->textLines(range);
    if(edition.first().isEmpty())
        edition.takeFirst();
    QStandardItem* it= new QStandardItem(edition.join("\n"));
    QStandardItem* action= new QStandardItem(type);
    
    it->setData(qVariantFromValue(range));
    it->setEditable(false);
    action->setEditable(false);
    mit->appendRow(QList<QStandardItem*>() << it << action);
    if(isFirst)
        jump(it->index());
}

void ApplyChangesWidget::jump( const QModelIndex & idx)
{
    d->jump(idx);
}

void ApplyChangesWidgetPrivate::jump( const QModelIndex & idx)
{
    Q_ASSERT( static_cast<int>(m_index) == m_documentTabs->currentIndex());
    
    QStandardItem *it=m_changes[m_index]->itemFromIndex(idx);
    KTextEditor::View* view=qobject_cast<KTextEditor::View*>(m_editParts[m_index]->widget());
    KTextEditor::Range r=it->data().value<KTextEditor::Range>();
    view->setSelection(r);
    view->setCursorPosition(r.start());
}

void ApplyChangesWidgetPrivate::createEditPart(const KUrl & url, const QString & info)
{
    QWidget * widget = m_documentTabs->currentWidget();
    Q_ASSERT(widget);
    
    QVBoxLayout *m=new QVBoxLayout(widget);
    QSplitter *v=new QSplitter(widget);
    
    KMimeType::Ptr mimetype = KMimeType::findByUrl( url, 0, true );
    
    m_editParts.insert(m_index, KMimeTypeTrader::self()->createPartInstanceFromQuery<KParts::ReadWritePart>(mimetype->name(), widget, widget));
    m_editParts[m_index]->openUrl(url);
    
    m_changes.insert(m_index, new QStandardItemModel(widget));
    m_changes[m_index]->setHorizontalHeaderLabels(QStringList(i18n("Text")) << i18n("Action"));
    
    QTreeView *changesView=new QTreeView(widget);
    changesView->setRootIsDecorated(false);
    changesView->setModel(m_changes[m_index]);
    v->addWidget(m_editParts[m_index]->widget());
    v->addWidget(changesView);
    v->setSizes(QList<int>() << 400 << 100);
    
    QLabel* l=new QLabel(info, widget);
    l->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    m->addWidget(l);
    m->addWidget(v);
    
    QObject::connect(m_editParts[m_index], SIGNAL(textChanged(KTextEditor::Document*, KTextEditor::Range, KTextEditor::Range)),
            parent, SLOT(change (KTextEditor::Document*, KTextEditor::Range, KTextEditor::Range)));
    QObject::connect(m_editParts[m_index], SIGNAL(textInserted(KTextEditor::Document*, KTextEditor::Range)),
            parent, SLOT(insertion (KTextEditor::Document*, KTextEditor::Range)));
    QObject::connect(m_editParts[m_index], SIGNAL(textRemoved(KTextEditor::Document*, KTextEditor::Range)),
            parent, SLOT(removal (KTextEditor::Document*, KTextEditor::Range)));
    QObject::connect(changesView, SIGNAL(activated(QModelIndex)),
            parent, SLOT(jump(QModelIndex)));
}

void ApplyChangesWidget::change (KTextEditor::Document *document, const KTextEditor::Range &,
                const KTextEditor::Range &newRange)
{
    d->addItem(d->m_changes[d->m_index], document, newRange, i18n("Change"));
}

void ApplyChangesWidget::insertion(KTextEditor::Document *document, const KTextEditor::Range &range)
{
    d->addItem(d->m_changes[d->m_index], document, range, i18n("Insert"));
}

void ApplyChangesWidget::removal(KTextEditor::Document *document, const KTextEditor::Range &range)
{
    d->addItem(d->m_changes[d->m_index], document, range, i18n("Remove"));
}

void ApplyChangesWidget::indexChanged(int newIndex)
{
    Q_ASSERT(newIndex != -1);
    d->m_index = newIndex;
}

}

#include "applychangeswidget.moc"
