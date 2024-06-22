/*
    SPDX-FileCopyrightText: 2010 Yannick Motta <yannick.motta@gmail.com>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "manpagemodel.h"
#include "manpageplugin.h"
#include "manpagedocumentation.h"
#include "debug.h"

#include "../openwith/iopenwith.h"

#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>

#include <KIO/ListJob>

#include <QDesktopServices>
#include <QStringListModel>
#include <QTimer>

#include <limits>

namespace {

const quintptr INVALID_ID = std::numeric_limits<quintptr>::max();

}

using namespace KDevelop;

ManPageModel::ManPageModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_indexModel(new QStringListModel(this))
{
    QMetaObject::invokeMethod(const_cast<ManPageModel*>(this), "initModel", Qt::QueuedConnection);
}

ManPageModel::~ManPageModel()
{
}

QModelIndex ManPageModel::parent(const QModelIndex& child) const
{
    if (child.isValid() && child.column() == 0 && child.internalId() != INVALID_ID) {
        return createIndex(child.internalId(), 0, INVALID_ID);
    }
    return QModelIndex();
}

QModelIndex ManPageModel::index(int row, int column, const QModelIndex& parent) const
{
    if (row < 0 || column != 0) {
        return QModelIndex();
    } else if (!parent.isValid() && row == m_sectionList.count()) {
        return QModelIndex();
    }

    return createIndex(row, column, parent.isValid() ? parent.row() : INVALID_ID);
}

QVariant ManPageModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid()) {
        if (role == Qt::DisplayRole) {
            int internal(index.internalId());
            if (internal >= 0) {
                int position = index.row();
                QString sectionUrl = m_sectionList.at(index.internalId()).first;
                return manPage(sectionUrl, position);
            } else {
                return m_sectionList.at(index.row()).second;
            }
        }
    }
    return QVariant();
}

int ManPageModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        return m_sectionList.count();
    } else if (parent.internalId() == INVALID_ID) {
        const QString sectionUrl = m_sectionList.at(parent.row()).first;
        return m_manMap.value(sectionUrl).count();
    }
    return 0;
}

QString ManPageModel::manPage(const QString& sectionUrl, int position) const
{
    return m_manMap.value(sectionUrl).at(position);
}

void ManPageModel::initModel()
{
    m_sectionList.clear();
    m_manMap.clear();
    auto list = KIO::listDir(QUrl(QStringLiteral("man://")), KIO::HideProgressInfo);
    connect(list, &KIO::ListJob::entries, this, &ManPageModel::indexEntries);
    connect(list, &KIO::ListJob::result, this, &ManPageModel::indexLoaded);
}

void ManPageModel::indexEntries(KIO::Job* /*job*/, const KIO::UDSEntryList& entries)
{
    for (const KIO::UDSEntry& entry : entries) {
        if (entry.isDir()) {
            const auto url = entry.stringValue(KIO::UDSEntry::UDS_URL);
            // Filter out the useless index entry ".". Clicking on this entry has no effect. Passing
            // an empty URL to KIO::listDir() in initSection() prints a warning: Invalid URL: QUrl("")
            if (!url.isEmpty()) {
                m_sectionList << ManSection{url, entry.stringValue(KIO::UDSEntry::UDS_NAME)};
            }
        }
    }
}

void ManPageModel::indexLoaded(KJob* job)
{
    if (job->error() != 0) {
        m_errorString = job->errorString();
        emit error(m_errorString);
        return;
    }

    emit sectionListUpdated();

    Q_ASSERT(m_nbSectionLoaded == 0);
    if (!m_sectionList.isEmpty()) {
        initSection();
    }
}

void ManPageModel::initSection()
{
    const QString sectionUrl = m_sectionList.at(m_nbSectionLoaded).first;
    m_manMap[sectionUrl].clear();
    auto list = KIO::listDir(QUrl(sectionUrl), KIO::HideProgressInfo);
    connect(list, &KIO::ListJob::entries, this, &ManPageModel::sectionEntries);
    connect(list, &KIO::ListJob::result, this, &ManPageModel::sectionLoaded);
}

void ManPageModel::sectionEntries(KIO::Job* /*job*/, const KIO::UDSEntryList& entries)
{
    const QString sectionUrl = m_sectionList.at(m_nbSectionLoaded).first;
    auto& pages = m_manMap[sectionUrl];
    pages.reserve(pages.size() + entries.size());
    for (const KIO::UDSEntry& entry : entries) {
        pages << entry.stringValue(KIO::UDSEntry::UDS_NAME);
    }
}

void ManPageModel::sectionLoaded()
{
    m_nbSectionLoaded++;
    emit sectionParsed();
    if (m_nbSectionLoaded < m_sectionList.size()) {
        initSection();
    } else {
        // End of init
        m_loaded = true;
        m_index.clear();
        for (const auto& entries : std::as_const(m_manMap)) {
            m_index += entries.toList();
        }
        m_index.sort();
        m_index.removeDuplicates();
        m_indexModel->setStringList(m_index);
        emit manPagesLoaded();
    }
}

void ManPageModel::showItem(const QModelIndex& idx)
{
    if (idx.isValid() && idx.internalId() != INVALID_ID) {
        QString sectionUrl = m_sectionList.at(idx.internalId()).first;
        QString page = manPage(sectionUrl, idx.row());
        IDocumentation::Ptr newDoc(new ManPageDocumentation(page, QUrl(sectionUrl + QLatin1Char('/') + page)));
        ICore::self()->documentationController()->showDocumentation(newDoc);
    }
}

void ManPageModel::showItemFromUrl(const QUrl& url)
{
    qCDebug(MANPAGE) << "showing" << url.toDisplayString(QUrl::PreferLocalFile);

    auto doc = ManPageDocumentation::s_provider->documentation(url);
    IDocumentationController* const controller = ICore::self()->documentationController();
    if (!doc) {
        doc = controller->documentation(url);
        if (!doc) {
            // Open the unsupported link externally and stay on the current
            // documentation page. Even if this is an external link we can
            // download the contents of, our support for website navigation is very poor.
            if (url.isLocalFile()) {
                // This is usually a system header file => open it in the internal editor.
                // HACK: the timer delay works around an inexplicable bug that temporarily
                // scales current Documentation view's QWebEnginePage as if its zoomFactor
                // equals 1 when the call to IOpenWith::openFiles() ends up opening a
                // document in DocumentController.
                QTimer::singleShot(100, [url] { IOpenWith::openFiles({url}); });
            } else {
                // This is usually a website or mailto link. IOpenWith::openFiles()
                // tends to open it in the internal editor, which is not nice. Let us
                // bypass IOpenWith and open the link in the user's preferred application.
                if (!QDesktopServices::openUrl(url)) {
                    qCWarning(MANPAGE) << "couldn't open URL" << url;
                }
            }
            return;
        }
    }
    controller->showDocumentation(doc);
}

QStringListModel* ManPageModel::indexList()
{
    return m_indexModel;
}

bool ManPageModel::containsIdentifier(const QString& identifier)
{
    return m_index.contains(identifier);
}

int ManPageModel::sectionCount() const
{
    return m_sectionList.count();
}

bool ManPageModel::isLoaded() const
{
    return m_loaded;
}

int ManPageModel::nbSectionLoaded() const
{
    return m_nbSectionLoaded;
}

bool ManPageModel::identifierInSection(const QString& identifier, const QString& section) const
{
    const QString sectionLink = QLatin1String("man:/(") + section + QLatin1Char(')');
    for (auto it = m_manMap.begin(); it != m_manMap.end(); ++it) {
        if (it.key().startsWith(sectionLink)) {
            return it.value().indexOf(identifier) != -1;
        }
    }
    return false;
}

bool ManPageModel::hasError() const
{
    return !m_errorString.isEmpty();
}

const QString& ManPageModel::errorString() const
{
    return m_errorString;
}

#include "moc_manpagemodel.cpp"
