
/* This template is based off of the KOffice example written by Torben Weis <weis@kde.org
   It was converted to a KDevelop template by Ian Reinhart Geiser <geiseri@yahoo.com>
*/

#ifndef %{APPNAME}_VIEW
#define %{APPNAME}_VIEW

#include <koView.h>
//Added by qt3to4:
#include <QPaintEvent>

class KAction;
class QPaintEvent;

class %{APPNAME}Part;

class %{APPNAME}View : public KoView
{
    Q_OBJECT
public:
    %{APPNAME}View( %{APPNAME}Part* part, QWidget* parent = 0, const char* name = 0 );

protected slots:
    void cut();

protected:
    void paintEvent( QPaintEvent* );

    virtual void updateReadWrite( bool readwrite );

private:
    //KAction* m_cut;
};

#endif
