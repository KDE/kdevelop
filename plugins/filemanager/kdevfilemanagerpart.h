/***************************************************************************
 *   Copyright (C) 2006 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef KDEVFILEMANAGERPART_H
#define KDEVFILEMANAGERPART_H

#include <kdevplugin.h>

class FileManager;

class KDevFileManagerPart: public KDevPlugin {
    Q_OBJECT
public:
    KDevFileManagerPart(QObject *parent, const QStringList &args);
    ~KDevFileManagerPart();

    // KDevPlugin methods
    virtual QWidget *pluginView() const;
    virtual Qt::DockWidgetArea dockWidgetAreaHint() const;

private slots:
    void init();

private:
    QPointer<FileManager> m_view;

};

#endif
