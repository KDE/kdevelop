/* KDevelop coverage plugin
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
class ReportDirData;
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

// TODO merge this with the CoveredFile class, it is totally redundant to have both waste resources
class VERITAS_COVERAGE_EXPORT ReportFileItem : public QStandardItem
{
public:
    ReportFileItem(const KUrl& url);
    virtual ~ReportFileItem();
    virtual int type() const;
    KUrl url();

    void addCoverageData(CoveredFile* f);

    ReportValueItem* coverageRatioItem() const;
    ReportValueItem* slocItem() const;
    ReportValueItem* nrofCoveredLinesItem() const;

private:
    KUrl m_fullUrl;
    ReportValueItem* m_coverageRatioItem;
    ReportValueItem* m_slocItem;
    ReportValueItem* m_nrofCoveredLinesItem;
    QSet<int> m_coveredLines;
    QSet<int> m_reachableLines;
};

/*!
 * Stores the data for the report of a directory.
 * The data stored are the SLOC and the number of covered lines.
 * Coverage percentage is calculated based on those values.
 */
class VERITAS_COVERAGE_EXPORT ReportDirData
{
public:

    /*!
     * Creates a new ReportDirData.
     * All the values are initialized to 0.
     */
    ReportDirData();

    int sloc() const;
    int nrofCoveredLines() const;
    double coverageRatio() const;
    void setSloc(int sloc);
    void setNrofCoveredLines(int nrof);

private:
    int m_sloc;
    int m_nrofCoveredLines;
};

class VERITAS_COVERAGE_EXPORT ReportDirItem : public QStandardItem
{
public:
    ReportDirItem(const QString& dir);
    virtual ~ReportDirItem();
    virtual int type() const;
    void updateStats();
    int sloc();
    int nrofCoveredLines();
    double coverageRatio();
    const ReportDirData& reportDirData() const;

private:
    ReportDirData m_reportDirData;
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
