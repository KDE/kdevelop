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

// only for KDE < 3.1
#include "../compat/kdeveditlistbox.h"

#include "flagboxes.h"

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
