#include <libraries/locale.h>
#include <proto/locale.h>

extern struct Catalog *catalog;

#define CAT(X,Y) (STRPTR)ILocale->GetCatalogStr(catalog,X,Y)

#include "catalogids.h"
