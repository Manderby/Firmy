
#ifndef FIRMY_INTERNAL_DEFINED
#define FIRMY_INTERNAL_DEFINED


#define fiError naError

#include FIRMY_NALIB_PATH(NAStack.h)

typedef struct FIAmount FIAmount; 
struct FIAmount{
  // A decimal fixpoint with 36 decimal digits.
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
FIAccount* fiAddPeriodMainAccount(const NAString* identifier, const NAString* name, const NAUTF8Char* parentidentifier, FIaccountType type);
NADateTime fiGetPeriodStartDate(void);
void fiCopyPeriodAccountsFromPrevPeriod(const FIPeriod* prevperiod);
void fiRegisterMainAccounts(void);
NAStack* fiGetPeriodBookings(void);

// Fungible
FIFungible* fiNewFungible(const NAUTF8Char* name, const NAUTF8Char* identifier, size_t decimals);
const NAString* fiGetFungibleIdentifier(const FIFungible* fungible);
size_t fiGetFungibleDecimals(const FIFungible* fungible);
NAi256 fiRoundFungiblei256(const FIFungible* fungible, NAi256 amount, NABool backToUnit);
FIAccount* fiGetExchangeAccount(const FIFungible* fromFungible, const FIFungible* toFungible);
FIAmount fiGetExchangeRate(const FIFungible* fromFungible, const FIFungible* toFungible);

// Account
// Registers a new account for the current period.
FIAccount* fiRegisterAccountWithType(const FIFungible* fungible, const NAUTF8Char* identifier, const NAUTF8Char* name, FIAccount* parentaccount, FIaccountType accountType);
FIAccount* fiNewAccount(FIaccountType accountType, const FIFungible* fungible, const NAString* newIdentifier, const NAString* newName, FIAccount* newParent);
FIAccount* fiNewExchangeAccount(const FIFungible* debitFungible, const FIFungible* creditFungible);
void fiCarryAccountOver(FIAccount* account, FIAmount amountDebit, FIAmount amountCredit);
void fiAddAccountChild(FIAccount* account, FIAccount* child);
const NAString* fiGetAccountIdentifier(const FIAccount* account);
const NAString* fiGetAccountName(const FIAccount* account);
FIaccountType fiGetAccountType(const FIAccount* account);
const FIFungible* fiGetAccountAnyFungible(const FIAccount* account);
const FIFungible* fiGetAccountDebitFungible(const FIAccount* account);
const FIFungible* fiGetAccountCreditFungible(const FIAccount* account);
FIAmount fiGetAccountExchangeRate(const FIAccount* account);
void fiSetAccountExchangeRate(FIAccount* account, double rate);
FIAmount fiGetAccountLocalDebitSum(const FIAccount* account);
FIAmount fiGetAccountLocalCreditSum(const FIAccount* account);
FIAmount fiGetAccountTotalDebitSum(const FIAccount* account);
FIAmount fiGetAccountTotalCreditSum(const FIAccount* account);
FIAccount* getAccountParent(const FIAccount* account);
void fiAddAccountDebitSum(FIAccount* account, FIAmount amount);
void fiAddAccountCreditSum(FIAccount* account, FIAmount amount);

// Booking
FIBooking* fiNewBooking(FIAmount amount, const FIAccount* accountdebit, const FIAccount* accountcredit, const NAUTF8Char* text);
void fiBookAmount(FIAmount amount, FIAccount* accountdebit, FIAccount* accountcredit, const NAUTF8Char* text);
void fiExchAmount(FIAmount amountDebit, FIAmount amountCredit, FIAccount* accountdebit, FIAccount* accountcredit, const NAUTF8Char* text);
const NADateTime* fiGetBookingDateTime      (const FIBooking* booking);
const NAString*   fiGetBookingText          (const FIBooking* booking);
const FIAmount*   fiGetBookingAmount        (const FIBooking* booking);
const FIAccount*  fiGetBookingDebitAccount  (const FIBooking* booking);
const FIAccount*  fiGetBookingCreditAccount (const FIBooking* booking);

// Amount
FIAmount fiAmountZero(void);
FIAmount fiAmountOne(void);
FIAmount fiAmount(double value);
FIAmount fiAmountWithDecimals(NAi256 decimals);
NABool   fiIsAmountZero(FIAmount amount);
NABool   fiSmallerAmount(FIAmount amount, FIAmount cmpAmount);
NABool   fiGreaterAmount(FIAmount amount, FIAmount cmpAmount);
FIAmount fiNegAmount(FIAmount amount);
FIAmount fiAddAmount(FIAmount amount1, FIAmount amount2);
FIAmount fiSubAmount(FIAmount amount1, FIAmount amount2);
FIAmount fiMulAmount(FIAmount amount1, FIAmount factor);

#endif // FIRMY_INTERNAL_DEFINED
