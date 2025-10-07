/*
    SPDX-FileCopyrightText: 1999 John Birch <jbb@kdevelop.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MEMVIEW_H_
#define MEMVIEW_H_

#include "config-gdb-plugin.h"
#include "dbgglobal.h"
#include "mi/mi.h"

#include <QWidget>

#if KDEV_WITH_QHEXEDIT
class QHexEdit;
#else
namespace Okteta {
class ByteArrayModel;
}
namespace Okteta {
class ByteArrayColumnView;
}
#endif

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

    private: // Okteta vs. QHexEdit normalizing methods
        inline qsizetype memorySize() const
#if KDEV_WITH_QHEXEDIT
        { return m_memSize; }
#else
        { return m_memData.size(); }
#endif
        inline bool isMemoryEmpty() const
#if KDEV_WITH_QHEXEDIT
        { return (m_memSize == 0); }
#else
        { return m_memData.isEmpty(); }
#endif

        MemoryRangeSelector* m_rangeSelector;
#if KDEV_WITH_QHEXEDIT
        QHexEdit *m_memViewView;
#else
        Okteta::ByteArrayModel *m_memViewModel;
        Okteta::ByteArrayColumnView *m_memViewView;
#endif

        quintptr m_memStart;
        QString m_memStartStr, m_memAmountStr;
#if KDEV_WITH_QHEXEDIT
        // Needed for now to manually remember the size of the current data.
        // Check https://github.com/Simsys/qhexedit2/issues/191
        qsizetype m_memSize;
#else
        QByteArray m_memData;
#endif
        bool m_appHasStarted = false;

        void currentSessionChanged(KDevelop::IDebugSession* session);
    };

} // end of namespace GDB
} // end of namespace KDevMI

#endif
