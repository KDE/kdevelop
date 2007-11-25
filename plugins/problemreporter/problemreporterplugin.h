/*
 * KDevelop Problem Reporter
 *
 * Copyright 2006 Adam Treat <treat@kde.org>
 * Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
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

#ifndef CLASSBROWSERPART_H
#define CLASSBROWSERPART_H

#include <iplugin.h>
#include <QtCore/QVariant>

class ProblemReporterPlugin : public KDevelop::IPlugin
{
    Q_OBJECT

  public:
    ProblemReporterPlugin(QObject *parent, const QVariantList & = QVariantList() );
    virtual ~ProblemReporterPlugin();

    // KDevelop::Plugin methods
    virtual void unload();

  private:
    class ProblemReporterFactory* m_factory;
};

#endif // CLASSBROWSERPART_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
