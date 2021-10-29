/*
    SPDX-FileCopyrightText: 2012 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "icoreobject.h"
#include <interfaces/icore.h>

ICoreObject::ICoreObject(QObject* parent)
    : QObject(parent)
{}

QObject* ICoreObject::self() const
{
    return KDevelop::ICore::self();
}
