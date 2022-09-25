/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ISOURCEFORMATTERCONTROLLER_H
#define KDEVPLATFORM_ISOURCEFORMATTERCONTROLLER_H

#include "interfacesexport.h"

#include <QObject>
#include <QString>

#include <memory>

class QUrl;

namespace KDevelop {
class IFileFormatter
{
    Q_DISABLE_COPY_MOVE(IFileFormatter)
public:
    IFileFormatter() = default;
    virtual ~IFileFormatter() = default;

    /**
     * Format text using packaged source formatter and style.
     * @param text the text to format
     * @param leftContext the context at the left side of the text.
     *        If it is in another line, it must end with a newline.
     * @param rightContext the context at the right side of the text.
     *        If it is in the next line, it must start with a newline.
     *
     * @note If the source formatter cannot work correctly with the context,
     *       it will just return the input text.
     */
    virtual QString format(const QString& text, const QString& leftContext = QString(),
                           const QString& rightContext = QString()) const = 0;
};

/** \short An interface to the controller managing all source formatter plugins
 */
class KDEVPLATFORMINTERFACES_EXPORT ISourceFormatterController : public QObject
{
    Q_OBJECT

public:
    explicit ISourceFormatterController(QObject* parent = nullptr);
    ~ISourceFormatterController() override;

    using FileFormatterPtr = std::unique_ptr<IFileFormatter>;

    /**
     * Read user configuration for the given URL and package it into a file formatter object.
     * @param url the URL of a document to be formatted
     * @return the requested file formatter object or nullptr if no formatter is
     *         configured for @p url
     */
    virtual FileFormatterPtr fileFormatter(const QUrl& url) const = 0;

    ///\return @c true if there are formatters at all, @c false otherwise
    virtual bool hasFormatters() const = 0;

    /**
     * Disable source formatting
     * Once disabled, source formatting cannot be reenabled. Call this from within tests.
     */
    virtual void disableSourceFormatting() = 0;
    ///\return Whether or not source formatting is enabled
    virtual bool sourceFormattingEnabled() = 0;

Q_SIGNALS:
    void hasFormattersChanged(bool hasFormatters);
};

}

#endif // KDEVPLATFORM_ISOURCEFORMATTERCONTROLLER_H
