/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEMPLATEENGINE_H
#define KDEVPLATFORM_TEMPLATEENGINE_H

#include <language/languageexport.h>

#include <QScopedPointer>

#include <QStringList>

namespace KDevelop {
class TemplateRenderer;
class TemplateEnginePrivate;

/**
 * Central template engine where template directories and archives can be registered.
 */
class KDEVPLATFORMLANGUAGE_EXPORT TemplateEngine
{
public:
    static TemplateEngine* self();

    /**
     * Adds @p directories to the list of directories searched for templates
     *
     **/
    void addTemplateDirectories(const QStringList& directories);

private:
    TemplateEngine();
    ~TemplateEngine();

    Q_DISABLE_COPY(TemplateEngine)

private:
    const QScopedPointer<class TemplateEnginePrivate> d_ptr;
    Q_DECLARE_PRIVATE(TemplateEngine)
    friend class TemplateRenderer;
};
}

#endif // KDEVPLATFORM_TEMPLATEENGINE_H
