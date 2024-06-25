/*
    SPDX-FileCopyrightText: 2010 Yannick Motta <yannick.motta@gmail.com>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MANPAGEMODEL_H
#define MANPAGEMODEL_H

#include <QAbstractItemModel>

#include <KIO/Job>
#include <KIO/UDSEntry>

#include <QListIterator>

class QStringListModel;

// id and name for man section
using ManSection = QPair<QString, QString>;

class ManPageModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit ManPageModel(QObject* parent = nullptr);
    ~ManPageModel() override;

    /**
     * You can use @p DeclarationRole to get the Declaration for a given index.
     * NOTE: If you use that, don't forget to lock the DUChain if you access the declaration!
     */
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& = QModelIndex()) const override { return 1; }
    QModelIndex parent(const QModelIndex& child = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    QStringListModel* indexList();
    bool containsIdentifier(const QString& identifier);
    int sectionCount() const;
    bool isLoaded() const;
    int nbSectionLoaded() const;
    bool identifierInSection(const QString &identifier, const QString &section) const;
    bool hasError() const;
    const QString& errorString() const;

Q_SIGNALS:
    void sectionParsed();
    void sectionListUpdated();
    void manPagesLoaded();
    void error(const QString& errorString);

public Q_SLOTS:
    void showItem(const QModelIndex& idx);
    void showItemFromUrl(const QUrl& url);

private Q_SLOTS:
    void initModel();

    void indexEntries(KIO::Job* job, const KIO::UDSEntryList& entries);
    void indexLoaded(KJob* job);

    void sectionEntries(KIO::Job* job, const KIO::UDSEntryList& entries);
    void sectionLoaded();

private:
    QString manPage(const QString &sectionUrl, int position) const;
    void initSection();

    QVector<ManSection> m_sectionList;
    QHash<QString, QVector<QString> > m_manMap;
    QStringList m_index;
    QStringListModel* m_indexModel;

    bool m_loaded = false;
    int m_nbSectionLoaded = 0;
    QString m_errorString;
};

#endif // MANPAGEMODEL_H
