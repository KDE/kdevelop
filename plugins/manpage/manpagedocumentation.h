/*
    SPDX-FileCopyrightText: 2010 Yannick Motta <yannick.motta@gmail.com>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MANPAGEDOCUMENTATION_H
#define MANPAGEDOCUMENTATION_H

#include "manpagemodel.h"

#include <interfaces/idocumentation.h>

#include <QUrl>
#include <QObject>
#include <QString>
#include <QIODevice>

class QWidget;

class ManPagePlugin;

class ManPageDocumentation : public KDevelop::IDocumentation
{
    Q_OBJECT
    public:
        ManPageDocumentation(const QString& name, const QUrl& url);
        QString name() const override { return m_name; }
        QString description() const override;
        virtual bool providesWidget() const;
        QWidget* documentationWidget(KDevelop::DocumentationFindWidget* findWidget, QWidget* parent = nullptr) override;
        KDevelop::IDocumentationProvider* provider() const override;
        static ManPagePlugin* s_provider;

    private Q_SLOTS:
        void finished(KJob*);

    private:
        const QUrl m_url;
        const QString m_name;
        QString m_description;

};

class ManPageHomeDocumentation : public KDevelop::IDocumentation
{
    Q_OBJECT
    public:
        KDevelop::IDocumentationProvider* provider() const override;
        QString name() const override;
        QString description() const override { return name(); }
        QWidget* documentationWidget ( KDevelop::DocumentationFindWidget* findWidget, QWidget* parent = nullptr ) override;
};


#endif // MANPAGEDOCUMENTATION_H
