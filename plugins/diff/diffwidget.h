/***************************************************************************
 *   Copyright (C) 2001 by Harald Fernengel                                *
 *   harry@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DIFFWIDGET_H_
#define _DIFFWIDGET_H_

#include <QWidget>
#include <q3textedit.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <QContextMenuEvent>
#include <Q3PopupMenu>

#include <kurl.h>

class KTempFile;

namespace KIO {
  class Job;
}

namespace KParts {
  class ReadOnlyPart;
}

// Helper class that displays a modified RMB popup menu
class KDiffTextEdit: public Q3TextEdit
{
  Q_OBJECT
public:
  KDiffTextEdit( QWidget* parent = 0, const char* name = 0 );
  virtual ~KDiffTextEdit();
  void applySyntaxHighlight();
  void clearSyntaxHighlight();

signals:
  void externalPartRequested( const QString& partName );

protected:
  virtual Q3PopupMenu* createPopupMenu( const QPoint& );
  virtual Q3PopupMenu* createPopupMenu();

private slots:
  void popupActivated( int );
  void toggleSyntaxHighlight();
  void saveAs();

private:
  static void searchExtParts();
  static QStringList extParts;
  static QStringList extPartsTranslated;
  bool _highlight;
};

class DiffWidget : public QWidget
{
    Q_OBJECT

public:
    DiffWidget( QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0 );
    virtual ~DiffWidget();

public slots:
    /** The URL has to point to a diff file */
    void openURL( const KUrl& url );
    /** Pass a diff file in here */
    void setDiff( const QString& diff );
    /** clears the difference viewer */
    void slotClear();

private slots:
    /** appends a piece of "diff" */
    void slotAppend( const QString& str );
    /** overloaded for convenience */
    void slotAppend( KIO::Job*, const QByteArray& ba );
    /** call this when the whole "diff" has been sent.
     *  Don't call slotAppend afterwards!
     */
    void slotFinished();
    void showExtPart();
    void showTextEdit();
    void loadExtPart( const QString& partName );

protected:
    void contextMenuEvent( QContextMenuEvent* e );

private:
    void setExtPartVisible( bool visible );
    void populateExtPart();

private:
    KDiffTextEdit* te;
    KIO::Job* job;
    KParts::ReadOnlyPart* extPart;
    KTempFile* tempFile;
    QString rawDiff;
};

#endif
