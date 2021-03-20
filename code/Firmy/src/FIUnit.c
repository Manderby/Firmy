
#include "Firmy.h"
#include "FirmyInternal.h"

#include FIRMY_NALIB_PATH(NAString.h)


struct FIUnit{
  NAString* name;
  NAStack periods;
  NAStack fungibles;

  FIPeriod* curPeriod;
  NADateTime curValueDate;  // only valid when curDoc is not null
  NAString* curDoc;
};


// The global unit currently running.
FIUnit* fi_unit = NA_NULL;



void fiDestructUnit(FIUnit* unit);
NA_RUNTIME_TYPE(FIUnit, fiDestructUnit, NA_FALSE);



void fiStart(const NAUTF8Char* name){
  FIUnit* unit = naNew(FIUnit);
  unit->name = naNewStringWithFormat(name);
  naInitStack(&(unit->periods), sizeof(FIPeriod*), 0, 0);
  naInitStack(&(unit->fungibles), sizeof(FIFungible*), 0, 0);
  unit->curPeriod = NA_NULL;
  unit->curDoc = NA_NULL;
  fi_unit = unit;
}



void fiStop(void){
  fiCloseCurrentDocument();
  naDelete(fi_unit);
}



void fiDestructUnit(FIUnit* unit){
  naForeachStackpMutable(&(unit->periods), naDelete);
  naClearStack(&(unit->periods));
  naForeachStackpMutable(&(unit->fungibles), naDelete);
  naClearStack(&(unit->fungibles));
  naDelete(unit->name);
}



FIPeriod* fiRegisterPeriod(
  const NAUTF8Char* name,
  const FIPeriod* prevperiod,
  NADateTime start,
  const FIFungible* mainFungible)
{
  #ifndef NDEBUG
    if(!mainFungible)
      fiError("Given fungible is null.");
  #endif
  FIPeriod** newperiod = naPushStack(&(fi_unit->periods));
  *newperiod = fiNewPeriod(name, start, mainFungible);
  fi_unit->curPeriod = *newperiod;
  fiCloseCurrentDocument();
  
  if(prevperiod){
    fiCopyPeriodAccountsFromPrevPeriod(prevperiod);
  }else{
    fiRegisterMainAccounts();
  }
  
  return *newperiod;
}



const FIFungible* fiRegisterFungible(
  const NAUTF8Char* name,
  const NAUTF8Char* identifier,
  size_t digits)
{
  #ifndef NDEBUG
    if(fiGetFungible(identifier))
      fiError("Fungible with given identifier has already been registered.");
  #endif
  FIFungible** newfungible = naPushStack(&(fi_unit->fungibles));
  *newfungible = fiNewFungible(name, identifier, digits);
  return *newfungible;
}



const FIFungible* fiGetFungible(const NAUTF8Char* identifier){
  NAStackIterator iter = naMakeStackAccessor(&(fi_unit->fungibles));
  const FIFungible* fungible = NA_NULL;
  while(naIterateStack(&iter)){
    const FIFungible* curfungible = naGetStackCurpConst(&iter);
    if(naEqualStringToUTF8CString(fiGetFungibleIdentifier(curfungible), identifier, NA_TRUE)){
      fungible = curfungible;
      break;
    }
  }
  naClearStackIterator(&iter);
  return fungible;
}



void fiCloseCurrentDocument(void){
  if(fi_unit->curDoc){
    naDelete(fi_unit->curDoc);
    fi_unit->curDoc = NA_NULL;
  }
}



void fiDocument(NADateTime valueDate, const NAUTF8Char* docString){
  fiCloseCurrentDocument();
  fi_unit->curValueDate = valueDate;
  fi_unit->curDoc = naNewStringWithFormat(docString);
}



FIPeriod* fiGetCurrentPeriod(void){
  return fi_unit->curPeriod;
}



const NAString* fiGetCurrentDocument(void){
  return fi_unit->curDoc;
}



NADateTime fiGetCurrentValueDate(void){
  #ifndef NDEBUG
    if(!fi_unit->curDoc)
      fiError("Value date invalid as current document undefined.");
  #endif
  return fi_unit->curValueDate;
}
