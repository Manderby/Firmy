
#include "Firmy.h"
#include "FirmyInternal.h"

struct FIFungible{
  NAString* name;
  NAString* identifier;
  size_t decimals;
  double doubleRounder;
  NAi256 amountRounder;
  NAStack exchangeAccounts; // accounts with same debitFungible
};

void fiDestructFungible(FIFungible* fungible);
NA_RUNTIME_TYPE(FIFungible, fiDestructFungible, NA_FALSE);



FIFungible* fiNewFungible(
  const NAUTF8Char* name,
  const NAUTF8Char* identifier,
  size_t decimals)
{
  #ifndef NDEBUG
    if(decimals > 36)
      fiError("Can not be more accurate than 36 decimal digits after the point");
  #endif
  FIFungible* fungible = naNew(FIFungible);
  fungible->name = naNewStringWithFormat(name);
  fungible->identifier = naNewStringWithFormat(identifier);
  fungible->decimals = decimals;
  naInitStack(&(fungible->exchangeAccounts), naSizeof(FIAccount*), 0, 0);

  fungible->doubleRounder = 1.;
  for(size_t i = 0; i < decimals; i++){
    fungible->doubleRounder *= 10.;
  }
  
  fungible->amountRounder = naMakei256WithDouble(1.);
  NAi256 decimalTen = naMakei256WithDouble(10.);
  for(size_t i = 0; i < 36 - decimals; i++){
    fungible->amountRounder = naMuli256(fungible->amountRounder, decimalTen);
  }
  fungible->amountRounder = naShri256(fungible->amountRounder, 1);

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



size_t fiGetFungibleDecimals(const FIFungible* fungible){
  return fungible->decimals;
}



NAi256 fiRoundFungiblei256(const FIFungible* fungible, NAi256 amount, NABool backToUnit){
  if(!naEquali256(fungible->amountRounder, NA_ZERO_i256)){
    amount = naDivi256(amount, fungible->amountRounder);
    amount = naAddi256(amount, NA_ONE_i256);
    amount = naShri256(amount, 1);
    if(backToUnit){
      amount = naShli256(amount, 1);
      amount = naMuli256(amount, fungible->amountRounder);
    }
  }
  return amount;
}



double fiRoundFungibleAmount(const FIFungible* fungible, double amount){
  amount *= fungible->doubleRounder;
  amount = naRound(amount);
  amount /= fungible->doubleRounder;
  return amount;
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



FIAmount fiGetExchangeRate(const FIFungible* fromFungible, const FIFungible* toFungible){
  FIAmount rate = fiAmountOne();
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

