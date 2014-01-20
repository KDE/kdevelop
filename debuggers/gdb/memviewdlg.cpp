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

#include <kaction.h>
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
#include <khexedit/charcolumninterface.h>
#include <khexedit/valuecolumninterface.h>

#include <ctype.h>

#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>

#include "debugsession.h"

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



    MemoryView::MemoryView(QWidget* parent)
    : QWidget(parent),
      // New memory view can be created only when debugger is active,
      // so don't set s_appNotStarted here.
      khexedit2_widget(0),
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
    }

    void MemoryView::currentSessionChanged(KDevelop::IDebugSession* s)
    {
        DebugSession *session = qobject_cast<DebugSession*>(s);
        if (!session) return;
        connect(session,
                SIGNAL(gdbStateChanged(DBGStateFlags,DBGStateFlags)),
                SLOT(slotStateChanged(DBGStateFlags,DBGStateFlags)));
    }

    void MemoryView::slotStateChanged(DBGStateFlags oldState, DBGStateFlags newState)
    {
        Q_UNUSED(oldState);
        debuggerStateChanged(newState);
    }

    void MemoryView::initWidget()
    {
        QVBoxLayout *l = new QVBoxLayout(this);

        khexedit2_widget = KHE::createBytesEditWidget(this);
        if (!khexedit2_widget)
        {
            QTextEdit* edit = new QTextEdit(this);
            l->addWidget(edit);

            edit->setText(
                "<h1>Not Available</h1>"
                "<p>Could not open a KHexEdit2 interface. "
                "Installing Okteta should provide the required components.</p>");
            return;
        }

        KHE::BytesEditInterface *bytesEdit = KHE::bytesEditInterface(khexedit2_widget);
        if (bytesEdit)
        {
            bytesEdit->setReadOnly(false);
            bytesEdit->setOverwriteMode(true);
            bytesEdit->setOverwriteOnly(true);
            bytesEdit->setAutoDelete(false);
        }

        KHE::ValueColumnInterface *valueColumn = KHE::valueColumnInterface(khexedit2_widget);
        if (valueColumn)
        {
            valueColumn->setCoding(KHE::ValueColumnInterface::HexadecimalCoding);
            valueColumn->setNoOfGroupedBytes(4);
            valueColumn->setByteSpacingWidth(2);
            valueColumn->setGroupSpacingWidth(12);
            valueColumn->setResizeStyle(KHE::ValueColumnInterface::LockGrouping);
        }

        KHE::CharColumnInterface *charColumn = KHE::charColumnInterface(khexedit2_widget);
        if(charColumn)
        {
            charColumn->setShowUnprintable(false);
            charColumn->setSubstituteChar('*');
        }

        rangeSelector_ = new MemoryRangeSelector(this);
        l->addWidget(rangeSelector_);

        connect(rangeSelector_->okButton, SIGNAL(clicked()),
                this,                     SLOT(slotChangeMemoryRange()));

        connect(rangeSelector_->cancelButton, SIGNAL(clicked()),
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
        DebugSession *session = qobject_cast<DebugSession*>(
            KDevelop::ICore::self()->debugController()->currentSession());
        if (!session) return;

        session->addCommand(new ExpressionValueCommand(
                rangeSelector_->amountLineEdit->text(),
                this, &MemoryView::sizeComputed));
    }

    void MemoryView::sizeComputed(const QString& size)
    {
        DebugSession *session = qobject_cast<DebugSession*>(
            KDevelop::ICore::self()->debugController()->currentSession());
        if (!session) return;

        session->addCommand(new GDBCommand(GDBMI::DataReadMemory,
                QString("%1 x 1 1 %2")
                    .arg(rangeSelector_->startAddressLineEdit->text())
                    .arg(size),
                this,
                &MemoryView::memoryRead));
    }

    void MemoryView::memoryRead(const GDBMI::ResultRecord& r)
    {
        const GDBMI::Value& content = r["memory"][0]["data"];
        bool startStringConverted;
        start_ = r["addr"].literal().toULongLong(&startStringConverted, 16);
        amount_ = content.size();

        startAsString_ = rangeSelector_->startAddressLineEdit->text();
        amountAsString_ = rangeSelector_->amountLineEdit->text();

        setWindowTitle(i18np("%2 (1 byte)","%2 (%1 bytes)",amount_,startAsString_));
        emit captionChanged(windowTitle());

        KHE::BytesEditInterface* bytesEditor = KHE::bytesEditInterface(khexedit2_widget);
        bytesEditor->setData(this->data_, 0);

        delete[] this->data_;
        this->data_ = new char[amount_];
        for(int i = 0; i < content.size(); ++i)
        {
            this->data_[i] = content[i].literal().toInt(0, 16);
        }

        bytesEditor->setData(this->data_, amount_);

        slotHideRangeDialog();
    }


    void MemoryView::memoryEdited(int start, int end)
    {
        DebugSession *session = qobject_cast<DebugSession*>(
            KDevelop::ICore::self()->debugController()->currentSession());
        if (!session) return;

        for(int i = start; i <= end; ++i)
        {
            session->addCommand(new GDBCommand(GDBMI::GdbSet,
                    QString("*(char*)(%1 + %2) = %3")
                        .arg(start_)
                        .arg(i)
                        .arg(QString::number(data_[i]))));
        }
    }

    void MemoryView::contextMenuEvent(QContextMenuEvent *e)
    {
        if (!isOk())
            return;

        KHE::BytesEditInterface *bytesEdit = KHE::bytesEditInterface(khexedit2_widget);
        KHE::ValueColumnInterface *valueColumn = KHE::valueColumnInterface(khexedit2_widget);

        QMenu menu;

        bool app_running = !(debuggerState_ & s_appNotStarted);

        QAction* reload = menu.addAction(i18n("&Reload"));
        reload->setIcon(KIcon("view-refresh"));
        reload->setEnabled(app_running && amount_ != 0);

        QActionGroup *formatGroup = NULL;
        QActionGroup *groupingGroup = NULL;
        if (valueColumn)
        {
            // make Format menu with action group
            QMenu *formatMenu = new QMenu(i18n("&Format"));
            formatGroup = new QActionGroup(formatMenu);

            QAction *binary = formatGroup->addAction(i18n("&Binary"));
            binary->setData(KHE::ValueColumnInterface::BinaryCoding);
            binary->setShortcut(Qt::Key_B);
            formatMenu->addAction(binary);

            QAction *octal = formatGroup->addAction(i18n("&Octal"));
            octal->setData(KHE::ValueColumnInterface::OctalCoding);
            octal->setShortcut(Qt::Key_O);
            formatMenu->addAction(octal);

            QAction *decimal = formatGroup->addAction(i18n("&Decimal"));
            decimal->setData(KHE::ValueColumnInterface::DecimalCoding);
            decimal->setShortcut(Qt::Key_D);
            formatMenu->addAction(decimal);

            QAction *hex = formatGroup->addAction(i18n("&Hexadecimal"));
            hex->setData(KHE::ValueColumnInterface::HexadecimalCoding);
            hex->setShortcut(Qt::Key_H);
            formatMenu->addAction(hex);

            foreach(QAction* act, formatGroup->actions())
            {
                act->setCheckable(true);
                act->setChecked(act->data().toInt() == valueColumn->coding());
                act->setShortcutContext(Qt::WidgetWithChildrenShortcut);
            }

            menu.addMenu(formatMenu);


            // make Grouping menu with action group
            QMenu *groupingMenu = new QMenu(i18n("&Grouping"));
            groupingGroup = new QActionGroup(groupingMenu);

            QAction *group0 = groupingGroup->addAction(i18n("&0"));
            group0->setData(0);
            group0->setShortcut(Qt::Key_0);
            groupingMenu->addAction(group0);

            QAction *group1 = groupingGroup->addAction(i18n("&1"));
            group1->setData(1);
            group1->setShortcut(Qt::Key_1);
            groupingMenu->addAction(group1);

            QAction *group2 = groupingGroup->addAction(i18n("&2"));
            group2->setData(2);
            group2->setShortcut(Qt::Key_2);
            groupingMenu->addAction(group2);

            QAction *group4 = groupingGroup->addAction(i18n("&4"));
            group4->setData(4);
            group4->setShortcut(Qt::Key_4);
            groupingMenu->addAction(group4);

            QAction *group8 = groupingGroup->addAction(i18n("&8"));
            group8->setData(8);
            group8->setShortcut(Qt::Key_8);
            groupingMenu->addAction(group8);

            QAction *group16 = groupingGroup->addAction(i18n("1&6"));
            group16->setData(16);
            group16->setShortcut(Qt::Key_6);
            groupingMenu->addAction(group16);

            foreach(QAction* act, groupingGroup->actions())
            {
                act->setCheckable(true);
                act->setChecked(act->data().toInt() == valueColumn->noOfGroupedBytes());
                act->setShortcutContext(Qt::WidgetWithChildrenShortcut);
            }

            menu.addMenu(groupingMenu);
        }

        QAction* write = menu.addAction(i18n("Write changes"));
        write->setIcon(KIcon("document-save"));
        write->setEnabled(app_running && bytesEdit && bytesEdit->isModified());

        QAction* range = menu.addAction(i18n("Change memory range"));
        range->setEnabled(app_running && !rangeSelector_->isVisible());
        range->setIcon(KIcon("document-edit"));

        QAction* close = menu.addAction(i18n("Close this view"));
        close->setIcon(KIcon("window-close"));


        QAction* result = menu.exec(e->globalPos());


        if (result == reload)
        {
            // We use numeric start_ and amount_ stored in this,
            // not textual startAsString_ and amountAsString_,
            // because program position might have changes and expressions
            // are no longer valid.
            DebugSession *session = qobject_cast<DebugSession*>(
                KDevelop::ICore::self()->debugController()->currentSession());
            if (session) {
                session->addCommand(new GDBCommand(GDBMI::DataReadMemory,
                        QString("%1 x 1 1 %2").arg(start_).arg(amount_),
                        this,
                        &MemoryView::memoryRead));
            }
        }

        if (result && formatGroup && formatGroup == result->actionGroup())
            valueColumn->setCoding((KHE::ValueColumnInterface::KCoding)result->data().toInt());

        if (result && groupingGroup && groupingGroup == result->actionGroup())
            valueColumn->setNoOfGroupedBytes(result->data().toInt());

        if (result == write)
        {
            memoryEdited(0, amount_);
            bytesEdit->setModified(false);
        }

        if (result == range)
        {
            rangeSelector_->startAddressLineEdit->setText(startAsString_);
            rangeSelector_->amountLineEdit->setText(amountAsString_);

            rangeSelector_->show();
            rangeSelector_->startAddressLineEdit->setFocus();
        }

        if (result == close)
            delete this;
    }

    bool MemoryView::isOk() const
    {
        return khexedit2_widget;
    }

    void MemoryView::slotEnableOrDisable()
    {
        bool app_started = !(debuggerState_ & s_appNotStarted);

        bool enabled_ = app_started &&
            !rangeSelector_->startAddressLineEdit->text().isEmpty() &&
            !rangeSelector_->amountLineEdit->text().isEmpty();

        rangeSelector_->okButton->setEnabled(enabled_);
    }


    MemoryViewerWidget::MemoryViewerWidget(CppDebuggerPlugin* /*plugin*/, QWidget* parent)
    : QWidget(parent)
    {
        setWindowIcon(KIcon("debugger"));
        setWindowTitle(i18n("Memory viewer"));

        KAction* newMemoryViewerAction = new KAction(this);
        newMemoryViewerAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        newMemoryViewerAction->setText(i18n("New memory viewer"));
        newMemoryViewerAction->setToolTip(i18nc("@info:tooltip", "Open a new memory viewer."));
        newMemoryViewerAction->setIcon(KIcon("window-new"));
        connect(newMemoryViewerAction, SIGNAL(triggered(bool)), this, SLOT(slotAddMemoryView()));
        addAction(newMemoryViewerAction);

        QVBoxLayout *l = new QVBoxLayout(this);

        toolBox_ = new QToolBox(this);
        l->addWidget(toolBox_);

        // Start with one empty memory view.
        slotAddMemoryView();
    }

    void MemoryViewerWidget::slotAddMemoryView()
    {
        MemoryView* widget = new MemoryView(this);
        toolBox_->addItem(widget, widget->windowTitle());
        toolBox_->setCurrentIndex(toolBox_->indexOf(widget));
        memoryViews_.push_back(widget);

        connect(widget, SIGNAL(captionChanged(QString)),
                this, SLOT(slotChildCaptionChanged(QString)));

        connect(widget, SIGNAL(destroyed(QObject*)),
                this, SLOT(slotChildDestroyed(QObject*)));
    }

    void MemoryViewerWidget::slotChildCaptionChanged(const QString& caption)
    {
        const QWidget* s = static_cast<const QWidget*>(sender());
        QWidget* ncs = const_cast<QWidget*>(s);
        QString cap = caption;
        // Prevent intepreting '&' as accelerator specifier.
        cap.replace('&', "&&");
        toolBox_->setItemText(toolBox_->indexOf(ncs), cap);
    }

    void MemoryViewerWidget::slotChildDestroyed(QObject* child)
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
    }

}

#include "memviewdlg.moc"
