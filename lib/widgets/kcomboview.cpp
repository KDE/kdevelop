#include <klineedit.h>
#include <qlistview.h>
#include <kcompletionbox.h>

#include "kcomboview.h"

KComboView::KComboView( bool rw, int defaultWidth, QWidget* parent, const char* name )
    :QComboView(rw, parent, name), m_defaultWidth(defaultWidth)
{
    if (rw)
    {
        KLineEdit *ed = new KLineEdit(this, "combo edit");
        ed->setCompletionMode(KGlobalSettings::CompletionPopup);
        ed->setCompletionObject(&m_comp);
        ed->completionBox()->setHScrollBarMode(QListBox::Auto);
        setLineEdit(ed);
    }
    setMinimumWidth(defaultWidth);
}

void KComboView::addItem(QListViewItem *it)
{
    m_comp.addItem(it->text(0));
}

void KComboView::removeItem(QListViewItem *it)
{
    m_comp.removeItem(it->text(0));
    delete it;
}

void KComboView::renameItem(QListViewItem *it, const QString &newName)
{
    m_comp.removeItem(it->text(0));
    it->setText(0, newName);
    m_comp.addItem(newName);
}

void KComboView::clear( )
{
    m_comp.clear();
    QComboView::clear();
}

int KComboView::defaultWidth( )
{
    return m_defaultWidth;
}
