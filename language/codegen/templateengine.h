/*
 * This file is part of KDevelop
 *
 * Copyright 2012 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_TEMPLATEENGINE_H
#define KDEVPLATFORM_TEMPLATEENGINE_H

#include "../languageexport.h"

#include <QScopedPointer>

class QStringList;

namespace KDevelop {

class TemplateRenderer;
struct TemplateEnginePrivate;

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

    Q_DISABLE_COPY(TemplateEngine)

    const QScopedPointer<TemplateEnginePrivate> d;
    friend class TemplateRenderer;
};

}

#endif // KDEVPLATFORM_TEMPLATEENGINE_H
