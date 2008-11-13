#ifndef APPLYCHANGESWIDGET_H
#define APPLYCHANGESWIDGET_H

#include <QWidget>
#include <QList>

#include <ktexteditor/range.h>

namespace KParts { class ReadWritePart; }
namespace KTextEditor { class Document; }
class KUrl;
class QStandardItem;
class QStandardItemModel;
class QModelIndex;

class ApplyChangesWidget : public QWidget
{
	Q_OBJECT
	public:
		ApplyChangesWidget(const KUrl& url, QWidget* parent=0);
		QSize sizeHint() const { return QSize(200, 200); }
		KTextEditor::Document* document() const;
	
	private Q_SLOTS:
		void change (KTextEditor::Document *document, const KTextEditor::Range &oldRange,
				const KTextEditor::Range &newRange);
		void insertion (KTextEditor::Document *document, const KTextEditor::Range &range);
		void removal (KTextEditor::Document *document, const KTextEditor::Range &range);
		void jump( const QModelIndex &);
	private:
		KParts::ReadWritePart* m_part;
		QStandardItemModel* m_changes;
		QStandardItem *m_itemChange;
		QStandardItem *m_itemRemove;
		QStandardItem *m_itemInsert;
};

#endif
