

#include "Firmy.h"
#include "FirmyInternal.h"



struct FIBooking{
  NADateTime datetime;
  NAString* text;
  FIAmount amount;
  const FIAccount* accountdebit;
  const FIAccount* accountcredit;
  NAString* doc;
};



void fiDestructBooking(FIBooking* booking);
NA_RUNTIME_TYPE(FIBooking, fiDestructBooking, NA_FALSE);




FIBooking* fiNewBooking(
  FIAmount amount,
  const FIAccount* accountdebit,
  const FIAccount* accountcredit,
  const NAUTF8Char* text)
{
  #ifndef NDEBUG
    if(!fiGetCurrentDocument())
      fiError("No document set.");
  #endif
  FIBooking* booking = naNew(FIBooking);
  booking->datetime = fiGetCurrentValueDate();
  booking->text = naNewStringWithFormat(text);
  booking->amount = amount;
  booking->accountdebit = accountdebit;
  booking->accountcredit = accountcredit;
  booking->doc = naNewStringExtraction(fiGetCurrentDocument(), 0, -1);
  return booking;
}



void fiDestructBooking(FIBooking* booking){
  naDelete(booking->text);
  naDelete(booking->doc);
}



const NADateTime* fiGetBookingDateTime      (const FIBooking* booking){return &(booking->datetime);}
const NAString*   fiGetBookingText          (const FIBooking* booking){return booking->text;}
const FIAmount*   fiGetBookingAmount        (const FIBooking* booking){return &(booking->amount);}
const FIAccount*  fiGetBookingDebitAccount  (const FIBooking* booking){return booking->accountdebit;}
const FIAccount*  fiGetBookingCreditAccount (const FIBooking* booking){return booking->accountcredit;}
const NAString*   fiGetBookingDocument      (const FIBooking* booking){return booking->doc;}




