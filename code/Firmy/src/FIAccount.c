
#include "Firmy.h"
#include "FirmyInternal.h"
#include FIRMY_NALIB_PATH(NAStack.h)
#include FIRMY_NALIB_PATH(NAHeap.h)


struct FIAccount{
  FIAccountType type;
  const FIFungible* fungible;
  NAString* identifier;
  NAString* name;
  FIAccount* parent;
  NAStack childs;
  FIAmount totaldebitsum;
  FIAmount totalcreditsum;
  FIAmount localdebitsum;
  FIAmount localcreditsum;
};



void fiDestructAccount(FIAccount* account);
NA_RUNTIME_TYPE(FIAccount, fiDestructAccount, NA_FALSE);



FIAccount* fiNewAccount(
  FIAccountType accounttype,
  const FIFungible* fungible,
  const NAString* newidentifier,
  const NAString* newname,
  FIAccount* newparent)
{
  FIAccount* account = naNew(FIAccount);
  account->type = accounttype;
  account->fungible = fungible;
  account->identifier = naNewStringExtraction(newidentifier, 0, -1);
  account->name = naNewStringExtraction(newname, 0, -1);
  account->parent = newparent;
  naInitStack(&(account->childs), naSizeof(FIAccount*), 2);
  account->totaldebitsum  = 0;
  account->totalcreditsum = 0;
  account->localdebitsum  = 0;
  account->localcreditsum = 0;
  if(account->parent){
    fiAddAccountChild(account->parent, account);
  }
  return account;
}


void fiDestructAccount(FIAccount* account){
  naDelete(account->identifier);
  naDelete(account->name);
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
FIAccountType fiGetAccountType(const FIAccount* account){
  return account->type;
}
const FIFungible* fiGetAccountFungible(const FIAccount* account){
  return account->fungible;
}
FIAmount fiGetAccountLocalDebitSum(const FIAccount* account){
  return account->localdebitsum;
}
FIAmount fiGetAccountLocalCreditSum(const FIAccount* account){
  return account->localcreditsum;
}
FIAmount fiGetAccountTotalDebitSum(const FIAccount* account){
  return account->totaldebitsum;
}
FIAmount fiGetAccountTotalCreditSum(const FIAccount* account){
  return account->totalcreditsum;
}
FIAccount* getAccountParent(const FIAccount* account){
  return account->parent;
}



void fiAddAccountDebitSum(FIAccount* account, FIAmount amount, NABool local){
  if(account->type != FIRMY_ACCOUNT_TYPE_MAIN_BOOK){
    fiAddAccountDebitSum(account->parent, -account->totaldebitsum, NA_FALSE);
    fiAddAccountCreditSum(account->parent, -account->totalcreditsum, NA_FALSE);
  }
  if((amount < 0.) && (account->totaldebitsum < -amount)){
    printf("amount is negative");
  }
  account->totaldebitsum = account->totaldebitsum + amount;
  if(local){account->localdebitsum = account->localdebitsum + amount;}
  if(account->type != FIRMY_ACCOUNT_TYPE_MAIN_BOOK){
    fiAddAccountDebitSum(account->parent, account->totaldebitsum, NA_FALSE);
    fiAddAccountCreditSum(account->parent, account->totalcreditsum, NA_FALSE);
  }
}


void fiAddAccountCreditSum(FIAccount* account, FIAmount amount, NABool local){
  if(account->type != FIRMY_ACCOUNT_TYPE_MAIN_BOOK){
    fiAddAccountDebitSum(account->parent, -account->totaldebitsum, NA_FALSE);
    fiAddAccountCreditSum(account->parent, -account->totalcreditsum, NA_FALSE);
  }
  if((amount < 0.) && (account->totalcreditsum < -amount)){
    printf("amount is negative");
  }
  account->totalcreditsum = account->totalcreditsum + amount;
  if(local){account->localcreditsum = account->localcreditsum + amount;}
  if(account->type != FIRMY_ACCOUNT_TYPE_MAIN_BOOK){
    fiAddAccountDebitSum(account->parent, account->totaldebitsum, NA_FALSE);
    fiAddAccountCreditSum(account->parent, account->totalcreditsum, NA_FALSE);
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

  const NAUTF8Char* nameptr = naGetStringUTF8Pointer(account->name);

  switch(fiGetAccountType(account)){
  case FIRMY_ACCOUNT_TYPE_MAIN_BOOK:
  case FIRMY_ACCOUNT_TYPE_BALANCE:
  case FIRMY_ACCOUNT_TYPE_ASSET:
  case FIRMY_ACCOUNT_TYPE_EXPENSE:
    printf("%s\t\t+\t-\n", nameptr);
    break;
  case FIRMY_ACCOUNT_TYPE_PROFITLOSS:
  case FIRMY_ACCOUNT_TYPE_LIABILITY:
  case FIRMY_ACCOUNT_TYPE_INCOME:
    printf("%s\t\t-\t+\n", nameptr);
    break;
  }
  
  FIAmount computeddebitsum = account->localdebitsum;
  FIAmount computedcreditsum = account->localcreditsum;
  
  iter = naMakeStackAccessor(&(account->childs));
  while(naIterateStack(&iter)){
    const FIAccount* childAccount = naGetStackCurpConst(&iter);
      
    FIAmount curdebitsum;
    FIAmount curcreditsum;
    
    curdebitsum = fiGetAccountTotalDebitSum(childAccount);
    curcreditsum = fiGetAccountTotalCreditSum(childAccount);
    
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
    debitstr = naNewStringWithAmount(curdebitsum);
    creditstr = naNewStringWithAmount(curcreditsum);
    
    if((curdebitsum == 0.) && (curcreditsum == 0.)){
      if((fiGetAccountType(childAccount) == FIRMY_ACCOUNT_TYPE_ASSET) || (fiGetAccountType(childAccount) == FIRMY_ACCOUNT_TYPE_EXPENSE)){
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
    }
    
    printf("%s\t%s\t%s\t%s\n",
      "Unter-Konto",
      naGetStringUTF8Pointer(fiGetAccountName(childAccount)),
      naGetStringUTF8Pointer(debitstr),
      naGetStringUTF8Pointer(creditstr));
    
    naDelete(debitstr);
    naDelete(creditstr);
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





