 
#ifndef _VALGRIND_DIALOG_H_
#define _VALGRIND_DIALOG_H_

#include <kdialogbase.h>

class KURLRequester;
class DialogWidget;

/**
 * 
 * Harald Fernengel
 **/
class ValgrindDialog : public KDialogBase
{

public:
  ValgrindDialog( QWidget* parent = 0 );
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
      
private:
  DialogWidget *w;
};

#endif
