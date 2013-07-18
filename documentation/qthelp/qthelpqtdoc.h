/*  This file is part of KDevelop
    Copyright 2009 Aleix Pol <aleixpol@kde.org>
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

#ifndef QTHELPQTDOC_H
#define QTHELPQTDOC_H

#include "qthelpproviderabstract.h"

class QtHelpQtDoc : public QtHelpProviderAbstract
{
    Q_OBJECT
	public:
        QtHelpQtDoc(QObject *parent, const QVariantList &args);
        virtual QIcon icon() const;
        virtual QString name() const;
        void registerDocumentations();

    private:
        void loadDirectory(const QString& path);

    private slots:
        void lookupDone(int code);
};

#endif // QTHELPQTDOC_H
