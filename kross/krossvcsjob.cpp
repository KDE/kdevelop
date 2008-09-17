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

#include "krossvcsjob.h"

#include <kross/core/action.h>

using namespace KDevelop;

KrossVcsJob::KrossVcsJob(const QString& funcname, const QVariantList& parameters,
Kross::Action* anAction, KDevelop::IPlugin* plugin, QObject* parent)
    : VcsJob(parent), m_funcname(funcname), m_parameters(parameters), action(anAction), m_status(JobNotStarted), m_plugin(plugin)
{
}

void KrossVcsJob::start()
{
    m_status=JobRunning;
    m_result=action->callFunction(m_funcname, m_parameters);
    m_status=JobSucceeded;
}

#include "krossvcsjob.moc"
