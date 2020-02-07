
#include "Firmy.h"
#include "FirmyInternal.h"

struct FIFungible{
  NAString* name;
  NAString* identifier;
  NAInt decimals;
};

void fiDestructFungible(FIFungible* fungible);
NA_RUNTIME_TYPE(FIFungible, fiDestructFungible, NA_FALSE);



FIFungible* fiNewFungible(
  const NAUTF8Char* name,
  const NAUTF8Char* identifier,
  NAInt decimals)
{
  FIFungible* fungible = naNew(FIFungible);
  fungible->name = naNewStringWithUTF8CStringLiteral(name);
  fungible->identifier = naNewStringWithUTF8CStringLiteral(identifier);
  fungible->decimals = decimals;
  return fungible;
}



void fiDestructFungible(FIFungible* fungible){
  naDelete(fungible->name);
  naDelete(fungible->identifier);
}



const NAString* fiGetFungibleIdentifier(const FIFungible* fungible){
  return fungible->identifier;
}



NAInt fiGetFungibleDecimals(FIFungible* fungible){
  return fungible->decimals;
}
