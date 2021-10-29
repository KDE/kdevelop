/*
    SPDX-FileCopyrightText: 2012 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_ICOREOBJECT_H
#define KDEVPLATFORM_PLUGIN_ICOREOBJECT_H

#include <QObject>

namespace KDevelop {
    class ICore;
}

class ICoreObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* self READ self CONSTANT)
    public:
        explicit ICoreObject(QObject* parent = nullptr);
        
        QObject* self() const;
};

#endif // KDEVPLATFORM_PLUGIN_ICOREOBJECT_H
