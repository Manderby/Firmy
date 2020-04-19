
#ifndef FIRMY_INTERNAL_DEFINED
#define FIRMY_INTERNAL_DEFINED


#define fiError naError

#include FIRMY_NALIB_PATH(NAStack.h)

struct FIAmount{
  int256 decimals;
};

// Unit
FIPeriod* fiGetCurrentPeriod(void);
NADateTime fiGetCurrentValueDate(void);
const NAString* fiGetCurrentDocument(void);
void fiCloseCurrentDocument(void);

// Period
FIPeriod* fiNewPeriod(const NAUTF8Char* name, NADateTime start, const FIFungible* mainFungible);
const FIFungible* fiGetPeriodMainFungible(void);
FIAccount* fiAddPeriodMainAccount(const NAString* identifier, const NAString* name, const NAUTF8Char* parentidentifier, FIAccountType type);
NADateTime fiGetPeriodStartDate(void);
void fiCopyPeriodAccountsFromPrevPeriod(const FIPeriod* prevperiod);
void fiRegisterMainAccounts(void);
NAStack* fiGetPeriodBookings(void);

// Fungible
FIFungible* fiNewFungible(const NAUTF8Char* name, const NAUTF8Char* identifier, NAInt decimals);
const NAString* fiGetFungibleIdentifier(const FIFungible* fungible);

// Account
// Registers a new account for the current period.
FIAccount* fiRegisterAccountWithType(const FIFungible* fungible, const NAUTF8Char* identifier, const NAUTF8Char* name, FIAccount* parentaccount, FIAccountType accounttype);
FIAccount* fiNewAccount(FIAccountType accounttype, const FIFungible* fungible, const NAString* newidentifier, const NAString* newname, FIAccount* newparent);
void fiCarryAccountOver(FIAccount* account, FIAmount amountDebit, FIAmount amountCredit);
void fiAddAccountChild(FIAccount* account, FIAccount* child);
const NAString* fiGetAccountIdentifier(const FIAccount* account);
const NAString* fiGetAccountName(const FIAccount* account);
FIAccountType fiGetAccountType(const FIAccount* account);
const FIFungible* fiGetAccountFungible(const FIAccount* account);
FIAmount fiGetAccountLocalDebitSum(const FIAccount* account);
FIAmount fiGetAccountLocalCreditSum(const FIAccount* account);
FIAmount fiGetAccountTotalDebitSum(const FIAccount* account);
FIAmount fiGetAccountTotalCreditSum(const FIAccount* account);
FIAccount* getAccountParent(const FIAccount* account);
void fiAddAccountDebitSum(FIAccount* account, FIAmount amount, NABool local);
void fiAddAccountCreditSum(FIAccount* account, FIAmount amount, NABool local);

// Booking
FIBooking* fiNewBooking(FIAmount amount, const FIAccount* accountdebit, const FIAccount* accountcredit, const NAUTF8Char* text);
void fiBookAmount(FIAmount amount, FIAccount* accountdebit, FIAccount* accountcredit, const NAUTF8Char* text);
const NADateTime* fiGetBookingDateTime      (const FIBooking* booking);
const NAString*   fiGetBookingText          (const FIBooking* booking);
const FIAmount*   fiGetBookingAmount        (const FIBooking* booking);
const FIAccount*  fiGetBookingDebitAccount  (const FIBooking* booking);
const FIAccount*  fiGetBookingCreditAccount (const FIBooking* booking);

// Amount
FIAmount fiAmount(double value);
NABool   fiEqualAmount(FIAmount amount, double cmpAmount);
NABool   fiSmallerAmount(FIAmount amount, FIAmount cmpAmount);
NABool   fiGreaterAmount(FIAmount amount, FIAmount cmpAmount);
FIAmount fiNegAmount(FIAmount amount);
FIAmount fiAddAmount(FIAmount amount1, FIAmount amount2);
FIAmount fiSubAmount(FIAmount amount1, FIAmount amount2);

#endif // FIRMY_INTERNAL_DEFINED
