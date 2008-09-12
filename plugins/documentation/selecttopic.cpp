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
#include "selecttopic.h"

SelectTopic::SelectTopic(IndexItem::List &urls, QWidget *parent, const char *name)
    :SelectTopicBase(parent, name), m_urls(urls)
{
    for (IndexItem::List::const_iterator it = m_urls.begin(); it != m_urls.end(); ++it)
        topicBox->insertItem((*it).first);
    if (topicBox->item(0))
    {
        topicBox->setCurrentItem(0);
        topicBox->setSelected(topicBox->item(0), true);
    }
}

void SelectTopic::accept()
{
    if (topicBox->currentItem() == -1)
        return;
    else
        return QDialog::accept();
}

KURL SelectTopic::selectedURL()
{
    if (topicBox->currentItem() != -1)
        return m_urls[topicBox->currentItem()].second;
    return KURL();
}

#include "selecttopic.moc"
