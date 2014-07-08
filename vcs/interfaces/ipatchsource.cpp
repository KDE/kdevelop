/*
   Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "ipatchsource.h"
#include <qicon.h>

using namespace KDevelop;


QIcon IPatchSource::icon() const
{
    return QIcon();
}

IPatchReview::~IPatchReview()
{

}

void IPatchSource::cancelReview()
{

}

bool IPatchSource::finishReview(QList< KUrl > selection)
{
    Q_UNUSED(selection);
    return true;
}

bool IPatchSource::canCancel() const
{
    return false;
}

QMap<KUrl, KDevelop::VcsStatusInfo::State> IPatchSource::additionalSelectableFiles() const
{
    return QMap<KUrl, KDevelop::VcsStatusInfo::State>();
}

bool IPatchSource::canSelectFiles() const
{
    return false;
}

QString IPatchSource::finishReviewCustomText() const
{
    return QString();
}

QWidget* IPatchSource::customWidget() const
{
    return 0;
}

uint IPatchSource::depth() const
{
    return 0;
}


