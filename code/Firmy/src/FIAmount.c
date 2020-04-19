
#include "Firmy.h"
#include "FirmyInternal.h"

//#include FIRMY_NALIB_PATH(NAString.h)

FIAmount fiAmount(double value){
  int digits = 2;
  FIAmount amount;
  int i;
  int256 eighteen = naMakeInt256WithDouble(1.);
  for(i = 0; i < 18 - digits; i++){
    eighteen = naMulInt256(eighteen, naMakeInt256WithDouble(10.));
  }

  value *= pow(10., (double)digits);
  amount.decimals = naMulInt256(naMakeInt256WithDouble(value), eighteen);
  return amount;
}

NABool fiEqualAmount(FIAmount amount, double cmpAmount){
  return naEqualInt256(amount.decimals, fiAmount(cmpAmount).decimals);
}

NABool fiSmallerAmount(FIAmount amount, FIAmount cmpAmount){
  return naSmallerInt256(amount.decimals, cmpAmount.decimals);
}

NABool fiGreaterAmount(FIAmount amount, FIAmount cmpAmount){
  return naGreaterInt256(amount.decimals, cmpAmount.decimals);
}

FIAmount fiNegAmount(FIAmount amount){
  FIAmount retamount;
  retamount.decimals = naNegInt256(amount.decimals);
  return retamount;
}

FIAmount fiAddAmount(FIAmount amount1, FIAmount amount2){
  FIAmount retamount;
  retamount.decimals = naAddInt256(amount1.decimals, amount2.decimals);
  return retamount;
}

FIAmount fiSubAmount(FIAmount amount1, FIAmount amount2){
  FIAmount retamount;
  retamount.decimals = naSubInt256(amount1.decimals, amount2.decimals);
  return retamount;
}

FIAmount fiMulAmount(FIAmount amount1, double factor){
  FIAmount retamount;
  NAInt256 decimals2 = naMakeInt256WithDouble(factor);
  retamount.decimals = naMulInt256(amount1.decimals, decimals2);
  return retamount;
}

