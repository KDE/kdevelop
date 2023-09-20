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

#include <QRegularExpression>
#include <QStandardItemModel>
#include <QList>

#include <language/codegen/documentchangeset.h>

class QModelIndex;

namespace KDevelop {
    class IStatus;
}

class GrepOutputItem : public QStandardItem
{
public:
    using List = QList<GrepOutputItem>;

    GrepOutputItem(const KDevelop::DocumentChangePointer& change, const QString& text, bool checkable);
    GrepOutputItem(const QString &filename = {}, const QString &text = {}, bool checkable = false);
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

Q_DECLARE_METATYPE(GrepOutputItem::List)

class GrepOutputModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit GrepOutputModel( QObject *parent = nullptr );
    ~GrepOutputModel() override;

    void setRegExp(const QRegularExpression& re);
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
    
    QRegularExpression m_regExp;
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
