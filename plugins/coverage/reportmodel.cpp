/* KDevelop xUnit plugin
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


#include "reportmodel.h"
#include "coveredfile.h"
#include <KLocale>

using Veritas::CoveredFile;
using Veritas::ReportModel;
using Veritas::ReportDirItem;
using Veritas::ReportFileItem;
using Veritas::ReportValueItem;

///////////////// ReportItems ////////////////////////////////////////////////

ReportValueItem* ReportFileItem::coverageItem() const
{
    return m_coverageItem;
}

ReportValueItem* ReportFileItem::slocItem() const
{
    return m_slocItem;
}

ReportValueItem* ReportFileItem::instrumentedItem() const
{
    return m_instrumentedItem;
}

ReportFileItem::ReportFileItem(const KUrl& url)
    : QStandardItem(url.fileName()),
      m_fullUrl(url),
      m_coverageItem(0),
      m_slocItem(0),
      m_instrumentedItem(0)
{
    setEditable(false);
    setSelectable(true);
    setCheckable(false);
}

void ReportFileItem::addCoverageData(CoveredFile* f)
{
    Q_ASSERT(f->url() == url());
    if (!m_coverageItem) {
        m_coverageItem = new ReportValueItem(f->coverage());
        m_slocItem = new ReportValueItem(f->sloc());
        m_instrumentedItem = new ReportValueItem(f->instrumented());
        m_coveredLines = f->coveredLines();
        m_reachableLines = f->reachableLines();
    } else {
        kDebug() << "";
        m_coveredLines += f->coveredLines(); // set union
        m_reachableLines += f->reachableLines();
        int covered = m_coveredLines.count();
        int reachable = m_reachableLines.count();
        m_instrumentedItem->setValue(covered);
        m_slocItem->setValue(reachable);
        m_coverageItem->setValue(100*(double)covered/reachable);
    }
}

KUrl ReportFileItem::url()
{
    return m_fullUrl;
}

ReportFileItem::~ReportFileItem()
{}

int ReportFileItem::type() const
{
    return ReportModel::File;
}

ReportDirItem::ReportDirItem(const QString& dir)
    : QStandardItem(dir) {
    setEditable(false);
    setSelectable(true);
    setCheckable(false);
    m_sloc = 0;
    m_instrumented = 0;
}

void ReportDirItem::updateStats()
{
    m_sloc = 0;
    m_instrumented = 0;
    QStandardItem* si;
    ReportFileItem* rfi;
    for (int row=0; row<rowCount(); row++) {
        si = child(row, 0);
        if (si->type() == ReportModel::File) {
            rfi = static_cast<ReportFileItem*>(si);
            m_instrumented += rfi->instrumentedItem()->value();
            m_sloc += rfi->slocItem()->value();
        }
    }
}

ReportDirItem::~ReportDirItem()
{}

int ReportDirItem::type() const
{
    return ReportModel::Dir;
}

int ReportDirItem::sloc()
{
    return m_sloc;
}

int ReportDirItem::instrumented()
{
    return m_instrumented;
}

double ReportDirItem::coverage()
{
    return (m_sloc == 0) ? 0 : 100*double(m_instrumented)/double(m_sloc);
}

ReportValueItem::ReportValueItem(double value)
{
    setValue(value);
    initProperties();
}

ReportValueItem::ReportValueItem(int value)
{
    setValue(value);
    initProperties();
}

double ReportValueItem::value() const
{
    return m_value;
}

void ReportValueItem::setValue(double value)
{
    m_value = value;
    setData(QString::number(value, 'f', 1), Qt::DisplayRole);
}

void ReportValueItem::setValue(int value)
{
    m_value = value;
    setData(QString::number(value), Qt::DisplayRole);
}

void ReportValueItem::initProperties()
{
    setEditable(false);
    setSelectable(false);
    setCheckable(false);
}

ReportValueItem::~ReportValueItem()
{}

int ReportValueItem::type() const
{
    return ReportModel::Value;
}

///////////////// ReportModel ////////////////////////////////////////////////

ReportModel::ReportModel(QObject* parent)
    : QStandardItemModel(parent)
{
    QStringList headers;
    headers << i18n("source") 
            << i18n("coverage")
            << i18n("visited")
            << i18n("SLOC");
    setHorizontalHeaderLabels(headers);
}

void ReportModel::setRootDirectory(const KUrl& root)
{
    m_root = root;
}

ReportModel::~ReportModel()
{}

QVariant ReportModel::data(const QModelIndex& i, int role) const
{
    if (role != ReportModel::SortRole) {
        return QStandardItemModel::data(i, role);
    }
    // SortRole
    QStandardItem* item = itemFromIndex(i);
    if (item->type() == ReportModel::Value) {
        ReportValueItem* rvi = static_cast<ReportValueItem*>(item);
        return QVariant(rvi->value());
    } else { // sort on display value
        return QStandardItemModel::data(i, Qt::DisplayRole);
    }
}

QList<QStandardItem*> ReportModel::createFileRow(CoveredFile* f)
{
    ReportFileItem* file = new ReportFileItem(f->url()); // do these get cleaned?
    m_files.insert(f->url(), file);
    file->addCoverageData(f);
    QList<QStandardItem*> row;
    row << file << file->coverageItem() << file->instrumentedItem() << file->slocItem();
    return row;
}

QBrush brushForCoverage(double percent)
{
    QString color;
    if      (percent < 5)  color = "black";
    else if (percent < 30) color = "red";
    else if (percent < 60) color = "orange";
    else                   color = "green";
    return QBrush(QColor(color));
}

// slot called when new CoveredFile has been parsed
void ReportModel::addCoverageData(CoveredFile* f)
{
    QString dir = f->url().directory();
    if (!m_dirs.contains(dir)) appendDir(dir);
    ReportDirItem* dirItem = m_dirs[dir];
    if (!m_files.contains(f->url())) {
        dirItem->appendRow(createFileRow(f));
    } else {
        m_files[f->url()]->addCoverageData(f);
    }
    dirItem->updateStats();
    updateColoredCoverageColumn(dirItem);
 }

// helper for addCoverageData
void ReportModel::appendDir(const QString& dir)
{
    ReportDirItem* dirItem = new ReportDirItem(dir);
    m_dirs[dir] = dirItem;
    QStandardItem *root = invisibleRootItem();
    QList<QStandardItem*> row;
    row << dirItem;
    root->appendRow(row);        
}

// helper for addCoverageData
void ReportModel::updateColoredCoverageColumn(ReportDirItem* dir)
{
    QModelIndex i = dir->index();
    i = i.sibling(i.row(), i.column()+1);
    QStandardItem* it = itemFromIndex(i);
    QBrush b = brushForCoverage(dir->coverage());
    it->setBackground(b);
    it->setForeground(b);
    it->setData(dir->coverage(), Qt::DisplayRole);
}


#include "reportmodel.moc"
