
#include "Firmy.h"
#include "FirmyInternal.h"

struct FIFungible{
  NAString* name;
  NAString* identifier;
  NAInt decimals;
  NAStack exchangeAccounts; // accounts with same debitFungible
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
  naInitStack(&(fungible->exchangeAccounts), naSizeof(FIAccount*), 2);
  return fungible;
}



void fiDestructFungible(FIFungible* fungible){
  naDelete(fungible->name);
  naDelete(fungible->identifier);
  naForeachStackpMutable(&(fungible->exchangeAccounts), naDelete);
  naClearStack(&(fungible->exchangeAccounts));
}



const NAString* fiGetFungibleIdentifier(const FIFungible* fungible){
  return fungible->identifier;
}



NAInt fiGetFungibleDecimals(const FIFungible* fungible){
  return fungible->decimals;
}



FIAccount* fiGetExchangeAccount(const FIFungible* fromFungible, const FIFungible* toFungible){
  FIAccount* foundaccount = NA_NULL;
  NAStackIterator iter = naMakeStackMutator(&(toFungible->exchangeAccounts));
  while(naIterateStack(&iter)){
    FIAccount* account = naGetStackCurpMutable(&iter);
    if(fiGetAccountCreditFungible(account) == fromFungible){foundaccount = account; break;}
  }
  naClearStackIterator(&iter);

  if(!foundaccount){
    FIFungible* mutableDebitFungible = (FIFungible*)toFungible;
    FIAccount** newAccount = naPushStack(&(mutableDebitFungible->exchangeAccounts));
    *newAccount = fiNewExchangeAccount(toFungible, fromFungible); // Here, to and from are reversed.
    foundaccount = *newAccount;
  }
  return foundaccount;
}



double fiGetExchangeRate(const FIFungible* fromFungible, const FIFungible* toFungible){
  double rate = 1.;
  if(fromFungible != toFungible){

    FIAccount* foundaccount = NA_NULL;
    NAStackIterator iter = naMakeStackMutator(&(toFungible->exchangeAccounts));
    while(naIterateStack(&iter)){
      FIAccount* account = naGetStackCurpMutable(&iter);
      if(fiGetAccountCreditFungible(account) == fromFungible){foundaccount = account; break;}
    }
    naClearStackIterator(&iter);
    
    if(!foundaccount){
      #ifndef NDEBUG
        fiError("No exchange between the two fungibles available.");
      #endif
    }else{
      rate = fiGetAccountExchangeRate(foundaccount);
    }
  }
  return rate;
}



void fiSetExchangeRate(const FIFungible* fromFungible, const FIFungible* toFungible, double rate){
  #ifndef NDEBUG
    if(fromFungible == toFungible)
      fiError("Can set rate only when fungibles differ.");
  #endif
  
  FIAccount* foundaccount = NA_NULL;
  NAStackIterator iter = naMakeStackMutator(&(toFungible->exchangeAccounts));
  while(naIterateStack(&iter)){
    FIAccount* account = naGetStackCurpMutable(&iter);
    if(fiGetAccountCreditFungible(account) == fromFungible){foundaccount = account; break;}
  }
  naClearStackIterator(&iter);

  if(!foundaccount){
    foundaccount = fiGetExchangeAccount(fromFungible, toFungible);
  }
  fiSetAccountExchangeRate(foundaccount, rate);
}

