/***************************************************************************
    begin                : Tue Oct 5 1999
    copyright            : (C) 1999 by John Birch
    email                : jbb@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MEMVIEW_H_
#define MEMVIEW_H_

#include "dbgglobal.h"
#include "mi/mi.h"

#include <QContextMenuEvent>
#include <QWidget>

namespace Okteta {
class ByteArrayModel;
}
namespace Okteta {
class ByteArrayColumnView;
}

namespace KDevelop {
class IDebugSession;
}

class QLineEdit;
class QToolBox;


namespace KDevMI
{
namespace GDB
{
    class CppDebuggerPlugin;
    class MemoryView;
    class GDBController;
    class MemoryRangeSelector;

    class MemoryViewerWidget : public QWidget
    {
        Q_OBJECT
    public:
        explicit MemoryViewerWidget(CppDebuggerPlugin* plugin, QWidget* parent = nullptr);

    public Q_SLOTS:
        /** Adds a new memory view. */
        void slotAddMemoryView();

    Q_SIGNALS:
        void requestRaise();


    private Q_SLOTS:
        void slotChildCaptionChanged(const QString& caption);
        void slotChildDestroyed(QObject* child);

    private: // Data
        QToolBox* m_toolBox;
        QList<MemoryView*> m_memoryViews;
    };

    class MemoryView : public QWidget
    {
        Q_OBJECT
    public:
        explicit MemoryView(QWidget* parent);

        void debuggerStateChanged(DBGStateFlags state);

    Q_SIGNALS:
        void captionChanged(const QString& caption);

    private: // Callbacks
        void sizeComputed(const QString& value);

        void memoryRead(const MI::ResultRecord& r);

        // Returns true is we successfully created the memoryView, and
        // can work.
        bool isOk() const;

    private Q_SLOTS:
        void memoryEdited(int start, int end);
        /** Informs the view about changes in debugger state.
         *  Allows view to disable itself when debugger is not running. */
        void slotStateChanged(DBGStateFlags oldState, DBGStateFlags newState);

        /** Invoked when user has changed memory range.
            Gets memory for the new range. */
        void slotChangeMemoryRange();
        void slotHideRangeDialog();
        void slotEnableOrDisable();

    private: // QWidget overrides
        void contextMenuEvent(QContextMenuEvent* e) override;

        void initWidget();

        MemoryRangeSelector* m_rangeSelector;
        Okteta::ByteArrayModel *m_memViewModel;
        Okteta::ByteArrayColumnView *m_memViewView;

        quintptr m_memStart;
        QString m_memStartStr, m_memAmountStr;
        QByteArray m_memData;
        int m_debuggerState;

    private slots:
        void currentSessionChanged(KDevelop::IDebugSession* session);
    };

} // end of namespace GDB
} // end of namespace KDevMI

#endif
