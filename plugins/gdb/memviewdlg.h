/*
    SPDX-FileCopyrightText: 1999 John Birch <jbb@kdevelop.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MEMVIEW_H_
#define MEMVIEW_H_

#include "dbgglobal.h"
#include "mi/mi.h"

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

class QMenu;
class QPoint;
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

    private Q_SLOTS:
        void slotChildCaptionChanged(const QString& caption);

    private: // Data
        QToolBox* m_toolBox;
    };

    class MemoryView : public QWidget
    {
        Q_OBJECT
    public:
        explicit MemoryView(QWidget* parent);

    Q_SIGNALS:
        void captionChanged(const QString& caption);

    private:
        void sizeComputed(const QString& value);
        void addReadMemoryCommand(const QString& arguments);
        void memoryRead(const MI::ResultRecord& r);
        void memoryEdited(int start, int end);

        void currentSessionChanged(KDevelop::IDebugSession* iSession, KDevelop::IDebugSession* iPreviousSession);
        /** Informs the view about changes in debugger state.
         *  Allows view to disable itself when debugger is not running. */
        void debuggerStateChanged(DBGStateFlags state);

        /** Invoked when user has changed memory range.
            Gets memory for the new range. */
        void changeMemoryRange();
        void hideRangeDialog();
        void enableOrDisable();

        void memoryViewContextMenuRequested(const QPoint& viewportPosition);
        void contextMenuEvent(QContextMenuEvent* e) override;
        void addActionsAndShowContextMenu(QMenu* menu, const QPoint& globalPosition);

        void initWidget();

        MemoryRangeSelector* m_rangeSelector;
        Okteta::ByteArrayModel *m_memViewModel;
        Okteta::ByteArrayColumnView *m_memViewView;

        quintptr m_memStart;
        QString m_memStartStr, m_memAmountStr;
        QByteArray m_memData;
        bool m_appHasStarted = false;
    };

} // end of namespace GDB
} // end of namespace KDevMI

#endif
