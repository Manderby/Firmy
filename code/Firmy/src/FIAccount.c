
#include "Firmy.h"
#include "FirmyInternal.h"
#include FIRMY_NALIB_PATH(NAStack.h)
#include FIRMY_NALIB_PATH(NAHeap.h)


struct FIAccount{
  FIaccountType type;
  const FIFungible* debitFungible;
  const FIFungible* creditFungible;
  double exchangeRage;
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
  account->exchangeRage = 1.;
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
  account->totalDebitSum  = 0;
  account->totalCreditSum = 0;
  account->localDebitSum  = 0;
  account->localCreditSum = 0;
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
  account->exchangeRage = 1.;
  account->identifier = naNewStringWithUTF8CStringLiteral("exchangeAccount");
  account->name = naNewStringWithUTF8CStringLiteral("Exchange Account");
  account->parent = NA_NULL;
  naInitStack(&(account->childs), naSizeof(FIAccount*), 2);
  account->totalDebitSum  = 0;
  account->totalCreditSum = 0;
  account->localDebitSum  = 0;
  account->localCreditSum = 0;
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
  if(amountDebit != 0){
    fiBook(amountDebit, account, NA_NULL, text);
  }else{
    fiBook(amountCredit, NA_NULL, account, text);
  }
}



const NAString* fiGetAccountIdentifier(const FIAccount* account){
  return account->identifier;
}
const NAString* fiGetAccountName(const FIAccount* account){
  return account->name;
}
FIaccountType fiGetaccountType(const FIAccount* account){
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
double fiGetAccountExchangeRate(const FIAccount* account){
  return account->exchangeRage;
}
void fiSetAccountExchangeRate(FIAccount* account, double rate){
  account->exchangeRage = rate;
}
FIAmount fiGetAccountlocalDebitSum(const FIAccount* account){
  return account->localDebitSum;
}
FIAmount fiGetAccountlocalCreditSum(const FIAccount* account){
  return account->localCreditSum;
}
FIAmount fiGetAccounttotalDebitSum(const FIAccount* account){
  return account->totalDebitSum;
}
FIAmount fiGetAccounttotalCreditSum(const FIAccount* account){
  return account->totalCreditSum;
}
FIAccount* getAccountParent(const FIAccount* account){
  return account->parent;
}



void fiAddAccountDebitSum(FIAccount* account, FIAmount amount, NABool local){
  #ifndef NDEBUG
    if((amount < 0.) && (account->totalDebitSum < -amount))
      fiError("amount is negative");
  #endif
  
  if(local){account->localDebitSum = account->localDebitSum + amount;}
  account->totalDebitSum = account->totalDebitSum + amount;
  
  if(account->parent){
    double rate = fiGetExchangeRate(fiGetAccountAnyFungible(account), fiGetAccountAnyFungible(account->parent));
    fiAddAccountDebitSum(account->parent, amount * rate, NA_FALSE);
  }
}



void fiAddAccountCreditSum(FIAccount* account, FIAmount amount, NABool local){
  #ifndef NDEBUG
    if((amount < 0.) && (account->totalCreditSum < -amount))
      fiError("amount is negative");
  #endif
  
  if(local){account->localCreditSum = account->localCreditSum + amount;}
  account->totalCreditSum = account->totalCreditSum + amount;

  if(account->parent){
    double rate = fiGetExchangeRate(fiGetAccountAnyFungible(account), fiGetAccountAnyFungible(account->parent));
    fiAddAccountCreditSum(account->parent, amount * rate, NA_FALSE);
  }
}



NA_HDEF NAString* naNewStringWithAmount(FIAmount amount){
  int digits = 2;
  int64 decfacor = naMakeInt64WithDouble(pow(10., (double)digits));
  int64 units = (int64)(amount * decfacor) / decfacor;
  int64 decimals = (int64)(amount * decfacor) % decfacor;
  if(digits == 0){
    return naNewStringWithFormat("%lld", units);
  }else{
    NAString* formatstring = naNewStringWithFormat("%%lld.%%0%dlld", (int)digits);
    NAString* retstring = naNewStringWithFormat(naGetStringUTF8Pointer(formatstring), units, decimals);
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

  switch(fiGetaccountType(account)){
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
    FIAmount fungibledebitsum = 0.;
    FIAmount fungiblecreditsum = 0.;
    
    curdebitsum = fiGetAccounttotalDebitSum(childAccount);
    curcreditsum = fiGetAccounttotalCreditSum(childAccount);
    
    double rate = fiGetExchangeRate(fiGetAccountAnyFungible(childAccount), fiGetAccountAnyFungible(account));
    fungibledebitsum = curdebitsum;
    fungiblecreditsum = curcreditsum;
    curdebitsum = fungibledebitsum * rate;
    curcreditsum = fungiblecreditsum * rate;
    
    if(curdebitsum > curcreditsum){
      curdebitsum -= curcreditsum;
      curcreditsum = 0.;
    }else{
      curcreditsum -= curdebitsum;
      curdebitsum = 0.;
    }
    
    computeddebitsum = computeddebitsum + curdebitsum;
    computedcreditsum = computedcreditsum + curcreditsum;
    
    NAString* debitstr;
    NAString* creditstr;
    NAString* fungibledebitstr;
    NAString* fungiblecreditstr;
    debitstr = naNewStringWithAmount(curdebitsum);
    creditstr = naNewStringWithAmount(curcreditsum);
    fungibledebitstr = naNewStringWithAmount(fungibledebitsum);
    fungiblecreditstr = naNewStringWithAmount(fungiblecreditsum);
    
    if((curdebitsum == 0.) && (curcreditsum == 0.)){
      if((fiGetaccountType(childAccount) == FIRMY_ACCOUNT_TYPE_ASSET) || (fiGetaccountType(childAccount) == FIRMY_ACCOUNT_TYPE_EXPENSE)){
        naDelete(creditstr);
        creditstr = naNewString();
      }else{
        naDelete(debitstr);
        debitstr = naNewString();
      }
    }else{
      if(curdebitsum == 0.){
        naDelete(debitstr);
        debitstr = naNewString();
      }
      if(curcreditsum == 0.){
        naDelete(creditstr);
        creditstr = naNewString();
      }
      if(fungibledebitsum == 0.){
        naDelete(fungibledebitstr);
        fungibledebitstr = naNewString();
      }
      if(fungiblecreditsum == 0.){
        naDelete(fungiblecreditstr);
        fungiblecreditstr = naNewString();
      }
    }
    
    if(rate != 1.)
    {
      printf("%s\t%s\t%s\t%s\tCUR\t%s\t%s\n",
        "Unter-Konto",
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
    NAString* dateString = naNewStringWithDateTime(fiGetBookingDateTime(booking), NA_DATETIME_FORMAT_NATURAL);
    NAString* amountString = naNewStringWithAmount(*fiGetBookingAmount(booking));
    if(fiGetBookingDebitAccount(booking) == account){
      printf("%s\t%s\t%s\t%s\n",
        naGetStringUTF8Pointer(dateString),
        naGetStringUTF8Pointer(fiGetBookingText(booking)),
        naGetStringUTF8Pointer(amountString),
        "0");
    }else if(fiGetBookingCreditAccount(booking) == account){
      printf("%s\t%s\t%s\t%s\n",
        naGetStringUTF8Pointer(dateString),
        naGetStringUTF8Pointer(fiGetBookingText(booking)),
        "0",
        naGetStringUTF8Pointer(amountString));
    }
    naDelete(amountString);
    naDelete(dateString);
  }

  naClearHeap(&relevantBookings);

  NAString* computeddebitsumstring;
  NAString* computedcreditsumstring;
  NAString* computeddiffstring;
  computeddebitsumstring = naNewStringWithAmount(computeddebitsum);
  computedcreditsumstring = naNewStringWithAmount(computedcreditsum);
  printf("\tSumme:\t%s\t%s\n", naGetStringUTF8Pointer(computeddebitsumstring), naGetStringUTF8Pointer(computedcreditsumstring));
  if(computeddebitsum > computedcreditsum){
    computeddiffstring = naNewStringWithAmount(computeddebitsum - computedcreditsum);
    printf("\tSaldo:\t\t%s\n", naGetStringUTF8Pointer(computeddiffstring));
  }else{
    computeddiffstring = naNewStringWithAmount(computedcreditsum - computeddebitsum);
    printf("\tSaldo:\t%s\t\n", naGetStringUTF8Pointer(computeddiffstring));
  }
  naDelete(computeddebitsumstring);
  naDelete(computedcreditsumstring);
  naDelete(computeddiffstring);
}





