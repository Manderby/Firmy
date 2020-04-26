
#include "Firmy.h"
#include "FirmyInternal.h"

//#include FIRMY_NALIB_PATH(NAString.h)

FIAmount fiAmountZero(){
  FIAmount amount;
  amount.decimals = NA_ZERO_256;
  return amount;
}

FIAmount fiAmount(double value, const FIFungible* fungible){
  FIAmount amount;
  int i;
  int256 eighteen = naMakeInt256WithDouble(1.);
  for(i = 0; i < 18 - fiGetFungibleDecimals(fungible); i++){
    eighteen = naMulInt256(eighteen, naMakeInt256WithDouble(10.));
  }

  value *= pow(10., (double)fiGetFungibleDecimals(fungible));
  amount.decimals = naMulInt256(naMakeInt256WithDouble(value), eighteen);
  return amount;
}

NABool fiIsAmountZero(FIAmount amount){
  return naEqualInt256(amount.decimals, NA_ZERO_256);
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


// 4 Unit, 3 Decimal = 7 Digits in total
// 123.456 * 9.87
// 123456 / 1000 * 987 / 100
// 123456 * 987 / 1000 / 100
// 123456 * (900 + 87) / 1000 / 100
// 123456 * (900 / 100 + 87 / 100) / 1000
// 123456 * (9 + 87 / 100) / 1000
// (123456 * 9 + 123456 * 87 / 100) / 1000
// (123456 * 9 + (123400 + 56) * 87 / 100) / 1000
// (123456 * 9 + (1234 * 100 + 56) * 87 / 100) / 1000
// (123456 * 9 + (1234 * 100 * 87 / 100 + 56 * 87 / 100)) / 1000
// (123456 * 9 + (1234 * 87 + (56 * 87) / 100)) / 1000 <--------------
// (123456 * 9 + (107358 + 4872 / 100)) / 1000
// (1111104 + (107358 + 48.72)) / 1000
// (1111104 + (107358 + 48.72)) / 1000
// (1111104 + 107406.72) / 1000
// 1218510.72 / 1000
// 1218.51072
// Desired result:
// 1218.51072

NAInt256 naMulInt256WithFixedDecimals(NAInt256 v1, NAInt256 v2, NAInt256 decimalBase){
  NAInt256 v1Hi = naDivInt256(v1, decimalBase);
  NAInt256 v1Lo = naModInt256(v1, decimalBase);
  NAInt256 v2Hi = naDivInt256(v2, decimalBase);
  NAInt256 v2Lo = naModInt256(v2, decimalBase);
  NAInt256 rounder = naShrInt256(decimalBase, 1); // todo: make this a true division by 2

  NAInt256 lolo = naMulInt256(v1Lo, v2Lo);
  lolo = naDivInt256(lolo, decimalBase);

  NAInt256 hilo = naMulInt256(v1Hi, v2Lo);
  NAInt256 lo = naAddInt256(hilo, lolo);

  NAInt256 hihi = naMulInt256(v1, v2Hi);
  return naAddInt256(hihi, lo);
}

FIAmount fiMulAmount(FIAmount amount1, double factor){
  FIAmount retamount;

  int i;
  int256 eighteen = naMakeInt256WithDouble(1.);
  for(i = 0; i < 18; i++){
    eighteen = naMulInt256(eighteen, naMakeInt256WithDouble(10.));
  }

  //NAInt256 decimals2 = naMakeInt256WithDouble(factor * 100.);
  //decimals2 = naMulInt256(decimals2, eighteen);
  //decimals2 = naDivInt256(decimals2, naMakeInt256WithDouble(100.));

  NAInt256 decimals2 = naMakeInt256(NA_ZERO_128, naMakeUInt128(NA_ZERO_64, 0x88f948b4dd1b0000));

  retamount.decimals = naMulInt256WithFixedDecimals(amount1.decimals, decimals2, eighteen, NA_FALSE);
  return retamount;
}

