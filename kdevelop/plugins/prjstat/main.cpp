#include <klocale.h>

#include "prjstatplugin.h"

extern "C" {
    KDev_Plugin* create();
    int minor_version();
    int major_version();
    const char *name();
    const char *author();
    const char *description();
}

KDev_Plugin* create(){
    return(new PrjStatPlugin);
}

int minor_version(){
    return(0);
}

int major_version(){
    return(1);
}

const char *name(){
    return(i18n("project statistic plugin"));
}

const char *authors(){
    return(i18n("project statistic plugin"));
}
const char *description(){
    return(i18n("This plugin display some information about\nthe current project"));
}

int main(){
    return(0);
}
