#Summary
The purpose of lab 1 is to duplicate the functionality of the Linux/UNIX "grep" command. grep is a command that searches one or more files for lines that contain a given string/pattern, and prints those lines out. This program only duplicates a subset of the grep command functionality.

#Program guidelines:
• it does not need to accept any options
• it will be used for a string, not a regular expression
• it only needs to handle one file argument.

#Syntax
*Note* I named my program "lab1" instead of "mygrep".

The syntax for mygrep is:
mygrep STRING [FILE] (the file argument is optional). If no argument is given, the program will read from standard input.

#Examples
Here are examples of syntactically valid calls to mygrep:
• mygrep printf lab1.c
• mygrep "printf (" lab1.c
• cat lab1.c | mygrep printf
• mygrep testing (read from stdin, use ctrl-d to terminate input)
