
#include "Firmy.h"
#include "FirmyInternal.h"
#include FIRMY_NALIB_PATH(NAStack.h)
#include FIRMY_NALIB_PATH(NAHeap.h)


struct FIAccount{
  FIaccountType type;
  const FIFungible* debitFungible;
  const FIFungible* creditFungible;
  NABool dirty;
  FIAmount exchangeRate;
  NAString* identifier;
  NAString* name;
  FIAccount* parent;
  NAStack childs;
  FIAmount totalDebitSum;
  FIAmount totalCreditSum;
  FIAmount localDebitSum;
  FIAmount localCreditSum;
};



void fiDestructAccount(FIAccount* account);
NA_RUNTIME_TYPE(FIAccount, fiDestructAccount, NA_FALSE);



FIAccount* fiNewAccount(
  FIaccountType accountType,
  const FIFungible* fungible,
  const NAString* newIdentifier,
  const NAString* newName,
  FIAccount* newParent)
{
  FIAccount* account = naNew(FIAccount);
  account->type = accountType;
  account->debitFungible = fungible;
  account->creditFungible = fungible;
  account->dirty = NA_TRUE;
  account->exchangeRate = fiAmountOne();
  if(newIdentifier){
    account->identifier = naNewStringExtraction(newIdentifier, 0, -1);
  }else{
    account->identifier = NA_NULL;
  }
  if(newName){
    account->name = naNewStringExtraction(newName, 0, -1);
  }else{
    account->name = NA_NULL;
  }
  account->parent = newParent;
  naInitStack(&(account->childs), naSizeof(FIAccount*), 2);
  account->totalDebitSum  = fiAmountZero();
  account->totalCreditSum = fiAmountZero();
  account->localDebitSum  = fiAmountZero();
  account->localCreditSum = fiAmountZero();
  if(account->parent){
    fiAddAccountChild(account->parent, account);
  }
  return account;
}



FIAccount* fiNewExchangeAccount(
  const FIFungible* debitFungible,
  const FIFungible* creditFungible)
{
  FIAccount* account = naNew(FIAccount);
  account->type = FIRMY_ACCOUNT_TYPE_EXCHANGE;
  account->debitFungible = debitFungible;
  account->creditFungible = creditFungible;
  account->exchangeRate = fiAmountOne();
  account->identifier = naNewStringWithUTF8CStringLiteral("exchangeAccount");
  account->name = naNewStringWithUTF8CStringLiteral("Exchange Account");
  account->parent = NA_NULL;
  naInitStack(&(account->childs), naSizeof(FIAccount*), 2);
  account->totalDebitSum  = fiAmountZero();
  account->totalCreditSum = fiAmountZero();
  account->localDebitSum  = fiAmountZero();
  account->localCreditSum = fiAmountZero();
  return account;
}



void fiDestructAccount(FIAccount* account){
  if(account->identifier){naDelete(account->identifier);}
  if(account->name){naDelete(account->name);}
  naClearStack(&(account->childs));
}



void fiAddAccountChild(FIAccount* account, FIAccount* child){
  FIAccount** newEntry = naPushStack(&(account->childs));
  *newEntry = child;
}



void fiCarryAccountOver(FIAccount* account, FIAmount amountDebit, FIAmount amountCredit){
  const NAUTF8Char* text = "Carry Over";
  if(!fiIsAmountZero(amountDebit)){
    fiBookAmount(amountDebit, account, NA_NULL, text);
  }else{
    fiBookAmount(amountCredit, NA_NULL, account, text);
  }
}



const NAString* fiGetAccountIdentifier(const FIAccount* account){
  return account->identifier;
}
const NAString* fiGetAccountName(const FIAccount* account){
  return account->name;
}
FIaccountType fiGetAccountType(const FIAccount* account){
  return account->type;
}
const FIFungible* fiGetAccountAnyFungible(const FIAccount* account){
  #ifndef NDEBUG
    if(account->debitFungible != account->creditFungible)
      fiError("Account has two different fungibles");
  #endif
  return account->debitFungible;
}
const FIFungible* fiGetAccountDebitFungible(const FIAccount* account){
  return account->debitFungible;
}
const FIFungible* fiGetAccountCreditFungible(const FIAccount* account){
  return account->creditFungible;
}
FIAmount fiGetAccountExchangeRate(const FIAccount* account){
  return account->exchangeRate;
}
void fiSetAccountExchangeRate(FIAccount* account, double rate){
  account->exchangeRate = fiAmount(rate);
}
FIAmount fiGetAccountLocalDebitSum(const FIAccount* account){
  return account->localDebitSum;
}
FIAmount fiGetAccountLocalCreditSum(const FIAccount* account){
  return account->localCreditSum;
}
void fiComputeTotalSums(FIAccount* account){
  account->totalDebitSum = account->localDebitSum;
  account->totalCreditSum = account->localCreditSum;
  NAStackIterator iter = naMakeStackAccessor(&(account->childs));
  while(naIterateStack(&iter)){
    const FIAccount* childAccount = naGetStackCurpConst(&iter);
    FIAmount debitRate = fiGetExchangeRate(fiGetAccountDebitFungible(childAccount), fiGetAccountDebitFungible(account));
    FIAmount childDebitTotal = fiGetAccountTotalDebitSum(childAccount);
    FIAmount childDebitRatedTotal = fiMulAmount(childDebitTotal, debitRate);
    account->totalDebitSum = fiAddAmount(account->totalDebitSum, childDebitRatedTotal);
    FIAmount creditRate = fiGetExchangeRate(fiGetAccountCreditFungible(childAccount), fiGetAccountCreditFungible(account));
    FIAmount childCreditTotal = fiGetAccountTotalCreditSum(childAccount);
    FIAmount childCreditRatedTotal = fiMulAmount(childCreditTotal, creditRate);
    account->totalCreditSum = fiAddAmount(account->totalCreditSum, childCreditRatedTotal);
  }
  naClearStackIterator(&iter);
}
FIAmount fiGetAccountTotalDebitSum(const FIAccount* account){
  if(account->dirty){
    FIAccount* mutableAccount = (FIAccount*)account;
    fiComputeTotalSums(mutableAccount);
    mutableAccount->dirty = NA_FALSE;
  }
  return account->totalDebitSum;
}
FIAmount fiGetAccountTotalCreditSum(const FIAccount* account){
  if(account->dirty){
    FIAccount* mutableAccount = (FIAccount*)account;
    fiComputeTotalSums(mutableAccount);
    mutableAccount->dirty = NA_FALSE;
  }
  return account->totalCreditSum;
}
FIAccount* getAccountParent(const FIAccount* account){
  return account->parent;
}



void fiAddAccountDebitSum(FIAccount* account, FIAmount amount){
  #ifndef NDEBUG
    if(fiSmallerAmount(amount, fiAmountZero()) && fiSmallerAmount(account->totalDebitSum, fiNegAmount(amount)))
      fiError("amount is negative");
  #endif
  
  account->localDebitSum = fiAddAmount(account->localDebitSum, amount);
  
  while(account->parent && !account->parent->dirty){
    account = account->parent;
    account->dirty = NA_TRUE;
  }
}



void fiAddAccountCreditSum(FIAccount* account, FIAmount amount){
  #ifndef NDEBUG
    if(fiSmallerAmount(amount, fiAmountZero()) && fiSmallerAmount(account->totalCreditSum, fiNegAmount(amount))){
      printf("amount is negative");
    }
  #endif

  account->localCreditSum = fiAddAmount(account->localCreditSum, amount);

  while(account->parent && !account->parent->dirty){
    account = account->parent;
    account->dirty = NA_TRUE;
  }
}



NA_HDEF NAString* naNewStringWithAmount(FIAmount amount, const FIFungible* fungible){
  NAi256 units = naDivi256(amount.decimals, fiAmountOne().decimals);
  NAi256 decimals = naModi256(amount.decimals, fiAmountOne().decimals);

  decimals = fiRoundFungiblei256(fungible, decimals, NA_FALSE);

  if(fiGetFungibleDecimals(fungible) == 0){
    return naNewStringWithFormat("%lld", naCasti256Toi32(units));
  }else{
    NAString* formatstring = naNewStringWithFormat("%%lld.%%0%dlld", (int)fiGetFungibleDecimals(fungible));
    NAString* retstring = naNewStringWithFormat(naGetStringUTF8Pointer(formatstring), naCasti256Toi64(units), naCasti256Toi64(decimals));
    naDelete(formatstring);
    return retstring;
  }
}



void fiPrintAccount(const FIAccount* account, NABool recursive){
  NAStackIterator iter;

  if(recursive){
    iter = naMakeStackAccessor(&(account->childs));
    while(naIterateStack(&iter)){
      const FIAccount* childAccount = naGetStackCurpConst(&iter);
      fiPrintAccount(childAccount, recursive);
    }
    naClearStackIterator(&iter);
  }

  printf("\n\n");

  const NAUTF8Char* namePtr = naGetStringUTF8Pointer(account->name);
  const NAUTF8Char* debitFungPtr = naGetStringUTF8Pointer(fiGetFungibleIdentifier(account->debitFungible));
  const NAUTF8Char* creditFungPtr = naGetStringUTF8Pointer(fiGetFungibleIdentifier(account->creditFungible));

  switch(fiGetAccountType(account)){
  case FIRMY_ACCOUNT_TYPE_MAIN_BOOK:
  case FIRMY_ACCOUNT_TYPE_BALANCE:
  case FIRMY_ACCOUNT_TYPE_ASSET:
  case FIRMY_ACCOUNT_TYPE_EXPENSE:
    printf("%s\t%s\t+\t-\n", namePtr, debitFungPtr);
    break;
  case FIRMY_ACCOUNT_TYPE_PROFITLOSS:
  case FIRMY_ACCOUNT_TYPE_LIABILITY:
  case FIRMY_ACCOUNT_TYPE_INCOME:
    printf("%s\t%s\t-\t+\n", namePtr, debitFungPtr);
    break;
  case FIRMY_ACCOUNT_TYPE_EXCHANGE:
    printf("Exchange\t\t%s\t%s\n", debitFungPtr, creditFungPtr);
    break;
  }
  
  FIAmount computeddebitsum = account->localDebitSum;
  FIAmount computedcreditsum = account->localCreditSum;
  
  iter = naMakeStackAccessor(&(account->childs));
  while(naIterateStack(&iter)){
    const FIAccount* childAccount = naGetStackCurpConst(&iter);
    
    FIAmount curdebitsum;
    FIAmount curcreditsum;
    FIAmount fungibledebitsum = fiAmountZero();
    FIAmount fungiblecreditsum = fiAmountZero();
    
    curdebitsum = fiGetAccountTotalDebitSum(childAccount);
    curcreditsum = fiGetAccountTotalCreditSum(childAccount);
    
    FIAmount rate = fiGetExchangeRate(fiGetAccountAnyFungible(childAccount), fiGetAccountAnyFungible(account));
    fungibledebitsum = curdebitsum;
    fungiblecreditsum = curcreditsum;
    curdebitsum = fiMulAmount(fungibledebitsum, rate);
    curcreditsum = fiMulAmount(fungiblecreditsum, rate);
    
    if(fiGreaterAmount(curdebitsum, curcreditsum)){
      curdebitsum = fiSubAmount(curdebitsum, curcreditsum);
      curcreditsum = fiAmountZero();
    }else{
      curcreditsum = fiSubAmount(curcreditsum, curdebitsum);
      curdebitsum = fiAmountZero();
    }
    
    computeddebitsum = fiAddAmount(computeddebitsum, curdebitsum);
    computedcreditsum = fiAddAmount(computedcreditsum, curcreditsum);
    
    NAString* debitstr;
    NAString* creditstr;
    NAString* fungibledebitstr;
    NAString* fungiblecreditstr;
    debitstr = naNewStringWithAmount(curdebitsum, fiGetAccountDebitFungible(childAccount));
    creditstr = naNewStringWithAmount(curcreditsum, fiGetAccountCreditFungible(childAccount));
    fungibledebitstr = naNewStringWithAmount(fungibledebitsum, fiGetAccountDebitFungible(childAccount));
    fungiblecreditstr = naNewStringWithAmount(fungiblecreditsum, fiGetAccountCreditFungible(childAccount));
    
    if(fiIsAmountZero(curdebitsum) && fiIsAmountZero(curcreditsum)){
      if((fiGetAccountType(childAccount) == FIRMY_ACCOUNT_TYPE_ASSET) || (fiGetAccountType(childAccount) == FIRMY_ACCOUNT_TYPE_EXPENSE)){
        naDelete(creditstr);
        creditstr = naNewString();
      }else{
        naDelete(debitstr);
        debitstr = naNewString();
      }
    }else{
      if(fiIsAmountZero(curdebitsum)){
        naDelete(debitstr);
        debitstr = naNewString();
      }
      if(fiIsAmountZero(curcreditsum)){
        naDelete(creditstr);
        creditstr = naNewString();
      }
      if(fiIsAmountZero(fungibledebitsum)){
        naDelete(fungibledebitstr);
        fungibledebitstr = naNewString();
      }
      if(fiIsAmountZero(fungiblecreditsum)){
        naDelete(fungiblecreditstr);
        fungiblecreditstr = naNewString();
      }
    }
    
    if(fiGetAccountDebitFungible(account) != fiGetAccountDebitFungible(childAccount)
      && fiGetAccountCreditFungible(account) != fiGetAccountCreditFungible(childAccount))
    {
      printf("%s\t%s\t%s\t%s\tCUR\t%s\t%s\n",
        "Unter-Konto Fremdwaehrung",
        naGetStringUTF8Pointer(fiGetAccountName(childAccount)),
        naGetStringUTF8Pointer(debitstr),
        naGetStringUTF8Pointer(creditstr),
        naGetStringUTF8Pointer(fungibledebitstr),
        naGetStringUTF8Pointer(fungiblecreditstr));
    }else{
      printf("%s\t%s\t%s\t%s\n",
        "Unter-Konto",
        naGetStringUTF8Pointer(fiGetAccountName(childAccount)),
        naGetStringUTF8Pointer(debitstr),
        naGetStringUTF8Pointer(creditstr));
    }
    
    naDelete(debitstr);
    naDelete(creditstr);
    naDelete(fungibledebitstr);
    naDelete(fungiblecreditstr);
  }
  naClearStackIterator(&iter);


  NAHeap relevantBookings;
  naInitHeap(&relevantBookings, -1, NA_HEAP_IS_MIN_HEAP | NA_HEAP_USES_DATETIME_KEY);

  NAStackIterator bookiter = naMakeStackAccessor(fiGetPeriodBookings());
  while(naIterateStack(&bookiter)){
    const FIBooking* booking = naGetStackCurpConst(&bookiter);
    if(fiGetBookingDebitAccount(booking) == account){
      naInsertHeapElementConst(&relevantBookings, booking, fiGetBookingDateTime(booking), NA_NULL);
    }else if(fiGetBookingCreditAccount(booking) == account){
      naInsertHeapElementConst(&relevantBookings, booking, fiGetBookingDateTime(booking), NA_NULL);
    }
  }
  naClearStackIterator(&bookiter);

  while(naGetHeapCount(&relevantBookings)){
    const FIBooking* booking = naRemoveHeapRootConst(&relevantBookings);
    NAString* amountString = NA_NULL;
    NAString* dateString = naNewStringWithDateTime(fiGetBookingDateTime(booking), NA_DATETIME_FORMAT_NATURAL);
    if(fiGetBookingDebitAccount(booking) == account){
      amountString = naNewStringWithAmount(*fiGetBookingAmount(booking), fiGetAccountDebitFungible(account));
      printf("%s\t%s\t%s\t%s\n",
        naGetStringUTF8Pointer(dateString),
        naGetStringUTF8Pointer(fiGetBookingText(booking)),
        naGetStringUTF8Pointer(amountString),
        "0");
    }else if(fiGetBookingCreditAccount(booking) == account){
      amountString = naNewStringWithAmount(*fiGetBookingAmount(booking), fiGetAccountCreditFungible(account));
      printf("%s\t%s\t%s\t%s\n",
        naGetStringUTF8Pointer(dateString),
        naGetStringUTF8Pointer(fiGetBookingText(booking)),
        "0",
        naGetStringUTF8Pointer(amountString));
    }
    if(amountString){naDelete(amountString);}
    naDelete(dateString);
  }

  naClearHeap(&relevantBookings);

  NAString* computeddebitsumstring;
  NAString* computedcreditsumstring;
  NAString* computeddiffstring;
  computeddebitsumstring = naNewStringWithAmount(computeddebitsum, fiGetAccountDebitFungible(account));
  computedcreditsumstring = naNewStringWithAmount(computedcreditsum, fiGetAccountCreditFungible(account));
  printf("\tSumme:\t%s\t%s\n", naGetStringUTF8Pointer(computeddebitsumstring), naGetStringUTF8Pointer(computedcreditsumstring));
  if(fiGreaterAmount(computeddebitsum, computedcreditsum)){
    computeddiffstring = naNewStringWithAmount(fiSubAmount(computeddebitsum, computedcreditsum), fiGetAccountDebitFungible(account));
    printf("\tSaldo:\t\t%s\n", naGetStringUTF8Pointer(computeddiffstring));
  }else{
    computeddiffstring = naNewStringWithAmount(fiSubAmount(computedcreditsum, computeddebitsum), fiGetAccountCreditFungible(account));
    printf("\tSaldo:\t%s\t\n", naGetStringUTF8Pointer(computeddiffstring));
  }
  naDelete(computeddebitsumstring);
  naDelete(computedcreditsumstring);
  naDelete(computeddiffstring);
}





