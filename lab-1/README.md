## Summary
The purpose of lab 1 is to duplicate the functionality of the Linux/UNIX "grep" command. grep is a command that searches one or more files for lines that contain a given string/pattern, and prints those lines out. This program only duplicates a subset of the grep command functionality. The program also takes in options such as ignore case and line number.

## Program Guidelines:
1. It does not need to accept any options (extra credit option)
 * the option "-i" or "--ignore-case" ignores the the casing of the search string, meaning that it searches for both upper and lower-case variations
 * the option "-n" or "--line-number" adds the line that the match was found on to the output
2. It will be used for a string, not a regular expression
3. It only needs to handle one file argument.

## Syntax

**mygrep STRING \[FILE\] \[OPTION(s)\]** (the file argument is optional).

If no argument is given, the program will read from standard input.

## Examples
Syntactically valid calls to mygrep:
* mygrep printf lab1.c
* mygrep "printf (" lab1.c
* cat lab1.c | mygrep printf
* mygrep testing (read from stdin, use ctrl-d to terminate input)
* mygrep test lab1.c -i
* mygrep test lab1.c -n
