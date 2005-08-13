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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef SELECT_TOPIC_H
#define SELECT_TOPIC_H

#include "selecttopicbase.h"

#include "kdevdocumentationplugin.h"

class SelectTopic: public SelectTopicBase{
Q_OBJECT
public:
    SelectTopic(IndexItem::List &urls, QWidget *parent = 0, const char *name = 0);
    KURL selectedURL();

public slots:
    virtual void accept();

private:
    IndexItem::List m_urls;
};

#endif
