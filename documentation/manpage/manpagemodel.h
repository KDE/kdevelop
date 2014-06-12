/*  This file is part of KDevelop

    Copyright 2010 Yannick Motta <yannick.motta@gmail.com>
    Copyright 2010 Benjamin Port <port.benjamin@gmail.com>
    Copyright 2014 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef MANPAGEMODEL_H
#define MANPAGEMODEL_H

#include <QStringListModel>

#include <KIO/Job>

#include <QListIterator>

// id and name for man section
typedef QPair<QString, QString> ManSection;

class ManPageModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    ManPageModel(QObject* parent = 0);
    virtual ~ManPageModel();
    /**
     * You can use @p DeclarationRole to get the Declaration for a given index.
     * NOTE: If you use that, don't forget to lock the DUChain if you access the declaration!
     */
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex&) const { return 1; }
    virtual QModelIndex parent(const QModelIndex& child) const;
    QModelIndex index(int row, int column, const QModelIndex& parent) const;
    QStringListModel* indexList();
    bool containsIdentifier(QString identifier);
    int sectionCount() const;
    bool isLoaded() const;
    int nbSectionLoaded() const;
    bool identifierInSection(const QString &identifier, const QString &section) const;
signals:
    void sectionParsed();
    void sectionListUpdated();
    void manPagesLoaded();

public slots:
    void showItem(const QModelIndex& idx);
    void showItemFromUrl(const QUrl& url);

private slots:
    void initModel();

    void indexEntries(KIO::Job* job, const KIO::UDSEntryList& entries);
    void indexLoaded();

    void sectionEntries(KIO::Job* job, const KIO::UDSEntryList& entries);
    void sectionLoaded();

private:
    QString manPage(const QString &sectionUrl, int position) const;
    void initSection();

    QListIterator<ManSection> *iterator;
    QList<ManSection> m_sectionList;
    QHash<QString, QVector<QString> > m_manMap;
    QStringList m_index;
    QStringListModel* m_indexModel;

    bool m_loaded;
    int m_nbSectionLoaded;
};

#endif // MANPAGEMODEL_H
