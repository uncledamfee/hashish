hashish
===
_under development_

Creates hash sums of executables found in directory
  
---
  
To Build
--------
You will need a C compiler, the standard C library with header files, and 
GNU Make. On Ubuntu/Debian, you can install these with
  ```
  $ sudo apt-get install build-essential
  ```
After that, build with
  ```
  $ sudo make
  ```
To Install
----------
If you want to install this program, run
  ```
  $ make install
  ```
Then, 
  ```
  $ make clean
  ```
Usage
-----
hashish creates a file with hash sums of the found binary files located in
the working directory
 
#### License
-------
hashish is free software. You are free to use, copy, modify, and redistribute it
under the terms of the GPLv3.
