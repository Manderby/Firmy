
#include "Firmy.h"
#include "FirmyInternal.h"

FIAmount fiAmountZero(){
  FIAmount amount;
  amount.decimals = NA_ZERO_i256;
  return amount;
}
FIAmount fiAmountOne(){
  FIAmount amount;
  // A 1 with 36 zeros.
  amount.decimals = naMakei256(NA_ZERO_i128, naMakeu128(naMakeu64(0x00c097ce, 0x7bc90715), naMakeu64(0xb34b9f10, NA_ZERO_u32)));
  return amount;
}
double fiAmountDoubleDecimalMultiplicand(){
  return naExp10d(15);  // 15 == DBL_DIG
}
NAi256 fiAmountDoubleRemainingMultiplicand(){
  // A 1 with (36 - 15) = 21 zeros.   15 == DBL_DIG
  return naMakei256(NA_ZERO_i128, naMakeu128(naMakeu64(NA_ZERO_u32, 0x00000036), naMakeu64(0x35c9adc5, 0xdea00000)));
}

FIAmount fiAmount(double value){
  // todo computation of negative numbers
  NAi64 integer = naGetDoubleInteger(value);
  NAi64 fraction = naGetDoubleFractionSlow(value);

  FIAmount amount;
  amount.decimals = naMuli256(naMakei256WithLo(naMakei128WithLo(integer)), fiAmountOne().decimals);
  NAi256 subDecimals = naMuli256(naMakei256WithLo(naMakei128WithLo(fraction)), fiAmountDoubleRemainingMultiplicand());
  amount.decimals = naAddi256(amount.decimals, subDecimals);

  return amount;
}

NABool fiIsAmountZero(FIAmount amount){
  return naEquali256(amount.decimals, NA_ZERO_i256);
}

NABool fiSmallerAmount(FIAmount amount, FIAmount cmpAmount){
  return naSmalleri256(amount.decimals, cmpAmount.decimals);
}

NABool fiGreaterAmount(FIAmount amount, FIAmount cmpAmount){
  return naGreateri256(amount.decimals, cmpAmount.decimals);
}

FIAmount fiNegAmount(FIAmount amount){
  FIAmount retamount;
  retamount.decimals = naNegi256(amount.decimals);
  return retamount;
}

FIAmount fiAddAmount(FIAmount amount1, FIAmount amount2){
  FIAmount retamount;
  retamount.decimals = naAddi256(amount1.decimals, amount2.decimals);
  return retamount;
}

FIAmount fiSubAmount(FIAmount amount1, FIAmount amount2){
  FIAmount retamount;
  retamount.decimals = naSubi256(amount1.decimals, amount2.decimals);
  return retamount;
}


// Assuming type of 2nd operand has 2 Decimal digits.
// Assuming type of 1st operand has 3 decimal digits.
// Assuming type of 1st operand can only accurately store 4 unit digits.
// Meaning: Any partial result MUST not exceed 7 digits.
// Except for the multiplication of the units which can in fact overflow.

// representation as floating point numbers:
// 123.456 * 9.87
// We multiply both operands such that they become integral.
// 123456 / 1000 * 987 / 100
// Rearrangement
// 123456 * 987 / 100 / 1000
// We split the 2nd operand into the unit and decimal parts
// 123456 * (900 + 87) / 100 / 1000
// Expand division by 100
// 123456 * (900 / 100 + 87 / 100) / 1000
// Shorten expresseion 900 / 100
// 123456 * (9 + 87 / 100) / 1000
// Expand multiplication with 123456
// (123456 * 9 + 123456 * 87 / 100) / 1000
// 123456 * 87 could overflow, so split 123456 into unit and decimal parts
// (123456 * 9 + (123400 + 56) * 87 / 100) / 1000
// replace 123400 with 1234 * 100
// (123456 * 9 + (1234 * 100 + 56) * 87 / 100) / 1000
// Expand multiplication with (87 / 100)
// (123456 * 9 + (1234 * 100 * 87 / 100 + 56 * 87 / 100)) / 1000
// Simplify 100 / 100
// (123456 * 9 + (1234 * 87 + (56 * 87) / 100)) / 1000
// Add 50 for rounding in stead of flooring of the last decimal digit
// (123456 * 9 + (1234 * 87 + (56 * 87 + 50) / 100)) / 1000
// And there we are. This last line is the one actually implemented below.
//
// This will compute as follows:
// (123456 * 9 + (107358 + (4872 + 50) / 100)) / 1000
// (123456 * 9 + (107358 + 4922 / 100)) / 1000
// (1111104 + (107358 + 49)) / 1000      last two digits lost
// (1111104 + 107407) / 1000
// 1218511 / 1000
// 1218.511
//
// Desired result when computed directly with floating point:
// 1218.51072
// 1218.511    round to 3 decimal digits

// Multiplies v1 with v2 but assumes v2 is a fixed point number. The number of fractional
// digits of v2 is given by the fractionBase. For example fractionBase = 1000 for 3 decimals.
NAi256 naMulInt256WithFixedDecimals(NAi256 v1, NAi256 v2, NAi256 fractionBase, NABool roundLastDecimal){
  // todo computation of negative numbers
  NAi256 v1Hi = naDivi256(v1, fractionBase);
  NAi256 v1Lo = naModi256(v1, fractionBase);
  NAi256 v2Hi = naDivi256(v2, fractionBase);
  NAi256 v2Lo = naModi256(v2, fractionBase);

  NAi256 lolo = naMuli256(v1Lo, v2Lo);
  if(roundLastDecimal){
    #if NA_SIGNED_INTEGER_ENCODING != NA_SIGNED_INTEGER_ENCODING_TWOS_COMPLEMENT
      naError("Using shift operation for division by two while not using 2s complement");
    #endif
    NAi256 rounder = naShri256(fractionBase, 1);
    lolo = naAddi256(lolo, rounder);
  }
  lolo = naDivi256(lolo, fractionBase);

  NAi256 hilo = naMuli256(v1Hi, v2Lo);
  NAi256 lo = naAddi256(hilo, lolo);

  NAi256 hihi = naMuli256(v1, v2Hi);
  return naAddi256(hihi, lo);
}

FIAmount fiMulAmount(FIAmount amount1, FIAmount factor){
  FIAmount retamount;
  retamount.decimals = naMulInt256WithFixedDecimals(amount1.decimals, factor.decimals, fiAmountOne().decimals, NA_FALSE);
  return retamount;
}

