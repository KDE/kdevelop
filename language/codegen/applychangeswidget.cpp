/* Copyright 2008 Aleix Pol <aleixpol@gmail.com>
 * Copyright 2009 Ramón Zarazúa <killerfox512+kde@gmail.com>
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

#include "komparesupport.h"
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <kparts/part.h>

#include <KTabWidget>
#include <KMimeType>
#include <KMimeTypeTrader>
#include <QBoxLayout>
#include <QLayout>
#include <QSplitter>
#include <QLabel>
#include <QStandardItemModel>
#include <QTreeView>
#include <QDebug>
#include <KPushButton>
#include "coderepresentation.h"
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <KTemporaryFile>
#include <KActionCollection>
#include <QAction>

namespace KDevelop
{
    
class ApplyChangesWidgetPrivate
{
public:

    ApplyChangesWidgetPrivate(ApplyChangesWidget * p)
        : parent(p), m_index(0) {}
    ~ApplyChangesWidgetPrivate()
    {
        qDeleteAll(m_temps);
    }
    
    void addItem(QStandardItemModel* mit, KTextEditor::Document *document, const KTextEditor::Range &range, const QString& type, const QString& removedText = QString());
    void jump( const QModelIndex & idx);
    void createEditPart(const KDevelop::IndexedString& url);
    void updateButtonLabel();

    
    ApplyChangesWidget * const parent;
    int m_index;
    QList<KParts::ReadWritePart*> m_editParts;
    QList<QStandardItemModel*> m_changes;
    QList<KTemporaryFile * > m_temps;
    QList<IndexedString > m_files;
    KTabWidget * m_documentTabs;
    QLabel* m_info;
    
    KompareWidgets m_kompare;
};

ApplyChangesWidget::ApplyChangesWidget(QWidget* parent)
    : KDialog(parent), d(new ApplyChangesWidgetPrivate(this))
{
    setSizeGripEnabled(true);
    setInitialSize(QSize(800, 400));
    
    KDialog::setButtons(KDialog::Ok | KDialog::Cancel | KDialog::User1);
    KPushButton * switchButton(KDialog::button(KDialog::User1));
    switchButton->setText(i18n("Edit Document"));
    switchButton->setEnabled(d->m_kompare.enabled);
    
    connect(switchButton, SIGNAL(released()),
            this, SLOT(switchEditView()));
    
    QWidget* w=new QWidget(this);
    d->m_info=new QLabel(w);
    d->m_documentTabs = new KTabWidget(w);
    connect(d->m_documentTabs, SIGNAL(currentChanged(int)),
            this, SLOT(indexChanged(int)));
            
    QVBoxLayout* l = new QVBoxLayout(w);
    l->addWidget(d->m_info);
    l->addWidget(d->m_documentTabs);
    
    setMainWidget(w);
}

ApplyChangesWidget::~ApplyChangesWidget()
{
    delete d;
}

bool ApplyChangesWidget::hasDocuments() const
{
    return d->m_editParts.size() > 0;
}

KTextEditor::Document* ApplyChangesWidget::document() const
{
    return qobject_cast<KTextEditor::Document*>(d->m_editParts[d->m_index]);
}

void ApplyChangesWidget::setInformation(const QString & info)
{
    d->m_info->setText(info);
}

void ApplyChangesWidget::addDocuments(const IndexedString & original)
{
    int idx=d->m_files.indexOf(original);
    if(idx<0) {
        QWidget * w = new QWidget;
        d->m_documentTabs->addTab(w, original.str());
        d->m_documentTabs->setCurrentWidget(w);

        
        d->m_files.insert(d->m_index, original);
        d->createEditPart(original);
    } else {
        d->m_index=idx;
    }
    switchEditView();
}

bool ApplyChangesWidget::applyAllChanges()
{
    /// @todo implement safeguard in case a file saving fails
    
    bool ret = true;
    for(int i = 0; i < d->m_files.size(); ++i )
        if(d->m_editParts[i]->saveAs(d->m_files[i].toUrl())) {
            IDocument* doc = ICore::self()->documentController()->documentForUrl(d->m_files[i].toUrl());
            if(doc && doc->state()==IDocument::Dirty)
                doc->reload();
        } else
            ret = false;
        
    return ret;
}

}

Q_DECLARE_METATYPE(KTextEditor::Range)

namespace KDevelop
{

void ApplyChangesWidgetPrivate::addItem(QStandardItemModel* mit, KTextEditor::Document *document, const KTextEditor::Range &range, const QString& type, const QString& removedText)
{
    bool isFirst=mit->rowCount()==0;
    QStringList edition=document->textLines(range);
    if(edition.first().isEmpty())
        edition.removeFirst();
    QStandardItem* it= new QStandardItem(edition.join("\n").append(removedText));
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
    Q_ASSERT( m_index == m_documentTabs->currentIndex());
    
    QStandardItem *it=m_changes[m_index]->itemFromIndex(idx);
    KTextEditor::View* view=qobject_cast<KTextEditor::View*>(m_editParts[m_index]->widget());
    KTextEditor::Range r=it->data().value<KTextEditor::Range>();
    view->setSelection(r);
    view->setCursorPosition(r.start());
}

void ApplyChangesWidgetPrivate::updateButtonLabel()
{
    KPushButton * switchButton(parent->button(KDialog::User1));
    
    if(m_kompare.widgetActive(m_index))
        switchButton->setText(i18n("Edit Document"));
    else
        switchButton->setText(i18n("View Differences"));
}

void ApplyChangesWidgetPrivate::createEditPart(const IndexedString & file)
{
    QWidget * widget = m_documentTabs->currentWidget();
    Q_ASSERT(widget);
    
    QVBoxLayout *m=new QVBoxLayout(widget);
    QSplitter *v=new QSplitter(widget);
    m->addWidget(v);
    
    KUrl url = file.toUrl();
    
    KMimeType::Ptr mimetype = KMimeType::findByUrl( url, 0, true );
    
    KParts::ReadWritePart* part=KMimeTypeTrader::self()->createPartInstanceFromQuery<KParts::ReadWritePart>(mimetype->name(), widget, widget);
    KTextEditor::Document* document=qobject_cast<KTextEditor::Document*>(part);
    Q_ASSERT(document);
    
    Q_ASSERT(document->action("file_save"));
    document->action("file_save")->setEnabled(false);
    
    m_editParts.insert(m_index, part);
    
    //Open the best code representation, even if it is artificial
    CodeRepresentation::Ptr repr = createCodeRepresentation(file);
    if(!repr->fileExists())
    {
        KTemporaryFile * temp(new KTemporaryFile);
        temp->setSuffix(url.fileName().split('.').last());
        temp->open();
        temp->write(repr->text().toUtf8());
        temp->close();
        
        url = temp->fileName();
        
        m_temps << temp;
    }
    m_editParts[m_index]->openUrl(url);
    
    m_changes.insert(m_index, new QStandardItemModel(widget));
    m_changes[m_index]->setHorizontalHeaderLabels(QStringList(i18n("Text")) << i18n("Action"));
    
    QTreeView *changesView=new QTreeView(widget);
    changesView->setRootIsDecorated(false);
    changesView->setModel(m_changes[m_index]);
    v->addWidget(m_editParts[m_index]->widget());
    v->addWidget(changesView);
    v->setSizes(QList<int>() << 400 << 100);
    
    QObject::connect(m_editParts[m_index], SIGNAL(textChanged(KTextEditor::Document*,KTextEditor::Range,KTextEditor::Range)),
            parent, SLOT(change(KTextEditor::Document*,KTextEditor::Range,KTextEditor::Range)));
    QObject::connect(m_editParts[m_index], SIGNAL(textInserted(KTextEditor::Document*,KTextEditor::Range)),
            parent, SLOT(insertion(KTextEditor::Document*,KTextEditor::Range)));
    QObject::connect(m_editParts[m_index], SIGNAL(textRemoved(KTextEditor::Document*,KTextEditor::Range,QString)),
            parent, SLOT(removal(KTextEditor::Document*,KTextEditor::Range,QString)));
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

void ApplyChangesWidget::removal(KTextEditor::Document *document, const KTextEditor::Range &range, const QString &oldText)
{
    d->addItem(d->m_changes[d->m_index], document, KTextEditor::Range(range.start(), range.start()), i18n("Remove"), oldText);
}

void ApplyChangesWidget::switchEditView()
{
    if(d->m_kompare.widgetActive(d->m_index))
    {
        //Chage into editPart
        d->m_editParts[d->m_index]->widget()->parentWidget()->setVisible(true);
        d->m_kompare.hideWidget(d->m_index);
    }
    else
    {
        d->m_editParts[d->m_index]->widget()->parentWidget()->setVisible(false);
        //Change into KomparePart
        d->m_kompare.compare(d->m_files[d->m_index], document()->text(),
                             d->m_documentTabs->widget(d->m_index), d->m_index);
    }
    
    d->updateButtonLabel();
}

void ApplyChangesWidget::indexChanged(int newIndex)
{
    Q_ASSERT(newIndex != -1);
    d->m_index = newIndex;
    d->updateButtonLabel();
}

void ApplyChangesWidget::updateDiffView(int index)
{
    int prevIndex = d->m_index;
    d->m_index = index == -1 ? d->m_index : index;
    
    switchEditView();
    switchEditView();
    
    d->m_index = prevIndex;
    
}

}

#include "applychangeswidget.moc"
