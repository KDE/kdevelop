/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ctagsdlg.h"

#include <qcheckbox.h>
#include <qfile.h>
#include <qhbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <kbuttonbox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klistbox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstdguiitem.h>
#include <kdeversion.h>

#include "kdevplugin.h"
#include "kdevcore.h"
#include "kdevpartcontroller.h"
#include "kdevproject.h"
#include "ctagskinds.h"


class CTagsResultItem : public QListBoxText
{
public:
    CTagsResultItem(QListBox *parent, const QString &fileName, const QString pattern,
                    const QString &kindString)
        : QListBoxText(parent, QString("%1:%2 (%3)").arg(fileName).arg(pattern).arg(kindString)),
          m_fileName(fileName), m_pattern(pattern), m_kindString(kindString)
    {}

    QString fileName() const
    { return m_fileName; }
    QString pattern() const
    { return m_pattern; }

private:
    QString m_fileName;
    QString m_pattern;
    QString m_kindString;
};


CTagsDialog::CTagsDialog(CTagsPart *part)
    : QDialog(0, "ctags dialog", false)
{
    setCaption(i18n("Search in Tags"));
    QFontMetrics fm(fontMetrics());

    QLabel *tagLabel = new QLabel(i18n("&Tag:"), this);

    tagEdit = new QLineEdit(this);
    tagEdit->setFocus();
    tagLabel->setBuddy(tagEdit);
    tagEdit->setMinimumWidth(fm.width('X')*30);

    regexpBox = new QCheckBox(i18n("&Regular expression match"), this);
    regexpBox->setChecked(true);

    QLabel *kindsLabel = new QLabel(i18n("&Kinds:"), this);

    kindsListView = new QListView(this);
    kindsLabel->setBuddy(kindsListView);
    kindsListView->setResizeMode(QListView::LastColumn);
    kindsListView->addColumn(QString::null);
    kindsListView->header()->hide();
    kindsListView->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));

    KButtonBox *actionBox = new KButtonBox(this, Qt::Vertical);
    actionBox->addStretch();
    QPushButton *regenerateButton = actionBox->addButton(i18n("&Regenerate"));
    regenerateButton->setDefault(true);
#if KDE_IS_VERSION( 3, 2, 90 )
    QPushButton *cancelButton = actionBox->addButton(KStdGuiItem::close());
#else
    QPushButton *cancelButton = actionBox->addButton(i18n("Close"));
#endif
    actionBox->addStretch();
    actionBox->layout();

    resultsListBox = new KListBox(this);
    resultsListBox->setMinimumHeight(fm.lineSpacing()*10);
    resultsListBox->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding));

    QGridLayout *layout = new QGridLayout(this, 5, 2, KDialog::marginHint(), KDialog::spacingHint());
    layout->addRowSpacing(3, 10);
    layout->addWidget(tagLabel, 0, 0);
    layout->addWidget(tagEdit, 0, 1);
    layout->addWidget(regexpBox, 1, 1);
    layout->addWidget(kindsLabel, 2, 0);
    layout->addWidget(kindsListView, 2, 1);
    layout->addMultiCellWidget(actionBox, 0, 2, 2, 2);
    layout->addMultiCellWidget(resultsListBox, 4, 4, 0, 2);

    connect( tagEdit, SIGNAL(textChanged(const QString&)),
             this, SLOT(slotSearch()) );
    connect( kindsListView, SIGNAL(clicked(QListViewItem*)),
             this, SLOT(slotSearch()) );
    connect( kindsListView, SIGNAL(returnPressed(QListViewItem*)),
             this, SLOT(slotSearch()) );
    connect( regexpBox, SIGNAL(toggled(bool)),
             this, SLOT(slotSearch()) );
    connect( regenerateButton, SIGNAL(clicked()),
             this, SLOT(slotRegenerate()) );
    connect( cancelButton, SIGNAL(clicked()),
             this, SLOT(reject()) );
    connect( resultsListBox, SIGNAL(clicked(QListBoxItem*)),
             this, SLOT(slotResultClicked(QListBoxItem*)) );
    connect( resultsListBox, SIGNAL(returnPressed(QListBoxItem*)),
             this, SLOT(slotResultClicked(QListBoxItem*)) );

    m_part = part;
    updateInfo();
}


CTagsDialog::~CTagsDialog()
{}


void CTagsDialog::updateInfo()
{
    m_tags = m_part->tags();
    m_kindStrings = m_part->kindStrings();

    kindsListView->clear();

    QStringList::ConstIterator it;
    for (it = m_kindStrings.begin(); it != m_kindStrings.end(); ++it) {
        QCheckListItem *item = new QCheckListItem(kindsListView, (*it), QCheckListItem::CheckBox);
        item->setOn(true);
    }
}


void CTagsDialog::slotSearch()
{
    kdDebug(9022) << "search tag" << endl;
    if (m_tags.isEmpty())
        return;

    // Collect wanted kinds
    QStringList kindStringList;
    QCheckListItem *clitem = static_cast<QCheckListItem*>(kindsListView->firstChild());
    while (clitem) {
        if (clitem->isOn())
            kindStringList.append(clitem->text());
        clitem = static_cast<QCheckListItem*>(clitem->nextSibling());
    }
    resultsListBox->clear();

    if (regexpBox->isChecked()) {
        // Do the regexp search
        QRegExp re(tagEdit->text());
        CTagsMapConstIterator it;
        for (it = m_tags.begin(); it != m_tags.end(); ++it)
            if (re.exactMatch(it.key()))
                insertResult(it.data(), kindStringList);
    } else {
        // Do the exact search
        CTagsMapIterator result = m_tags.find(tagEdit->text());
        if (result != m_tags.end())
            insertResult(*result, kindStringList);
    }
}


void CTagsDialog::insertResult(const CTagsTagInfoList &result, const QStringList &kindStringList)
{
    // Iterate over all found items, check if they have one of the wanted
    // kinds, and insert them in the result box
    CTagsTagInfoListConstIterator it;
    for (it = result.begin(); it != result.end(); ++it) {
        QString extension;
        if ((*it).fileName.right(9) == "/Makefile")
            extension = "mak";
        else {
            int pos = (*it).fileName.findRev('.');
            if (pos > 0)
                extension = (*it).fileName.mid(pos+1);
        }
        if (extension.isNull())
            continue;
        QString kindString = CTagsKinds::findKind((*it).kind, extension);
        if (!kindStringList.contains(kindString))
            continue;

        new CTagsResultItem(resultsListBox, (*it).fileName, (*it).pattern, kindString);
    }
}


void CTagsDialog::slotRegenerate()
{
    if (!m_part->createTagsFile()) {
        KMessageBox::sorry(this, i18n("Could not create tags file"));
        return;
    }

    m_part->loadTagsFile();

    updateInfo();
}


void CTagsDialog::slotResultClicked(QListBoxItem *item)
{
    if (!item)
        return;

    CTagsResultItem *ritem = static_cast<CTagsResultItem*>(item);
    QString fileName = ritem->fileName();
    if (!fileName.startsWith("/"))
        fileName.prepend(m_part->project()->projectDirectory() + "/");
    QString pattern = ritem->pattern();
    bool ok;
    int lineNum = pattern.toInt(&ok);
    if (!ok) {
        KMessageBox::sorry(0, i18n("Currently, only tags with line numbers (option -n) are supported"));
        return;
    }

    m_part->partController()->editDocument(KURL::fromPathOrURL( fileName ), lineNum-1);
}

#include "ctagsdlg.moc"
