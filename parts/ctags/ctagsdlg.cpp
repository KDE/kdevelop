/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qfile.h>
#include <qfileinfo.h>
#include <qhbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qtextstream.h>
#include <kbuttonbox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klistbox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kregexp.h>

#include "kdevpart.h"
#include "kdevcore.h"
#include "kdevproject.h"
#include "ctagsdlg.h"


struct CTagsKindMapping {
    char abbrev;
    char *verbose;
};


struct CTagsExtensionMapping {
    char *extension;
    CTagsKindMapping *kinds;
};


static CTagsKindMapping kindMappingC[] = {
    { 'c', I18N_NOOP("class")            },
    { 'd', I18N_NOOP("macro")            },
    { 'e', I18N_NOOP("enumerator")       },
    { 'f', I18N_NOOP("function")         },
    { 'g', I18N_NOOP("enumeration")      },
    { 'm', I18N_NOOP("member")           }, 
    { 'n', I18N_NOOP("namespace")        },
    { 'p', I18N_NOOP("prototype")        },
    { 's', I18N_NOOP("struct")           },
    { 't', I18N_NOOP("typedef")          },
    { 'u', I18N_NOOP("union")            },
    { 'v', I18N_NOOP("variable")         },
    { 'x', I18N_NOOP("external variable")},
    { 0  , 0                             }
};


static CTagsKindMapping kindMappingJava[] = {
    { 'c', I18N_NOOP("class")            },
    { 'f', I18N_NOOP("field")            },
    { 'i', I18N_NOOP("interface")        },
    { 'm', I18N_NOOP("method")           },
    { 'p', I18N_NOOP("package")          },
    { 0  , 0                   }
};


static CTagsKindMapping kindMappingFortran[] = {
    { 'b', I18N_NOOP("block")         },
    { 'c', I18N_NOOP("common")        },
    { 'e', I18N_NOOP("entry")         },
    { 'f', I18N_NOOP("function")      },
    { 'i', I18N_NOOP("interface")     },
    { 'k', I18N_NOOP("type component")},
    { 'l', I18N_NOOP("label")         },
    { 'L', I18N_NOOP("local")         },
    { 'm', I18N_NOOP("module")        },
    { 'n', I18N_NOOP("namelist")      },
    { 'p', I18N_NOOP("program")       },
    { 's', I18N_NOOP("subroutine")    },
    { 't', I18N_NOOP("type")          },
    { 'v', I18N_NOOP("variable")      },
    { 0  , 0                }
};


static CTagsExtensionMapping extensionMapping[] = {
    { "c++",  kindMappingC       },
    { "cc",   kindMappingC       },
    { "cp" ,  kindMappingC       },
    { "cpp",  kindMappingC       },
    { "cxx",  kindMappingC       },
    { "h"  ,  kindMappingC       },
    { "h++",  kindMappingC       },
    { "hh" ,  kindMappingC       },
    { "hp" ,  kindMappingC       },
    { "hpp",  kindMappingC       },
    { "hxx",  kindMappingC       },
    { "java", kindMappingJava    },
    { "f"   , kindMappingFortran },
    { "for" , kindMappingFortran },
    { "ftn" , kindMappingFortran },
    { "f77" , kindMappingFortran },
    { "f90" , kindMappingFortran },
    { "f95" , kindMappingFortran },
    { 0     , 0               }
};


static CTagsKindMapping *findKindMapping(const QString &extension)
{
    const char *pextension = extension.latin1();
    
    CTagsExtensionMapping *pem = extensionMapping;
    while (pem->extension != 0) {
        if (strcmp(pem->extension, pextension) == 0)
            return pem->kinds;
        ++pem;
    }

    return 0;
}


static QString findKind(char kindChar, const QString &extension)
{
    QString res;

    CTagsKindMapping *kindMapping = findKindMapping(extension);
    if (kindMapping) {
        CTagsKindMapping *pkm = kindMapping;
        while (pkm->verbose != 0) {
            if (pkm->abbrev == kindChar)
                return i18n(QString::fromLatin1(pkm->verbose));
            ++pkm;
        }
    }

    return QString::null;
}


struct CTagsTagInfo
{
    QString fileName;
    QString pattern;
    int lineNum;
    char kind;
};



class CTagsResultItem : public QListBoxText
{
public:
    CTagsResultItem(QListBox *parent, const QString &fileName, const QString pattern,
                    const QString &kindString)
        : QListBoxText(parent, i18n("%1:%2 (%3)").arg(fileName).arg(pattern).arg(kindString)),
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


CTagsDialog::CTagsDialog(KDevPart *part)
    : QDialog(0, "ctags dialog", false)
{
    setCaption("Search in Tags");
    QFontMetrics fm(fontMetrics());

    QLabel *tag_label = new QLabel(i18n("&Tag:"), this);

    tag_edit = new QLineEdit(this);
    tag_edit->setFocus();
    tag_label->setBuddy(tag_edit);
    tag_edit->setMinimumWidth(fm.width('X')*30);
    
    QLabel *kinds_label = new QLabel(i18n("&Kinds:"), this);

    kinds_listview = new QListView(this);
    kinds_label->setBuddy(kinds_listview);
    kinds_listview->addColumn("");
    kinds_listview->header()->hide();
    kinds_listview->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));

    KButtonBox *actionbox = new KButtonBox(this, Qt::Vertical);
    actionbox->addStretch();
    QPushButton *search_button = actionbox->addButton(i18n("&Search"));
    search_button->setDefault(true);
    QPushButton *cancel_button = actionbox->addButton(i18n("Close"));
    actionbox->addStretch();
    actionbox->layout();

    results_listbox = new KListBox(this);
    results_listbox->setMinimumHeight(fm.lineSpacing()*10);
    results_listbox->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding));

    QGridLayout *layout = new QGridLayout(this, 4, 2, KDialog::marginHint(), KDialog::spacingHint());
    layout->addRowSpacing(2, 10);
    layout->addWidget(tag_label, 0, 0);
    layout->addWidget(tag_edit, 0, 1);
    layout->addWidget(kinds_label, 1, 0);
    layout->addWidget(kinds_listview, 1, 1);
    layout->addMultiCellWidget(actionbox, 0, 1, 2, 2);
    layout->addMultiCellWidget(results_listbox, 3, 3, 0, 2);

    connect( search_button, SIGNAL(clicked()), this, SLOT(slotSearch()) );
    connect( cancel_button, SIGNAL(clicked()), this, SLOT(reject()) );
    connect( results_listbox, SIGNAL(executed(QListBoxItem*)), this, SLOT(slotExecuted(QListBoxItem*)) );
    connect( results_listbox, SIGNAL(returnPressed(QListBoxItem*)), this, SLOT(slotExecuted(QListBoxItem*)) );

    connect( part->core(), SIGNAL(projectOpened()), this, SLOT(projectChanged()) );
    connect( part->core(), SIGNAL(projectClosed()), this, SLOT(projectChanged()) );
    
    m_part = part;
    m_tags = 0;

    projectChanged();
}


CTagsDialog::~CTagsDialog()
{}


void CTagsDialog::projectChanged()
{
    if (!m_part->project()) {
        delete m_tags;
        m_tags = 0;
    } else
        ensureTagsLoaded();
}


void CTagsDialog::slotSearch()
{
    kdDebug(9021) << "search tag" << endl;
    ensureTagsLoaded();
    if (!m_tags)
        return;

    // Collect wanted kinds
    QStringList kindStringList;
    QCheckListItem *clitem = static_cast<QCheckListItem*>(kinds_listview->firstChild());
    while (clitem) {
        if (clitem->isOn())
            kindStringList.append(clitem->text());
        clitem = static_cast<QCheckListItem*>(clitem->nextSibling());
    }
    results_listbox->clear();

    // Do the search
    CTagsTagInfoList *result = m_tags->find(tag_edit->text());
    if (!result)
        return;
    
    // Iterate over all found items, check if they have one of the wanted
    // kinds, and insert them in the result box
    CTagsTagInfoListIterator it;
    for (it = result->begin(); it != result->end(); ++it) {
        QString kindString;
        int pos = (*it).fileName.findRev('.');
        if (pos > 0)
            kindString = findKind((*it).kind, (*it).fileName.mid(pos+1));
        if (kindStringList.contains(kindString))
            new CTagsResultItem(results_listbox, (*it).fileName, (*it).pattern, kindString);
    }
}


void CTagsDialog::slotExecuted(QListBoxItem *item)
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
    
    m_part->core()->gotoSourceFile(fileName, lineNum-1, KDevCore::Replace);
}


void CTagsDialog::ensureTagsLoaded()
{
    if (!m_tags) {
        if (!m_part->project())
            return;

        kdDebug(9021) << "create/load tags" << endl;
        
        QString tagsFileName = m_part->project()->projectDirectory() + "/tags";
        QFileInfo fi(tagsFileName);
        if (!fi.exists()) {
            int r = KMessageBox::questionYesNo(this, i18n("A ctags file for this project does not exist yet. Create it now?"));
            if (r != KMessageBox::Yes)
                return;
            if (!createTagsFile()) {
                KMessageBox::sorry(this, i18n("Could not create tags file"));
                return;
            }
        }
        kdDebug(9021) << "load tags from " << tagsFileName << endl;
        loadTagsFile(tagsFileName);
    }
}


bool CTagsDialog::createTagsFile()
{
    kdDebug(9021) << "create tags file" << endl;

    QString cmd = "cd ";
    cmd += m_part->project()->projectDirectory();
    cmd += " && ctags -n ";

    QStringList l = m_part->project()->allSourceFiles();
    QStringList::ConstIterator it;
    for (it = l.begin(); it != l.end(); ++it) {
        cmd += (*it);
        cmd += " ";
    }

    KShellProcess proc("/bin/sh");
    proc << cmd;
    return proc.start(KProcess::Block);
}


void CTagsDialog::loadTagsFile(const QString &fileName)
{
    kdDebug(9021) << "load tags file" << endl;

    kinds_listview->clear();
    
    QFile f(fileName);
    if (!f.open(IO_ReadOnly))
        return;

    m_tags = new QDict<CTagsTagInfoList>;
    m_tags->setAutoDelete(true);

    QTextStream stream(&f);
    KRegExp re("^([^\t]*)\t([^\t]*)\t([^;]*);\"\t(.*)$");
        
    QCString line;
    while (!stream.atEnd()) {
        line = stream.readLine();
        //        kdDebug() << "Line: " << line << endl;
        if (re.match(line)) {
            QString tag = re.group(1);
            QString file = re.group(2);
            QString pattern = re.group(3);
            QString extfield = re.group(4);
            //            kdDebug() <<"Tag " << tag << ", file " << file << ", pattern "
            //                      << pattern << ", extfield " << extfield << endl;
            CTagsTagInfoList *tilist = m_tags->find(tag);
            if (!tilist) {
                tilist = new CTagsTagInfoList;
                m_tags->insert(tag, tilist);
            }
            CTagsTagInfo ti;
            ti.fileName = re.group(2);
            ti.pattern = re.group(3);
            ti.kind = re.group(4)[0];
            tilist->append(ti);

            // Put kind in kind list view if not already there
            int pos = ti.fileName.findRev('.');
            if (pos > 0) {
                QString kindString = findKind(ti.kind, ti.fileName.mid(pos+1));
                QCheckListItem *clitem = static_cast<QCheckListItem*>(kinds_listview->firstChild());
                while (clitem && clitem->text(0) != kindString)
                    clitem = static_cast<QCheckListItem*>(clitem->nextSibling());
                if (!clitem) {
                    QCheckListItem *item = new QCheckListItem(kinds_listview, kindString, QCheckListItem::CheckBox);
                    item->setOn(true);
                }
            }
        }
    }

    f.close();

#if 0
    QDictIterator<CTagsTagInfoList> it(tags);
    for (; it.current(); ++it) {
        kdDebug() << "Id: " << it.currentKey() << endl;
        CTagsTagInfoList *l = it.current();
        QValueList<CTagsTagInfo>::ConstIterator it2;
        for (it2 = l->begin(); it2 != l->end(); ++it2)
            kdDebug() << "at " << (*it2).fileName << "," << (*it2).pattern << endl;
    }
#endif
}

#include "ctagsdlg.moc"
