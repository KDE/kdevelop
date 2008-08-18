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

#ifndef VERITAS_COVERAGE_REPORTMODEL_H
#define VERITAS_COVERAGE_REPORTMODEL_H

#include <QtCore/QList>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>
#include <QSet>
#include <QHash>
#include <KUrl>

#include "coverageexport.h"

namespace Veritas
{

class CoveredFile;
class ReportDirItem;
class ReportFileItem;
class ReportValueItem;

/*! Main model for the coverage report */
class VERITAS_COVERAGE_EXPORT ReportModel : public QStandardItemModel
{
Q_OBJECT
public:
    ReportModel(QObject* parent=0);
    virtual ~ReportModel();
    void setRootDirectory(const KUrl& root);

    QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const;
    enum
    {
        File  = QStandardItem::UserType + 1,
        Dir   = QStandardItem::UserType + 2,
        Value = QStandardItem::UserType + 3
    };
    enum { SortRole = Qt::UserRole + 1 };

public Q_SLOTS:
    void addCoverageData(CoveredFile*);

private:
    QList<QStandardItem*> createFileRow(CoveredFile*);
    void appendDir(const QString& dir);
    void updateColoredCoverageColumn(ReportDirItem* dir);

private:
    KUrl m_root;
    QHash<KUrl, ReportFileItem*> m_files;
    QMap<QString, ReportDirItem*> m_dirs;
};

// TODO merge this with the CoveredFile class, it is totally redundant to have both waste ressources
class VERITAS_COVERAGE_EXPORT ReportFileItem : public QStandardItem
{
public:
    ReportFileItem(const KUrl& url);
    virtual ~ReportFileItem();
    virtual int type() const;
    KUrl url();

    void addCoverageData(CoveredFile* f);

    ReportValueItem* coverageItem() const;
    ReportValueItem* slocItem() const;
    ReportValueItem* instrumentedItem() const;

private:
    KUrl m_fullUrl;
    ReportValueItem* m_coverageItem;
    ReportValueItem* m_slocItem;
    ReportValueItem* m_instrumentedItem;
    QSet<int> m_coveredLines;
    QSet<int> m_reachableLines;
};

class VERITAS_COVERAGE_EXPORT ReportDirItem : public QStandardItem
{
public:
    ReportDirItem(const QString& dir);
    virtual ~ReportDirItem();
    virtual int type() const;
    void updateStats();
    int sloc();
    int instrumented();
    double coverage();

private:
    int m_sloc;
    int m_instrumented;
 };

class VERITAS_COVERAGE_EXPORT ReportValueItem: public QStandardItem
{
public:
    ReportValueItem(double value);
    ReportValueItem(int value);
    virtual ~ReportValueItem();
    virtual int type() const;
    double value() const;
    void setValue(double value);
    void setValue(int value);

private:
    void initProperties();
    double m_value;
};

}

#endif // VERITAS_COVERAGE_REPORTMODEL_h
