/*
 *  File : snippetconfig.h
 *
 *  Author: Robert Gruber <rgruber@users.sourceforge.net>
 *
 *  Copyright: See COPYING file that comes with this distribution
 */

#ifndef SNIPPETCONFIG_H
#define SNIPPETCONFIG_H

#include <qstring.h>
#include <qrect.h>


/**
This class stores the values that can be configured via the
KDevelop settings dialog
@author Robert Gruber
*/
class SnippetConfig{
public:
    SnippetConfig();

    ~SnippetConfig();

  bool useToolTips() { return (bToolTip); };
  int getInputMethod() { return (iInputMethod); };
  QString getDelimiter() { return (strDelimiter); };
  QRect getSingleRect() { return (rSingle); };
  QRect getMultiRect() { return (rMulti); };
  int getAutoOpenGroups() { return iAutoOpenGroups; }
  
  void setToolTips(bool b) { bToolTip=b; };
  void setInputMethod(int i) { iInputMethod=i; };
  void setDelimiter(QString s) { strDelimiter=s; };
  void setSingleRect(QRect r) {
    rSingle = (r.isValid())?r:QRect();
  }
  void setMultiRect(QRect r) {
    rMulti = (r.isValid())?r:QRect();
  }
  void setAutoOpenGroups(int autoopen) { iAutoOpenGroups = autoopen; }

protected:
    bool bToolTip;
    int iInputMethod;
    QString strDelimiter;
    QRect rSingle;
    QRect rMulti;
    int iMultiBasicHeight;
    int iMultiCount;
    int iAutoOpenGroups;
};

#endif
