/***************************************************************************
 *   Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svnlocationwidget.h"
#include <vcs/vcslocation.h>

SvnLocationWidget::SvnLocationWidget(QWidget* parent, Qt::WindowFlags f)
    : StandardVcsLocationWidget(parent, f)
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
