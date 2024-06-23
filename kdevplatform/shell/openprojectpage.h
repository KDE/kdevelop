/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_OPENPROJECTPAGE_H
#define KDEVPLATFORM_OPENPROJECTPAGE_H

#include <QList>
#include <QWidget>

class KFileFilter;
class KFileWidget;

class QUrl;

namespace KDevelop
{

class OpenProjectPage : public QWidget
{
    Q_OBJECT

public:
    explicit OpenProjectPage(const QUrl& startUrl, const QList<KFileFilter>& filters, QWidget* parent = nullptr);
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
