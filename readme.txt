The software package "Firmy" is a small accounting implementation, providing
a basic interface for "double book-keeping".

It is published with the unilicense and hence is public domain.

Currently, the master contains a beta version.
Future release versions will be available as tags.



Installation:
-------------
Firmy requires "NALib", a software package provided by Tobias Stamm.

1. Download (or clone) the Firmy package onto your computer.
2. Download (or clone) the NALib package onto your computer in the same
   parent folder you downloaded the Firmy package into. Your folder
   structure should look something like this:

   ParentFolder --+-- Firmy
                  |-- NALib

3. Open the provided project/solution file in the Firmy/proj folder.
4. It should compile and run without errors in XCode and VisualStudio.

5. In case you choose to have a different folder structure, make sure to
   include the NALib source and/or project files manually with the
   correct path.
   The easiest way though is to adjust the macro FIRMY_NALIB_PATH on top
   of the Firmy.h header file.



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
