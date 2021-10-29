/*
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>
    SPDX-FileCopyrightText: 2007-2008 Aleix Pol <aleixpol@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CMAKEMODELITEMS_H
#define CMAKEMODELITEMS_H

#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <language/duchain/topducontext.h>
#include <parser/cmakelistsparser.h>
#include <util/path.h>

class CMakeTargetItem : public KDevelop::ProjectExecutableTargetItem
{
    public:
        CMakeTargetItem(KDevelop::ProjectBaseItem* parent, const QString& name, const KDevelop::Path &builtUrl);

        void setBuiltUrl(const KDevelop::Path &builtUrl) { m_builtUrl = builtUrl; }
        QUrl builtUrl() const override;
        QUrl installedUrl() const override;

    private:
        KDevelop::Path m_builtUrl;
};

#endif
