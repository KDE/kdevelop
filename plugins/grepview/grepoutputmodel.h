/***************************************************************************
 *   Copyright 1999-2001 Bernd Gehrmann and the KDevelop Team              *
 *   bernd@kdevelop.org                                                    *
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *   Copyright 2010 Silvère Lestang <silvere.lestang@gmail.com>            *
 *   Copyright 2010 Julien Desgats <julien.desgats@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_GREPOUTPUTMODEL_H
#define KDEVPLATFORM_PLUGIN_GREPOUTPUTMODEL_H

#include <QStandardItemModel>
#include <QList>

#include <language/codegen/documentchangeset.h>

class QModelIndex;
class QRegExp;

namespace KDevelop {
    class IStatus;
}

class GrepOutputItem : public QStandardItem
{
public:
    typedef QList<GrepOutputItem> List;

    GrepOutputItem(KDevelop::DocumentChangePointer change, const QString& text, bool checkable);
    GrepOutputItem(const QString &filename, const QString &text, bool checkable);
    ~GrepOutputItem();

    QString filename() const ;
    int lineNumber() const ;
    KDevelop::DocumentChangePointer change() const ;
    bool isText() const ;
    /// Recursively apply check state to children
    void propagateState() ;
    /// Check children to determine current state
    void refreshState() ;

    virtual QVariant data ( int role = Qt::UserRole + 1 ) const;

private:
    KDevelop::DocumentChangePointer m_change;
};

Q_DECLARE_METATYPE(GrepOutputItem::List);

class GrepOutputModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit GrepOutputModel( QObject *parent = 0 );
    ~GrepOutputModel();

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
    void showErrorMessage(const QString & message, int timeout = 0);
    
private:    
    void makeItemsCheckable(bool checkable, GrepOutputItem* item);
    
    QRegExp m_regExp;
    QString m_replacement;
    QString m_replacementTemplate;
    QString m_finalReplacement;
    bool m_finalUpToDate;  /// says if m_finalReplacement is up to date or must be regenerated
    GrepOutputItem *m_rootItem;
    int m_fileCount;
    int m_matchCount;
    QString m_savedMessage;
    KDevelop::IStatus *m_savedIStatus;
    bool m_itemsCheckable;

private slots:
    void updateCheckState(QStandardItem*);
};

#endif
