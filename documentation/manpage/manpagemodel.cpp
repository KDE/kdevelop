/*  This file is part of KDevelop

    Copyright 2010 Yannick Motta <yannick.motta@gmail.com>
    Copyright 2010 Benjamin Port <port.benjamin@gmail.com>

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

#include "manpagemodel.h"
#include "manpageplugin.h"
#include "manpagedocumentation.h"

#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>

#include <language/duchain/types/structuretype.h>

#include <interfaces/icore.h>
#include <interfaces/ilanguage.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/backgroundparser/parsejob.h>

#include <KStandardDirs>
#include <KLocalizedString>

#include <KIO/TransferJob>
#include <KIO/StoredTransferJob>
#include <KIO/Job>

#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QWebElementCollection>

#include <QTreeView>
#include <QHeaderView>
#include <interfaces/idocumentationcontroller.h>

using namespace KDevelop;

namespace {

KUrl urlForSection(const QString& section, const QString& page = {})
{
    KUrl ret("man:(" + section  + ")/" + page);
    return ret;
}

}

ManPageModel::ManPageModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_indexModel(new QStringListModel)
    , m_loaded(false)
    , m_nbSectionLoaded(0)
{
    QMetaObject::invokeMethod(const_cast<ManPageModel*>(this), "initModel", Qt::QueuedConnection);
}

ManPageModel::~ManPageModel()
{
    delete m_indexModel;
}


QModelIndex ManPageModel::parent(const QModelIndex& child) const
{
    if(child.isValid() && child.column()==0 && int(child.internalId())>=0)
        return createIndex(child.internalId(),0, -1);
    return QModelIndex();
}

QModelIndex ManPageModel::index(int row, int column, const QModelIndex& parent) const
{
    if(row<0 || column!=0)
        return QModelIndex();
    if(!parent.isValid() && row==m_sectionList.count())
        return QModelIndex();

    return createIndex(row,column, int(parent.isValid() ? parent.row() : -1));
}

QVariant ManPageModel::data(const QModelIndex& index, int role) const
{
    if(index.isValid()){
        if(role==Qt::DisplayRole) {
            int internal(index.internalId());
            if(internal>=0){
                int position = index.row();
                QString sectionId = m_sectionList.at(index.internalId()).first;
                return manPage(sectionId, position);
            } else {
                return m_sectionList.at(index.row()).second;
            }
        }
    }
    return QVariant();
}

int ManPageModel::rowCount(const QModelIndex& parent) const
{
    if(!parent.isValid()){
        return m_sectionList.count();
    }else if(int(parent.internalId())<0) {
        QString sectionId = m_sectionList.at(parent.row()).first;
        return m_manMap.value(sectionId).count();
    }
    return 0;
}

QString ManPageModel::manPage(const QString &sectionId, int position) const{
    return m_manMap.value(sectionId).at(position);
}

void ManPageModel::initModel(){
    m_manMainIndexBuffer.clear();
    KIO::TransferJob  * transferJob = 0;

    transferJob = KIO::get(KUrl("man://"), KIO::NoReload, KIO::HideProgressInfo);
    connect( transferJob, SIGNAL(data(KIO::Job*,QByteArray)),
             this, SLOT(readDataFromMainIndex(KIO::Job*,QByteArray)) );
    connect(transferJob, SIGNAL(result(KJob*)), this, SLOT(indexDataReceived(KJob*)));
}

void ManPageModel::indexDataReceived(KJob *job){
    if (!job->error()){
        m_sectionList = indexParser();
    }
    emit sectionListUpdated();
    iterator = new QListIterator<ManSection>(m_sectionList);
    if(iterator->hasNext()){
        initSection();
    }
}

void ManPageModel::initSection(){
    KIO::StoredTransferJob  * transferJob = KIO::storedGet(urlForSection(iterator->peekNext().first), KIO::NoReload, KIO::HideProgressInfo);
    connect(transferJob, SIGNAL(result(KJob*)), this, SLOT(sectionDataReceived(KJob*)));
}

void ManPageModel::sectionDataReceived(KJob *job){
    if (!job->error()){
        KIO::StoredTransferJob *stjob = dynamic_cast<KIO::StoredTransferJob*>(job);
        sectionParser(iterator->peekNext().first, QString::fromUtf8(stjob->data()));
    }
    iterator->next();
    m_nbSectionLoaded++;
    emit sectionParsed();
    if(iterator->hasNext()){
        initSection();
    } else {
        // End of init
        m_loaded = true;
        m_index.removeDuplicates();
        m_index.sort();
        m_indexModel->setStringList(m_index);
        delete iterator;
        emit manPagesLoaded();
    }
}

void ManPageModel::readDataFromMainIndex(KIO::Job * job, const QByteArray &data){
    Q_UNUSED(job);
    m_manMainIndexBuffer.append(QString::fromUtf8(data));
}

QList<ManSection> ManPageModel::indexParser(){
     QWebPage page;
     QWebFrame * frame = page.mainFrame();
     frame->setHtml(m_manMainIndexBuffer);
     QWebElement document = frame->documentElement();
     QWebElementCollection links = document.findAll("a");
     QList<ManSection> list;
     foreach(QWebElement e, links){
         QString sectionId = e.attribute("href");
         sectionId = sectionId.mid(5,sectionId.size()-6);
         list.append(qMakePair(sectionId, e.parent().parent().findAll("td").at(2).toPlainText()));
     }
     return list;
}

void ManPageModel::sectionParser(const QString &sectionId, const QString &data){
    // the regex version is much faster than using QWebKit for parsing...
    static QRegExp linkRegex("<a href=\"man:[^\"#]+\">\\s*([^<]+)\\s*</a>", Qt::CaseSensitive, QRegExp::RegExp2);
    int pos = 0;
    QVector<QString> pageList;
    while (-1 != (pos = data.indexOf(linkRegex, pos))) {
        const QString text = linkRegex.cap(1);
        pageList.append(text);
        m_index.append(text);
        pos++;
    }
    m_manMap.insert(sectionId, pageList);
}

void ManPageModel::showItem(const QModelIndex& idx){
    if(idx.isValid() && int(idx.internalId())>=0) {
        QString sectionId = m_sectionList.at(idx.internalId()).first;
        QString page = manPage(sectionId, idx.row());
        KSharedPtr<KDevelop::IDocumentation> newDoc(new ManPageDocumentation(page, urlForSection(sectionId, page)));
        KDevelop::ICore::self()->documentationController()->showDocumentation(newDoc);
    }
}

void ManPageModel::showItemFromUrl(const QUrl& url){
    if(url.toString().startsWith("man")){
        KSharedPtr<KDevelop::IDocumentation> newDoc(new ManPageDocumentation(url.path(), KUrl(url)));
        KDevelop::ICore::self()->documentationController()->showDocumentation(newDoc);
    }
}

QStringListModel* ManPageModel::indexList(){
    return m_indexModel;
}

bool ManPageModel::containsIdentifier(QString identifier)
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

bool ManPageModel::identifierInSection(const QString &identifier, const QString& section) const
{
    return m_manMap.value(section).indexOf(identifier) != -1;
}

#include "manpagemodel.moc"
