/*
 * This file is part of KDevelop
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
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

#ifndef KDEVPLATFORM_PLUGIN_PASTEBINPLUGIN_H
#define KDEVPLATFORM_PLUGIN_PASTEBINPLUGIN_H

#include <interfaces/iplugin.h>
#include <vcs/interfaces/ipatchexporter.h>

#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(PLUGIN_PASTEBIN)

namespace KIO {
class Job;
}

class PastebinPlugin : public KDevelop::IPlugin, KDevelop::IPatchExporter
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IPatchExporter )
    public:
        PastebinPlugin ( QObject* parent, const QList<QVariant>& args  );
        virtual ~PastebinPlugin();

        virtual void exportPatch(KDevelop::IPatchSource::Ptr source);

    public slots:
        void data(KIO::Job* job, const QByteArray &data);
    private:
        QMap<KIO::Job*, QString> m_result;
};

#endif
