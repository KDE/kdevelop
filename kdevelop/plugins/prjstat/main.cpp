#include <klocale.h>

#include "prjstatplugin.h"

extern "C" {
  KDev_Plugin* create();
  const char* version();
  const char* copyright();
  const char* email();
  const char* homepage();
  const char* name();
  const char* author();
  const char* description();
}

KDev_Plugin* create(){
  return(new PrjStatPlugin);
}

const char* copyright(){
  return(i18n("(C) 1999, published under GPL");
}

const char* email(){
  return(i18n("kdevelop-team@barney.cs.uni-potsdam.de");
}

const char* homepage(){
  return(i18n("www.kdevelop.org"));
}

const char* version(){
  return(i18n("v0.1"));
}

const char *name(){
  return(i18n("project statistic plugin"));
}

const char *authors(){
  return(i18n("KDevelop Team"));
}

const char *description(){
  return(i18n("This plugin display some information about\nthe current project"));
}

int main(){
  return(0);
}
