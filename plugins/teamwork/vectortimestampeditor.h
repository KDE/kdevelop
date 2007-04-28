/***************************************************************************
copyright            : (C) 2006 by David Nolden
email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef VECTORTIMESTAMPEDITOR_H
#define VECTORTIMESTAMPEDITOR_H

#include "network/sharedptr.h"
#include <QObject>
#include "ui_kdevteamwork_vectortimestampwidget.h"
#include "ui_kdevteamwork_timestampwidget.h"
#include "safelogger.h"
#include <QList>
#include <QDateTime>
#include <QPointer>
#include "loglevel.h"
#include "dynamictext/vectortimestamp.h"
#include "network/logger.h"

class VectorTimestampEditor;
class TimestampEditor;
class QDynamicText;
class QStandardItemModel;
class Replacement;
class BoostSerialization;
class QVBoxLayout;

typedef SharedPtr< QDynamicText, BoostSerializationNormal > QDynamicTextPointer;
typedef SharedPtr<TimestampEditor> TimestampEditorPointer;
typedef SharedPtr< Replacement, BoostSerializationNormal > ReplacementPointer;


class TimestampEditor : public QObject, public Shared, public SafeLogger {
  Q_OBJECT
  public:
    TimestampEditor( Teamwork::LoggerPointer logger, VectorTimestampEditor* parent, uint index );

    void update();
  private slots:
    void tail();
    void none();
    void enableAll();
    void stampChanged();
    void dateChanged( const QDateTime& );
    void enableChanged( bool );
  private:
    void fillWidgets();

    bool trySeekTo( Timestamp s );

    ///This returns the currently edited stamp. It the first one not currently applied to the tree.
    Timestamp stamp();

    ///This returns the currently edited replacement, or zero.
    ReplacementPointer replacement();

    VectorTimestampEditor* m_parent;
    uint m_index;
    Timestamp m_currentStamp;
    Ui_TimestampWidget m_widgets;
    bool m_block;
};

class VectorTimestampEditorLogger : public Teamwork::Logger {
  public:
  VectorTimestampEditorLogger( VectorTimestampEditor* ed );
  virtual void log( const std::string& str , Level lv );
  private:
    VectorTimestampEditor* m_editor;
};

///If the parent is destroyed, VectorTimestampEditor is destroyed too. Also it destroys itself if "finish" is clicked, or the dynamic text is destroyed.
class VectorTimestampEditor : public QObject, public SafeLogger {
  Q_OBJECT
  public:
    ///If embed is false, a new dialog will be created
    VectorTimestampEditor( Teamwork::LoggerPointer logger, QObject* parent, bool embed, QDynamicTextPointer text, bool editable );
    ~VectorTimestampEditor();

    QDynamicTextPointer text();

    void log( const QString& str, LogLevel level = Info );

  private slots:
    void toTailTimestamp();
    void clearLog();
    void finish();
    void textChanged();
    void textDestroyed();

  private:
    void fillWidgets();

    friend class TimestampEditor;

    QDynamicText* m_text;
    bool m_editable;
    bool m_embed;
    Ui_VectorTimestampWidget m_widgets;
    QList< TimestampEditorPointer > m_timestamps;
    QStandardItemModel* m_logModel;
    QVBoxLayout* m_layout;
    QPointer<QDialog> m_dialog; ///Can be zero
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
