
#ifndef _VALGRIND_DIALOG_H_
#define _VALGRIND_DIALOG_H_

#include <kdialog.h>

class KUrlRequester;
class DialogWidget;

namespace Ui { class ValgrindDialog; }

/**
 *
 * Harald Fernengel
 **/
class ValgrindDialog : public KDialog
{
    Q_OBJECT
public:
  enum Type { Memcheck = 0, Calltree = 1 };

  ValgrindDialog( Type type, QWidget* parent = 0 );
  ~ValgrindDialog();

  // the app to check
  QString executableName() const;
  void setExecutable( const QString& url );

  // command line parameters for the app
  QString parameters() const;
  void setParameters( const QString& params );

  // name and/or path to the valgrind executable
  QString valExecutable() const;
  void setValExecutable( const QString& ve );

  // command line parameters for valgrind
  QString valParams() const;
  void setValParams( const QString& params );

  // name and/or path to the calltree executable
  QString ctExecutable() const;
  void setCtExecutable( const QString& ce );

  // command line parameters for calltree
  QString ctParams() const;
  void setCtParams( const QString& params );

  // name and/or path to the kcachegrind executable
  QString kcExecutable() const;
  void setKcExecutable( const QString& ke );

protected:
  bool isNewValgrindVersion() const;

private:
  Ui::ValgrindDialog* w;
  Type m_type;

private slots:
  void valgrindTextChanged();
};

#endif
