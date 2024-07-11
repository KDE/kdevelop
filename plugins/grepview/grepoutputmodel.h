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

#include <QList>
#include <QRegExp>
#include <QStandardItemModel>

class QModelIndex;

namespace KDevelop {
    class IStatus;
}

class GrepOutputItem : public QStandardItem
{
public:
    using List = QList<GrepOutputItem>;

    GrepOutputItem(const KDevelop::DocumentChangePointer& change, const QString& text, bool checkable);
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

    /**
     * @warning Do not use the default constructor. It is provided only to prevent a compilation error in
     * operator>>(QDataStream&, QList<T>&) [with @c T = GrepOutputItem]. This specialization of
     * @c operator>> is instantiated by the signal and the slot with a @c const GrepOutputItem::List&
     * parameter, as well as by the declaration Q_DECLARE_METATYPE(GrepOutputItem::List).
     */
    [[deprecated(
        "Do not use the default constructor. It is never called, not tested and asserts false.")]] GrepOutputItem();

    /**
     * @warning Do not read from or write to a stream. read() and write() are overridden only to detect
     * when they are invoked and fail. The reason is: serializing @a m_change is not straightforward
     * and not implemented until we know when and why a GrepOutputItem needs to be serialized.
     */
    [[deprecated("Do not read from a stream. This function is never called, not tested and asserts false.")]] void
    read(QDataStream& in) override;
    /**
     * @copydoc read
     */
    [[deprecated("Do not write to a stream. This function is never called, not tested and asserts false.")]] void
    write(QDataStream& out) const override;

private:
    KDevelop::DocumentChangePointer m_change;
};

Q_DECLARE_METATYPE(GrepOutputItem::List)

class GrepOutputModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit GrepOutputModel( QObject *parent = nullptr );
    ~GrepOutputModel() override;

    void setRegExp(const QRegExp& re);
    void setReplacementTemplate(const QString &tmpl);
    /// applies replacement on given text
    QString replacementFor(const QString &text);
    void clear();  // resets file & match counts
    bool hasResults();
 
    QModelIndex previousItemIndex(const QModelIndex &currentIdx) const;
    QModelIndex nextItemIndex(const QModelIndex &currentIdx) const;
    const GrepOutputItem *getRootItem() const;

    void makeItemsCheckable(bool checkable);
    bool itemsCheckable() const;
    
public Q_SLOTS:
    void appendOutputs( const QString &filename, const GrepOutputItem::List &lines );
    void activate( const QModelIndex &idx );
    void doReplacements();
    void setReplacement(const QString &repl);
    //receive status message from GrepJob, and store it
    void showMessageSlot( KDevelop::IStatus*, const QString& message );
    //emit stored message as signal 'showMessage' to GrepOutputView.
    //called when user selects a search with the combobox
    void showMessageEmit();

Q_SIGNALS:
    void showMessage( KDevelop::IStatus*, const QString& message );
    void showErrorMessage(const QString& message);

private:    
    void makeItemsCheckable(bool checkable, GrepOutputItem* item);
    
    QRegExp m_regExp;
    QString m_replacement;
    QString m_replacementTemplate;
    QString m_finalReplacement;
    bool m_finalUpToDate = false;  /// says if m_finalReplacement is up to date or must be regenerated
    GrepOutputItem *m_rootItem = nullptr;
    int m_fileCount = 0;
    int m_matchCount = 0;
    QString m_savedMessage;
    KDevelop::IStatus *m_savedIStatus;
    bool m_itemsCheckable = false;

private Q_SLOTS:
    void updateCheckState(QStandardItem*);
};

#endif
