
#define FIRMYTEST_STRINGIFY(A) #A
#define FIRMYTEST_NALIB_PATH(file) FIRMYTEST_STRINGIFY(../../../../NALib/code/NALib/src/file)
#define FIRMYTEST_FIRMY_PATH(file) FIRMYTEST_STRINGIFY(../../Firmy/src/file)

#include FIRMYTEST_FIRMY_PATH(Firmy.h)



FIPeriod* createYear1(FIPeriod* prevperiod){
  const FIFungible* chf = fiGetFungible("CHF");

  FIPeriod* y2020 = fiRegisterPeriod(
    "Year of the rat",
    prevperiod,
    naMakeDateTime(2020, 2, 5, 0, 0, 0),
    chf);

  FIAccount* hauptbuch = fiGetAccount(FIRMY_MAIN_BOOK_IDENTIFIER);

  FIAccount* aktiven = fiGetAccount(FIRMY_ASSET_IDENTIFIER);
  FIAccount* umlauf10 = fiRegisterAccount(chf, "umlauf", "Umlaufverfmoegen (10)", aktiven); 
  FIAccount* fluessig100 = fiRegisterAccount(chf, "fluessig", "Fluessigmittel (100)", umlauf10); 
  FIAccount* bankkonto1020 = fiRegisterAccount(chf, "bankkonto", "Raiffeisen Konto (1020)", fluessig100); 
  FIAccount* forderungen110 = fiRegisterAccount(chf, "forderungen", "Forderungen (110)", umlauf10);
  FIAccount* debitorenkunden1100 = fiRegisterAccount(chf, "debitorenkunden", "Debitoren Kunden (1100)", forderungen110);

  FIAccount* passiven = fiGetAccount(FIRMY_LIABILITY_IDENTIFIER);
  FIAccount* fremdkapitalkurz20 = fiRegisterAccount(chf, "fremdkapitalkurz", "Fremdkapital kurzfristig (20)", passiven);
  FIAccount* sonstverb220 = fiRegisterAccount(chf, "sonstverb", "Sonstige kurzfristige Finanzverbindlichkeiten (220)", fremdkapitalkurz20);
  FIAccount* mehrwertsteuer2200 = fiRegisterAccount(chf, "mehrwertsteuer", "Kreditor Mehrwertsteuer (2200)", sonstverb220);
  FIAccount* passabgrenz230 = fiRegisterAccount(chf, "passabgrenz", "Passiven Rechnungsabgrenzung", fremdkapitalkurz20);

  FIAccount* transpass2300 = fiRegisterAccount(chf, "transpass", "Transitorische Passiven", passabgrenz230);
  FIAccount* eigenkap28 = fiRegisterAccount(chf, "eigenkap", "Eigenkapital (28)", passiven); 
  FIAccount* grundkapital280 = fiRegisterAccount(chf, "grundkapital", "Grundkapital (280)", eigenkap28); 
  FIAccount* aktkapital2800 = fiRegisterAccount(chf, "aktkapital", "Aktienkapital (2800)", grundkapital280); 
  FIAccount* zuwachskapital290 = fiRegisterAccount(chf, "zuwachskapital", "Zuwachskapital (290)", eigenkap28);
  FIAccount* gewinnvortrag2990 = fiRegisterAccount(chf, "gewinnvortrag", "Gewinnvortrag (2990)", zuwachskapital290);

  FIAccount* ertrag = fiGetAccount(FIRMY_INCOME_IDENTIFIER);
  FIAccount* ertragwaren320 = fiRegisterAccount(chf, "ertragwaren", "Warenertrag (320)", ertrag);
  FIAccount* ertragwarenonline3200 = fiRegisterAccount(chf, "ertragwarenonline", "Online-Verkauf (3200)", ertragwaren320);

  FIAccount* aufwand = fiGetAccount(FIRMY_EXPENSE_IDENTIFIER);
  FIAccount* aufwandpersonal50 = fiRegisterAccount(chf, "aufwandpersonal", "Personalaufwand", aufwand);
  FIAccount* aufwandpersonaldienst540 = fiRegisterAccount(chf, "aufwandpersonaldienst", "Personalaufwand Dienstleistungen", aufwandpersonal50);
  FIAccount* aufwandlohn5400 = fiRegisterAccount(chf, "aufwandlohn", "Lohnaufwand (Dienstleistungen)", aufwandpersonaldienst540);
  FIAccount* aufwandpersonalsozdienst5470 = fiRegisterAccount(chf, "aufwandpersonalsozdienst", "Sozialleistungen (Dienstleistungen)", aufwandpersonaldienst540);



  // Gruendungsbeitrag
  fiDocument(naMakeDateTime(2020, 2, 5, 21, 46, 24), "Ordner 2020 Januar Firmenbudget, RefNr 1");
  fiBook(100000.00, bankkonto1020, aktkapital2800, "Gruendungsbeitrag");

  // Lohn
  fiDocument(naMakeDateTime(2020, 2, 6, 0, 0, 0), "Lohnauszug Manderby Februar");
  fiBook(546.21, aufwandpersonalsozdienst5470, aufwandlohn5400, "Lohn Feb Manderby AHV/IV/EO");
  fiBook(121.53, aufwandpersonalsozdienst5470, aufwandlohn5400, "Lohn Feb Manderby ALV");
  fiBook(63.62, aufwandpersonalsozdienst5470, aufwandlohn5400, "Lohn Feb Manderby FAK");
  fiBook(22.81, aufwandpersonalsozdienst5470, aufwandlohn5400, "Lohn Feb Manderby SVA");
  fiBook(60.06, aufwandpersonalsozdienst5470, aufwandlohn5400, "Lohn Feb Manderby NBU");
  fiBook(5.88, aufwandpersonalsozdienst5470, aufwandlohn5400, "Lohn Feb Manderby BU");
  fiBook(430.26, aufwandpersonalsozdienst5470, aufwandlohn5400, "Lohn Feb Manderby BVG");
  fiBook(4852.55, aufwandlohn5400, bankkonto1020, "Lohn Feb Manderby");

  // Verkaufsaufstellung / Rechnungsstellung
  fiDocument(naMakeDateTime(2020, 3, 13, 0, 0, 0), "docs/2020/Mar/AppStoreVolume.txt");
  fiBook(24.53, debitorenkunden1100, ertragwarenonline3200, "Online-Verkauf MacAppStore EU, EUR 20.44, BuchKurs 1.2");
  fiBook(12.08, debitorenkunden1100, ertragwarenonline3200, "Online-Verkauf MacAppStore GB, GBP 8.05, BuchKurs 1.5");
  fiBook(385.26, debitorenkunden1100, ertragwarenonline3200, "Online-Verkauf MacAppStore JP, JPY 42807, BuchKurs 0.009");
  fiBook(570.54, debitorenkunden1100, ertragwarenonline3200, "Online-Verkauf MacAppStore US, USD 633.93, BuchKurs 0.9");
  fiBook(25.77, debitorenkunden1100, ertragwarenonline3200, "Online-Verkauf MacAppStore CH");
  fiBook(1.68, ertragwarenonline3200, mehrwertsteuer2200, "WMST Abzug Online-Verkauf MacAppStore CH");

  // Waehrungskorrektur
  fiDocument(naMakeDateTime(2020, 4, 3, 0, 0, 0), "docs/2020/Mar/AppStoreAbrechnung.pdf");
  fiBook(21.26, debitorenkunden1100, ertragwarenonline3200, "Einnahmen MacAppStore EUR 20.44, TagKurs 1.04029");
  fiBook(24.53, ertragwarenonline3200, debitorenkunden1100, "Währungsstorno vom 13.3.2020, EUR 20.44, BuchKurs 1.2");
  fiBook(11.42, debitorenkunden1100, ertragwarenonline3200, "Einnahmen MacAppStore GBP 8.05, TagKurs 1.41924");
  fiBook(12.08, ertragwarenonline3200, debitorenkunden1100, "Währungsstorno vom 13.3.2020, GBP 8.05, BuchKurs 1.5");
  fiBook(344.60, debitorenkunden1100, ertragwarenonline3200, "Einnahmen MacAppStore JPY 42807, TagKurs 0.00805");
  fiBook(385.26, ertragwarenonline3200, debitorenkunden1100, "Währungsstorno vom 13.3.2020, JPY 42807, BuchKurs 0.009");
  fiBook(607.50, debitorenkunden1100, ertragwarenonline3200, "Einnahmen MacAppStore USD 633.93, TagKurs 0.95831");
  fiBook(570.54, ertragwarenonline3200, debitorenkunden1100, "Währungsstorno vom 13.3.2020, USD 633.93, BuchKurs 0.9");
  // Tatsaechliche Auszahlung aufs Bankkonto
  fiBook(1010.55, bankkonto1020, debitorenkunden1100, "Einnahmen MacAppStore");

  // Transitorische Passiven
  fiDocument(naMakeDateTime(2020, 12, 31, 0, 0, 0), "Email vom 2021-01-13 Abgrenzung.txt");
  fiBook(3406.50, aufwandlohn5400, transpass2300, "Abgrenzung SVA Rechnung 2020");

  // Jahresergebnis
  fiDocument(naMakeDateTime(2020, 12, 31, 0, 0, 0), "docs/2021/Jan/Jahresabschluss.doc");
  fiBook(7250.17, gewinnvortrag2990, hauptbuch, "Umbuchung Jahresergebnis 2020");

  fiDocument(naMakeDateTime(2020, 4, 3, 0, 0, 0), "docs/2020/Mar/AppStoreAbrechnung.pdf");
  fiBook(11.42, debitorenkunden1100, hauptbuch, "XXXXXXXXXXXXXXXXXXXXXXXXXXX");

  return y2020;
}



FIPeriod* createYear2(FIPeriod* prevperiod){

  const FIFungible* chf = fiGetFungible("CHF");

  FIPeriod* y2021 = fiRegisterPeriod(
    "Year of the Manderby",
    prevperiod,
    naMakeDateTime(2021, 1, 1, 0, 0, 0),
    chf);

  FIAccount* transpass2300 = fiGetAccount("transpass");
  FIAccount* aufwandlohn5400 = fiGetAccount("aufwandlohn");

  fiBook(3406.50, transpass2300, aufwandlohn5400, "Rueckbuchung SVA Rechnung 2020");

  return y2021;
}



int main(int argc, const char** argv){
  NA_UNUSED(argc);
  NA_UNUSED(argv);

  naStartRuntime();
  fiStart("Manderprise AG");

  fiRegisterFungible(
    "Schweizer Franken",
    "CHF",
    2);

  /*FIPeriod* year1 = */createYear1(NA_NULL);
//  createYear2(year1);
  fiPrintAccount(fiGetAccount(FIRMY_MAIN_BOOK_IDENTIFIER), NA_TRUE);

  fiStop();
  naStopRuntime();

  printf("Programm ended.\n");
  getc(stdin);
  return 0;
}
