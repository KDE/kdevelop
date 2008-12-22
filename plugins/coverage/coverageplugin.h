/* KDevelop coverage plugin
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
#include <interfaces/iuicontroller.h>

class QDir;

namespace Veritas
{
class ReportViewFactory;

/*! GCC GCOV coverage plugin. Code coverage report with source annotations (line call count). */
class CoveragePlugin : public KDevelop::IPlugin
{
Q_OBJECT

public:
    explicit CoveragePlugin(QObject* parent, const QVariantList& = QVariantList());
    virtual ~CoveragePlugin();

private:
    QFileInfoList findGcdaFilesIn(QDir& dir);

private:
    ReportViewFactory* m_factory;
};

/*! Report toolview factory */
class ReportViewFactory : public KDevelop::IToolViewFactory
{
public:
    ReportViewFactory();
    virtual ~ReportViewFactory();
    
    virtual QWidget* create(QWidget *parent = 0);
    virtual Qt::DockWidgetArea defaultPosition();
    virtual QString id() const;
};

} // namespace Veritas

#endif // VERITAS_COVERAGEPLUGIN_H
