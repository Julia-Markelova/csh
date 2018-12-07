# csh
## Simple shell implementation with _yacc_, _lex_ and _C_

* Builtin commands:
  ```cd```, ```export```, ```exit```, ```help```, ```history```.
* Set variables
 (example):
 ```
 > a = b
 > export a
 > echo $a
 > b 
 ```
 
* Pipe support
* IO redirection only with file names
* Print errors
* Save 5 recent commands, to see print 'history' or !n, where n [1;5].
