/* KDevelop xUnit plugin
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#ifndef VERITAS_COVERAGEPLUGIN_H
#define VERITAS_COVERAGEPLUGIN_H

#include <KUrl>
#include <QVariantList>
#include <QFileInfo>
#include <interfaces/iplugin.h>

class KActionMenu;
class QDir;
namespace KDevelop
{
class ContextMenuExtension;
class Context;
}

namespace Veritas
{
class CovOutputDelegate;

class CoveragePlugin : public KDevelop::IPlugin
{
Q_OBJECT

public:
    explicit CoveragePlugin(QObject* parent, const QVariantList& = QVariantList());
    virtual ~CoveragePlugin();
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context);

private Q_SLOTS:
    void startLcovJob();
    void removeGcdaFiles();
    void insertGcovFlags();

private:
    void initReportAction();
    void initCleanGcdaAction();
    void initBuildFlagsAction();
    QFileInfoList findGcdaFilesIn(QDir& dir);

private:
    CovOutputDelegate* m_delegate;
    KUrl m_buildRoot;
    KActionMenu* m_ctxMenu;
};

}

#endif // VERITAS_COVERAGEPLUGIN_H
