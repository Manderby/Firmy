
#ifndef FIRMY_INTERNAL_DEFINED
#define FIRMY_INTERNAL_DEFINED


#define fiError naError

#include FIRMY_NALIB_PATH(NAStack.h)

// Unit
FIPeriod* fiGetCurrentPeriod(void);
NADateTime fiGetCurrentValueDate(void);
const NAString* fiGetCurrentDocument(void);
void fiCloseCurrentDocument(void);

// Period
FIPeriod* fiNewPeriod(const NAUTF8Char* name, NADateTime start, const FIFungible* mainFungible);
const FIFungible* fiGetPeriodMainFungible(void);
FIAccount* fiAddPeriodMainAccount(const NAString* identifier, const NAString* name, const NAUTF8Char* parentidentifier, FIaccountType type);
NADateTime fiGetPeriodStartDate(void);
void fiCopyPeriodAccountsFromPrevPeriod(const FIPeriod* prevperiod);
void fiRegisterMainAccounts(void);
NAStack* fiGetPeriodBookings(void);

// Fungible
FIFungible* fiNewFungible(const NAUTF8Char* name, const NAUTF8Char* identifier, NAInt decimals);
const NAString* fiGetFungibleIdentifier(const FIFungible* fungible);
FIAccount* fiGetExchangeAccount(const FIFungible* fromFungible, const FIFungible* toFungible);
double fiGetExchangeRate(const FIFungible* fromFungible, const FIFungible* toFungible);

// Account
// Registers a new account for the current period.
FIAccount* fiRegisterAccountWithType(const FIFungible* fungible, const NAUTF8Char* identifier, const NAUTF8Char* name, FIAccount* parentaccount, FIaccountType accountType);
FIAccount* fiNewAccount(FIaccountType accountType, const FIFungible* fungible, const NAString* newIdentifier, const NAString* newName, FIAccount* newParent);
FIAccount* fiNewExchangeAccount(const FIFungible* debitFungible, const FIFungible* creditFungible);
void fiCarryAccountOver(FIAccount* account, FIAmount amountDebit, FIAmount amountCredit);
void fiAddAccountChild(FIAccount* account, FIAccount* child);
const NAString* fiGetAccountIdentifier(const FIAccount* account);
const NAString* fiGetAccountName(const FIAccount* account);
FIaccountType fiGetaccountType(const FIAccount* account);
const FIFungible* fiGetAccountAnyFungible(const FIAccount* account);
const FIFungible* fiGetAccountDebitFungible(const FIAccount* account);
const FIFungible* fiGetAccountCreditFungible(const FIAccount* account);
double fiGetAccountExchangeRate(const FIAccount* account);
void fiSetAccountExchangeRate(FIAccount* account, double rate);
FIAmount fiGetAccountlocalDebitSum(const FIAccount* account);
FIAmount fiGetAccountlocalCreditSum(const FIAccount* account);
FIAmount fiGetAccounttotalDebitSum(const FIAccount* account);
FIAmount fiGetAccounttotalCreditSum(const FIAccount* account);
FIAccount* getAccountParent(const FIAccount* account);
void fiAddAccountDebitSum(FIAccount* account, FIAmount amount, NABool local);
void fiAddAccountCreditSum(FIAccount* account, FIAmount amount, NABool local);

// Booking
FIBooking* fiNewBooking(FIAmount amount, const FIAccount* accountdebit, const FIAccount* accountcredit, const NAUTF8Char* text);
const NADateTime* fiGetBookingDateTime      (const FIBooking* booking);
const NAString*   fiGetBookingText          (const FIBooking* booking);
const FIAmount*   fiGetBookingAmount        (const FIBooking* booking);
const FIAccount*  fiGetBookingDebitAccount  (const FIBooking* booking);
const FIAccount*  fiGetBookingCreditAccount (const FIBooking* booking);


#endif // FIRMY_INTERNAL_DEFINED
