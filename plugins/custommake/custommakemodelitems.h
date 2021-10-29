/*
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CUSTOMMAKEMODELITEMS_H
#define CUSTOMMAKEMODELITEMS_H

#include <project/projectmodel.h>

class CustomMakeTargetItem : public KDevelop::ProjectTargetItem
{
public:
    CustomMakeTargetItem(KDevelop::IProject* project, const QString& name, ProjectBaseItem* parent = nullptr);

    virtual QList<QUrl> includeDirectories() const;
    virtual QHash<QString, QString> environment() const;
    virtual QVector<QPair<QString, QString>> defines() const;

private:
    QList<QUrl> m_includeDirs;
    QHash<QString, QString> m_envs;
    QVector<QPair<QString, QString>> m_defines;

};

#endif
