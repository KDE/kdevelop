/*  This file is part of KDevelop

    Copyright 2010 Yannick Motta <yannick.motta@gmail.com>

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

#include <QtGui/QStringListModel>

#include <language/duchain/indexedstring.h>
#include <language/duchain/declaration.h>
#include <KIO/FileJob>
#include <QMap>

namespace KDevelop
{
    class Declaration;
    class ParseJob;
}

class ManPageModel : public QAbstractListModel
{
    Q_OBJECT

public:
    ManPageModel(QObject* parent = 0);

    enum CustomDataRoles {
        /// returns the Declaration that a given index in the model represents
        DeclarationRole = Qt::UserRole
    };

    /**
     * You can use @p DeclarationRole to get the Declaration for a given index.
     * NOTE: If you use that, don't forget to lock the DUChain if you access the declaration!
     */
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual bool hasChildren(const QModelIndex& parent) const;
    virtual bool canFetchMore(const QModelIndex& parent) const;

    /// Returns the Declaration for a given index
    /// NOTE: Don't forget to lock the DUChain if you access the declaration!
    KDevelop::DeclarationPointer declarationForIndex(const QModelIndex& index) const;

    /// Returns the destination of the internal PHP function file
    /// @see PhpLanguageSupport
    const KDevelop::IndexedString& internalFunctionFile() const;

    void getManPage(const KUrl& page);
    void getManMainIndex();
    void getManSectionIndex(const QString section);



private:
    /// fills model with all declarations from the internal PHP functions file
    void fillModel();

    /// List of pointers to _all_ PHP internal declarations
    QList<KDevelop::DeclarationPointer> m_declarations;

    /// internal function file
    const KDevelop::IndexedString m_internalFunctionsFile;

    void sectionParser();
    void indexParser();

    /// Slave buffer
    QString m_manPageBuffer;
    QString m_manMainIndexBuffer;
    QString m_manSectionIndexBuffer;

public slots:
    void slotParseJobFinished( KDevelop::ParseJob* job );
    void readDataFromManPage(KIO::Job * job, const QByteArray &data);
    void readDataFromMainIndex(KIO::Job * job, const QByteArray &data);
    void readDataFromSectionIndex(KIO::Job * job, const QByteArray &data);


};

Q_DECLARE_METATYPE( KDevelop::DeclarationPointer )

#endif // MANPAGEMODEL_H
