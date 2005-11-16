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

#include "memviewdlg.h"
#include "gdbcontroller.h"

#include <kbuttonbox.h>
#include <klineedit.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kstdguiitem.h>
#include <kdeversion.h>
#include <kdebug.h>
#include <kiconloader.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qvariant.h>
#include <qpopupmenu.h>
#include <qhbox.h>
#include <qtoolbox.h>
#include <qtextedit.h>

#include <kmessagebox.h>

#include <khexedit/byteseditinterface.h>

#include <ctype.h>

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
// click ok buton to send the request to gdb
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
        QLineEdit* startAddressLineEdit;
        QLineEdit* amountLineEdit;
        QPushButton* okButton;
        QPushButton* cancelButton;

        MemoryRangeSelector(QWidget* parent) 
        : QWidget(parent)
        {
            QVBoxLayout* l = new QVBoxLayout(this);

            // Grid layout: labels + address field
            QGridLayout* gl = new QGridLayout(l);

            gl->setColSpacing(0, 2);
            gl->setColSpacing(1, 4);
            gl->setRowSpacing(1, 2);
            
            QLabel* l1 = new QLabel(i18n("Start"), this);
            gl->addWidget(l1, 0, 1);
            
            startAddressLineEdit = new QLineEdit(this);            
            gl->addWidget(startAddressLineEdit, 0, 3);
            
            QLabel* l2 = new QLabel(i18n("Amount"), this);
            gl->addWidget(l2, 2, 1);
            
            amountLineEdit = new QLineEdit(this);
            gl->addWidget(amountLineEdit, 2, 3);

            l->addSpacing(2);
                        
            QHBoxLayout* hb = new QHBoxLayout(l);
            hb->addStretch();
            
            okButton = new QPushButton(i18n("OK"), this);
            hb->addWidget(okButton);
            
            cancelButton = new QPushButton(i18n("Cancel"), this);
            hb->addWidget(cancelButton);            

            l->addSpacing(2);

            connect(startAddressLineEdit, SIGNAL(returnPressed()),
                    okButton, SLOT(animateClick()));

            connect(amountLineEdit, SIGNAL(returnPressed()),
                    okButton, SLOT(animateClick()));
        }
    };


    
    MemoryView::MemoryView(QWidget* parent, const char* name)
    : QWidget(parent, name), 
      // New memory view can be created only when debugger is active,
      // so don't set s_appNotStarted here.
      khexedit2_real_widget(0),
      amount_(0), data_(0),
      debuggerState_(0)
    {
        setCaption(i18n("Memory view"));
        emit captionChanged(caption());

        initWidget();

        if (isOk())
            slotEnableOrDisable();
    }

    void MemoryView::initWidget()
    {
        QVBoxLayout *l = new QVBoxLayout(this, 0, 0);

        khexedit2_widget = KHE::createBytesEditWidget(this);               

        bool ok_ = false;

        if (khexedit2_widget)
        {
            QWidget* real_widget = (QWidget*)
                khexedit2_widget->child("BytesEdit");
            
            if (real_widget)
            {
                ok_ = true;

                connect(real_widget, SIGNAL(bufferChanged(int, int)),
                        this, SLOT(memoryEdited(int, int)));
                
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
                    SIGNAL(textChanged(const QString&)),
                    this,
                    SLOT(slotEnableOrDisable()));
            
            connect(rangeSelector_->amountLineEdit,
                    SIGNAL(textChanged(const QString&)),
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

    void MemoryView::debuggerStateChanged(int state)
    {
        if (isOk())
        {
            debuggerState_ = state;
            slotEnableOrDisable();
        }
    }

    void MemoryView::memoryContentAvailable(unsigned start, unsigned amount,
                                            char* data)
    {
        start_ = start;
        amount_ = amount;

        startAsString_ = rangeSelector_->startAddressLineEdit->text();
        amountAsString_ = rangeSelector_->amountLineEdit->text();

        setCaption(QString("%1 (%2 bytes)")
                   .arg(startAsString_).arg(amount));
        emit captionChanged(caption());

        KHE::BytesEditInterface* bytesEditor
            = KHE::bytesEditInterface(khexedit2_widget);

        delete[] this->data_;
        this->data_ = data;
                    
        bytesEditor->setData( this->data_, amount );
        bytesEditor->setReadOnly(false);
        // Overwrite data, not insert new
        bytesEditor->setOverwriteMode( true );
        // Not sure this is needed, but prevent
        // inserting new data.
        bytesEditor->setOverwriteOnly( true );
                   
        QVariant start_v(start);
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

    void MemoryView::slotHideRangeDialog()
    {
        rangeSelector_->hide();
    }

    void MemoryView::slotChangeMemoryRange()
    {
        ValueCallback* cb = this;
        emit evaluateExpression(cb, rangeSelector_->amountLineEdit->text());
    }

    void MemoryView::updateValue(char* data)
    {
        char* value = 0;

        if (data || *data == '$')
        {
            char* p = strchr(data, '=');
            if (p)
            {
                ++p;
                while(*p && isspace(*p))
                    ++p;

                value = p;
                while(*p && '0' <= *p && *p <= '9')
                    ++p;
                while(*p && isspace(*p))
                    ++p;
                if (*p != '\0')
                    value = 0;                    
            }
        }

        if (!value)
        {
            KMessageBox::error(
                this, 
                i18n(
                    "<b>Bad memory amount</b>"
                    "<p>The gdb debugger could not evaluate the amount "
                    "expression you've given."),
                i18n("Bad memory amount"));
        }
        else
        {
            emit getMemory(rangeSelector_->startAddressLineEdit->text(), value);
        }
    }

    
    void MemoryView::memoryEdited(int start, int end)
    {        
        for(int i = start; i <= end; ++i)
        {
            emit setValue(QString("*(char*)(%1 + %2)").arg(start_).arg(i),
                          QString::number(data_[i]));
        }
    }
    
    void MemoryView::contextMenuEvent ( QContextMenuEvent * e ) 
    {
        if (!isOk())
            return;
        
        QPopupMenu menu;

        bool app_running = !(debuggerState_ & s_appNotStarted);

        int idRange = menu.insertItem(i18n("Change memory range"));
        // If address selector is show, 'set memory range' can't
        // do anything more.
        menu.setItemEnabled(idRange, 
                            app_running && !rangeSelector_->isShown());
        int idReload = menu.insertItem(i18n("Reload"));
        // If amount is zero, it means there's not data yet, so
        // reloading does not make sense.
        menu.setItemEnabled(idReload, app_running && amount_ != 0);        
        int idClose = menu.insertItem(i18n("Close this view"));
        
        int result = menu.exec(e->globalPos());

        if (result == idRange)
        {
            rangeSelector_->startAddressLineEdit->setText(startAsString_);
            rangeSelector_->amountLineEdit->setText(amountAsString_);

            rangeSelector_->show();
            rangeSelector_->startAddressLineEdit->setFocus();
        }        
        if (result == idReload)
        {
            // We use numeric start_ and amount_ stored in this, 
            // not textual startAsString_ and amountAsString_,
            // because program position might have changes and expressions
            // are no longer valid.
            emit getMemory(QString::number(start_), QString::number(amount_));
        }

        if (result == idClose)
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


    ViewerWidget::ViewerWidget(QWidget* parent, 
                               const char* name) : QWidget(parent, name) 
    {
        setIcon(SmallIcon("math_brace"));
        
        QVBoxLayout *l = new QVBoxLayout(this, 0, 0);
        
        toolBox_ = new QToolBox(this);
        l->addWidget(toolBox_);
    }

    void ViewerWidget::slotGetMemory(
        const QString& start, const QString& amount)
    {
        const QObject* obj = sender();
        // The signal should come from MemoryView widget.
        const MemoryView* mv = dynamic_cast<const MemoryView*>(obj);
        Q_ASSERT(mv);
        if (mv)
        {
            // 'sender()' returns 'const', but eventually we
            // need to modify the widget...
            MemoryView* mv2 = const_cast<MemoryView*>(mv);
            emit getMemory(mv2, start, amount);
        }
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

        MemoryView* widget = new MemoryView(this);
        toolBox_->addItem(widget, widget->caption());
        toolBox_->setCurrentItem(widget);
        memoryViews_.push_back(widget);

        connect(widget, SIGNAL(captionChanged(const QString&)),
                this, SLOT(slotChildCaptionChanged(const QString&)));

        connect(widget, SIGNAL(getMemory(const QString&, const QString&)),
                this, SLOT(slotGetMemory(const QString&, const QString&)));

        connect(widget, SIGNAL(evaluateExpression(ValueCallback*, 
                                                  const QString&)),
                this,   SIGNAL(evaluateExpression(ValueCallback*, 
                                                  const QString&)));

        connect(widget, SIGNAL(setValue(const QString&, const QString&)),
                this,   SIGNAL(setValue(const QString&, const QString&)));

        connect(widget, SIGNAL(destroyed(QObject*)),
                this, SLOT(slotChildDestroyed(QObject*)));
    }

    void ViewerWidget::slotDebuggerState(const QString&, int state)
    {
        for(unsigned i = 0; i < memoryViews_.size(); ++i)
        {
            memoryViews_[i]->debuggerStateChanged(state);
        }
    }

    void ViewerWidget::slotChildCaptionChanged(const QString& caption)
    {
        const QWidget* s = static_cast<const QWidget*>(sender());
        QWidget* ncs = const_cast<QWidget*>(s);
        QString cap = caption;
        // Prevent intepreting '&' as accelerator specifier.
        cap.replace("&", "&&");
        toolBox_->setItemLabel(toolBox_->indexOf(ncs), cap);
    }

    void ViewerWidget::slotChildDestroyed(QObject* child)
    {
        QValueVector<MemoryView*>::iterator i, e;
        for(i = memoryViews_.begin(), e = memoryViews_.end(); i != e; ++i)
        {
            if (*i == child)
            {
                memoryViews_.erase(i);
                break;
            }
        }

        if (toolBox_->count() == 0)
            setShown(false);
    }

MemoryViewDialog::MemoryViewDialog(QWidget *parent, const char *name)
    : KDialog(parent, name, true),      // modal
      start_(new KLineEdit(this)),
      end_(new KLineEdit(this)),
      output_(new QMultiLineEdit(this))
{
    setCaption(i18n("Memory/Misc Viewer"));
    // Make the top-level layout; a vertical box to contain all widgets
    // and sub-layouts.
    QBoxLayout *topLayout = new QVBoxLayout(this, 5);

    QGridLayout *grid = new QGridLayout(2, 2, 5);
    topLayout->addLayout(grid);

    QLabel *label = new QLabel(start_, i18n("&Start:"), this);
    label->setBuddy(start_);
    grid->addWidget(label, 0, 0);
    grid->setRowStretch(0, 0);
    grid->addWidget(start_, 1, 0);
    grid->setRowStretch(1, 0);

    label = new QLabel(end_, i18n("Amount/&End address (memory/disassemble):"), this);
    label->setBuddy(end_);
    grid->addWidget(label, 0, 1);
    grid->addWidget(end_, 1, 1);

    label = new QLabel(i18n("Memory&View:"), this);
    label->setBuddy(output_);
    topLayout->addWidget(label, 0);
    topLayout->addWidget(output_, 5);
    output_->setFont(KGlobalSettings::fixedFont());

    KButtonBox *buttonbox = new KButtonBox(this, Horizontal, 5);
    QPushButton *memoryDump = buttonbox->addButton(i18n("&Memory"));
    QPushButton *disassemble = buttonbox->addButton(i18n("&Disassemble"));
    QPushButton *registers = buttonbox->addButton(i18n("&Registers"));
    QPushButton *libraries = buttonbox->addButton(i18n("&Libraries"));
#if KDE_IS_VERSION( 3, 2, 90 )
    QPushButton *cancel = buttonbox->addButton(KStdGuiItem::cancel());
#else
    QPushButton *cancel = buttonbox->addButton(KStdGuiItem::cancel().text());
#endif
    memoryDump->setDefault(true);
    buttonbox->layout();
    topLayout->addWidget(buttonbox);

    start_->setFocus();

    connect(memoryDump, SIGNAL(clicked()), SLOT(slotMemoryDump()));
    connect(disassemble, SIGNAL(clicked()), SLOT(slotDisassemble()));
    connect(registers, SIGNAL(clicked()), SIGNAL(registers()));
    connect(libraries, SIGNAL(clicked()), SIGNAL(libraries()));
    connect(cancel, SIGNAL(clicked()), SLOT(reject()));
}

// **************************************************************************

MemoryViewDialog::~MemoryViewDialog()
{
}

// **************************************************************************

void MemoryViewDialog::slotRawGDBMemoryView(char *buf)
{
    // just display the resultant output from GDB in the edit box
    output_->clear();
    output_->insertLine(buf);
    output_->setCursorPosition(0,0);
}

// **************************************************************************

// get gdb to supply the disassembled data.
void MemoryViewDialog::slotDisassemble()
{
    QString start(start_->text());
    QString end(end_->text());
    emit disassemble(start, end);
}

// **************************************************************************

void MemoryViewDialog::slotMemoryDump()
{ 
    QString start(start_->text());
    QString size(end_->text());
    emit memoryDump(start, size);
}


// **************************************************************************
// **************************************************************************
// **************************************************************************

}

#include "memviewdlg.moc"
