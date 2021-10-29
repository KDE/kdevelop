/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CMAKEDOC_H
#define CMAKEDOC_H

#include <interfaces/idocumentation.h>
class CMakeDocumentation;

class CMakeDoc : public KDevelop::IDocumentation
{
    Q_OBJECT

    public:
        CMakeDoc(const QString& name, const QString& desc) : mName(name), mDesc(desc) {}
        
        QString description() const override { return mDesc; }
        QString name() const override { return mName; }
        KDevelop::IDocumentationProvider* provider() const override;
        
        QWidget* documentationWidget(KDevelop::DocumentationFindWidget* findWidget, QWidget* parent = nullptr) override;
        
        static CMakeDocumentation* s_provider;
        
    private:
        QString mName, mDesc;
};

#endif
