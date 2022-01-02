
#ifndef FIRMY_COMMON_DEFINED
#define FIRMY_COMMON_DEFINED

#define FIRMY_STRINGIFY(A) #A
#define FIRMY_NALIB_PATH(file) FIRMY_STRINGIFY(../../../lib/NALib/code/NALib/src/file)

#define FIRMY_VERSION 0

typedef struct FIPeriod     FIPeriod;
typedef struct FIUnit       FIUnit;
typedef struct FIFungible   FIFungible;
typedef struct FIBooking    FIBooking;
typedef struct FIAccount    FIAccount;
typedef struct FIExAccount  FIExAccount;

typedef enum{
  FIRMY_ACCOUNT_TYPE_MAIN_BOOK = 0,
  FIRMY_ACCOUNT_TYPE_BALANCE,
  FIRMY_ACCOUNT_TYPE_PROFITLOSS,
  FIRMY_ACCOUNT_TYPE_ASSET,
  FIRMY_ACCOUNT_TYPE_LIABILITY,
  FIRMY_ACCOUNT_TYPE_EXPENSE,
  FIRMY_ACCOUNT_TYPE_INCOME,
  FIRMY_ACCOUNT_TYPE_EXCHANGE,
} FIaccountType;

#define FIRMY_MAIN_BOOK_IDENTIFIER  "mainbook"
#define FIRMY_BALANCE_IDENTIFIER    "balance"
#define FIRMY_PROFITLOSS_IDENTIFIER "profitloss"
#define FIRMY_ASSET_IDENTIFIER      "asset"
#define FIRMY_LIABILITY_IDENTIFIER  "liability"
#define FIRMY_EXPENSE_IDENTIFIER    "expense"
#define FIRMY_INCOME_IDENTIFIER     "income"



#include FIRMY_NALIB_PATH(NADateTime.h)


typedef enum{
  FI_AMOUNT_BUYS_RATIO,   // Debit: value1             Credit: value1 / value2
  FI_RATIO_BUYS_AMOUNT,   // Debit: value2 * value1    Credit: value2
  FI_AMOUNT_SELLS_RATIO,  // Debit: value1             Credit: value1 * value2
  FI_RATIO_SELLS_AMOUNT,  // Debit: value2 / value1    Credit: value2
  FI_AMOUNT_BUYS_AMOUNT,  // Debit: value1             Credit: value2
  FI_AMOUNT_SELLS_AMOUNT, // Debit: value2             Credit: value1
} FIExchangeType;

// Starts and stops a business unit (Company, Enterprise, Household)
void fiStart(const NAUTF8Char* name);
void fiStop(void);

// Registers a fungible (Currency, stock item, fonds)
const FIFungible* fiRegisterFungible(
  const NAUTF8Char* name,
  const NAUTF8Char* identifier,
  size_t digits);

// Rounds the given amount to the number of decimals defined by the fungible.
double fiRoundFungibleAmount(const FIFungible* fungible, double amount);

// Searches for an already registered fungible and returns it.
const FIFungible* fiGetFungible(const NAUTF8Char* identifier);

// Registers a new financial period. After that, the period is selected to be
// the current period. No current document is selected after that.
//
// If prevperiod is Null, the default accounts (see FIaccountType) will be
// automatically created. If prevperiod is not Null, all accounts of the
// previous period which have any sum greater than zero will be created for
// the new period and the saldo of the previous period will be carried over. 
FIPeriod* fiRegisterPeriod(
  const NAUTF8Char* name,
  const FIPeriod* prevperiod,
  NADateTime start,
  const FIFungible* mainFungible);

// Returns the name of the current period.
const NAString* fiGetPeriodName(void);

// Registers a new account for the current period.
FIAccount* fiRegisterAccount(
  const FIFungible* fungible,
  const NAUTF8Char* identifier,
  const NAUTF8Char* name,
  FIAccount* parentaccount);

// Returns an account of the current period.
FIAccount* fiGetAccount(const NAUTF8Char* identifier);

// Marks the given string to be the new document currently being accounted.
void fiDocument(NADateTime valueDate, const NAUTF8Char* docString);

// Adds a booking to the current document.
void fiBook(
  double amount,
  FIAccount* accountdebit,
  FIAccount* accountcredit,
  const NAUTF8Char* text);

void fiExchange(
  double amountSrc,
  FIAccount* accountSrc,
  FIAccount* accountSrcCurrency,
  FIAccount* accountDstCurrency,
  FIAccount* accountDst,
  double amountDst,
  const NAUTF8Char* text);

// Adds an exchange booking to the current document.
void fiExch(
  FIExchangeType exchangeType,
  double amount,
  double bookrate,
  FIAccount* accountdebit,
  FIAccount* accountcredit,
  const NAUTF8Char* text);

void fiSetExchangeRate(
  const FIFungible* fromFungible,
  const FIFungible* toFungible,
  double rate);

// Outputs a txt representation of the double bookkeeping.
void fiPrintAccount(const FIAccount* account, NABool recursive);

#endif // FIRMY_COMMON_DEFINED
