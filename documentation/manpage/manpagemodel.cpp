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
#include <kio/jobclasses.h>

#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QWebElementCollection>

#include <QtDebug>
#include <QTreeView>
#include <QHeaderView>
#include "manpagedocumentation.h"
#include <interfaces/idocumentationcontroller.h>

#include "manpageplugin.h"
using namespace KDevelop;

ManPageModel::ManPageModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_indexModel(new QStringListModel)
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
                return manPage(sectionId, position).first;
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
        return manPageList(sectionId).count();
    }
    return 0;
}

QList<ManPage> ManPageModel::manPageList(const QString &sectionId) const{
    return m_manMap.value(sectionId);
}

ManPage ManPageModel::manPage(const QString &sectionId, int position) const{
    return manPageList(sectionId).at(position);
}

void ManPageModel::initModel(){
    m_manMainIndexBuffer.clear();
    KIO::TransferJob  * transferJob = 0;

    transferJob = KIO::get(KUrl("man://"), KIO::NoReload, KIO::HideProgressInfo);
    connect( transferJob, SIGNAL( data  (  KIO::Job *, const QByteArray &)),
             this, SLOT( readDataFromMainIndex( KIO::Job *, const QByteArray & ) ) );
    connect(transferJob, SIGNAL(result(KJob *)), this, SLOT(indexDataReceived(KJob *)));
}

void ManPageModel::indexDataReceived(KJob *job){
    if (!job->error()){
        m_sectionList = this->indexParser();
    } else {
        qDebug() << "ManPageModel transferJob error";
    }
    emit sectionCount(m_sectionList.count());
    iterator = new QListIterator<ManSection>(m_sectionList);
    if(iterator->hasNext()){
        initSection();
    }

}

void ManPageModel::initSection(){
    KIO::StoredTransferJob  * transferJob = transferJob = KIO::storedGet(KUrl("man:(" + iterator->peekNext().first + ")"), KIO::NoReload, KIO::HideProgressInfo);
    connect(transferJob, SIGNAL(result(KJob *)), this, SLOT(sectionDataReceived(KJob *)));
}

void ManPageModel::sectionDataReceived(KJob *job){
    if (!job->error()){
        KIO::StoredTransferJob *stjob = dynamic_cast<KIO::StoredTransferJob*>(job);
        this->sectionParser(iterator->peekNext().first, QString(stjob->data()));
    } else {
        qDebug() << "ManPageModel transferJob error";
    }
    iterator->next();
    emit sectionParsed();
    if(iterator->hasNext()){
        initSection();
    } else {
        // End of init
        m_indexModel->setStringList(m_index);
        delete iterator;
        KDevelop::ICore::self()->documentationController()->showDocumentation(ManPageDocumentation::s_provider->homePage());
    }
}

void ManPageModel::readDataFromMainIndex(KIO::Job * job, const QByteArray &data){
     m_manMainIndexBuffer.append(data);
}

void ManPageModel::readDataFromSectionIndex(KIO::Job * job, const QByteArray &data){
     m_manSectionIndexBuffer.append(data);
}

QList<ManSection> ManPageModel::indexParser(){

     QWebPage * page = new QWebPage();
     QWebFrame * frame = page->mainFrame();
     frame->setHtml(m_manMainIndexBuffer);
     QWebElement document = frame->documentElement();
     QWebElementCollection links = document.findAll("a");
     QList<ManSection> list;
     foreach(QWebElement e, links){
        list.append(qMakePair(e.attribute("accesskey"), e.parent().parent().findAll("td").at(2).toPlainText()));
     }
     return list;
}

void ManPageModel::sectionParser(const QString &sectionId, const QString &data){
     QWebPage * page = new QWebPage();
     QWebFrame * frame = page->mainFrame();
     frame->setHtml(data);
     QWebElement document = frame->documentElement();
     QWebElementCollection links = document.findAll("a");
     QList<ManPage> pageList;
     foreach(QWebElement e, links){
         if(e.hasAttribute("href") && !(e.attribute("href").contains(QRegExp( "#." )))){
             pageList.append(qMakePair(e.toPlainText(), KUrl(e.attribute("href"))));
             m_index.append(e.toPlainText());
         }
     }
     m_manMap.insert(sectionId, pageList);
}

void ManPageModel::showItem(const QModelIndex& idx){
    if(idx.isValid() && int(idx.internalId())>=0) {
        QString sectionId = m_sectionList.at(idx.internalId()).first;
        ManPage page = manPage(sectionId, idx.row());
        KSharedPtr<KDevelop::IDocumentation> newDoc(new ManPageDocumentation(page));
        KDevelop::ICore::self()->documentationController()->showDocumentation(newDoc);
    }
}

void ManPageModel::showItemFromUrl(const QUrl& url){
    KSharedPtr<KDevelop::IDocumentation> newDoc(new ManPageDocumentation(qMakePair(url.path(), KUrl(url))));
    KDevelop::ICore::self()->documentationController()->showDocumentation(newDoc);
}

QStringListModel* ManPageModel::indexList(){
    return m_indexModel;
}

bool ManPageModel::containsIdentifier(QString identifier)
{
    return m_index.contains(identifier);
}


#include "manpagemodel.moc"
