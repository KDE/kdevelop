/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
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
#include "indexview.h"

#include <qapplication.h>
#include <qevent.h>
#include <qlayout.h>
#include <qlabel.h>

#include <klineedit.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kurl.h>
#include <kdebug.h>

#include <kdevpartcontroller.h>
#include <kdevdocumentationplugin.h>

#include "docutils.h"
#include "selecttopic.h"
#include "documentation_part.h"
#include "documentation_widget.h"

IndexView::IndexView(DocumentationWidget *parent, const char *name)
     :QWidget(parent, name), m_widget(parent)
{
    QVBoxLayout *l = new QVBoxLayout(this, 0, 0);
    
    QHBoxLayout *hl = new QHBoxLayout(this, 0, 0);
    m_edit = new KLineEdit(this);
    hl->addWidget(new QLabel(m_edit, i18n("&Look for:"), this));
    hl->addWidget(m_edit);
    l->addLayout(hl);

    m_index = new IndexBox(this);
    l->addWidget(m_index);
       
    connect(m_index, SIGNAL(executed(QListBoxItem* )), this, SLOT(searchInIndex(QListBoxItem* )));
    connect(m_index, SIGNAL(mouseButtonPressed(int, QListBoxItem*, const QPoint& )),
        this, SLOT(itemMouseButtonPressed(int, QListBoxItem*, const QPoint& )));
    connect(m_edit, SIGNAL(returnPressed()), this, SLOT(searchInIndex()));
    connect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(showIndex(const QString&)));
    
    m_edit->installEventFilter(this);
}

IndexView::~IndexView()
{
}

void IndexView::searchInIndex()
{
    if (m_index->currentItem() != -1)
        searchInIndex(m_index->item(m_index->currentItem()));
}

void IndexView::searchInIndex(QListBoxItem *item)
{
    kdDebug() << "IndexView::searchInIndex" << endl;
    IndexItem *indexItem = dynamic_cast<IndexItem*>(item);
    if (!indexItem)
        return;
    
    IndexItem::List urls = indexItem->urls();
    if (urls.count() == 1)
        m_widget->part()->partController()->showDocument(urls.first().second);
    else if (urls.count() == 0) ;
    else
    {
        SelectTopic *dia = new SelectTopic(urls);
        dia->topicLabel->setText(dia->topicLabel->text().arg(item->text()));
        if (dia->exec())
            m_widget->part()->partController()->showDocument(dia->selectedURL());
        delete dia;
    }
}

void IndexView::showIndex(const QString &term)
{
    QListBoxItem *i = m_index->firstItem();
    QString sl = term.lower();
    while (i)
    {
        QString t = i->text();
        if ((t.length() >= sl.length()) && (i->text().left(term.length()).lower() == sl))
        {
            m_index->setCurrentItem(i);
            m_index->setTopItem(m_index->index(i));
            break;
        }
        i = i->next();
    }
/*
    for (uint i = 0; i < m_index->count(); ++ i)
    {
        if (m_index->text(i).startsWith(term))
        {
            m_index->setCurrentItem(i);
            m_index->setTopItem(i);
        }
    }*/
}

bool IndexView::eventFilter(QObject *watched, QEvent *e)
{
    if (!watched || !e)
        return true;

    if ((watched == m_edit) && (e->type() == QEvent::KeyPress))
    {
        QKeyEvent *ke = (QKeyEvent*)e;
        if (ke->key() == Key_Up)
        {
            int i = m_index->currentItem();
            if (--i >= 0)
            {
                m_index->setCurrentItem(i);
                m_edit->blockSignals(true);
                m_edit->setText(m_index->currentText());
                m_edit->blockSignals(false);
            }
            return true;
        } else if (ke->key() == Key_Down)
        {
            int i = m_index->currentItem();
            if ( ++i < int(m_index->count()) ) 
            {
                m_index->setCurrentItem(i);
                m_edit->blockSignals(true);
                m_edit->setText(m_index->currentText());
                m_edit->blockSignals(false);
            }
            return true;
        } else if ((ke->key() == Key_Next) || (ke->key() == Key_Prior))
        {
            QApplication::sendEvent(m_index, e);
            m_edit->blockSignals(true);
            m_edit->setText(m_index->currentText());
            m_edit->blockSignals(false);
        }
    }

    return QWidget::eventFilter(watched, e);    
}

void IndexView::itemMouseButtonPressed(int button, QListBoxItem *item, const QPoint &pos)
{
    if ((button != Qt::RightButton) || (!item))
        return;
    IndexItem *docItem = dynamic_cast<IndexItem*>(item);
    if (!docItem)
        return;
    
    DocUtils::docItemPopup(m_widget->part(), docItem, pos, false, true);
}

void IndexView::setSearchTerm(const QString &term)
{
    m_edit->setFocus();
    m_edit->setText(term);
}

void IndexView::askSearchTerm( )
{
    m_edit->setFocus();
}

void IndexView::focusInEvent(QFocusEvent */*e*/)
{
    m_edit->setFocus();
}

#include "indexview.moc"
