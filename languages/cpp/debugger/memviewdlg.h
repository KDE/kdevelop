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

#ifndef _MEMVIEW_H_
#define _MEMVIEW_H_

#include "callbacks.h"

#include <kdialog.h>

#include <qvaluevector.h>

class KLineEdit;
class QMultiLineEdit;
class QToolBox;

namespace GDBDebugger
{
    class MemoryView;

    class ViewerWidget : public QWidget
    {
        Q_OBJECT
    public:
        ViewerWidget(QWidget* parent, const char* name);

    public slots:
        /** Adds a new memory view to *this, initially showing
            no data. */
        void slotAddMemoryView();
        /** Informs *this about change in debugger state. Should always
            be connected to, so that *this can disable itself when
            debugger is not running. */
        void slotDebuggerState(const QString&, int state);

    signals:
        /** Emitted to get specified region of memory. */
        void getMemory(MemoryCallback* callback, const QString& start,
                       const QString& amount);
        void evaluateExpression(ValueCallback*, const QString&);
        void setValue(const QString& expression, const QString& value);

        void setViewShown(bool shown);


    private slots:
        /* Handles request to get memory from child MemoryView. */
        void slotGetMemory(const QString& start, const QString& amount);

        void slotChildCaptionChanged(const QString& caption);
        void slotChildDestroyed(QObject* child);
     
    private: // Data
        QToolBox* toolBox_;
        QValueVector<MemoryView*> memoryViews_;
    };

    class MemoryView : public QWidget,
                       public MemoryCallback,
                       public ValueCallback
    {
        Q_OBJECT
    public:
        MemoryView(QWidget* parent, const char* name = 0);

        void debuggerStateChanged(int state);

    signals:
        void getMemory(const QString& start, const QString& amount);
        void evaluateExpression(ValueCallback*, const QString&);
        void setValue(const QString& expression, const QString& value);
     
        void captionChanged(const QString& caption);

    private: // Callbacks
        void memoryContentAvailable(unsigned start, unsigned amount,
                                    char* data);
        void updateValue(char* data);

    private slots:
        void memoryEdited(int start, int end);

    private:        
        // Returns true is we successfully created the hexeditor, and so
        // can work.
        bool isOk() const;

        

    private slots:
        /** Invoked when user has changed memory range.
            Gets memory for the new range. */
        void slotChangeMemoryRange();
        void slotHideRangeDialog();
        void slotEnableOrDisable();

    private: // QWidget overrides
        void contextMenuEvent(QContextMenuEvent* e);

    private:
        void initWidget();

    private:
        class MemoryRangeSelector* rangeSelector_;
        QWidget* khexedit2_widget;
        QWidget* khexedit2_real_widget;

        unsigned start_, amount_;
        QString startAsString_, amountAsString_;
        char* data_;
        
        int debuggerState_;
    };



class MemoryViewDialog : public KDialog
{
    Q_OBJECT

public:
    MemoryViewDialog( QWidget *parent=0, const char *name=0 );
    ~MemoryViewDialog();

signals:
    void disassemble(const QString &start, const QString &end);
    void memoryDump(const QString &start, const QString &end);
    void registers();
    void libraries();

public slots:
    void slotRawGDBMemoryView(char *buf);

private slots:
    void slotDisassemble();
    void slotMemoryDump();

private:
    KLineEdit *start_, *end_;
    QMultiLineEdit *output_;
};

}

#endif
