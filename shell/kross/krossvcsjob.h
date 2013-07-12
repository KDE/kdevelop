/* KDevPlatform Kross Support
 *
 * Copyright 2008 Aleix Pol <aleixpol@gmail.com>
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

#ifndef KDEVPLATFORM_KROSSVCSJOB_H
#define KDEVPLATFORM_KROSSVCSJOB_H

#include <QObject>
#include <QList>

#include <vcs/vcsjob.h>
#include <vcs/interfaces/idistributedversioncontrol.h>

namespace Kross { class Action; }

class KrossVcsJob : public KDevelop::VcsJob
{
Q_OBJECT
public:
    KrossVcsJob(const QString& funcname, const QVariantList& parameters, Kross::Action* anAction, KDevelop::IPlugin* plugin, QObject* parent);

    void start();
    QVariant fetchResults() { return m_result; }
    JobStatus status() const { return m_status; }
    KDevelop::IPlugin* vcsPlugin() const { return m_plugin; }
private:
    QString m_funcname;
    QVariantList m_parameters;
    Kross::Action *action;

    QVariant m_result;
    JobStatus m_status;
    KDevelop::IPlugin* m_plugin;
};


#endif
