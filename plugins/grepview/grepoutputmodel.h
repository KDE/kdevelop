/*
    SPDX-FileCopyrightText: 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 1999-2001 the KDevelop Team
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>
    SPDX-FileCopyrightText: 2010 Silvère Lestang <silvere.lestang@gmail.com>
    SPDX-FileCopyrightText: 2010 Julien Desgats <julien.desgats@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_GREPOUTPUTMODEL_H
#define KDEVPLATFORM_PLUGIN_GREPOUTPUTMODEL_H

#include <language/codegen/documentchangeset.h>
#include <util/owningrawpointercontainer.h>
#include <util/toggleonlybool.h>

#include <QList>
#include <QRegExp>
#include <QStandardItemModel>

class QModelIndex;

namespace KDevelop {
    class IStatus;
}

class GrepJob;

enum class MessageType {
    Information,
    Error
};

class GrepOutputItem : public QStandardItem
{
public:
    /**
     * A list of GrepOutputItem pointers stored as QStandardItem pointers
     * for compatibility with the QStandardItem::appendRows() API.
     */
    using List = KDevelop::OwningRawPointerContainer<QList<QStandardItem*>>;

    GrepOutputItem(KDevelop::DocumentChangePointer&& change, const QString& text);
    GrepOutputItem(const QString &filename, const QString &text, bool checkable);
    ~GrepOutputItem() override;

    QString filename() const ;
    int lineNumber() const ;
    KDevelop::DocumentChangePointer change() const ;
    bool isText() const ;
    /// Recursively apply check state to children
    void propagateState() ;
    /// Check children to determine current state
    void refreshState() ;

    QVariant data ( int role = Qt::UserRole + 1 ) const override;

private:
    KDevelop::DocumentChangePointer m_change;
};

class GrepOutputModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit GrepOutputModel( QObject *parent = nullptr );
    ~GrepOutputModel() override;

    /**
     * Associate @p job with this model and clear the model to prepare for a new search.
     */
    void setJob(GrepJob& job);
    void setRegExp(const QRegExp& re);
    void setReplacementTemplate(const QString &tmpl);
    /// applies replacement on given text
    QString replacementFor(const QString &text);
    void clear();  // resets file & match counts
    bool hasResults();
    void appendOutputs(const QString& filename, GrepOutputItem::List&& lines);

    QModelIndex previousItemIndex(const QModelIndex &currentIdx) const;
    QModelIndex nextItemIndex(const QModelIndex &currentIdx) const;
    const GrepOutputItem *getRootItem() const;

    void makeItemsCheckable(bool checkable);
    bool itemsCheckable() const;
    
public Q_SLOTS:
    void activate( const QModelIndex &idx );
    void doReplacements();
    void setReplacement(const QString &repl);

    //emit stored message as signal 'showMessage' to GrepOutputView.
    //called when user selects a search with the combobox
    void showMessageEmit();

Q_SIGNALS:
    void showMessage(GrepOutputModel* model, MessageType type, const QString& message);

    /**
     * This signal is emitted when the result-adding job associated with this model finishes.
     *
     * @param model the model that emitted this signal
     */
    void finishedAddingResults(GrepOutputModel* model);

private:    
    void makeItemsCheckable(bool checkable, GrepOutputItem* item);

    /**
     * @return a guard object that inhibits updateCheckState() while alive
     */
    [[nodiscard]] QScopedValueRollback<KDevelop::ToggleOnlyBool> updateCheckStateGuard();

    /**
     * Receive a status message from GrepJob and store it.
     */
    void showMessageSlot(KDevelop::IStatus*, const QString& message);
    void showErrorMessageSlot(const QString& message);

    QRegExp m_regExp;
    QString m_replacement;
    QString m_replacementTemplate;
    QString m_finalReplacement;
    bool m_finalUpToDate = false;  /// says if m_finalReplacement is up to date or must be regenerated
    GrepOutputItem *m_rootItem = nullptr;
    int m_fileCount = 0;
    int m_matchCount = 0;
    QString m_savedMessage;
    MessageType m_savedMessageType = MessageType::Information;
    bool m_itemsCheckable = false;
    /**
     * Set to @c true to temporarily block updateCheckState() in order to prevent wrong behavior or optimize.
     */
    KDevelop::ToggleOnlyBool m_inhibitUpdateCheckState{false};

    GrepJob* m_job = nullptr;

private Q_SLOTS:
    void updateCheckState(QStandardItem*);
};

#endif
