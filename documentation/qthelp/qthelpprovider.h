/*  This file is part of KDevelop
    Copyright 2009 Aleix Pol <aleixpol@kde.org>
    Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    Copyright 2010 Benjamin Port <port.benjamin@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef QTHELPPROVIDER_H
#define QTHELPPROVIDER_H

#include "qthelpproviderabstract.h"

class QtHelpProvider : public QtHelpProviderAbstract
{
    Q_OBJECT
    //Q_INTERFACES( KDevelop::IDocumentationProvider )
public:
    QtHelpProvider(QObject *parent, const QString &fileName, const QString &name, const QString &iconName, const QVariantList & args);

    virtual QIcon icon() const;
    virtual QString name() const;
    QString fileName() const;
    QString iconName() const;
    void setName(QString name);
    void setIconName(QString iconName);
private:
    QString m_fileName;
    QString m_name;
    QString m_iconName;
};

#endif // QTHELPPROVIDER_H
