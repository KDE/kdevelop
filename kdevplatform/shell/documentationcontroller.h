/*
    SPDX-FileCopyrightText: 2009 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_DOCUMENTATIONCONTROLLER_H
#define KDEVPLATFORM_DOCUMENTATIONCONTROLLER_H

#include <interfaces/idocumentationcontroller.h>

class DocumentationViewFactory;

class QAction;

namespace KDevelop
{

class Core;
class Context;
class ContextMenuExtension;

class DocumentationController : public IDocumentationController
{
    Q_OBJECT
public:
    explicit DocumentationController(Core* core);
    ~DocumentationController() override;

    void initialize();

    QList<IDocumentationProvider*> documentationProviders() const override;
    IDocumentation::Ptr documentationForDeclaration(Declaration* declaration) override;
    IDocumentation::Ptr documentation(const QUrl& url) const override;
    void showDocumentation(const IDocumentation::Ptr& doc) override;
    ContextMenuExtension contextMenuExtension(Context* context, QWidget* parent);

public Q_SLOTS:
    void changedDocumentationProviders() override;

private Q_SLOTS:
    void doShowDocumentation();

private:
    DocumentationViewFactory* m_factory;
    QAction* m_showDocumentation;
};

}

#endif // KDEVPLATFORM_DOCUMENTATIONCONTROLLER_H
