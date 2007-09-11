#include "cmakebuilddircreator.h"

#include "ui_cmakebuilddircreator.h"

CMakeBuildDirCreator::CMakeBuildDirCreator(const QString& srcDir, QWidget* parent, Qt::WindowFlags f)
	: QDialog(parent, f), m_srcFolder(srcDir)
{
	m_creatorUi = new Ui::CMakeBuildDirCreator;
	m_creatorUi->setupUi( this );
	m_creatorUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	
	QString cmakeBin=executeProcess("which", QStringList("cmake"));
	setCMakeBinary(KUrl(cmakeBin));
	
	connect(m_creatorUi->run, SIGNAL(clicked()), this, SLOT(runBegin()));
	connect(m_creatorUi->cmakeBin, SIGNAL(textChanged(const QString &)), this, SLOT(updated()));
	connect(m_creatorUi->buildFolder, SIGNAL(textChanged(const QString &)), this, SLOT(updated()));
	connect(m_creatorUi->buildType, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(updated()));
	connect(&m_proc, SIGNAL(readyReadStandardError()), this, SLOT(addError()));
	connect(&m_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(addOutput()));
	connect(&m_proc, SIGNAL(finished ( int , QProcess::ExitStatus )), this, SLOT(cmakeCommandDone ( int , QProcess::ExitStatus )));
	updated();
}

void CMakeBuildDirCreator::runBegin()
{
	m_creatorUi->cmakeOutput->clear();
	QStringList args;
	kDebug(9032) << "Type of build: " << buildType();
	kDebug(9032) << "Installing to: " << installPrefix();
	kDebug(9032) << "Build directory: " << buildFolder();
	args += m_srcFolder;
	args += "-DCMAKE_INSTALL_PREFIX="+installPrefix().toLocalFile();
	args += "-DCMAKE_BUILD_TYPE="+buildType();
	
	m_proc.setWorkingDirectory(buildFolder().toLocalFile());
	m_proc.setProgram(cmakeBinary().toLocalFile(), args);
	m_proc.setOutputChannelMode(KProcess::MergedChannels);
	m_proc.start();
	updated();
	m_creatorUi->status->setText(i18n("Running"));
}

void CMakeBuildDirCreator::runEnd()
{
}

void CMakeBuildDirCreator::addError()
{
	QByteArray output=m_proc.readAllStandardError();
	QString s;
	s.append(output.trimmed());
	m_creatorUi->cmakeOutput->setPlainText(m_creatorUi->cmakeOutput->toPlainText()+s+'\n');
}

void CMakeBuildDirCreator::addOutput()
{
	QByteArray output=m_proc.readAllStandardOutput();
	QString s;
	s.append(output.trimmed());
	m_creatorUi->cmakeOutput->setPlainText(m_creatorUi->cmakeOutput->toPlainText()+s+'\n');
}

void CMakeBuildDirCreator::cmakeCommandDone(int exitCode, QProcess::ExitStatus exitStatus)
{
	/*QByteArray b = m_proc.readAllStandardOutput();
	QString t;
	t.prepend(b.trimmed());
	m_creatorUi->cmakeOutput->setPlainText(t);*/
	
	bool successful=exitCode==0;
	if(successful) {
		m_creatorUi->status->setText(i18n("Created successfully"));
	} else {
		m_creatorUi->status->setText(i18n("An error occurred"));
	}
	m_creatorUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(successful);
}

void CMakeBuildDirCreator::updated()
{
	bool haveCMake=QFile::exists(m_creatorUi->cmakeBin->url().toLocalFile());
	m_creatorUi->buildFolder->setEnabled(haveCMake);
	m_creatorUi->installPrefix->setEnabled(haveCMake);
	m_creatorUi->buildType->setEnabled(haveCMake);
// 	m_creatorUi->generator->setEnabled(haveCMake);
	m_creatorUi->run->setEnabled(haveCMake);
	if(!haveCMake) {
		m_creatorUi->status->setText(i18n("You need to select a cmake binary"));
	} else {
        bool dirCorrect=!m_creatorUi->buildFolder->url().isEmpty();
        if(dirCorrect) {
            QDir d(m_creatorUi->buildFolder->url().toLocalFile());
            dirCorrect=d.exists() && d.count()<=2;
        }
// 		m_creatorUi->buildFolder->setEnabled(true);
		m_creatorUi->installPrefix->setEnabled(dirCorrect);
		m_creatorUi->buildType->setEnabled(dirCorrect);
// 		m_creatorUi->generator->setEnabled(dirCorrect);
		m_creatorUi->run->setEnabled(dirCorrect);
		if(!dirCorrect)
			m_creatorUi->status->setText(i18n("The selected directory does not exist or is not empty")); //Useful to prevent disasters
		else
			m_creatorUi->status->setText(i18n("Click run when you are ready"));
	}
}

void CMakeBuildDirCreator::setCMakeBinary(const KUrl& url) { m_creatorUi->cmakeBin->setUrl(url); }

void CMakeBuildDirCreator::setInstallPrefix(const KUrl& url) { m_creatorUi->installPrefix->setUrl(url); }

void CMakeBuildDirCreator::setBuildFolder(const KUrl& url) { m_creatorUi->buildFolder->setUrl(url); }

void CMakeBuildDirCreator::setBuildType(const QString& s) { m_creatorUi->buildType->addItem(s); }

KUrl CMakeBuildDirCreator::cmakeBinary() const { return m_creatorUi->cmakeBin->url(); }

KUrl CMakeBuildDirCreator::installPrefix() const { return m_creatorUi->installPrefix->url(); }

KUrl CMakeBuildDirCreator::buildFolder() const { return m_creatorUi->buildFolder->url(); }

QString CMakeBuildDirCreator::buildType() const { return m_creatorUi->buildType->currentText(); }

QString CMakeBuildDirCreator::executeProcess(const QString& execName, const QStringList& args)
{
	kDebug(9032) << "Executing:" << execName << "::" << args /*<< "into" << *m_vars*/;

	KProcess p;
	p.setOutputChannelMode(KProcess::MergedChannels);
	p.setProgram(execName, args);
	p.start();

	if(!p.waitForFinished())
	{
		kDebug(9032) << "failed to execute:" << execName;
	}

	QByteArray b = p.readAllStandardOutput();
	QString t;
	t.prepend(b.trimmed());
	kDebug(9032) << "executed" << execName << "<" << t;

	return t;
}

#include "cmakebuilddircreator.h"
