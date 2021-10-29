/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CMAKEHELPDOCUMENTATION_H
#define CMAKEHELPDOCUMENTATION_H

#include <interfaces/idocumentation.h>

class CMakeHomeDocumentation : public KDevelop::IDocumentation
{
        Q_OBJECT
    public:
        KDevelop::IDocumentationProvider* provider() const override;
        QString name() const override;
        QString description() const override { return name(); }

        QWidget* documentationWidget ( KDevelop::DocumentationFindWidget* findWidget, QWidget* parent = nullptr ) override;

    private:
        bool eventFilter(QObject* watched, QEvent* event) override;
};

#endif
