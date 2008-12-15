/***************************************************************************
 *   Copyright (C) 2008 by Andreas Pakulat <apaku@gmx.de                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef OPENPROJECTPAGE_H
#define OPENPROJECTPAGE_H

#include <QtGui/QWidget>

class KFilePlacesView;
class KFileTreeView;
class KHistoryComboBox;
class KUrl;
class QModelIndex;

namespace KDevelop
{

class OpenProjectPage : public QWidget
{
Q_OBJECT
public:
    OpenProjectPage( QWidget* parent = 0 );
signals:
    void urlSelected( const KUrl& );
private slots:
    void changeUrl( const KUrl& );
    void expandTreeView( const QModelIndex& idx );
    void changeUrl( const QString& );
    void activateUrl( const QString& );
private:
    KFilePlacesView* filePlacesView;
    KFileTreeView* fileTreeView;
    KHistoryComboBox* historyCombo;
};

}

#endif
