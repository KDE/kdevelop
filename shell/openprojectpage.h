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
#include <QtCore/QMap>

class KUrl;
class KFileItem;
class KFileWidget;

namespace KDevelop
{

class OpenProjectPage : public QWidget
{
Q_OBJECT
public:
    OpenProjectPage( QWidget* parent = 0 );
    QMap<QString,QStringList> projectFilters() const;
signals:
    void urlSelected( const KUrl& );
private slots:
    void highlightFile(const QString&);
    void opsEntered(const KUrl& item );
    void comboTextChanged(const QString& );
private:
    KUrl getAbsoluteUrl( const QString& ) const;
    KFileWidget* fileWidget;
    QMap<QString,QStringList> m_projectFilters;
};

}

#endif
