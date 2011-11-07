/***************************************************************************
    begin                : Tue Oct 5 1999
    copyright            : (C) 1999 by John Birch
    email                : jbb@kdevelop.org
 **************************************************************************
 * Copyright 2006 Vladimir Prus <ghost@cs.msu.su>
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "memviewdlg.h"
#include "gdbcommand.h"
#include "gdbglobal.h"
#include "debuggerplugin.h"

#include <klineedit.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kstdguiitem.h>
#include <kdeversion.h>
#include <kdebug.h>
#include <kiconloader.h>

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QVariant>
#include <QMenu>

#include <qtoolbox.h>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QContextMenuEvent>
#include <QGridLayout>
#include <QVBoxLayout>

#include <kmessagebox.h>

#include <khexedit/byteseditinterface.h>

#include <ctype.h>

#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>

#include "debugsession.h"

// **************************************************************************
//
// Dialog allows the user to enter
//  - A starting address
//  - An ending address
//
//  this can be in the form
//            functiom/method name
//            variable address (ie &Var, str)
//            Memory address 0x8040abc
//
//  When disassembling and you enter a method name without an
//  ending address then the whole method is disassembled.
//  No data means disassemble the method we're curently in.(from the
//  start of the method)
//
// click ok button to send the request to gdb
// the output is returned (some time later) in the raw data slot
// and displayed as is, so it's rather crude, but it works!
// **************************************************************************

namespace GDBDebugger
{
    /** Container for controls that select memory range.

        The memory range selection is embedded into memory view widget,
        it's not a standalone dialog. However, we want to have easy way
        to hide/show all controls, so we group them in this class.
    */
    class MemoryRangeSelector : public QWidget
    {
    public:
        KLineEdit* startAddressLineEdit;
        KLineEdit* amountLineEdit;
        QPushButton* okButton;
        QPushButton* cancelButton;

        MemoryRangeSelector(QWidget* parent)
        : QWidget(parent)
        {
            QVBoxLayout* l = new QVBoxLayout(this);

            // Grid layout: labels + address field
            QGridLayout* gl = new QGridLayout(this);
            l->addLayout(gl);

            QLabel* l1 = new QLabel(i18n("Start"), this);
            gl->addWidget(l1, 0, 1);

            startAddressLineEdit = new KLineEdit(this);
            gl->addWidget(startAddressLineEdit, 0, 3);

            QLabel* l2 = new QLabel(i18n("Amount"), this);
            gl->addWidget(l2, 2, 1);

            amountLineEdit = new KLineEdit(this);
            gl->addWidget(amountLineEdit, 2, 3);

            l->addSpacing(2);

            QHBoxLayout* hb = new QHBoxLayout(this);
            l->addLayout(hb);
            hb->addStretch();

            okButton = new QPushButton(i18n("OK"), this);
            hb->addWidget(okButton);

            cancelButton = new QPushButton(i18n("Cancel"), this);
            hb->addWidget(cancelButton);

            l->addSpacing(2);
            setLayout(l);

            connect(startAddressLineEdit, SIGNAL(returnPressed()),
                    okButton, SLOT(animateClick()));

            connect(amountLineEdit, SIGNAL(returnPressed()),
                    okButton, SLOT(animateClick()));
        }
    };



    MemoryView::MemoryView(CppDebuggerPlugin* plugin, GDBController* controller,
                           QWidget* parent)
    : QWidget(parent),
      controller_(controller),
      // New memory view can be created only when debugger is active,
      // so don't set s_appNotStarted here.
      khexedit2_real_widget(0),
      amount_(0), data_(0),
      debuggerState_(0)
    {
        setWindowTitle(i18n("Memory view"));
        emit captionChanged(windowTitle());

        initWidget();

        if (isOk())
            slotEnableOrDisable();

        connect(KDevelop::ICore::self()->debugController(), 
                SIGNAL(currentSessionChanged(KDevelop::IDebugSession*)),
                SLOT(currentSessionChanged(KDevelop::IDebugSession*)));
        connect(this,        SIGNAL(setViewShown(bool)),
                plugin,      SLOT(slotShowView(bool)));
    }

    void MemoryView::currentSessionChanged(KDevelop::IDebugSession* s)
    {
        DebugSession *session = qobject_cast<DebugSession*>(s);
        if (!session) return;
        connect( session, 
                 SIGNAL(gdbStateChanged(DBGStateFlags,DBGStateFlags)),
                 SLOT(slotStateChanged(DBGStateFlags,DBGStateFlags)));
    }

    void MemoryView::initWidget()
    {
        QVBoxLayout *l = new QVBoxLayout(this);

        khexedit2_widget = KHE::createBytesEditWidget(this);

        bool ok_ = false;

        if (khexedit2_widget)
        {
            QWidget* real_widget = khexedit2_widget->findChild<QWidget*>("BytesEdit");

            if (real_widget)
            {
                ok_ = true;

                connect(real_widget, SIGNAL(bufferChanged(int,int)),
                        this, SLOT(memoryEdited(int,int)));

                khexedit2_real_widget = real_widget;

                QVariant resize_style(2); // full size usage.
                real_widget->setProperty("ResizeStyle", resize_style);

                //QVariant group(8);
                //real_widget->setProperty("StartOffset", start);
                //real_widget->setProperty("NoOfBytesPerLine", group);

                // HACK: use hardcoded constant taht should match
                // khexedit2
                // 3 -- binary
                // 1 -- decimal
                // 0 -- hex
                //QVariant coding(3);
                //real_widget->setProperty("Coding", coding);

                //QVariant gap(32);
                //real_widget->setProperty("BinaryGapWidth", gap);

            }
            else
            {
                delete khexedit2_widget;
            }
        }

        if (ok_) {

            rangeSelector_ = new MemoryRangeSelector(this);
            l->addWidget(rangeSelector_);

            connect(rangeSelector_->okButton, SIGNAL(clicked()),
                    this,                     SLOT(slotChangeMemoryRange()));


            connect(rangeSelector_->cancelButton,  SIGNAL(clicked()),
                    this,                         SLOT(slotHideRangeDialog()));

            connect(rangeSelector_->startAddressLineEdit,
                    SIGNAL(textChanged(QString)),
                    this,
                    SLOT(slotEnableOrDisable()));

            connect(rangeSelector_->amountLineEdit,
                    SIGNAL(textChanged(QString)),
                    this,
                    SLOT(slotEnableOrDisable()));

            l->addWidget(khexedit2_widget);

        } else {

            QTextEdit* edit = new QTextEdit(this);
            l->addWidget(edit);

            edit->setText(
                "<h1>Not available</h1>"
                "<p>Could not open the khexedit2 library. "
                "Make sure that the KHexEdit package (part of kdeutils) is installed. "
                "Specifically, check for the following files:"
                "<ul><li>libkhexeditcommon.so.0.0.0\n"
                "<li>libkbyteseditwidget.so\n"
                "<li>kbyteseditwidget.desktop\n"
                "</ul>");
        }

    }

    void MemoryView::debuggerStateChanged(DBGStateFlags state)
    {
        if (isOk())
        {
            debuggerState_ = state;
            slotEnableOrDisable();
        }
    }


    void MemoryView::slotHideRangeDialog()
    {
        rangeSelector_->hide();
    }

    void MemoryView::slotChangeMemoryRange()
    {
        DebugSession *session = qobject_cast<DebugSession*>(KDevelop::ICore::self()->debugController()->currentSession());
        if (!session) return;
        session->addCommand(
            new ExpressionValueCommand(
                rangeSelector_->amountLineEdit->text(),
                this, &MemoryView::sizeComputed));
    }

    void MemoryView::sizeComputed(const QString& size)
    {
        DebugSession *session = qobject_cast<DebugSession*>(KDevelop::ICore::self()->debugController()->currentSession());
        if (!session) return;
        session->addCommand(
            new
            GDBCommand(
                GDBMI::DataReadMemory,
                QString("%1 x 1 1 %2")
                .arg(rangeSelector_->startAddressLineEdit->text())
                .arg(size),
                this,
                &MemoryView::memoryRead));
    }

    void MemoryView::memoryRead(const GDBMI::ResultRecord& r)
    {
        const GDBMI::Value& content = r["memory"][0]["data"];

        amount_ = content.size();

        startAsString_ = rangeSelector_->startAddressLineEdit->text();
        amountAsString_ = rangeSelector_->amountLineEdit->text();
        start_ = startAsString_.toUInt(0, 0);

        setWindowTitle(i18np("%2 (1 byte)","%2 (%1 bytes)",amount_,startAsString_));
        emit captionChanged(windowTitle());

        KHE::BytesEditInterface* bytesEditor
            = KHE::bytesEditInterface(khexedit2_widget);

        delete[] this->data_;
        this->data_ = new char[amount_];
        for(int i = 0; i < content.size(); ++i)
        {
            this->data_[i] = content[i].literal().toInt(0, 16);
        }


        bytesEditor->setData( this->data_, amount_ );
        bytesEditor->setReadOnly(false);
        // Overwrite data, not insert new
        bytesEditor->setOverwriteMode( true );
        // Not sure this is needed, but prevent
        // inserting new data.
        bytesEditor->setOverwriteOnly( true );

        QVariant start_v(start_);
        khexedit2_real_widget->setProperty("FirstLineOffset", start_v);

        //QVariant bsw(0);
        //khexedit2_real_widget->setProperty("ByteSpacingWidth", bsw);

        // HACK: use hardcoded constant taht should match
        // khexedit2
        // 3 -- binary
        // 1 -- decimal
        // 0 -- hex
        //QVariant coding(1);
        //khexedit2_real_widget->setProperty("Coding", coding);


        slotHideRangeDialog();
    }


    void MemoryView::memoryEdited(int start, int end)
    {
        DebugSession *session = qobject_cast<DebugSession*>(KDevelop::ICore::self()->debugController()->currentSession());
        if (!session) return;
        for(int i = start; i <= end; ++i)
        {
            session->addCommand(
                new GDBCommand(GDBMI::GdbSet,
                    QString("*(char*)(%1 + %2) = %3")
                        .arg(start_)
                        .arg(i)
                        .arg(QString::number(data_[i]))));
        }
    }

    void MemoryView::contextMenuEvent ( QContextMenuEvent * e )
    {
        if (!isOk())
            return;

        QMenu menu;

        bool app_running = !(debuggerState_ & s_appNotStarted);

        QAction* range = menu.addAction(i18n("Change memory range"));
        // If address selector is show, 'set memory range' can't
        // do anything more.
        range->setEnabled(app_running && !rangeSelector_->isVisible());

        QAction* reload = menu.addAction(i18n("Reload"));
        // If amount is zero, it means there's not data yet, so
        // reloading does not make sense.
        reload->setEnabled(app_running && amount_ != 0);

        QAction* close = menu.addAction(i18n("Close this view"));

        QAction* result = menu.exec(e->globalPos());

        if (result == range)
        {
            rangeSelector_->startAddressLineEdit->setText(startAsString_);
            rangeSelector_->amountLineEdit->setText(amountAsString_);

            rangeSelector_->show();
            rangeSelector_->startAddressLineEdit->setFocus();
        }
        if (result == reload)
        {
            // We use numeric start_ and amount_ stored in this,
            // not textual startAsString_ and amountAsString_,
            // because program position might have changes and expressions
            // are no longer valid.
            DebugSession *session = qobject_cast<DebugSession*>(KDevelop::ICore::self()->debugController()->currentSession());
            if (session) {
                session->addCommand(
                    new
                    GDBCommand(
                        GDBMI::DataReadMemory,
                        QString("%1 x 1 1 %2")
                        .arg(start_).arg(amount_),
                        this,
                        &MemoryView::memoryRead));
            }
        }

        if (result == close)
            delete this;


    }

    bool MemoryView::isOk() const
    {
        return khexedit2_real_widget;
    }

    void MemoryView::slotEnableOrDisable()
    {
        bool app_started = !(debuggerState_ & s_appNotStarted);

        bool enabled_ = app_started &&
            !rangeSelector_->startAddressLineEdit->text().isEmpty() &&
            !rangeSelector_->amountLineEdit->text().isEmpty();

        rangeSelector_->okButton->setEnabled(enabled_);
    }


    ViewerWidget::ViewerWidget(CppDebuggerPlugin* plugin, QWidget* parent)
    : QWidget(parent),
      m_plugin(plugin)
    {
//         setWindowIcon(KIcon("math_brace"));
        setWindowIcon(KIcon("debugger"));
        setWindowTitle(i18n("Special debugger views"));

        QVBoxLayout *l = new QVBoxLayout(this);

        toolBox_ = new QToolBox(this);
        l->addWidget(toolBox_);

        connect(plugin, SIGNAL(addMemoryView()), this, SLOT(slotAddMemoryView()));
    }

    void ViewerWidget::slotAddMemoryView()
    {
        // For unclear reasons, this call, that indirectly
        // does
        //
        //    mainWindow()->setViewAvailable(this)
        //    mainWindow()->raiseView(this)
        //
        // should be done before creating the child widget.
        // Otherwise, the child widget won't be freely resizable --
        // there will be not-so-small minimum size.
        // Problem exists both with KMDI and S/IDEAL.

        setViewShown(true);

        MemoryView* widget = new MemoryView(m_plugin, controller_, this);
        toolBox_->addItem(widget, widget->windowTitle());
        toolBox_->setCurrentIndex(toolBox_->indexOf(widget));
        memoryViews_.push_back(widget);

        connect(widget, SIGNAL(captionChanged(QString)),
                this, SLOT(slotChildCaptionChanged(QString)));

        connect(widget, SIGNAL(destroyed(QObject*)),
                this, SLOT(slotChildDestroyed(QObject*)));
    }

    void ViewerWidget::slotStateChanged(DBGStateFlags oldState, DBGStateFlags newState)
    {
        Q_UNUSED(oldState);
        for(int i = 0; i < memoryViews_.size(); ++i)
        {
            memoryViews_[i]->debuggerStateChanged(newState);
        }
    }

    void ViewerWidget::slotChildCaptionChanged(const QString& caption)
    {
        const QWidget* s = static_cast<const QWidget*>(sender());
        QWidget* ncs = const_cast<QWidget*>(s);
        QString cap = caption;
        // Prevent intepreting '&' as accelerator specifier.
        cap.replace('&', "&&");
        toolBox_->setItemText(toolBox_->indexOf(ncs), cap);
    }

    void ViewerWidget::slotChildDestroyed(QObject* child)
    {
        QList<MemoryView*>::iterator i, e;
        for(i = memoryViews_.begin(), e = memoryViews_.end(); i != e; ++i)
        {
            if (*i == child)
            {
                memoryViews_.erase(i);
                break;
            }
        }

        if (toolBox_->count() == 0)
            setViewShown(false);
    }

// **************************************************************************
// **************************************************************************
// **************************************************************************

}

#include "memviewdlg.moc"
