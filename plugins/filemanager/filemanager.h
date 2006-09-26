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
#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QtGui/QWidget>

class QTreeView;
class QModelIndex;
class KDirModel;
class KFileItem;
class DrillDownView;
class KDevFileManagerPart;

class FileManager: public QWidget {
    Q_OBJECT
public:
    FileManager(KDevFileManagerPart *part);

private slots:
    void open(const QModelIndex &index);
    void openFile(KFileItem *fileItem);

private:
    void init();

    KDirModel *m_model;
    DrillDownView *m_view;
    KDevFileManagerPart *m_part;

};

#endif
