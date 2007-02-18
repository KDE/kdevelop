/***************************************************************************
 *   Copyright (C) 2006 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *   Copyright (C) 2006 by Andreas Pakulat                                 *
 *   apaku@gmx.de                                                          *
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
#include <kurl.h>

class QTreeView;
class QModelIndex;
class KDirModel;
class KFileItem;
class DrillDownView;
class KDevFileManagerPart;

class FileManager: public QWidget {
    Q_OBJECT
public:
    FileManager(KDevFileManagerPart *part, QWidget* parent);

private:
    void setupActions();

    Q_PRIVATE_SLOT(d, void goUp())
    Q_PRIVATE_SLOT(d, void goHome())

    Q_PRIVATE_SLOT(d, void init())
    Q_PRIVATE_SLOT(d, void open(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void openFile(KFileItem *fileItem))

    struct FileManagerPrivate* const d;

};

#endif
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on;
