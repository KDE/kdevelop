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
#include <qfileinfo.h>
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
#include <kprocess.h>
#include <kregexp.h>

#include "kdevplugin.h"
#include "kdevcore.h"
#include "kdevpartcontroller.h"
#include "kdevproject.h"


struct CTagsKindMapping {
    char abbrev;
    char *verbose;
};


struct CTagsExtensionMapping {
    char *extension;
    CTagsKindMapping *kinds;
};


static CTagsKindMapping kindMappingAsm[] = {
    { 'd', I18N_NOOP("define")              },
    { 'l', I18N_NOOP("label")               },
    { 'm', I18N_NOOP("macro")               },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingAsp[] = {
    { 'f', I18N_NOOP("function")            },
    { 's', I18N_NOOP("subroutine")          },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingAwk[] = {
    { 'f', I18N_NOOP("function")            },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingBeta[] = {
    { 'f', I18N_NOOP("fragment definition") },
    { 'p', I18N_NOOP("any pattern")         },
    { 's', I18N_NOOP("slot")                },
    { 'v', I18N_NOOP("pattern")             },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingC[] = {
    { 'c', I18N_NOOP("class")               },
    { 'd', I18N_NOOP("macro")               },
    { 'e', I18N_NOOP("enumerator")          },
    { 'f', I18N_NOOP("function")            },
    { 'g', I18N_NOOP("enumeration")         },
    { 'm', I18N_NOOP("member")              }, 
    { 'n', I18N_NOOP("namespace")           },
    { 'p', I18N_NOOP("prototype")           },
    { 's', I18N_NOOP("struct")              },
    { 't', I18N_NOOP("typedef")             },
    { 'u', I18N_NOOP("union")               },
    { 'v', I18N_NOOP("variable")            },
    { 'x', I18N_NOOP("external variable")   },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingCobol[] = {
    { 'p', I18N_NOOP("paragraph")           },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingEiffel[] = {
    { 'c', I18N_NOOP("class")               },
    { 'f', I18N_NOOP("feature")             },
    { 'l', I18N_NOOP("local entity")        },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingFortran[] = {
    { 'b', I18N_NOOP("block")               },
    { 'c', I18N_NOOP("common")              },
    { 'e', I18N_NOOP("entry")               },
    { 'f', I18N_NOOP("function")            },
    { 'i', I18N_NOOP("interface")           },
    { 'k', I18N_NOOP("type component")      },
    { 'l', I18N_NOOP("label")               },
    { 'L', I18N_NOOP("local")               },
    { 'm', I18N_NOOP("module")              },
    { 'n', I18N_NOOP("namelist")            },
    { 'p', I18N_NOOP("program")             },
    { 's', I18N_NOOP("subroutine")          },
    { 't', I18N_NOOP("type")                },
    { 'v', I18N_NOOP("variable")            },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingJava[] = {
    { 'c', I18N_NOOP("class")               },
    { 'f', I18N_NOOP("field")               },
    { 'i', I18N_NOOP("interface")           },
    { 'm', I18N_NOOP("method")              },
    { 'p', I18N_NOOP("package")             },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingLisp[] = {
    { 'f', I18N_NOOP("function")            },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingMake[] = {
    { 'm', I18N_NOOP("macro")               },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingPascal[] = {
    { 'f', I18N_NOOP("function")            },
    { 'p', I18N_NOOP("procedure")           },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingPerl[] = {
    { 's', I18N_NOOP("subroutine")          },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingPHP[] = {
    { 'c', I18N_NOOP("class")               },
    { 'f', I18N_NOOP("function")            },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingPython[] = {
    { 'c', I18N_NOOP("class")               },
    { 'f', I18N_NOOP("function")            },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingRexx[] = {
    { 's', I18N_NOOP("subroutine")          },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingRuby[] = {
    { 'c', I18N_NOOP("class")               },
    { 'f', I18N_NOOP("function")            },
    { 'm', I18N_NOOP("mixin")               },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingScheme[] = {
    { 'f', I18N_NOOP("function")            },
    { 's', I18N_NOOP("set")                 },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingSh[] = {
    { 'f', I18N_NOOP("function")            },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingSlang[] = {
    { 'f', I18N_NOOP("function")            },
    { 'n', I18N_NOOP("namespace")           },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingTcl[] = {
    { 'p', I18N_NOOP("procedure")           },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingVim[] = {
    { 'f', I18N_NOOP("function")            },
    { 0  , 0                                }
};


static CTagsExtensionMapping extensionMapping[] = {
    { "asm",    kindMappingAsm     },
    { "s",      kindMappingAsm     },
    { "S",      kindMappingAsm     },
    { "asp",    kindMappingAsp     },
    { "asa",    kindMappingAsp     },
    { "awk",    kindMappingAwk     },
    { "c++",    kindMappingC       },
    { "cc",     kindMappingC       },
    { "cp" ,    kindMappingC       },
    { "cpp",    kindMappingC       },
    { "cxx",    kindMappingC       },
    { "h"  ,    kindMappingC       },
    { "h++",    kindMappingC       },
    { "hh" ,    kindMappingC       },
    { "hp" ,    kindMappingC       },
    { "hpp",    kindMappingC       },
    { "hxx",    kindMappingC       },
    { "beta",   kindMappingBeta    },
    { "cob",    kindMappingCobol   },
    { "COB",    kindMappingCobol   },
    { "e",      kindMappingEiffel  },
    { "f"   ,   kindMappingFortran },
    { "for" ,   kindMappingFortran },
    { "ftn" ,   kindMappingFortran },
    { "f77" ,   kindMappingFortran },
    { "f90" ,   kindMappingFortran },
    { "f95" ,   kindMappingFortran },
    { "java",   kindMappingJava    },
    { "cl",     kindMappingLisp    },
    { "clisp",  kindMappingLisp    },
    { "el",     kindMappingLisp    },
    { "l",      kindMappingLisp    },
    { "lisp",   kindMappingLisp    },
    { "lsp",    kindMappingLisp    },
    { "ml",     kindMappingLisp    },
    { "mak",    kindMappingMake    },
    { "p",      kindMappingPascal  },
    { "pas",    kindMappingPascal  },
    { "pl",     kindMappingPerl    },
    { "pm",     kindMappingPerl    },
    { "perl",   kindMappingPerl    },
    { "php",    kindMappingPHP     },
    { "php3",   kindMappingPHP     },
    { "phtml",  kindMappingPHP     },
    { "py",     kindMappingPython  },
    { "python", kindMappingPython  },
    { "cmd",    kindMappingRexx    },
    { "rexx",   kindMappingRexx    },
    { "rx",     kindMappingRexx    },
    { "rb",     kindMappingRuby    },
    { "sch",    kindMappingScheme  },
    { "scheme", kindMappingScheme  },
    { "scm",    kindMappingScheme  },
    { "sm",     kindMappingScheme  },
    { "SCM",    kindMappingScheme  },
    { "SM",     kindMappingScheme  },
    { "sh",     kindMappingSh      },
    { "SH",     kindMappingSh      },
    { "bsh",    kindMappingSh      },
    { "bash",   kindMappingSh      },
    { "ksh",    kindMappingSh      },
    { "zsh",    kindMappingSh      },
    { "sl",     kindMappingSlang   },
    { "tcl",    kindMappingTcl     },
    { "wish",   kindMappingTcl     },
    { "vim",    kindMappingVim     },
    { 0     , 0                    }
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


CTagsDialog::CTagsDialog(KDevPlugin *part)
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

    regexp_box = new QCheckBox(i18n("&Regular expression match"), this);
    regexp_box->setChecked(true);
    
    KButtonBox *actionbox = new KButtonBox(this, Qt::Vertical);
    actionbox->addStretch();
    QPushButton *regenerate_button = actionbox->addButton(i18n("&Regenerate"));
    regenerate_button->setDefault(true);
    QPushButton *cancel_button = actionbox->addButton(i18n("Close"));
    actionbox->addStretch();
    actionbox->layout();

    results_listbox = new KListBox(this);
    results_listbox->setMinimumHeight(fm.lineSpacing()*10);
    results_listbox->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding));

    QGridLayout *layout = new QGridLayout(this, 5, 2, KDialog::marginHint(), KDialog::spacingHint());
    layout->addRowSpacing(3, 10);
    layout->addWidget(tag_label, 0, 0);
    layout->addWidget(tag_edit, 0, 1);
    layout->addWidget(kinds_label, 2, 0);
    layout->addWidget(regexp_box, 1, 1);
    layout->addWidget(kinds_listview, 2, 1);
    layout->addMultiCellWidget(actionbox, 0, 2, 2, 2);
    layout->addMultiCellWidget(results_listbox, 4, 4, 0, 2);

    connect( tag_edit, SIGNAL(textChanged(const QString&)),
             this, SLOT(slotSearch()) );
    connect( kinds_listview, SIGNAL(clicked(QListViewItem*)),
             this, SLOT(slotSearch()) );
    connect( kinds_listview, SIGNAL(returnPressed(QListViewItem*)),
             this, SLOT(slotSearch()) );
    connect( regexp_box, SIGNAL(toggled(bool)),
             this, SLOT(slotSearch()) );
    connect( regenerate_button, SIGNAL(clicked()),
             this, SLOT(slotRegenerate()) );
    connect( cancel_button, SIGNAL(clicked()),
             this, SLOT(reject()) );
    connect( results_listbox, SIGNAL(clicked(QListBoxItem*)),
             this, SLOT(slotResultClicked(QListBoxItem*)) );
    connect( results_listbox, SIGNAL(returnPressed(QListBoxItem*)),
             this, SLOT(slotResultClicked(QListBoxItem*)) );

    connect( part->core(), SIGNAL(projectOpened()),
             this, SLOT(projectChanged()) );
    connect( part->core(), SIGNAL(projectClosed()),
             this, SLOT(projectChanged()) );
    
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
    kdDebug(9022) << "search tag" << endl;
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

    if (regexp_box->isChecked()) {
        // Do the regexp search
        QRegExp re("^" + tag_edit->text() + "$");
        QDictIterator<CTagsTagInfoList> it(*m_tags);
        for (; it.current(); ++it)
            if (re.match(it.currentKey(), 0) != -1)
                insertResult(it.current(), kindStringList);
    } else {
        // Do the exact search
        CTagsTagInfoList *result = m_tags->find(tag_edit->text());
        if (result)
            insertResult(result, kindStringList);
    }
}


void CTagsDialog::insertResult(CTagsTagInfoList *result, const QStringList &kindStringList)
{
    // Iterate over all found items, check if they have one of the wanted
    // kinds, and insert them in the result box
    CTagsTagInfoListIterator it;
    for (it = result->begin(); it != result->end(); ++it) {
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
        QString kindString = findKind((*it).kind, extension);
        if (!kindStringList.contains(kindString))
            continue;
        
        new CTagsResultItem(results_listbox, (*it).fileName, (*it).pattern, kindString);
    }
}


void CTagsDialog::slotRegenerate()
{
    QString tagsFileName = m_part->project()->projectDirectory() + "/tags";
    QFileInfo fi(tagsFileName);
    if (!createTagsFile()) {
        KMessageBox::sorry(this, i18n("Could not create tags file"));
        return;
    }
    loadTagsFile(tagsFileName);
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
    
    m_part->partController()->editDocument(fileName, lineNum-1);
}


void CTagsDialog::ensureTagsLoaded()
{
    if (!m_tags) {
        if (!m_part->project())
            return;

        kdDebug(9022) << "create/load tags" << endl;
        
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
        kdDebug(9022) << "load tags from " << tagsFileName << endl;
        loadTagsFile(tagsFileName);
    }
}


bool CTagsDialog::createTagsFile()
{
    kdDebug(9022) << "create tags file" << endl;

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
    kdDebug(9022) << "load tags file" << endl;

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
        if (!re.match(line))
            continue;

        
        QString tag = re.group(1);
        QString file = re.group(2);
        QString pattern = re.group(3);
        QString extfield = re.group(4);
        //        kdDebug() <<"Tag " << tag << ", file " << file << ", pattern "
        //                  << pattern << ", extfield " << extfield << endl;
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
        QString extension;
        if (ti.fileName.right(9) == "/Makefile")
            extension = "mak";
        else {
            int pos = ti.fileName.findRev('.');
            if (pos > 0)
                extension = ti.fileName.mid(pos+1);
        }
        if (extension.isNull())
            continue;
        
        QString kindString = findKind(ti.kind, extension);
        if (kindString.isNull())
            continue;
        
        QCheckListItem *clitem = static_cast<QCheckListItem*>(kinds_listview->firstChild());
        while (clitem && clitem->text(0) != kindString)
            clitem = static_cast<QCheckListItem*>(clitem->nextSibling());
        if (!clitem) {
            kdDebug() << "New kind " << kindString << " with extension " << extension << endl;
            QCheckListItem *item = new QCheckListItem(kinds_listview, kindString, QCheckListItem::CheckBox);
            item->setOn(true);
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
