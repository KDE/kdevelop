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

#include <QWidget>
#include <QtCore/QMap>

class QUrl;
class KFileWidget;

namespace KDevelop
{

class OpenProjectPage : public QWidget
{
    Q_OBJECT

public:
    OpenProjectPage( const QUrl& startUrl, QWidget* parent = 0 );
    QMap<QString,QStringList> projectFilters() const;
    void setUrl(const QUrl& url);

signals:
    void urlSelected(const QUrl&);
    void accepted();

protected:
    virtual void showEvent(QShowEvent*);

private slots:
    void highlightFile(const QUrl&);
    void opsEntered(const QUrl& item);
    void comboTextChanged(const QString&);
    void dirChanged(const QUrl& url);

private:
    QUrl getAbsoluteUrl(const QString&) const;
    KFileWidget* fileWidget;
    QMap<QString,QStringList> m_projectFilters;
};

}

#endif
