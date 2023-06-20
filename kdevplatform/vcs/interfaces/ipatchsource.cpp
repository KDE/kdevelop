/*
    SPDX-FileCopyrightText: 2006 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "ipatchsource.h"

#include <QIcon>

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

bool IPatchSource::finishReview(const QList<QUrl>& selection)
{
    Q_UNUSED(selection);
    return true;
}

bool IPatchSource::canCancel() const
{
    return false;
}

QMap<QUrl, KDevelop::VcsStatusInfo::State> IPatchSource::additionalSelectableFiles() const
{
    return QMap<QUrl, KDevelop::VcsStatusInfo::State>();
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
    return nullptr;
}

uint IPatchSource::depth() const
{
    return 0;
}

#include "moc_ipatchsource.cpp"
