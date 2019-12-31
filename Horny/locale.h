#include <libraries/locale.h>
#include <proto/locale.h>

extern struct Catalog *catalog;

#define CAT(X,Y) (STRPTR)GetCatalogStr(catalog,X,Y)

#include "catalogids.h"
