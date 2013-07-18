/***************************************************************************
 *   Copyright (C) 2008 by Andreas Pakulat <apaku@gmx.de                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_OPENPROJECTPAGE_H
#define KDEVPLATFORM_OPENPROJECTPAGE_H

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
    OpenProjectPage( const KUrl& startUrl, QWidget* parent = 0 );
    QMap<QString,QStringList> projectFilters() const;
    void setUrl(const KUrl& url);

signals:
    void urlSelected( const KUrl& );
    void accepted();

private slots:
    void highlightFile(const QString&);
    void opsEntered(const KUrl& item );
    void comboTextChanged(const QString& );
    void dirChanged(const KUrl& url);

private:
    KUrl getAbsoluteUrl( const QString& ) const;
    KFileWidget* fileWidget;
    QMap<QString,QStringList> m_projectFilters;
};

}

#endif
