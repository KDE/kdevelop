/***************************************************************************
 *   Copyright (C) 2008 by Peter Penz <peter.penz@gmx.at>                  *
 *             modified by Manuel Breugelmans <mbr.nxi@gmail.com>          *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#ifndef VERITAS_TOSOURCETOGGLE_H
#define VERITAS_TOSOURCETOGGLE_H

#include <QPixmap>
#include <QModelIndex>
#include "overlaytoggle.h"

class QTimeLine;

namespace Veritas
{
class Test;

class ToSourceToggle : public OverlayButton
{
Q_OBJECT

public:
    explicit ToSourceToggle(QWidget* parent);
    virtual ~ToSourceToggle();
 
    virtual int offset(Test*);
    bool shouldShow(Test*);

protected:
    virtual bool eventFilter(QObject* obj, QEvent* event);
    virtual void enterEvent(QEvent* event);

private slots:
    void setIcon();
    void refreshIcon();

};

}

#endif // VERITAS_SELECTIONTOGGLE_H
