#include "applychangeswidget.h"
#include <ktexteditor/document.h>
// #include <ktexteditor/editor.h>
// #include <ktexteditor/editorchooser.h>
#include <ktexteditor/view.h>

#include <kparts/part.h>
#include <KMimeType>
#include <KMimeTypeTrader>
#include <QHBoxLayout>
#include <QDebug>
#include <QStandardItemModel>
#include <QTreeView>

ApplyChangesWidget::ApplyChangesWidget(const KUrl& url, QWidget* parent)
	: QWidget(parent), m_itemChange(0), m_itemRemove(0), m_itemInsert(0)
{
	QHBoxLayout *v=new QHBoxLayout(this);
	KMimeType::Ptr mimetype = KMimeType::findByUrl( url, 0, true );
	
	m_part = KMimeTypeTrader::self()->createPartInstanceFromQuery<KParts::ReadWritePart>(
		mimetype->name(), this, this);
	m_part->widget()->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	
	if(m_part)
		m_part->openUrl(url);
	
	m_changes = new QStandardItemModel(this);
	m_changes->setHorizontalHeaderLabels(QStringList(i18n("Text")) << i18n("Action"));
	v->addWidget(m_part->widget());
	
	QTreeView *changesView=new QTreeView(this);
	changesView->setModel(m_changes);
	changesView->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
	v->addWidget(changesView);
	
	connect(m_part, SIGNAL(textChanged(KTextEditor::Document*, KTextEditor::Range, KTextEditor::Range)),
			this, SLOT(change (KTextEditor::Document*, KTextEditor::Range, KTextEditor::Range)));
	connect(m_part, SIGNAL(textInserted(KTextEditor::Document*, KTextEditor::Range)),
			this, SLOT(insertion (KTextEditor::Document*, KTextEditor::Range)));
	connect(m_part, SIGNAL(textRemoved(KTextEditor::Document*, KTextEditor::Range)),
			this, SLOT(removal (KTextEditor::Document*, KTextEditor::Range)));
	connect(changesView, SIGNAL(activated(QModelIndex)),
			this, SLOT(jump(QModelIndex)));
}

KTextEditor::Document* ApplyChangesWidget::document() const
{
	return qobject_cast<KTextEditor::Document*>(m_part);
}

Q_DECLARE_METATYPE(KTextEditor::Range)
void addItem(QStandardItemModel* mit, KTextEditor::Document *document, const KTextEditor::Range &range, const QString& type)
{
	QStringList edition=document->textLines(range);
	if(edition.first().isEmpty())
		edition.takeFirst();
	QStandardItem* it= new QStandardItem(edition.join("\n"));
	QStandardItem* action= new QStandardItem(type);
	
	it->setData(qVariantFromValue(range));
	it->setEditable(false);
	action->setEditable(false);
	mit->appendRow(QList<QStandardItem*>() << it << action);
}

void ApplyChangesWidget::jump( const QModelIndex & idx)
{
	QStandardItem *it=m_changes->itemFromIndex(idx);
	KTextEditor::View* view=qobject_cast<KTextEditor::View*>(m_part->widget());
	KTextEditor::Range r=it->data().value<KTextEditor::Range>();
	view->setSelection(r);
	view->setCursorPosition(r.start());
}

void ApplyChangesWidget::change (KTextEditor::Document *document, const KTextEditor::Range &oldRange,
				const KTextEditor::Range &newRange)
{
	addItem(m_changes, document, newRange, i18n("Change"));
}

void ApplyChangesWidget::insertion(KTextEditor::Document *document, const KTextEditor::Range &range)
{
	addItem(m_changes, document, range, i18n("Insert"));
}

void ApplyChangesWidget::removal(KTextEditor::Document *document, const KTextEditor::Range &range)
{
	addItem(m_changes, document, range, i18n("Remove"));
}

#include "applychangeswidget.moc"
