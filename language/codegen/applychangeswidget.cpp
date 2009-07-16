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
#include <KMimeType>
#include <KMimeTypeTrader>
#include <QLayout>
#include <QSplitter>
#include <QLabel>
#include <QStandardItemModel>
#include <QTreeView>
#include <QDebug>

namespace KDevelop
{
    
struct ApplyChangesWidgetPrivate
{
    ApplyChangesWidgetPrivate()
        : m_itemChange(0), m_itemRemove(0), m_itemInsert(0) {}
    
    void addItem(QStandardItemModel* mit, KTextEditor::Document *document, const KTextEditor::Range &range, const QString& type);
    void jump( const QModelIndex & idx);

    KParts::ReadWritePart* m_part;
    QStandardItemModel* m_changes;
    QStandardItem *m_itemChange;
    QStandardItem *m_itemRemove;
    QStandardItem *m_itemInsert;
};

ApplyChangesWidget::ApplyChangesWidget(const QString& info, const KUrl& url, QWidget* parent)
    : KDialog(parent), d(new ApplyChangesWidgetPrivate)
{
    setSizeGripEnabled(true);
    setInitialSize(QSize(800, 400));
    
    QWidget *w=new QWidget(this);
    QVBoxLayout *m=new QVBoxLayout(w);
    QSplitter *v=new QSplitter(w);
    KMimeType::Ptr mimetype = KMimeType::findByUrl( url, 0, true );
    
    d->m_part = KMimeTypeTrader::self()->createPartInstanceFromQuery<KParts::ReadWritePart>(mimetype->name(), w, w);
    d->m_part->openUrl(url);
    
    d->m_changes = new QStandardItemModel(w);
    d->m_changes->setHorizontalHeaderLabels(QStringList(i18n("Text")) << i18n("Action"));
    
    QTreeView *changesView=new QTreeView(w);
    changesView->setRootIsDecorated(false);
    changesView->setModel(d->m_changes);
    v->addWidget(d->m_part->widget());
    v->addWidget(changesView);
    v->setSizes(QList<int>() << 400 << 100);
    
    QLabel* l=new QLabel(info, w);
    l->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    m->addWidget(l);
    m->addWidget(v);
    
    setMainWidget(w);
    
    connect(d->m_part, SIGNAL(textChanged(KTextEditor::Document*, KTextEditor::Range, KTextEditor::Range)),
            this, SLOT(change (KTextEditor::Document*, KTextEditor::Range, KTextEditor::Range)));
    connect(d->m_part, SIGNAL(textInserted(KTextEditor::Document*, KTextEditor::Range)),
            this, SLOT(insertion (KTextEditor::Document*, KTextEditor::Range)));
    connect(d->m_part, SIGNAL(textRemoved(KTextEditor::Document*, KTextEditor::Range)),
            this, SLOT(removal (KTextEditor::Document*, KTextEditor::Range)));
    connect(changesView, SIGNAL(activated(QModelIndex)),
            this, SLOT(jump(QModelIndex)));
}

ApplyChangesWidget::~ApplyChangesWidget()
{
    delete d;
}

KTextEditor::Document* ApplyChangesWidget::document() const
{
    return qobject_cast<KTextEditor::Document*>(d->m_part);
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
    QStandardItem *it=m_changes->itemFromIndex(idx);
    KTextEditor::View* view=qobject_cast<KTextEditor::View*>(m_part->widget());
    KTextEditor::Range r=it->data().value<KTextEditor::Range>();
    view->setSelection(r);
    view->setCursorPosition(r.start());
}

void ApplyChangesWidget::change (KTextEditor::Document *document, const KTextEditor::Range &,
                const KTextEditor::Range &newRange)
{
    d->addItem(d->m_changes, document, newRange, i18n("Change"));
}

void ApplyChangesWidget::insertion(KTextEditor::Document *document, const KTextEditor::Range &range)
{
    d->addItem(d->m_changes, document, range, i18n("Insert"));
}

void ApplyChangesWidget::removal(KTextEditor::Document *document, const KTextEditor::Range &range)
{
    d->addItem(d->m_changes, document, range, i18n("Remove"));
}

}

#include "applychangeswidget.moc"
