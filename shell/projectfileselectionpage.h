/***************************************************************************
 *   Copyright (C) 2008 by Andreas Pakulat <apaku@gmx.de                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PROJECTFILESELECTIONPAGE_H
#define PROJECTFILESELECTIONPAGE_H

#include <QtGui/QWidget>

class QStringListModel;
class QModelIndex;

namespace Ui
{
class ProjectFileSelectionPage;
}

namespace KDevelop
{

class ProjectFileSelectionPage : public QWidget
{
Q_OBJECT
public:
    ProjectFileSelectionPage( QWidget* parent = 0 );
    void setEntries( const QStringList& );
signals:
    void fileSelected( const QString& );
private slots:
    void currentChanged( const QModelIndex& );
private:
    Ui::ProjectFileSelectionPage* ui;
    QStringListModel* model;
};

}

#endif
