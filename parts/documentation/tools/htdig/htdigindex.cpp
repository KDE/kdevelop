/***************************************************************************
 *   Copyright (C) 1999-2001 by Matthias Hoelzer-Kluepfel                  *
 *   hoelzer@kde.org                                                       *
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "htdigindex.h"

#include <iostream>

#include <qapplication.h>
#include <qdir.h>
#include <qfile.h>
#include <qlayout.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qlabel.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <kdeversion.h>
#include <kprogress.h>

#define INDEXER

ProgressDialog::ProgressDialog(bool index, QWidget *parent, const char *name)
    :KDialogBase(KDialogBase::Plain, i18n("Generating Search Index"), Cancel | Ok, Close,
    parent, name, false)
{
    proc = 0;

    indexdir = kapp->dirs()->saveLocation("data", "kdevdocumentation/search");
    QDir d; d.mkdir(indexdir);

    KConfig config("kdevdocumentation", true);
    config.setGroup("htdig");
    databaseDir = config.readPathEntry("databaseDir", indexdir);

    if (!index)
        return;

    d.mkdir( databaseDir );

    showButtonOK( false );
    QGridLayout *grid = new QGridLayout(plainPage(), 5,3, spacingHint());

    QLabel *l = new QLabel(i18n("Scanning for files"), plainPage());
    grid->addMultiCellWidget(l, 0, 0, 1, 2);

    filesLabel = new QLabel(plainPage());
    grid->addWidget(filesLabel, 1, 2);
    setFilesScanned(0);

    check1 = new QLabel(plainPage());
    grid->addWidget(check1, 0, 0);

    l = new QLabel(i18n("Extracting search terms"), plainPage());
    grid->addMultiCellWidget(l, 2,2, 1,2);

    bar = new KProgress(plainPage());
    grid->addWidget(bar, 3,2);

    check2 = new QLabel(plainPage());
    grid->addWidget(check2, 2,0);

    l = new QLabel(i18n("Generating index..."), plainPage());
    grid->addMultiCellWidget(l, 4,4, 1,2);

    check3 = new QLabel(plainPage());
    grid->addWidget(check3, 4,0);

    setState(0);

    setMinimumWidth(300);
    connect(this, SIGNAL(cancelClicked()), this, SLOT(cancelClicked()));
    connect(this, SIGNAL(okClicked()), this, SLOT(okClicked()));
    QTimer::singleShot(0, this, SLOT(slotDelayedStart()));
}

ProgressDialog::~ProgressDialog()
{
}

void ProgressDialog::slotDelayedStart()
{
    procdone = false;
    scanDirectories();
    if (!createConfig())
    {
      done(1);
      return;
    }
    generateIndex();
}

void ProgressDialog::done(int r)
{
    if (!r)
    {
        showButtonCancel( false );
        showButtonOK( true );
    }
    else
        KDialogBase::done(r);
}

void ProgressDialog::setFilesScanned(int n)
{
    filesLabel->setText(i18n("Files processed: %1").arg(n));
}

void ProgressDialog::setFilesToDig(int n)
{
    bar->setRange(0, n);
}

void ProgressDialog::setFilesDigged(int n)
{
    bar->setValue(n);
}

void ProgressDialog::setState(int n)
{
    QPixmap unchecked = QPixmap(locate("data", "kdevdocumentation/pics/unchecked.xpm"));
    QPixmap checked = QPixmap(locate("data", "kdevdocumentation/pics/checked.xpm"));

    check1->setPixmap( n > 0 ? checked : unchecked);
    check2->setPixmap( n > 1 ? checked : unchecked);
    check3->setPixmap( n > 2 ? checked : unchecked);
}


void ProgressDialog::addDir(const QString &dir)
{
    kdDebug(9002) << "Add dir : " << dir << endl;
    QDir d(dir, "*.html", QDir::Name|QDir::IgnoreCase, QDir::Files | QDir::Readable);
    QStringList list = d.entryList();

    QStringList::ConstIterator it;
    for ( it=list.begin(); it!=list.end(); ++it )
    {
        if( (*it).right( 12 ).lower( ) == "-source.html" )
            continue;

        files.append(dir + "/" + *it);
        setFilesScanned(++filesScanned);
    }

    QDir d2(dir, QString::null, QDir::Name|QDir::IgnoreCase, QDir::Dirs);
    QStringList dlist = d2.entryList();

    for ( it=dlist.begin(); it != dlist.end(); ++it ) 
    {
        if (*it != "." && *it != "..") 
        {
            addDir(dir + "/" + *it);
            kapp->processEvents();
        }
        if (procdone)
        {
          return;
        }
    }
    kapp->processEvents();
}

void ProgressDialog::scanDirectories()
{
    QString ftsLocationsFile = locateLocal("data", "kdevdocumentation/search/locations.txt");

    QFile f(ftsLocationsFile);
    if (!f.open(IO_ReadOnly))
        return;
    QTextStream str(&f);
    
    filesScanned = 0;
    
    while (!str.eof())
    {
        QString loc = str.readLine();
        if (loc.isEmpty())
            continue;
        QFileInfo fi(loc);
        if (fi.isDir())
            addDir(loc);
        else if (fi.isFile())
        {
            files.append(loc);
            setFilesScanned(++filesScanned);
        }
    }
}

bool ProgressDialog::createConfig()
{
    // locate the common dir
    QString language = KGlobal::locale()->language();
    if (language == "C")
        language = "en";

    QString wrapper = locate("data", QString("kdevdocumentation/%1/wrapper.html").arg(language));
    if (wrapper.isEmpty())
        wrapper = locate("data", QString("kdevdocumentation/en/wrapper.html"));
    if (wrapper.isEmpty())
        return false;
    wrapper = wrapper.left(wrapper.length()-12);

    // locate the image dir
    QString images = locate("data", "kdevdocumentation/pics/star.png");
    if (images.isEmpty())
        return false;
    images = images.left(images.length()-8);

    QFile f(indexdir + "/htdig.conf");
    if (f.open(IO_WriteOnly)) 
    {
        QTextStream ts(&f);

        ts << "database_dir:\t\t" << databaseDir << endl;
        ts << "start_url:\t\t`" << indexdir << "/files`" << endl;
        ts << "local_urls:\t\thttp://localhost/=/" << endl;
//        ts << "local_urls:\t\tfile://=" << endl;
        ts << "local_urls_only:\ttrue" << endl;
        ts << "limit_urls_to:\t\tfile:// http://localhost/" << endl;
        ts << "maximum_pages:\t\t1" << endl;
        ts << "image_url_prefix:\t" << images << endl;
        ts << "star_image:\t\t" << images << "star.png" << endl;
        ts << "star_blank:\t\t" << images << "star_blank.png" << endl;
        ts << "compression_level:\t6" << endl;
        ts << "max_hop_count:\t\t0" << endl;

        ts << "search_results_wrapper:\t" << wrapper << "wrapper.html" << endl;
        ts << "nothing_found_file:\t" << wrapper << "nomatch.html" << endl;
        ts << "syntax_error_file:\t" << wrapper << "syntax.html" << endl;
        ts << "bad_word_list:\t\t" << wrapper << "bad_words" << endl;

        f.close();
        return true;
    }

    return false;
}

#define CHUNK_SIZE 100

void ProgressDialog::startHtdigProcess(bool initial)
{
    kdDebug(9002) << "htdig started" << endl;
    delete proc;
    proc = new KProcess();
    *proc << exe << "-c" << (indexdir + "/htdig.conf");
    if (initial) {
        *proc << "-i";
    }
    connect(proc, SIGNAL(processExited(KProcess *)),
            this, SLOT(htdigExited(KProcess *)));
    
    htdigRunning = true;
    
    // write out file
    QFile f(indexdir+"/files");
    if (!f.open(IO_WriteOnly)) {
        kdDebug(9002) << "Could not open `files` for writing" << endl;
        done(1);
        return;
    }
    QTextStream ts(&f);
    for (int i=0; i<CHUNK_SIZE; ++i, ++count) {
        if (count >= filesToDig) {
            procdone = true;
            break;
        }
    //    ts << "file://localhost/" + files[count] << endl;
        ts << "http://localhost/" + files[count] << endl;
    }
    f.close();
    
    // execute htdig
    proc->start(KProcess::NotifyOnExit, KProcess::Stdout);   
}

bool ProgressDialog::generateIndex()
{
    setState(1);
    procdone = false;
    // run htdig
    KConfig config("kdevdocumentation", true);
    config.setGroup("htdig");
    exe = config.readPathEntry("htdigbin", kapp->dirs()->findExe("htdig"));
    if (exe.isEmpty())
    {
        done(1);
        return true;
    }
    filesToDig = files.count();
    count = 0;
    setFilesToDig(filesToDig);
    filesDigged = 0;

    //    QDir d; d.mkdir(indexdir);
    startHtdigProcess(true);
    return true;
}

void ProgressDialog::htdigStdout(KProcess *, char *buffer, int len)
{
    QString line = QString(buffer).left(len);

    int cnt=0, index=-1;
    while ( (index = line.find("http://", index+1)) > 0)
        cnt++;
    filesDigged += cnt;

    cnt=0, index=-1;
    while ( (index = line.find("not changed", index+1)) > 0)
        cnt++;
    filesDigged -= cnt;

    setFilesDigged(filesDigged);
}

void ProgressDialog::htdigExited(KProcess *proc)
{
    kdDebug(9002) << "htdig terminated" << endl;
    if (!proc->normalExit())
    {
        delete proc;
        proc = 0L;
        done(1);
        return;
    }
    if (proc && proc->exitStatus() != 0) 
    {
        KMessageBox::sorry(0, i18n("Running htdig failed"));
        delete proc;
        proc = 0L;
        done(1);
        return;
    }
    htdigRunning = false;
    filesDigged += CHUNK_SIZE;
    setFilesDigged(filesDigged);
    if (!procdone)
    {
        startHtdigProcess(false);
    }
    else
    {
        setFilesDigged(filesToDig);
        setState(2);
        
        KConfig config("kdevdocumentation", true);
        config.setGroup("htdig");
        // run htmerge -----------------------------------------------------
        exe = config.readPathEntry("htmergebin", kapp->dirs()->findExe("htmerge"));
        if (exe.isEmpty())
        {
            done(1);
            return;
        }
        startHtmergeProcess();
    }
}

void ProgressDialog::startHtmergeProcess()
{
    kdDebug(9002) << "htmerge started" << endl;
    delete proc;
    proc = new KProcess();
    *proc << exe << "-c" << (indexdir + "/htdig.conf");

    kdDebug(9002) << "Running htmerge" << endl;

    connect(proc, SIGNAL(processExited(KProcess *)),
            this, SLOT(htmergeExited(KProcess *)));

    htmergeRunning = true;

    proc->start(KProcess::NotifyOnExit, KProcess::Stdout);
}

void ProgressDialog::htmergeExited(KProcess *proc)
{
    kdDebug(9002) << "htmerge terminated" << endl;
    htmergeRunning = false;
    if (!proc->normalExit())
    {
        delete proc;
        proc = 0L;
        done(1);
        return;
    }
    if (proc && proc->exitStatus() != 0) 
    {
        KMessageBox::sorry(0, i18n("Running htmerge failed"));
        delete proc;
        proc = 0L;
        done(1);
        return;
    }
    setState(3);
    done(0);
}

void ProgressDialog::cancelClicked()
{
    if ((htdigRunning || htmergeRunning) && proc && proc->isRunning())
    {
        kdDebug(9002) << "Killing " << (htdigRunning ? "htdig" : "htmerge") << "daemon with Sig. 9" << endl;
        proc->kill(9);
        htdigRunning = htmergeRunning = false;
    }
    else
    {
        procdone = true;
        done(2);
    }
}

void ProgressDialog::okClicked()
{
    if (proc)
        proc->kill();

    KDialogBase::done(0);
}

int main(int argc, char *argv[])
{
    static const KCmdLineOptions options[] =
    {
        { "c", I18N_NOOP( "Update user's htdig configuration file only" ), 0 },
        { "i",  I18N_NOOP( "-c and generate index" ), 0 },
        KCmdLineLastOption
    };

    KAboutData aboutData("kdevelop-htdig", I18N_NOOP("KDevelop ht://Dig Indexer"),
                         "0.2", I18N_NOOP("KDE Index generator for documentation files."));

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    KGlobal::locale()->setMainCatalogue("kdevelop");

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if (args->isSet("c"))
    {
        ProgressDialog *search = new ProgressDialog( false, 0, "progress dialog");

        if (search->createConfig())
            KMessageBox::information(0, i18n("Configuration file updated"));
        else
            KMessageBox::error(0, i18n("Configuration file update failed."));
    }
    else
        if (args->isSet("i"))
        {
            ProgressDialog *search = new ProgressDialog(true, 0, "progress dialog");
            app.setMainWidget(search);
            search->show();
            app.exec();
        }
        else
        {
            std::cerr << "Internal error generating index - unknown argument\n" << std::endl;
            return 1;
        }

    return 0;
}

#include "htdigindex.moc"
