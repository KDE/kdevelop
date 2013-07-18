/***************************************************************************
 *   Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_SVNLOCATIONWIDGET_H
#define KDEVPLATFORM_PLUGIN_SVNLOCATIONWIDGET_H

#include <vcs/widgets/standardvcslocationwidget.h>


class SvnLocationWidget : public KDevelop::StandardVcsLocationWidget
{
    public:
        SvnLocationWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
        virtual KDevelop::VcsLocation location() const;
        virtual bool isCorrect() const;
};

#endif // KDEVPLATFORM_PLUGIN_SVNLOCATIONWIDGET_H
