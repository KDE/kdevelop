#include <QLayout>
#include <QCheckBox>
#include <QRegExp>
#include <QStackedWidget>

#include <kprocess.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

#include "ui_valgrind_dialog.h"
#include "valgrind_dialog.h"

#include "valgrind_dialog.moc"


ValgrindDialog::ValgrindDialog( Type type, QWidget* parent )
  : KDialog( parent, i18n("Valgrind Memory Check"), Ok|Cancel )
  , m_type(type)
{
  w = new Ui::ValgrindDialog();
  QWidget* top = new QWidget(this);
  w->setupUi(top);
  w->valExecutableEdit->setUrl( KUrl("valgrind") );
  w->executableEdit->setFocus();
  w->stack->setCurrentWidget(type == Memcheck ? w->memcheckWidget : w->callgrindWidget);
  setMainWidget( top );
  connect( w->executableEdit->lineEdit(),  SIGNAL( textChanged( const QString &)), this, SLOT( valgrindTextChanged()));
  connect( w->valExecutableEdit->lineEdit(), SIGNAL( textChanged( const QString &)), this, SLOT( valgrindTextChanged()));
  connect( w->ctExecutableEdit->lineEdit(),  SIGNAL( textChanged( const QString &)), this, SLOT( valgrindTextChanged()));
  connect( w->kcExecutableEdit->lineEdit(), SIGNAL( textChanged( const QString &)), this, SLOT( valgrindTextChanged()));
  enableButtonOK( false );
}


ValgrindDialog::~ValgrindDialog()
{
}

void ValgrindDialog::valgrindTextChanged()
{
    if (m_type == Memcheck)
        enableButtonOK( !w->valExecutableEdit->lineEdit()->text().isEmpty() &&  !w->executableEdit->lineEdit()->text().isEmpty() );
    else if (m_type == Calltree)
        enableButtonOK( !w->executableEdit->lineEdit()->text().isEmpty() &&  !w->ctExecutableEdit->lineEdit()->text().isEmpty() && !w->kcExecutableEdit->lineEdit()->text().isEmpty() );
}

QString ValgrindDialog::executableName() const
{
  return w->executableEdit->url();
}

void ValgrindDialog::setExecutable( const QString& url )
{
  w->executableEdit->setUrl( url );
}

QString ValgrindDialog::valExecutable() const
{
  return w->valExecutableEdit->url();
}

QString ValgrindDialog::parameters() const
{
  return w->paramEdit->text();
}

void ValgrindDialog::setParameters( const QString& params )
{
  w->paramEdit->setText( params );
}

void ValgrindDialog::setValExecutable( const QString& ve )
{
  QString vUrl = ve;
  if ( vUrl.isEmpty() ) {
    vUrl = KStandardDirs::findExe( "valgrind" );
  }
  if ( vUrl.isEmpty() ) {
    KMessageBox::sorry( this, i18n( "Could not find valgrind in your $PATH. Please make "
                                    "sure it is installed properly." ),
                        i18n( "Valgrind Not Found" ) );
    w->valExecutableEdit->setUrl( KUrl("valgrind") );
  } else {
    w->valExecutableEdit->setUrl( vUrl );
  }
}

static const QString leakCheckParam( "--leak-check=yes" );
static const QString reachableParam( "--show-reachable=yes" );
static const QString childrenParam( "--trace-children=yes" );

QString ValgrindDialog::valParams() const
{
  QString params;
  if (isNewValgrindVersion())
    params = QString::fromLatin1( "--tool=memcheck " );
  params += w->valParamEdit->text();
  if ( w->memleakBox->isChecked() )
    params += " " + leakCheckParam;
  if ( w->reachableBox->isChecked() )
    params += " " + reachableParam;
  if ( w->childrenBox->isChecked() )
    params += " " + childrenParam;

  return params;
}

void ValgrindDialog::setValParams( const QString& params )
{
  QString myParams = params;
  if ( myParams.contains( leakCheckParam ) )
    w->memleakBox->setChecked( true );
  if ( myParams.contains( reachableParam ) )
    w->reachableBox->setChecked( true );
  if ( myParams.contains( childrenParam ) )
    w->childrenBox->setChecked( true );

  myParams = myParams.replace( QRegExp( leakCheckParam ), "" );
  myParams = myParams.replace( QRegExp( reachableParam ), "" );
  myParams = myParams.replace( QRegExp( childrenParam ), "" );
  myParams = myParams.replace( "--tool=memcheck ", "" );
  myParams = myParams.trimmed();
  w->valParamEdit->setText( myParams );
}

QString ValgrindDialog::ctExecutable() const
{
  return w->ctExecutableEdit->url();
}

void ValgrindDialog::setCtExecutable( const QString& ce )
{
  QString vUrl = ce;
  if ( vUrl.isEmpty() ) {
    vUrl = KStandardDirs::findExe( "calltree" );
  }
  if ( vUrl.isEmpty() ) {
    KMessageBox::sorry( this, i18n( "Could not find calltree in your $PATH. Please make "
                                    "sure it is installed properly." ),
                        i18n( "Calltree Not Found" ) );
    w->ctExecutableEdit->setUrl( KUrl("calltree") );
  } else {
    w->ctExecutableEdit->setUrl( vUrl );
  }
}

QString ValgrindDialog::ctParams() const
{
  QString params = w->ctParamEdit->text();
  if ( w->ctChildrenBox->isChecked() )
    params += " " + childrenParam;

  return params;
}

void ValgrindDialog::setCtParams( const QString& params )
{
  QString myParams = params;
  if ( myParams.contains( childrenParam ) )
    w->ctChildrenBox->setChecked( true );

  myParams = myParams.replace( QRegExp( childrenParam ), "" );
  myParams = myParams.trimmed();
  w->ctParamEdit->setText( myParams );
}

QString ValgrindDialog::kcExecutable( ) const
{
  return w->kcExecutableEdit->url();
}

void ValgrindDialog::setKcExecutable( const QString& ke )
{
  QString vUrl = ke;
  if ( vUrl.isEmpty() ) {
    vUrl = KStandardDirs::findExe( "kcachegrind" );
  }
  if ( vUrl.isEmpty() ) {
    KMessageBox::sorry( this, i18n( "Could not find kcachegrind in your $PATH. Please make "
                                    "sure it is installed properly." ),
                        i18n( "KCachegrind Not Found" ) );
    w->kcExecutableEdit->setUrl( KUrl("kcachegrind") );
  } else {
    w->kcExecutableEdit->setUrl( vUrl );
  }
}

bool ValgrindDialog::isNewValgrindVersion( ) const
{
  KProcess *proc = new KProcess;
  proc->setUseShell(true);
  *proc << "test \"valgrind-20\" == `valgrind --version | awk -F \\. '{print $1$2}'`";
  proc->start(KProcess::Block);
  if (proc->normalExit())
    return proc->exitStatus();
  return true;
}

// kate: space-indent on; indent-width 2; tab-width 2; show-tabs on;

