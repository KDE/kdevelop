/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "svnlocationwidget.h"
#include <vcs/vcslocation.h>

SvnLocationWidget::SvnLocationWidget(QWidget* parent)
    : StandardVcsLocationWidget(parent)
{}

KDevelop::VcsLocation SvnLocationWidget::location() const
{
    QUrl loc =url();
    return KDevelop::VcsLocation(loc.toDisplayString());
}

bool SvnLocationWidget::isCorrect() const
{
    return !url().isRelative();
}
