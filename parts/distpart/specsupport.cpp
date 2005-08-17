/***************************************************************************
 *   Copyright (C) 2004 by ian reinhart geiser                             *
 *   geiseri@kde.org                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "specsupport.h"
#include "kdevproject.h"
#include "kdevmakefrontend.h"
#include "distpart_widget.h"

#include <kdebug.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kprocess.h>
#include <qfile.h>
#include <qdir.h>
#include <qerrormessage.h>
#include <qregexp.h>
#include <qpushbutton.h>
//#include <qvbox.h>
#include <q3groupbox.h>
#include <qtabwidget.h>
#include <qmessagebox.h>
//Added by qt3to4:
#include <QTextStream>

/// \FIXME This is at least the fifth place in the kdevelop code something like this exists
QString QRegExp_escape(const QString& str )
{
#if QT_VERSION >= 0x030100
    return QRegExp::escape(str);
#else
    // this block is copyrighted by Trolltech AS (GPL)
    static const char meta[] = "$()*+.?[\\]^{|}";
    QString quoted = str;
    int i = 0;

    while ( i < (int) quoted.length() ) {
       if ( strchr(meta, quoted[i].latin1()) != 0 )
           quoted.insert( i++, "\\" );
       i++;
    }
    return quoted;
#endif
}

SpecSupport::SpecSupport(DistpartPart *part) : packageBase(), m_part(part) {
	dir = "";

//     srcPackagePushButton = new QPushButton(i18n("Src Package"),area());
//     buildAllPushButton = new QPushButton(i18n("Src/Binary Packages"),area());
//     exportSPECPushButton = new QPushButton(i18n("Export SPEC File"),area());
//     importSPECPushButton = new QPushButton(i18n("Import SPEC File"),area());
// 
// 
// 
//     connect(buildAllPushButton, SIGNAL(clicked()),
//             this, SLOT(slotbuildAllPushButtonPressed()));
//     connect(exportSPECPushButton, SIGNAL(clicked()),
//             this, SLOT(slotexportSPECPushButtonPressed()));
//     connect(importSPECPushButton, SIGNAL(clicked()),
//             this, SLOT(slotimportSPECPushButtonPressed()));
//     connect(srcPackagePushButton, SIGNAL(clicked()),
//             this, SLOT(slotsrcPackagePushButtonPressed()));

    parseDotRpmmacros();
}

SpecSupport::~SpecSupport() {
}

//    QPushButton* buildAllPushButton;
void SpecSupport::slotbuildAllPushButtonPressed() {
    QMap<QString,QString>::Iterator it;
    QFile file1(dir + "/" + getAppSource());
    QFile file2(*(map.find("_sourcedir")) + "/" + getAppSource());
    if (!file2.exists()) {
	if (!file1.exists()) {
	    QMessageBox::critical(0 ,i18n("Error"),i18n("You need to create a source archive first."));
	    return;
	}
	else
            if (KDevMakeFrontend *makeFrontend = m_part->extension<KDevMakeFrontend>("KDevelop/MakeFrontend"))
                makeFrontend->queueCommand(dir,"cd " + KProcess::quote(dir) + 
		" && cp " + KProcess::quote(getAppSource()) + " " + KProcess::quote(*(map.find("_sourcedir"))));
    }
    if (KDevMakeFrontend *makeFrontend = m_part->extension<KDevMakeFrontend>("KDevelop/MakeFrontend"))
        makeFrontend->queueCommand(dir,"cd " + KProcess::quote((((it = map.find("_specdir")) != map.end()) ? (*it) : dir)) +
		 " && rpmbuild -ba " + m_part->project()->projectName() + ".spec");
}

//    QPushButton* exportSPECPushButton;
void SpecSupport::slotexportSPECPushButtonPressed() {
    QMap<QString,QString>::Iterator it;
    QString specname = ((it = map.find("_specdir")) != map.end()) ? (*it) : (m_part->project()->projectDirectory());
    specname += ("/" + m_part->project()->projectName() + ".spec");
    QFile file(specname);

    if(file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
	stream << generatePackage();
        file.close();
    } else {
        kdDebug() << "TODO : intercept write error in SpecSupport::slotexportSPECPushButtonPressed()";
    }
}

QString SpecSupport::getInfo(QString s, QString motif) {
    QRegExp re(motif + "[ \t]*([^ \t].*[^ \t])[ \t]*");
    if (re.exactMatch(s))
        return re.cap(1);
    return QString::null;
}

//    QPushButton* importSPECPushButton;
void SpecSupport::slotimportSPECPushButtonPressed() {
       QString fileName = KFileDialog::getOpenFileName(dir,"*.spec");
       if( fileName.isEmpty())
               return;
    QFile file(fileName);
 
    if(file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);

        while (!stream.atEnd()) {
            QString s = stream.readLine();
            QString info;
            if (!(info = getInfo(s,"Name:")).isEmpty())
                setAppName(info);
            else if (!(info = getInfo(s,"Version:")).isEmpty())
                setAppVersion(info);
            else if (!(info = getInfo(s,"Release:")).isEmpty())
                setAppRevision(info);
            else if (!(info = getInfo(s,"Vendor:")).isEmpty())
                setAppVendor(info);
            else if (!(info = getInfo(s,"Copyright:")).isEmpty())
                setAppLicense(info);
            else if (!(info = getInfo(s,"Summary:")).isEmpty())
                setAppSummary(info);
            else if (!(info = getInfo(s,"Group:")).isEmpty())
                setAppGroup(info);
            else if (!(info = getInfo(s,"Packager:")).isEmpty())
                setAppPackager(info);
            else if (s.startsWith("%description")) {
                QString desc;
                while (!stream.atEnd()) {
                    QString str = stream.readLine();
                    if (str.startsWith("%")) break;
                    else desc += str + "\n";
                }
                setAppDescription(desc);
            }
	    else if (s.startsWith("%changelog")) {
		QString change;
		while (!stream.atEnd()) {
		    QString str = stream.readLine();
		    if (str.startsWith("%")) break;
                    else change += str + "\n";
                }
		setAppChangelog(change);
            }
        }
    }
}

//    QPushButton* srcPackagePushButton;
void SpecSupport::slotsrcPackagePushButtonPressed() {
    QMap<QString,QString>::Iterator it;

    QFile file1(dir + "/" + getAppSource());
    QFile file2(*(map.find("_sourcedir")) + "/" + getAppSource());
    if (!file2.exists()) {
	if (!file1.exists()) {
	    QMessageBox::critical(0,i18n("Error"),i18n("You need to create a source archive first."));
	    return;
	}
	else
            if (KDevMakeFrontend *makeFrontend = m_part->extension<KDevMakeFrontend>("KDevelop/MakeFrontend"))
	        makeFrontend->queueCommand(dir,"cd " + KProcess::quote(dir) + 
		" && cp " + KProcess::quote(getAppSource()) + " " + KProcess::quote(*(map.find("_sourcedir"))));
    }
    if (KDevMakeFrontend *makeFrontend = m_part->extension<KDevMakeFrontend>("KDevelop/MakeFrontend"))
        makeFrontend->queueCommand(dir,"cd " + KProcess::quote((((it = map.find("_specdir")) != map.end()) ? (*it) : dir)) +
		 " && rpmbuild -bs " + m_part->project()->projectName() + ".spec");
}

void SpecSupport::parseDotRpmmacros() {
    QFile dotfile(QDir::homeDirPath() + "/.rpmmacros");

    if (!dotfile.open(QIODevice::ReadOnly)) {
//        QErrorMessage * msg = new QErrorMessage(this);
//        msg->message("It seems you don't have a ~/.rpmmacros\nYou may experience problems building packages.\n");
//        msg->exec();
        return;
    }
    QTextStream stream(&dotfile);

    // Perhaps will it appear as a necessity to parse the global rpm config file?

    // Pre defined macros :
    map.insert("name",getAppName());

    // .rpmmacros parsing :
    while (!stream.atEnd()) {
        QString s = stream.readLine();
        QRegExp re("%([^ \t]*)[ \t][ \t]*([^\t]*)$");
        if(re.exactMatch(s)) {
            QRegExp subst("%\\{([^%]*)\\}");
            QString value = re.cap(2).stripWhiteSpace();

            while(subst.search(value) != -1) {
                value.replace(QRegExp("%\\{"+ QRegExp_escape( subst.cap(1) ) +"\\}"),*map.find(subst.cap(1)));
            }
            map.insert(re.cap(1),value);
        }
    }
    dotfile.close();

    // create directories if necessary :
    createRpmDirectoryFromMacro("_topdir");
    createRpmDirectoryFromMacro("_tmppath");
    createRpmDirectoryFromMacro("_builddir");
    createRpmDirectoryFromMacro("_rpmdir");
    createRpmDirectoryFromMacro("_sourcedir");
    createRpmDirectoryFromMacro("_specdir");
    createRpmDirectoryFromMacro("_srcrpmdir");
}

bool SpecSupport::createRpmDirectoryFromMacro(const QString & name) {
    QMap<QString,QString>::Iterator it;
    if((it = map.find(name)) != map.end()) {
        QDir dir(*it);
        if (!dir.exists()) return dir.mkdir(*it);
    }
    return false;
}

QString SpecSupport::generatePackage( )
{
	QString spec;
        spec += "# This spec file was generated by KDevelop \n";
        spec += "# Please report any problem to KDevelop Team <kdevelop-devel@kdevelop.org> \n";
        spec += "# Thanks to Matthias Saou for his explanations on http://freshrpms.net/docs/fight.html\n\n";

        spec += "Name: " + getAppName() + "\n";
        spec += "Version: " + getAppVersion() + "\n";
        spec += "Release: " + getAppRevision() + "\n";
        spec += "Vendor: " + getAppVendor() + "\n";
        spec += "Copyright: " + getAppLicense() + "\n";
        spec += "Summary: " + getAppSummary() + "\n";
        spec += "Group: " + getAppGroup() + "\n";
        spec += "Packager: " + getAppPackager() + "\n";
        spec += "BuildRoot:  %{_tmppath}/%{name}-root \n";
        spec += "Source: " + getAppSource() + "\n";

        spec += "\n";
        spec += "%description\n";
        spec += getAppDescription()+ "\n";

        spec += "\n";
        spec += "%prep\n";
        spec += "%setup\n";
        spec += "CFLAGS=\"$RPM_OPT_FLAGS\" CXXFLAGS=\"$RPM_OPT_FLAGS\" ./configure \\ \n";
        spec += "--target=" + getAppArch() + "\n";
	spec += "--disable-debug --enable-debug=no \n";

        spec += "\n";
        spec += "%build\n";
        spec += "%configure\n";
        spec += "make\n";

        spec += "\n";
        spec += "%install\n";
        spec += "rm -rf %{buildroot}\n";
        spec += "%makeinstall\n";

        spec += "\n";
        spec += "%clean\n";
        spec += "rm -rf %{buildroot}\n";

        spec += "\n";
        spec += "%post -p /sbin/ldconfig\n";

        spec += "%postun -p /sbin/ldconfig\n";

        spec += "%files\n";
        spec += "%defattr(-, root, root)\n";
        spec += "%doc AUTHORS COPYING ChangeLog NEWS README TODO\n";
        spec += "%{_bindir}/*\n";
        spec += "%{_libdir}/*.so.*\n";
        spec += "%{_datadir}/%{name}\n";
        spec += "%{_mandir}/man8/*\n";

        spec += "%changelog\n";
        spec += getAppChangelog() + "\n";
	
	return spec;
}
