/***************************************************************************
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <klocale.h>
#include <kdebug.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <kdialogbase.h>
#include <kdeversion.h>

#include <qapplication.h>
#include <qtooltip.h>
#include <qheader.h>
#include <qstringlist.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qregexp.h>

#include "flagboxes.h"

//---------copy-of-kde-3.1-----------------------------------------------------
#if KDE_VERSION <= 305
# include <qgroupbox.h>
# include <qobjectlist.h>
# include <knotifyclient.h>
# include <kcombobox.h>
# include <assert.h>

class KEditListBoxPrivate
{
public:
    bool m_checkAtEntering;
    int buttons;
};

/**
 * An editable listbox
 *
 * This class provides a editable listbox ;-), this means
 * a listbox which is accompanied by a line edit to enter new
 * items into the listbox and pushbuttons to add and remove
 * items from the listbox and two buttons to move items up and down.
 */
class KEditListBox : public QGroupBox
{
   Q_OBJECT

public:
    /// @since 3.1
    class CustomEditor
    {
    public:
        CustomEditor()
            : m_representationWidget( 0L ),
              m_lineEdit( 0L ) {}
        CustomEditor( QWidget *repWidget, KLineEdit *edit )
            : m_representationWidget( repWidget ),
              m_lineEdit( edit ) {}
        CustomEditor( KComboBox *combo );

        void setRepresentationWidget( QWidget *repWidget ) {
            m_representationWidget = repWidget;
        }
        void setLineEdit( KLineEdit *edit ) {
            m_lineEdit = edit;
        }

        virtual QWidget   *representationWidget() const {
            return m_representationWidget;
        }
        virtual KLineEdit *lineEdit() const {
            return m_lineEdit;
        }

    protected:
        QWidget *m_representationWidget;
        KLineEdit *m_lineEdit;
    };

   public:

      /**
       * Enumeration of the buttons, the listbox offers. Specify them in the
       * constructor in the buttons parameter.
       */
      enum Button { Add = 1, Remove = 2, UpDown = 4, All = Add|Remove|UpDown };

      /**
       * Create an editable listbox.
       *
       * If @p checkAtEntering is true, after every character you type
       * in the line edit KEditListBox will enable or disable
       * the Add-button, depending whether the current content of the
       * line edit is already in the listbox. Maybe this can become a
       * performance hit with large lists on slow machines.
       * If @p checkAtEntering is false,
       * it will be checked if you press the Add-button. It is not
       * possible to enter items twice into the listbox.
       */
      KEditListBox(QWidget *parent = 0, const char *name = 0,
		   bool checkAtEntering=false, int buttons = All );
      /**
       * Create an editable listbox.
       *
       * The same as the other constructor, additionally it takes
       * @title, which will be the title of the frame around the listbox.
       */
      KEditListBox(const QString& title, QWidget *parent = 0,
		   const char *name = 0, bool checkAtEntering=false,
		   int buttons = All );

      /**
       * Another constructor, which allows to use a custom editing widget
       * instead of the standard KLineEdit widget. E.g. you can use a
       * @ref KURLRequester or a @ref KComboBox as input widget. The custom
       * editor must consist of a lineedit and optionally another widget that
       * is used as representation. A KComboBox or a KURLRequester have a
       * KLineEdit as child-widget for example, so the KComboBox is used as
       * the representation widget.
       *
       * @see KURLRequester::customEditor()
       * @since 3.1
       */
      KEditListBox( const QString& title,
                    const CustomEditor &customEditor,
                    QWidget *parent = 0, const char *name = 0,
                    bool checkAtEntering = false, int buttons = All );

      virtual ~KEditListBox();

      /**
       * Return a pointer to the embedded QListBox.
       */
      QListBox* listBox() const     { return m_listBox; }
      /**
       * Return a pointer to the embedded QLineEdit.
       */
      KLineEdit* lineEdit() const     { return m_lineEdit; }
      /**
       * Return a pointer to the Add button
       */
      QPushButton* addButton() const     { return servNewButton; }
      /**
       * Return a pointer to the Remove button
       */
      QPushButton* removeButton() const     { return servRemoveButton; }
      /**
       * Return a pointer to the Up button
       */
      QPushButton* upButton() const     { return servUpButton; }
      /**
       * Return a pointer to the Down button
       */
      QPushButton* downButton() const     { return servDownButton; }

      /**
       * See @ref QListBox::count()
       */
      int count() const   { return int(m_listBox->count()); }
      /**
       * See @ref QListBox::insertStringList()
       */
      void insertStringList(const QStringList& list, int index=-1);
      /**
       * See @ref QListBox::insertStringList()
       */
      void insertStrList(const QStrList* list, int index=-1);
      /**
       * See @ref QListBox::insertStrList()
       */
      void insertStrList(const QStrList& list, int index=-1);
      /**
       * See @ref QListBox::insertStrList()
       */
      void insertStrList(const char ** list, int numStrings=-1, int index=-1);
      /**
       * See @ref QListBox::insertItem()
       */
      void insertItem(const QString& text, int index=-1) {m_listBox->insertItem(text,index);}
      /**
       * Clears both the listbox and the line edit.
       */
      void clear();
      /**
       * See @ref QListBox::text()
       */
      QString text(int index) const { return m_listBox->text(index); }
      /**
       * See @ref QListBox::currentItem()
       */
      int currentItem() const;
      /**
       * See @ref QListBox::currentText()
       */
      QString currentText() const  { return m_listBox->currentText(); }

      /**
       * @returns a stringlist of all items in the listbox
       */
      QStringList items() const;

   signals:
      void changed();

   protected slots:
      //the names should be self-explaining
      void moveItemUp();
      void moveItemDown();
      void addItem();
      void removeItem();
      void enableMoveButtons(int index);
      void typedSomething(const QString& text);

   private:
      QListBox *m_listBox;
      QPushButton *servUpButton, *servDownButton;
      QPushButton *servNewButton, *servRemoveButton;
      KLineEdit *m_lineEdit;

      //this is called in both ctors, to avoid code duplication
      void init( bool checkAtEntering, int buttons,
                 QWidget *representationWidget = 0L );

   protected:
      virtual void virtual_hook( int id, void* data );
   private:
      //our lovely private d-pointer
      KEditListBoxPrivate *d;
};

KEditListBox::KEditListBox(QWidget *parent, const char *name,
			   bool checkAtEntering, int buttons )
    :QGroupBox(parent, name )
{
    init( checkAtEntering, buttons );
}

KEditListBox::KEditListBox(const QString& title, QWidget *parent,
			   const char *name, bool checkAtEntering, int buttons)
    :QGroupBox(title, parent, name )
{
    init( checkAtEntering, buttons );
}

KEditListBox::KEditListBox(const QString& title, const CustomEditor& custom,
                           QWidget *parent, const char *name,
                           bool checkAtEntering, int buttons)
    :QGroupBox(title, parent, name )
{
    m_lineEdit = custom.lineEdit();
    init( checkAtEntering, buttons, custom.representationWidget() );
}

KEditListBox::~KEditListBox()
{
    delete d;
    d=0;
}

void KEditListBox::init( bool checkAtEntering, int buttons,
                         QWidget *representationWidget )
{
    d=new KEditListBoxPrivate;
    d->m_checkAtEntering=checkAtEntering;
    d->buttons = buttons;

    int lostButtons = 0;
    if ( (buttons & Add) == 0 )
        lostButtons++;
    if ( (buttons & Remove) == 0 )
        lostButtons++;
    if ( (buttons & UpDown) == 0 )
        lostButtons += 2;


    servNewButton = servRemoveButton = servUpButton = servDownButton = 0L;
    setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                              QSizePolicy::MinimumExpanding));

    QWidget * gb = this;
    QGridLayout * grid = new QGridLayout(gb, 7 - lostButtons, 2,
                                         KDialog::marginHint(),
                                         KDialog::spacingHint());
    grid->addRowSpacing(0, fontMetrics().lineSpacing());
    for ( int i = 1; i < 7 - lostButtons; i++ )
        grid->setRowStretch(i, 1);

    grid->setMargin(15);

    if ( representationWidget )
        representationWidget->reparent( gb, QPoint(0,0) );
    else
        m_lineEdit=new KLineEdit(gb);

    m_listBox = new QListBox(gb);

    QWidget *editingWidget = representationWidget ?
                             representationWidget : m_lineEdit;
    grid->addMultiCellWidget(editingWidget,1,1,0,1);
    grid->addMultiCellWidget(m_listBox, 2, 6 - lostButtons, 0, 0);
    int row = 2;
    if ( buttons & Add ) {
        servNewButton = new QPushButton(i18n("&Add"), gb);
        servNewButton->setEnabled(false);
        connect(servNewButton, SIGNAL(clicked()), SLOT(addItem()));

        grid->addWidget(servNewButton, row++, 1);
    }

    if ( buttons & Remove ) {
        servRemoveButton = new QPushButton(i18n("&Remove"), gb);
        servRemoveButton->setEnabled(false);
        connect(servRemoveButton, SIGNAL(clicked()), SLOT(removeItem()));

        grid->addWidget(servRemoveButton, row++, 1);
    }

    if ( buttons & UpDown ) {
        servUpButton = new QPushButton(i18n("Move &Up"), gb);
        servUpButton->setEnabled(false);
        connect(servUpButton, SIGNAL(clicked()), SLOT(moveItemUp()));

        servDownButton = new QPushButton(i18n("Move &Down"), gb);
        servDownButton->setEnabled(false);
        connect(servDownButton, SIGNAL(clicked()), SLOT(moveItemDown()));

        grid->addWidget(servUpButton, row++, 1);
        grid->addWidget(servDownButton, row++, 1);
    }

    connect(m_lineEdit,SIGNAL(textChanged(const QString&)),this,SLOT(typedSomething(const QString&)));
    m_lineEdit->setTrapReturnKey(true);
    connect(m_lineEdit,SIGNAL(returnPressed()),this,SLOT(addItem()));
    connect(m_listBox, SIGNAL(highlighted(int)), SLOT(enableMoveButtons(int)));

    // maybe supplied lineedit has some text already
    typedSomething( m_lineEdit->text() );
}

void KEditListBox::typedSomething(const QString& text)
{
    if(currentItem() >= 0) {
        if(currentText() != m_lineEdit->text())
        {
            // IMHO changeItem() shouldn't do anything with the value
            // of currentItem() ... like changing it or emitting signals ...
            // but TT disagree with me on this one (it's been that way since ages ... grrr)
            bool block = m_listBox->signalsBlocked();
            m_listBox->blockSignals( true );
            m_listBox->changeItem(text, currentItem());
            m_listBox->blockSignals( block );
            emit changed();
        }
    }

    if ( !servNewButton )
        return;

    if (!d->m_checkAtEntering)
        servNewButton->setEnabled(!text.isEmpty());
    else
    {
        if (text.isEmpty())
        {
            servNewButton->setEnabled(false);
        }
        else
        {
            StringComparisonMode mode = (StringComparisonMode) (ExactMatch | CaseSensitive );
            bool enable = (m_listBox->findItem( text, mode ) == 0L);
            servNewButton->setEnabled( enable );
        }
    }
}

void KEditListBox::moveItemUp()
{
    if (!m_listBox->isEnabled())
    {
        KNotifyClient::beep();
        return;
    }

    unsigned int selIndex = m_listBox->currentItem();
    if (selIndex == 0)
    {
        KNotifyClient::beep();
        return;
    }

    QListBoxItem *selItem = m_listBox->item(selIndex);
    m_listBox->takeItem(selItem);
    m_listBox->insertItem(selItem, selIndex-1);
    m_listBox->setCurrentItem(selIndex - 1);

    emit changed();
}

void KEditListBox::moveItemDown()
{
    if (!m_listBox->isEnabled())
    {
        KNotifyClient::beep();
        return;
    }

    unsigned int selIndex = m_listBox->currentItem();
    if (selIndex == m_listBox->count() - 1)
    {
        KNotifyClient::beep();
        return;
    }

    QListBoxItem *selItem = m_listBox->item(selIndex);
    m_listBox->takeItem(selItem);
    m_listBox->insertItem(selItem, selIndex+1);
    m_listBox->setCurrentItem(selIndex + 1);

    emit changed();
}

void KEditListBox::addItem()
{
    // when m_checkAtEntering is true, the add-button is disabled, but this
    // slot can still be called through Key_Return/Key_Enter. So we guard
    // against this.
    if ( !servNewButton || !servNewButton->isEnabled() )
        return;

    const QString& currentTextLE=m_lineEdit->text();
    bool alreadyInList(false);
    //if we didn't check for dupes at the inserting we have to do it now
    if (!d->m_checkAtEntering)
    {
        // first check current item instead of dumb iterating the entire list
        if ( m_listBox->currentText() == currentTextLE )
            alreadyInList = true;
        else
        {
            StringComparisonMode mode = (StringComparisonMode) (ExactMatch | CaseSensitive );
            alreadyInList =(m_listBox->findItem(currentTextLE, mode) != 0);
        }
    }

    if ( servNewButton )
        servNewButton->setEnabled(false);

    bool block = m_lineEdit->signalsBlocked();
    m_lineEdit->blockSignals(true);
    m_lineEdit->clear();
    m_lineEdit->blockSignals(block);

    m_listBox->setSelected(currentItem(), false);

    if (!alreadyInList)
    {
        block = m_listBox->signalsBlocked();
        m_listBox->blockSignals( true );
        m_listBox->insertItem(currentTextLE);
        m_listBox->blockSignals( block );
        emit changed();
    }
}

int KEditListBox::currentItem() const
{
    int nr = m_listBox->currentItem();
    if(nr >= 0 && !m_listBox->item(nr)->isSelected()) return -1;
    return nr;
}

void KEditListBox::removeItem()
{
    int selected = m_listBox->currentItem();

    if ( selected >= 0 )
    {
        m_listBox->removeItem( selected );
        if ( count() > 0 )
            m_listBox->setSelected( QMIN( selected, count() - 1 ), true );

        emit changed();
    }

    if ( servRemoveButton && m_listBox->currentItem() == -1 )
        servRemoveButton->setEnabled(false);
}

void KEditListBox::enableMoveButtons(int index)
{
    // Update the lineEdit when we select a different line.
    if(currentText() != m_lineEdit->text())
        m_lineEdit->setText(currentText());

    bool moveEnabled = servUpButton && servDownButton;

    if (moveEnabled )
    {
        if (m_listBox->count() <= 1)
        {
            servUpButton->setEnabled(false);
            servDownButton->setEnabled(false);
        }
        else if ((uint) index == (m_listBox->count() - 1))
        {
            servUpButton->setEnabled(true);
            servDownButton->setEnabled(false);
        }
        else if (index == 0)
        {
            servUpButton->setEnabled(false);
            servDownButton->setEnabled(true);
        }
        else
        {
            servUpButton->setEnabled(true);
            servDownButton->setEnabled(true);
        }
    }

    if ( servRemoveButton )
        servRemoveButton->setEnabled(true);
}

void KEditListBox::clear()
{
    m_lineEdit->clear();
    m_listBox->clear();
    emit changed();
}

void KEditListBox::insertStringList(const QStringList& list, int index)
{
    m_listBox->insertStringList(list,index);
}

void KEditListBox::insertStrList(const QStrList* list, int index)
{
    m_listBox->insertStrList(list,index);
}

void KEditListBox::insertStrList(const QStrList& list, int index)
{
    m_listBox->insertStrList(list,index);
}

void KEditListBox::insertStrList(const char ** list, int numStrings, int index)
{
    m_listBox->insertStrList(list,numStrings,index);
}

QStringList KEditListBox::items() const
{
    QStringList list;
    for ( uint i = 0; i < m_listBox->count(); i++ )
	list.append( m_listBox->text( i ));

    return list;
}

void KEditListBox::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

KEditListBox::CustomEditor::CustomEditor( KComboBox *combo )
{
    m_representationWidget = combo;
    m_lineEdit = dynamic_cast<KLineEdit*>( combo->lineEdit() );
    assert( m_lineEdit );
}

#endif
//-----------------------------------------------------------------------------

class FlagListToolTip : public QToolTip
{
public:
    FlagListToolTip(QWidget *parent);
protected:
    void maybeTip(const QPoint &p);
};


FlagListToolTip::FlagListToolTip(QWidget *parent)
    : QToolTip(parent)
{}


void FlagListToolTip::maybeTip(const QPoint &pos)
{
    FlagListBox *listbox = static_cast<FlagListBox*>(parentWidget());
    QListViewItem *item = listbox->itemAt(pos);
    FlagListItem *flitem = static_cast<FlagListItem*>(item);

    if (item)
        tip(listbox->itemRect(item), flitem->desc);
}


FlagListItem::FlagListItem(FlagListBox *parent, const QString &flagstr,
                           const QString &description)
    : QCheckListItem(parent, flagstr, QCheckListItem::CheckBox),
      flag(flagstr), desc(description)
{}


FlagListItem::FlagListItem(FlagListBox *parent, const QString &flagstr,
                           const QString &description, const QString &offstr)
    : QCheckListItem(parent, flagstr, QCheckListItem::CheckBox),
      flag(flagstr), off(offstr), desc(description)
{}


FlagListBox::FlagListBox(QWidget *parent, const char *name)
    : QListView(parent, name)
{
    setResizeMode(LastColumn);
    header()->hide();
    addColumn(i18n("Flags"));
    (void) new FlagListToolTip(this);
}


void FlagListBox::readFlags(QStringList *list)
{
    QListViewItem *item = firstChild();
    for (; item; item = item->nextSibling()) {
        FlagListItem *flitem = static_cast<FlagListItem*>(item);
        QStringList::Iterator sli = list->find(flitem->flag);
        if (sli != list->end()) {
            flitem->setOn(true);
            list->remove(sli);
        }
        sli = list->find(flitem->off);
        if (sli != list->end()) {
            flitem->setOn(false);
            list->remove(sli);
        }
    }
}


void FlagListBox::writeFlags(QStringList *list)
{
    QListViewItem *item = firstChild();
    for (; item; item = item->nextSibling()) {
        FlagListItem *flitem = static_cast<FlagListItem*>(item);
        if (flitem->isOn())
            (*list) << flitem->flag;
    }
}


FlagCheckBox::FlagCheckBox(QWidget *parent, FlagCheckBoxController *controller,
                           const QString &flagstr, const QString &description)
    : QCheckBox(description, parent), flag(flagstr)
{
    QToolTip::add(this, flagstr);
    controller->addCheckBox(this);
}


FlagCheckBox::FlagCheckBox(QWidget *parent, FlagCheckBoxController *controller,
                           const QString &flagstr, const QString &description,
                           const QString &offstr)
    : QCheckBox(description, parent), flag(flagstr), off(offstr)
{
    QToolTip::add(this, flagstr);
    controller->addCheckBox(this);
}


FlagCheckBoxController::FlagCheckBoxController(QStringList multiKeys)
    :m_multiKeys(multiKeys)
{
}


void FlagCheckBoxController::addCheckBox(FlagCheckBox *item)
{
    cblist.append(item);
}


void FlagCheckBoxController::readFlags(QStringList *list)
{
    //handle keys like -vxyz -> transform they into -vx -vy -vz
    //very "effective" algo :(
/*    QStringList addons;
    for (QStringList::Iterator mk = m_multiKeys.begin(); mk != m_multiKeys.end(); ++ mk)
    {
        kdDebug() << "multikey " << *mk << endl;
        for (QStringList::Iterator sli = list->begin(); sli != list->end(); ++sli)
        {
            QString key = *sli;
            kdDebug() << "current key: " << key << endl;
            if ( (key.length() > 3) && (key.startsWith(*mk)) )
            {
                list->remove(sli);
                key = key.remove(*mk);
                kdDebug() << "refined key " << key << endl;
                for (int i = 0; i < key.length(); ++i)
                {
                    kdDebug() << "adding key " << *mk + key[i] << endl;
                    addons << *mk + key[i];
                }
            }
        }
    }
    kdDebug() << "good" << endl;
    *list += addons;

    for (QStringList::Iterator sli = list->begin(); sli != list->end(); ++sli)
    {
        kdDebug() << "KEYS: " << *sli << endl;
    }
*/
    QPtrListIterator<FlagCheckBox> it(cblist);
    for (; it.current(); ++it) {
        FlagCheckBox *fitem = it.current();
        QStringList::Iterator sli = list->find(fitem->flag);
        if (sli != list->end()) {
            fitem->setChecked(true);
            list->remove(sli);
        }
        sli = list->find(fitem->off);
        if (sli != list->end()) {
            fitem->setChecked(false);
            list->remove(sli);
        }
    }
}


void FlagCheckBoxController::writeFlags(QStringList *list)
{
    QPtrListIterator<FlagCheckBox> it(cblist);
    for (; it.current(); ++it) {
        FlagCheckBox *fitem = it.current();
        if (fitem->isChecked())
            (*list) << fitem->flag;
    }
}

 FlagPathEditController::FlagPathEditController( )
{
}

 FlagPathEditController::~ FlagPathEditController( )
{
}

void FlagPathEditController::readFlags( QStringList * list )
{
//    kdDebug() << "read path flags" << endl;
    QPtrListIterator<FlagPathEdit> it(plist);
    for (; it.current(); ++it) {
        FlagPathEdit *peitem = it.current();

        for (QStringList::Iterator sli = list->begin(); sli != list->end(); ++sli)
        {
  //          kdDebug() << "option: " << (*sli) << " flag is: " << peitem->flag << endl;
            if ((*sli).startsWith(peitem->flag))
            {
//                kdDebug() << "Processing.." << endl;
                peitem->setText((*sli).replace(QRegExp(peitem->flag),""));
                sli = list->remove(sli);
            }
        }
/*        QStringList::Iterator sli = list->find(peitem->flag);
        if (sli != list->end()) {
            peitem->setText((*sli).remove(peitem->flag));
            list->remove(sli);
        }*/
    }
}

void FlagPathEditController::writeFlags( QStringList * list )
{
    QPtrListIterator<FlagPathEdit> it(plist);
    for (; it.current(); ++it) {
        FlagPathEdit *pitem = it.current();
        if (!pitem->isEmpty())
            (*list) << pitem->flag + pitem->text();
    }
}

void FlagPathEditController::addPathEdit( FlagPathEdit * item )
{
    plist.append(item);
}

FlagPathEdit::FlagPathEdit( QWidget * parent, QString pathDelimiter,
    FlagPathEditController * controller, const QString & flagstr, const QString & description,
    KFile::Mode mode )
    : QWidget(parent), delimiter(pathDelimiter), flag(flagstr), m_description(description)
{
    QBoxLayout *topLayout = new QVBoxLayout(this, 0, 1);
    topLayout->addWidget(new QLabel(description, this));
    QBoxLayout *layout = new QHBoxLayout(topLayout, 1);

    if (delimiter.isEmpty())
    {
        url = new KURLRequester(this);
        url->setMode(mode);
        layout->addWidget(url);
    }
    else
    {
        edit = new KLineEdit(this);
        layout->addWidget(edit);
        details = new QPushButton("...", this);
        details->setMaximumWidth(30);
        connect(details, SIGNAL(clicked()), this, SLOT(showPathDetails()));
        layout->addWidget(details);
    }

    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    QToolTip::add(this, flagstr);
    controller->addPathEdit(this);
}

void FlagPathEdit::showPathDetails( )
{
    KDialogBase *dia = new KDialogBase(0, "flag_path_edit_dia", true, m_description,
        KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true);

    QBoxLayout *diaLayout = new QVBoxLayout(dia, KDialog::marginHint(), KDialog::spacingHint());
    diaLayout->setAutoAdd(true);

    KURLRequester *req = new KURLRequester( dia );
    req->setMode(KFile::Directory);
    KEditListBox::CustomEditor* pCustomEditor;
#if KDE_VERSION > 305
    pCustomEditor = &req->customEditor();
#else
    QObjectList* pOL = req->queryList("KLineEdit"); // dirty hack :)
    KLineEdit* pEdit = dynamic_cast<KLineEdit*>(pOL->first());
    assert(pEdit);
    KEditListBox::CustomEditor editor(req, pEdit);
    pCustomEditor = &editor;
#endif
    KEditListBox *elb = new KEditListBox( "", *pCustomEditor, dia );
    dia->setMainWidget(elb);

    elb->insertStringList(QStringList::split(delimiter, text()));

    if (dia->exec() == QDialog::Accepted)
    {
        setText(elb->items().join(delimiter));
    }

    delete dia;
}

void FlagPathEdit::setText( const QString text )
{
    if (delimiter.isEmpty())
        url->setURL(text);
    else
        edit->setText(text);
}

QString FlagPathEdit::text( )
{
    if (delimiter.isEmpty())
        return url->url();
    else
        return edit->text();
}

bool FlagPathEdit::isEmpty( )
{
    if (delimiter.isEmpty())
        return url->url().isEmpty();
    else
        return edit->text().isEmpty();
}

FlagRadioButton::FlagRadioButton( QWidget * parent, FlagRadioButtonController * controller, const QString & flagstr, const QString & description )
    : QRadioButton(description, parent), flag(flagstr)
{
    QToolTip::add(this, flagstr);
    controller->addRadioButton(this);
}

FlagRadioButtonController::FlagRadioButtonController(QStringList multiKeys)
    :m_multiKeys(multiKeys)
{
}

void FlagRadioButtonController::addRadioButton(FlagRadioButton *item)
{
    cblist.append(item);
}


void FlagRadioButtonController::readFlags(QStringList *list)
{
    //handle keys like -vxyz -> transform they into -vx -vy -vz
    //very "effective" algo :(
/*    QStringList addons;
    for (QStringList::Iterator mk = m_multiKeys.begin(); mk != m_multiKeys.end(); ++ mk)
    {
        kdDebug() << "multikey " << *mk << endl;
        for (QStringList::Iterator sli = list->begin(); sli != list->end(); ++sli)
        {
            QString key = *sli;
            kdDebug() << "current key: " << key << endl;
            if ( (key.length() > 3) && (key.startsWith(*mk)) )
            {
                list->remove(sli);
                key = key.remove(*mk);
                kdDebug() << "refined key " << key << endl;
                for (int i = 0; i < key.length(); ++i)
                {
                    kdDebug() << "adding key " << *mk + key[i] << endl;
                    addons << *mk + key[i];
                }
            }
        }
    }
    kdDebug() << "good" << endl;
    *list += addons;

    for (QStringList::Iterator sli = list->begin(); sli != list->end(); ++sli)
    {
        kdDebug() << "KEYS: " << *sli << endl;
    }
*/
    QPtrListIterator<FlagRadioButton> it(cblist);
    for (; it.current(); ++it) {
        FlagRadioButton *fitem = it.current();
        QStringList::Iterator sli = list->find(fitem->flag);
        if (sli != list->end()) {
            fitem->setChecked(true);
            list->remove(sli);
        }
    }
}


void FlagRadioButtonController::writeFlags(QStringList *list)
{
    QPtrListIterator<FlagRadioButton> it(cblist);
    for (; it.current(); ++it) {
        FlagRadioButton *fitem = it.current();
        if (fitem->isChecked())
            (*list) << fitem->flag;
    }
}

#include "flagboxes.moc"
