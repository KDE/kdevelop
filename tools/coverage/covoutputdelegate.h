/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>                     *
 *   Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>                 *
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

#ifndef VERITAS_COVERAGE_COVOUTPUTDELEGATE_H
#define VERITAS_COVERAGE_COVOUTPUTDELEGATE_H

#include <QtGui/QItemDelegate>
#include <kcolorscheme.h>
#include "coverageexport.h"

namespace Veritas
{

/*! Colorizes the lcovjob somewhat */
class VERITAS_COVERAGE_EXPORT CovOutputDelegate : public QItemDelegate
{
public:
    CovOutputDelegate(QObject* parent = 0);
    virtual ~CovOutputDelegate();

    void paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;
private:
    KStatefulBrush textBrush;
    KStatefulBrush processBrush;
    KStatefulBrush warningBrush;
};

}

#endif // VERITAS_COVERAGE_COVOUTPUTDELEGATE_H
