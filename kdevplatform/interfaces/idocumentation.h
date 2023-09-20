/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IDOCUMENTATION_H
#define KDEVPLATFORM_IDOCUMENTATION_H

#include <QString>
#include <QExplicitlySharedDataPointer>
#include "interfacesexport.h"
#include "idocumentationprovider.h"
#include <util/ksharedobject.h>

class QWidget;

namespace KDevelop
{

class DocumentationFindWidget;
class IDocumentationProvider;

class KDEVPLATFORMINTERFACES_EXPORT IDocumentation : public QObject, public KSharedObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(IDocumentationProvider* provider READ provider CONSTANT)
public:
    using Ptr = QExplicitlySharedDataPointer<IDocumentation>;
    IDocumentation();

    ~IDocumentation() override;

    /** @returns the name of the documented information*/
    virtual QString name() const = 0;

    /** @returns a HTML-formatted short description. */
    virtual QString description() const = 0;

    /** @returns a widget with all the needed documentation information.
        @param parent defines the widget's parent
        @param findWidget can be used to tell how do we want to deal with Search
            inside the documentation widget. The implementation will have to enable the
            widget if it means to support the search feature.
    */
    virtual QWidget* documentationWidget(DocumentationFindWidget* findWidget, QWidget* parent = nullptr) = 0;

    virtual IDocumentationProvider* provider() const = 0;

Q_SIGNALS:
    void descriptionChanged();
};

}

Q_DECLARE_METATYPE(KDevelop::IDocumentation::Ptr)

#endif
