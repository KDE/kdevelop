/*
* This file is part of KDevelop
*
* Copyright 2009 Niko Sams <niko.sams@gmail.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


#ifndef KDEVPLATFORM_PATHMAPPINGSWIDGET_H
#define KDEVPLATFORM_PATHMAPPINGSWIDGET_H

#include <QWidget>

#include <QUrl>
#include <KConfigGroup>

#include <debugger/debuggerexport.h>

class QTableView;

namespace KDevelop {

class KDEVPLATFORMDEBUGGER_EXPORT PathMappings
{
public:
    static const QString pathMappingsEntry;
    static const QString pathMappingRemoteEntry;
    static const QString pathMappingLocalEntry;

    static QUrl convertToLocalUrl(const KConfigGroup &config, const QUrl& remoteUrl);
    static QUrl convertToRemoteUrl(const KConfigGroup &config, const QUrl& localUrl);
};

class KDEVPLATFORMDEBUGGER_EXPORT PathMappingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PathMappingsWidget(QWidget* parent = nullptr);

    void loadFromConfiguration(const KConfigGroup& cfg);
    void saveToConfiguration(const KConfigGroup& cfg) const;
Q_SIGNALS:
    void changed();

private Q_SLOTS:
    void deletePath();
private:
    QTableView *m_pathMappingTable;
};

}

#endif