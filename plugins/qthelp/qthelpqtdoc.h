/*  This file is part of KDevelop
    Copyright 2009 Aleix Pol <aleixpol@kde.org>
    Copyright 2010 Benjamin Port <port.benjamin@gmail.com>
    Copyright 2016 Andreas Cord-Landwehr <cordlandwehr@kde.org>

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
    explicit QtHelpQtDoc(QObject* parent, const QVariantList& args);
    ~QtHelpQtDoc();

    QIcon icon() const override;
    QString name() const override;
    void registerDocumentations();
    void loadDocumentation();
    void unloadDocumentation();
    /** @return local paths to all QCH files found in QT_INSTALL_DOCS directory **/
    QStringList qchFiles() const;

    /** @return true once the async initialization has finished */
    bool isInitialized() const { return m_isInitialized; }

private:
    QString m_path;
    bool m_isInitialized = false;
};

#endif // QTHELPQTDOC_H
