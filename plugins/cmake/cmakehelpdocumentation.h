/* KDevelop CMake Support
 *
 * Copyright 2009 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
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
