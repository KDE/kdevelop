/***************************************************************************
 *   Copyright (C) 2006-2007 by Alexander Dymo  <adymo@kdevelop.org>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "view.h"

#include <QWidget>

#include "document.h"
#include "view_p.h"

namespace Sublime {

View::View(Document *doc)
    :QObject(doc)
{
    d = new ViewPrivate(this);
    d->doc = doc;
}

View::~View()
{
    delete d;
}

Document *View::document() const
{
    return d->doc;
}

QWidget *View::widget(QWidget *parent)
{
    return d->initializeWidget(parent);
}

}

#include "view.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
