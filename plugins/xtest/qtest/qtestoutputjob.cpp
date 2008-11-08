/* KDevelop xUnit plugin
 *    Copyright 1999-2001 Bernd Gehrmann and the KDevelop Team <bernd@kdevelop.org>
 *    Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *    Copyright 2008 by Hamish Rodda <rodda@kde.org>
 *    Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#include "qtestoutputjob.h"
#include "qtestoutputmodel.h"
#include "qtestoutputdelegate.h"

#include "qtestcase.h"

#include <KLocale>
#include <KDebug>
#include <QTextStream>

#include <interfaces/icore.h>

using namespace KDevelop;
using QTest::QTestCase;

QTestOutputJob::QTestOutputJob(QTestOutputDelegate* parent, QTestCase* caze)
        : OutputJob(parent), m_caze(caze)
{}

void QTestOutputJob::start()
{
    setToolTitle(i18n("QTest verbose output"));
    setToolIcon(KIcon("edit-find"));
    setViewType(KDevelop::IOutputView::HistoryView);
    setStandardToolView(KDevelop::IOutputView::TestView);
    setBehaviours(KDevelop::IOutputView::AutoScroll | KDevelop::IOutputView::AllowUserClose);
    setModel(new QTestOutputModel(delegate()), KDevelop::IOutputView::TakeOwnership);
    setDelegate(delegate());

    startOutput();
    if (!m_caze->outFile().isEmpty()) {
        outputFile(m_caze->outFile());
    }
    if (!m_caze->errorFile().isEmpty()) {
        outputFile(m_caze->errorFile());
    }
    model()->slotCompleted();
    emitResult();
}

void QTestOutputJob::outputFile(const KUrl& path)
{
    QString path_ = path.path();
    QFile f(path_);
    if (!f.exists()) {
        kError() << "Test output file does not exist [" << path_ << "]";
        return;
    }
    f.open(QIODevice::ReadOnly);
    if (!f.isOpen()) {
        kError() << "Failed to open test output file for reading [" << path_ << "]";
        return;
    }
    QTextStream stream(&f);
    QStringList lines;
    while (!f.atEnd()) {
        lines.clear();
        for (int i=0; i<1000 && !f.atEnd(); ++i) {
            QString line = f.readLine();
            line.chop(1);
            lines.append(line);
        }
        model()->appendOutputs(lines);
    }
}

QTestOutputDelegate* QTestOutputJob::delegate() const
{
    return const_cast<QTestOutputDelegate*>(static_cast<const QTestOutputDelegate*>(parent()));
}

QTestOutputModel* QTestOutputJob::model() const
{
    return static_cast<QTestOutputModel*>(OutputJob::model());
}

#include "qtestoutputjob.moc"
