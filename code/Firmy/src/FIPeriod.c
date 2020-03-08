
#include "Firmy.h"
#include "FirmyInternal.h"
#include FIRMY_NALIB_PATH(NAStack.h)
#include FIRMY_NALIB_PATH(NAString.h)


struct FIPeriod{
  NAString* name;
  NADateTime start;
  const FIFungible* mainFungible;
  NAStack accounts;
  NAStack bookings;
};


void fiDestructPeriod(FIPeriod* period);
NA_RUNTIME_TYPE(FIPeriod, fiDestructPeriod, NA_FALSE);



FIPeriod* fiNewPeriod(
  const NAUTF8Char* name,
  NADateTime start,
  const FIFungible* mainFungible)
{  
  #ifndef NDEBUG
    if(!mainFungible)
      fiError("Given fungible is null.");
  #endif
  FIPeriod* period = naNew(FIPeriod);
  period->name = naNewStringWithFormat(name);
  period->start = start;
  period->mainFungible = mainFungible;
  naInitStack(&(period->accounts), sizeof(FIAccount*), 2);
  naInitStack(&(period->bookings), sizeof(FIBooking*), 2);
  return period;
}


void fiDestructPeriod(FIPeriod* period){
  naDelete(period->name);
  naForeachStackpMutable(&(period->accounts), naDelete);
  naClearStack(&(period->accounts));
  naForeachStackpMutable(&(period->bookings), naDelete);
  naClearStack(&(period->bookings));
}




void fiRegisterMainAccounts(void){
  NAString* idstr;
  NAString* namestr;
  
  idstr = naNewStringWithUTF8CStringLiteral(FIRMY_MAIN_BOOK_IDENTIFIER);
  namestr = naNewStringWithUTF8CStringLiteral("Hauptbuch");
  fiAddPeriodMainAccount(idstr, namestr, FIRMY_MAIN_BOOK_IDENTIFIER, FIRMY_ACCOUNT_TYPE_MAIN_BOOK);
  naDelete(idstr);
  naDelete(namestr);

  idstr = naNewStringWithUTF8CStringLiteral(FIRMY_BALANCE_IDENTIFIER);
  namestr = naNewStringWithUTF8CStringLiteral("Bilanz");
  fiAddPeriodMainAccount(idstr, namestr, FIRMY_MAIN_BOOK_IDENTIFIER, FIRMY_ACCOUNT_TYPE_BALANCE);
  naDelete(idstr);
  naDelete(namestr);

  idstr = naNewStringWithUTF8CStringLiteral(FIRMY_PROFITLOSS_IDENTIFIER);
  namestr = naNewStringWithUTF8CStringLiteral("Erfolgsrechnung");
  fiAddPeriodMainAccount(idstr, namestr, FIRMY_MAIN_BOOK_IDENTIFIER, FIRMY_ACCOUNT_TYPE_PROFITLOSS);
  naDelete(idstr);
  naDelete(namestr);

  idstr = naNewStringWithUTF8CStringLiteral(FIRMY_ASSET_IDENTIFIER);
  namestr = naNewStringWithUTF8CStringLiteral("Aktiven");
  fiAddPeriodMainAccount(idstr, namestr, FIRMY_BALANCE_IDENTIFIER, FIRMY_ACCOUNT_TYPE_ASSET);
  naDelete(idstr);
  naDelete(namestr);

  idstr = naNewStringWithUTF8CStringLiteral(FIRMY_LIABILITY_IDENTIFIER);
  namestr = naNewStringWithUTF8CStringLiteral("Passiven");
  fiAddPeriodMainAccount(idstr, namestr, FIRMY_BALANCE_IDENTIFIER, FIRMY_ACCOUNT_TYPE_LIABILITY);
  naDelete(idstr);
  naDelete(namestr);

  idstr = naNewStringWithUTF8CStringLiteral(FIRMY_EXPENSE_IDENTIFIER);
  namestr = naNewStringWithUTF8CStringLiteral("Aufwand");
  fiAddPeriodMainAccount(idstr, namestr, FIRMY_PROFITLOSS_IDENTIFIER, FIRMY_ACCOUNT_TYPE_EXPENSE);
  naDelete(idstr);
  naDelete(namestr);

  idstr = naNewStringWithUTF8CStringLiteral(FIRMY_INCOME_IDENTIFIER);
  namestr = naNewStringWithUTF8CStringLiteral("Ertrag");
  fiAddPeriodMainAccount(idstr, namestr, FIRMY_PROFITLOSS_IDENTIFIER, FIRMY_ACCOUNT_TYPE_INCOME);
  naDelete(idstr);
  naDelete(namestr);

}



FIAccount* fiAddPeriodMainAccount(const NAString* identifier, const NAString* name, const NAUTF8Char* parentidentifier, FIaccountType type){
  #ifndef NDEBUG
    if(!fiGetCurrentPeriod())
      fiError("No current period available");
  #endif
  
  FIAccount* parent;
  if(naEqualStringToUTF8CString(identifier, FIRMY_MAIN_BOOK_IDENTIFIER, NA_TRUE)){
    parent = NULL;
  }else{
    parent = fiGetAccount(parentidentifier);
  }

  FIAccount** newaccount = naPushStack(&(fiGetCurrentPeriod()->accounts));
  *newaccount = fiNewAccount(type, fiGetPeriodMainFungible(), identifier, name, parent);
  #ifndef NDEBUG
  if(!*newaccount)
    naError("new account returned null");
  #endif
  
  return *newaccount;
}



void fiCopyPeriodAccountsFromPrevPeriod(const FIPeriod* prevperiod){
  #ifndef NDEBUG
    if(!fiGetCurrentPeriod())
      fiError("No current period available");
    if(fiGetCurrentDocument())
      fiError("There is a current document set but there should never be any document before carrying over.");
  #endif
  NAStackIterator iter = naMakeStackAccessor(&(prevperiod->accounts));
  fiDocument(fiGetPeriodStartDate(), naGetStringUTF8Pointer(fiGetPeriodName()));
  while(naIterateStack(&iter)){
    const FIAccount* oldaccount = naGetStackCurpConst(&iter);
    FIAmount localDebitSum = fiGetAccountlocalDebitSum(oldaccount);
    FIAmount localCreditSum = fiGetAccountlocalCreditSum(oldaccount);
    FIAmount totalDebitSum = fiGetAccounttotalDebitSum(oldaccount);
    FIAmount totalCreditSum = fiGetAccounttotalCreditSum(oldaccount);
    if((fiGetaccountType(oldaccount) == FIRMY_ACCOUNT_TYPE_MAIN_BOOK) || (totalDebitSum != 0.) || (totalCreditSum != 0.)){
      if(getAccountParent(oldaccount)){
        FIAccount* newaccount = fiRegisterAccountWithType(
          fiGetAccountAnyFungible(oldaccount),
          naGetStringUTF8Pointer(fiGetAccountIdentifier(oldaccount)),
          naGetStringUTF8Pointer(fiGetAccountName(oldaccount)),
          fiGetAccount(naGetStringUTF8Pointer(fiGetAccountIdentifier(getAccountParent(oldaccount)))),
          fiGetaccountType(oldaccount));

        naGetStringUTF8Pointer(fiGetAccountIdentifier(newaccount));
        
        
        if((fiGetaccountType(oldaccount) == FIRMY_ACCOUNT_TYPE_ASSET) || (fiGetaccountType(oldaccount) == FIRMY_ACCOUNT_TYPE_LIABILITY)){
          if((localDebitSum != 0.) || (localCreditSum != 0.)){
            if(localDebitSum > localCreditSum){
              fiCarryAccountOver(newaccount, localDebitSum - localCreditSum, 0);
            }else{
              fiCarryAccountOver(newaccount, 0, localCreditSum - localDebitSum);
            }
          }
        }
      }else{
        NAString* idstr;
        NAString* namestr;
        idstr = naNewStringWithUTF8CStringLiteral(FIRMY_MAIN_BOOK_IDENTIFIER);
        namestr = naNewStringWithUTF8CStringLiteral("Hauptbuch");
        fiAddPeriodMainAccount(idstr, namestr, FIRMY_MAIN_BOOK_IDENTIFIER, FIRMY_ACCOUNT_TYPE_MAIN_BOOK);
        naDelete(idstr);
        naDelete(namestr);
      }
    }
  }
  naClearStackIterator(&iter);
  fiCloseCurrentDocument();
}



FIAccount* fiRegisterAccount(
  const FIFungible* fungible,
  const NAUTF8Char* identifier,
  const NAUTF8Char* name,
  FIAccount* parentaccount)
{
  return fiRegisterAccountWithType(fungible, identifier, name, parentaccount, FIRMY_ACCOUNT_TYPE_MAIN_BOOK);
}

FIAccount* fiRegisterAccountWithType(
  const FIFungible* fungible,
  const NAUTF8Char* identifier,
  const NAUTF8Char* name,
  FIAccount* parentaccount,
  FIaccountType accountType)
{
  #ifndef NDEBUG
    if(!fiGetCurrentPeriod())
      fiError("No current period defined.");
  #endif
  NAStackIterator iter = naMakeStackAccessor(&(fiGetCurrentPeriod()->accounts));
  while(naIterateStack(&iter)){
    const FIAccount* account = naGetStackCurpConst(&iter);
    if(naEqualStringToUTF8CString(fiGetAccountIdentifier(account), identifier, NA_TRUE)){
      printf("Account identifier %s already present.", (const char*)identifier);
      exit(1);
    }
  }
  naClearStackIterator(&iter);
  

  FIAccount** newaccount = naPushStack(&(fiGetCurrentPeriod()->accounts));
  
  NAString* idstr;
  NAString* namestr;
  idstr = naNewStringWithFormat(identifier);
  namestr = naNewStringWithFormat(name);
  
  if(naEqualStringToUTF8CString(idstr, FIRMY_MAIN_BOOK_IDENTIFIER, NA_TRUE)){
    *newaccount = fiNewAccount(FIRMY_ACCOUNT_TYPE_MAIN_BOOK, fungible, idstr, namestr, NULL);
  }else{
    FIaccountType desiredtype = accountType;
    if(accountType == FIRMY_ACCOUNT_TYPE_MAIN_BOOK){
      desiredtype = fiGetaccountType(parentaccount);
    }
    *newaccount = fiNewAccount(desiredtype, fungible, idstr, namestr, parentaccount);
  }
  
  #ifndef NDEBUG
  if(!*newaccount)
    naError("new account returned null");
  #endif

  naDelete(idstr);
  naDelete(namestr);
  
  return *newaccount;
}



void fiBook(FIAmount amount, FIAccount* accountdebit, FIAccount* accountcredit, const NAUTF8Char* text){
  #ifndef NDEBUG
    if(accountdebit && accountcredit && fiGetAccountDebitFungible(accountdebit) != fiGetAccountCreditFungible(accountcredit))
      fiError("Accounts do not have the same fungible. Use fiBookEx.");
  #endif

  if(accountdebit){fiAddAccountDebitSum(accountdebit, amount, NA_TRUE);}
  if(accountcredit){fiAddAccountCreditSum(accountcredit, amount, NA_TRUE);}

  FIBooking** newbooking = (FIBooking**)naPushStack(fiGetPeriodBookings());
  *newbooking = fiNewBooking(amount, accountdebit, accountcredit, text);
}



void fiExch(FIAmount amount, double bookrate, FIAccount* accountdebit, FIAccount* accountcredit, const NAUTF8Char* text){
  #ifndef NDEBUG
    if(fiGetAccountDebitFungible(accountdebit) == fiGetAccountCreditFungible(accountcredit))
      fiError("Accounts have the same fungible. Use fiBook.");
    if(!accountdebit)
      fiError("Debit account is null.");
    if(!accountcredit)
      fiError("Credit account is null.");
  #endif

  FIAccount* exac = fiGetExchangeAccount(fiGetAccountCreditFungible(accountdebit), fiGetAccountDebitFungible(accountcredit));

  fiBook(amount, accountdebit, exac, text);
  fiBook(amount * naInv(bookrate), exac, accountcredit, text);
}



const NAString* fiGetPeriodName(void){
  #ifndef NDEBUG
    if(!fiGetCurrentPeriod())
      fiError("No current period defined");
  #endif
  return fiGetCurrentPeriod()->name;
}



const FIFungible* fiGetPeriodMainFungible(void){
  #ifndef NDEBUG
    if(!fiGetCurrentPeriod())
      fiError("No current period defined");
  #endif
  return fiGetCurrentPeriod()->mainFungible;
}



NAStack* fiGetPeriodBookings(void){
  #ifndef NDEBUG
    if(!fiGetCurrentPeriod())
      fiError("No current period defined");
  #endif
  return &(fiGetCurrentPeriod()->bookings);
}



FIAccount* fiGetAccount(const NAUTF8Char* identifier){
  #ifndef NDEBUG
    if(!fiGetCurrentPeriod())
      naError("No current period set");
  #endif
  FIAccount* foundaccount = NA_NULL;
  NAStackIterator iter = naMakeStackMutator(&(fiGetCurrentPeriod()->accounts));
  while(naIterateStack(&iter)){
    FIAccount* account = naGetStackCurpMutable(&iter);
    #ifndef NDEBUG
      if(!account)
        naError("account during iteration is null");
    #endif
    const NAString* tmpstr = fiGetAccountIdentifier(account);
    naGetStringUTF8Pointer(tmpstr);
    if(naEqualStringToUTF8CString(fiGetAccountIdentifier(account), identifier, NA_TRUE)){foundaccount = account; break;}
  }
  naClearStackIterator(&iter);

  if(!foundaccount){
    printf("Account %s not found", identifier);
    exit(1);
  }
  return foundaccount;
}



NADateTime fiGetPeriodStartDate(void){
  #ifndef NDEBUG
    if(!fiGetCurrentPeriod())
      fiError("No current period set.");
  #endif
  return fiGetCurrentPeriod()->start;
}



