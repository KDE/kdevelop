/*
    SPDX-FileCopyrightText: 2009 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_IDOCUMENTATIONCONTROLLER_H
#define KDEVPLATFORM_IDOCUMENTATIONCONTROLLER_H

#include <interfaces/idocumentation.h>
#include <QObject>

class QUrl;

namespace KDevelop {
class IDocumentationProvider;
class Declaration;

/**
 * Allows to access the documentation.
 *
 * @author Aleix Pol <aleixpol@kde.org>
 */
class KDEVPLATFORMINTERFACES_EXPORT IDocumentationController: public QObject
{
    Q_OBJECT
public:
    IDocumentationController();
    ~IDocumentationController() override;

    /** Return the documentation provider plugin instances. */
    virtual QList<IDocumentationProvider*> documentationProviders() const = 0;

    /** Return the corresponding documentation instance for a determinate declaration. */
    virtual IDocumentation::Ptr documentationForDeclaration(Declaration* declaration) = 0;
    /** Returns a corresponding documentation if a provider has one for the given URL*/
    virtual IDocumentation::Ptr documentation(const QUrl& url) const = 0;
public Q_SLOTS:
    /** Show the documentation specified by @p doc. */
    virtual void showDocumentation(const IDocumentation::Ptr& doc) = 0;

    /** Emit signal when the documentation providers list changed. */
    virtual void changedDocumentationProviders() = 0;

Q_SIGNALS:
    /** Emitted when providers list changed */
    void providersChanged();
};

}

#endif
