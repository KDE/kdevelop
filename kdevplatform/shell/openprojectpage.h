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

class QUrl;
class KFileWidget;

namespace KDevelop
{

class OpenProjectPage : public QWidget
{
    Q_OBJECT

public:
    explicit OpenProjectPage( const QUrl& startUrl, const QStringList& filters,
        QWidget* parent = nullptr );
    void setUrl(const QUrl& url);

Q_SIGNALS:
    void urlSelected(const QUrl&);
    void accepted();

protected:
    void showEvent(QShowEvent*) override;

private Q_SLOTS:
    void highlightFile(const QUrl&);
    void opsEntered(const QUrl& item);
    void comboTextChanged(const QString&);
    void dirChanged(const QUrl& url);

private:
    QUrl absoluteUrl(const QString& file) const;

private:
    KFileWidget* fileWidget;
};

}

#endif
