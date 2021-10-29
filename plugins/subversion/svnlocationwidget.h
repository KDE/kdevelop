/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNLOCATIONWIDGET_H
#define KDEVPLATFORM_PLUGIN_SVNLOCATIONWIDGET_H

#include <vcs/widgets/standardvcslocationwidget.h>


class SvnLocationWidget : public KDevelop::StandardVcsLocationWidget
{
    Q_OBJECT
    public:
        explicit SvnLocationWidget(QWidget* parent = nullptr);
        KDevelop::VcsLocation location() const override;
        bool isCorrect() const override;
};

#endif // KDEVPLATFORM_PLUGIN_SVNLOCATIONWIDGET_H
