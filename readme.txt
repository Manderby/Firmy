The software package "Firmy" is a small accounting implementation,
providing a basic interface for "double book-keeping".

It is published with the unilicense and hence is public domain.

Currently, the master contains a beta version.
Future release versions will be available as tags.



Cloning with submodules:
------------------------

Firmy requires "NALib", a submodule provided by Tobias Stamm.

1. Either clone Firmy with the following git command:
   git clone --recurse-submodules https://XXXXXX/Firmy

2. Or write the following commands:
   git clone https://XXXXXX/Firmy
   cd Firmy
   git submodule init
   git submodule update

3. Open the provided project/solution file in the Firmy/proj folder.
   It should compile and run without errors in XCode and VisualStudio.



Documentation:
--------------
- Can be found directly within the Firmy.h header file.
- See the provided examples for examples.



Contribute:
-----------
This project is open for people to contribute.



History:
--------

Firmy originally was created in 2011 by Tobias Stamm at the time he
founded his company Manderim GmbH. It served to do all financial
calculations concerning the company for the following 8 years.

Within those years, the code was enhanced and rewritten several times
and was translated from C++ to C. In 2019, the company did close down and
the code had been rewritten for a public domain release.
