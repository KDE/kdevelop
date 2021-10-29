/*
    SPDX-FileCopyrightText: 2009 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ICMAKEMANAGER_H
#define ICMAKEMANAGER_H

#include <QObject>
#include <QPair>

namespace KDevelop
{
class IProject;
}

class ICMakeManager
{
public:
    virtual ~ICMakeManager() {}

    virtual QPair<QString, QString> cacheValue(KDevelop::IProject* project, const QString& id) const=0;

};

Q_DECLARE_INTERFACE( ICMakeManager, "org.kdevelop.ICMakeManager" )

#endif
