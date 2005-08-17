/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ctagskinds.h"

#include <klocale.h>

struct CTagsKindMapping {
    char abbrev;
    const char *verbose;
};


struct CTagsExtensionMapping {
    const char *extension;
    CTagsKindMapping *kinds;
};


static CTagsKindMapping kindMappingAsm[] = {
    { 'd', I18N_NOOP("define")              },
    { 'l', I18N_NOOP("label")               },
    { 'm', I18N_NOOP("macro")               },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingAsp[] = {
    { 'f', I18N_NOOP("function")            },
    { 's', I18N_NOOP("subroutine")          },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingAwk[] = {
    { 'f', I18N_NOOP("function")            },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingBeta[] = {
    { 'f', I18N_NOOP("fragment definition") },
    { 'p', I18N_NOOP("any pattern")         },
    { 's', I18N_NOOP("slot")                },
    { 'v', I18N_NOOP("pattern")             },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingC[] = {
    { 'c', I18N_NOOP("class")               },
    { 'd', I18N_NOOP("macro")               },
    { 'e', I18N_NOOP("enumerator")          },
    { 'f', I18N_NOOP("function")            },
    { 'g', I18N_NOOP("enumeration")         },
    { 'm', I18N_NOOP("member")              },
    { 'n', I18N_NOOP("namespace")           },
    { 'p', I18N_NOOP("prototype")           },
    { 's', I18N_NOOP("struct")              },
    { 't', I18N_NOOP("typedef")             },
    { 'u', I18N_NOOP("union")               },
    { 'v', I18N_NOOP("variable")            },
    { 'x', I18N_NOOP("external variable")   },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingCobol[] = {
    { 'p', I18N_NOOP("paragraph")           },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingEiffel[] = {
    { 'c', I18N_NOOP("class")               },
    { 'f', I18N_NOOP("feature")             },
    { 'l', I18N_NOOP("local entity")        },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingFortran[] = {
    { 'b', I18N_NOOP("block")               },
    { 'c', I18N_NOOP("common")              },
    { 'e', I18N_NOOP("entry")               },
    { 'f', I18N_NOOP("function")            },
    { 'i', I18N_NOOP("interface")           },
    { 'k', I18N_NOOP("type component")      },
    { 'l', I18N_NOOP("label")               },
    { 'L', I18N_NOOP("local")               },
    { 'm', I18N_NOOP("module")              },
    { 'n', I18N_NOOP("namelist")            },
    { 'p', I18N_NOOP("program")             },
    { 's', I18N_NOOP("subroutine")          },
    { 't', I18N_NOOP("type")                },
    { 'v', I18N_NOOP("variable")            },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingJava[] = {
    { 'c', I18N_NOOP("class")               },
    { 'f', I18N_NOOP("field")               },
    { 'i', I18N_NOOP("interface")           },
    { 'm', I18N_NOOP("method")              },
    { 'p', I18N_NOOP("package")             },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingLisp[] = {
    { 'f', I18N_NOOP("function")            },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingMake[] = {
    { 'm', I18N_NOOP("macro")               },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingPascal[] = {
    { 'f', I18N_NOOP("function")            },
    { 'p', I18N_NOOP("procedure")           },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingPerl[] = {
    { 's', I18N_NOOP("subroutine")          },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingPHP[] = {
    { 'c', I18N_NOOP("class")               },
    { 'f', I18N_NOOP("function")            },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingPython[] = {
    { 'c', I18N_NOOP("class")               },
    { 'f', I18N_NOOP("function")            },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingRexx[] = {
    { 's', I18N_NOOP("subroutine")          },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingRuby[] = {
    { 'c', I18N_NOOP("class")               },
    { 'f', I18N_NOOP("function")            },
    { 'm', I18N_NOOP("mixin")               },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingScheme[] = {
    { 'f', I18N_NOOP("function")            },
    { 's', I18N_NOOP("set")                 },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingSh[] = {
    { 'f', I18N_NOOP("function")            },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingSlang[] = {
    { 'f', I18N_NOOP("function")            },
    { 'n', I18N_NOOP("namespace")           },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingTcl[] = {
    { 'p', I18N_NOOP("procedure")           },
    { 0  , 0                                }
};


static CTagsKindMapping kindMappingVim[] = {
    { 'f', I18N_NOOP("function")            },
    { 0  , 0                                }
};


static CTagsExtensionMapping extensionMapping[] = {
    { "asm",    kindMappingAsm     },
    { "s",      kindMappingAsm     },
    { "S",      kindMappingAsm     },
    { "asp",    kindMappingAsp     },
    { "asa",    kindMappingAsp     },
    { "awk",    kindMappingAwk     },
    { "c++",    kindMappingC       },
    { "cc",     kindMappingC       },
    { "cp" ,    kindMappingC       },
    { "cpp",    kindMappingC       },
    { "cxx",    kindMappingC       },
    { "h"  ,    kindMappingC       },
    { "h++",    kindMappingC       },
    { "hh" ,    kindMappingC       },
    { "hp" ,    kindMappingC       },
    { "hpp",    kindMappingC       },
    { "hxx",    kindMappingC       },
    { "beta",   kindMappingBeta    },
    { "cob",    kindMappingCobol   },
    { "COB",    kindMappingCobol   },
    { "e",      kindMappingEiffel  },
    { "f"   ,   kindMappingFortran },
    { "for" ,   kindMappingFortran },
    { "ftn" ,   kindMappingFortran },
    { "f77" ,   kindMappingFortran },
    { "f90" ,   kindMappingFortran },
    { "f95" ,   kindMappingFortran },
    { "java",   kindMappingJava    },
    { "cl",     kindMappingLisp    },
    { "clisp",  kindMappingLisp    },
    { "el",     kindMappingLisp    },
    { "l",      kindMappingLisp    },
    { "lisp",   kindMappingLisp    },
    { "lsp",    kindMappingLisp    },
    { "ml",     kindMappingLisp    },
    { "mak",    kindMappingMake    },
    { "p",      kindMappingPascal  },
    { "pas",    kindMappingPascal  },
    { "pl",     kindMappingPerl    },
    { "pm",     kindMappingPerl    },
    { "perl",   kindMappingPerl    },
    { "php",    kindMappingPHP     },
    { "php3",   kindMappingPHP     },
    { "phtml",  kindMappingPHP     },
    { "py",     kindMappingPython  },
    { "python", kindMappingPython  },
    { "cmd",    kindMappingRexx    },
    { "rexx",   kindMappingRexx    },
    { "rx",     kindMappingRexx    },
    { "rb",     kindMappingRuby    },
    { "sch",    kindMappingScheme  },
    { "scheme", kindMappingScheme  },
    { "scm",    kindMappingScheme  },
    { "sm",     kindMappingScheme  },
    { "SCM",    kindMappingScheme  },
    { "SM",     kindMappingScheme  },
    { "sh",     kindMappingSh      },
    { "SH",     kindMappingSh      },
    { "bsh",    kindMappingSh      },
    { "bash",   kindMappingSh      },
    { "ksh",    kindMappingSh      },
    { "zsh",    kindMappingSh      },
    { "sl",     kindMappingSlang   },
    { "tcl",    kindMappingTcl     },
    { "wish",   kindMappingTcl     },
    { "vim",    kindMappingVim     },
    { 0     , 0                    }
};


static CTagsKindMapping *findKindMapping(const QString &extension)
{
    const char *pextension = extension.latin1();

    CTagsExtensionMapping *pem = extensionMapping;
    while (pem->extension != 0) {
        if (strcmp(pem->extension, pextension) == 0)
            return pem->kinds;
        ++pem;
    }

    return 0;
}


QString CTagsKinds::findKind(char kindChar, const QString &extension)
{
    QString res;

    CTagsKindMapping *kindMapping = findKindMapping(extension);
    if (kindMapping) {
        CTagsKindMapping *pkm = kindMapping;
        while (pkm->verbose != 0) {
            if (pkm->abbrev == kindChar)
                return i18n(pkm->verbose);
            ++pkm;
        }
    }

    return QString();
}
