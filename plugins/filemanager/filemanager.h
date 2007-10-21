/***************************************************************************
 *   Copyright 2006 Alexander Dymo <adymo@kdevelop.org>                    *
 *   Copyright 2006 Andreas Pakulat <apaku@gmx.de>                         *
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

class QModelIndex;
class KFileItem;
class KDevFileManagerPart;

class FileManager: public QWidget {
    Q_OBJECT
public:
    FileManager(KDevFileManagerPart *part, QWidget* parent);

private:
    void setupActions();

    Q_PRIVATE_SLOT(d, void goUp())
    Q_PRIVATE_SLOT(d, void goHome())
    Q_PRIVATE_SLOT(d, void goLeft())
    Q_PRIVATE_SLOT(d, void goRight())
    Q_PRIVATE_SLOT(d, void goToUrl(const KUrl &url))
    Q_PRIVATE_SLOT(d, void goToUrl(const QString &url))

    Q_PRIVATE_SLOT(d, void newFolder())

    Q_PRIVATE_SLOT(d, void init())
    Q_PRIVATE_SLOT(d, void open(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void openFile(const KFileItem &fileItem))

    Q_PRIVATE_SLOT(d, void urlChanged(const QModelIndex &index))

    struct FileManagerPrivate* const d;

};

#endif
