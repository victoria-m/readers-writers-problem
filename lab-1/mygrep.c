#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define BUFF_SIZE 512
#define MAX_ARGS 4

// How to run the program: ./mygrep STRING FILE [OPTION(s)]

// required prototypes:
int grep_stream(FILE *fpntr, char *string, char *file_pathname);  // returns 0 if string not found, 1 if found
char* get_next_line(FILE *fpntr); // returns current line

// additional prototypes:
FILE *open_file(char *file_pathname, FILE *fpntr);  /* opens file and checks if there were any errors with fopen(). I included this function because
                                                    the submission guidelines state that code should not be reused for different cases. */
// global variables:
char buff[BUFF_SIZE];
char *temp = buff;
int ch; // stored in temp, also used to track line position
int found = 0; // 0 if string not found, 1 if some found, also used as a flag to determine main's return status
char opt = '\0'; // keeps track of options entered

int main(int argc, char **argv) {
  char *string;     // search string
  char *file_pathname;
  FILE *fpntr = 0;  // must be initialized in order to be used in file_open()

  // check if correct number of args were entered
  if (argc > MAX_ARGS || argc == 1) {
    fprintf(stderr, "Usage: mygrep [OPTION] STRING FILE\n");
    exit (2);
  }

  // transfer argument values to string and file_pathname variables
  int i;
  for (i = 0; i < argc; ++i) {
    switch (i) {
      case 1:
        string = argv[i];
        if (argc == 2) { // if stdin is used, there will only be two args but we still need to set the file_pathname
          file_pathname = "/dev/stdin";
          fpntr = open_file(file_pathname, fpntr);
        } break;
      case 2:  // if file_pathname is specified
        file_pathname = argv[i];
        fpntr = open_file(file_pathname, fpntr);
        break;
      case 3: // if -i (ignore case) or if -n (line number) options are included
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--ignore-case") == 0) opt = 'i';
        else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--line-number") == 0) opt = 'n';
        else if (strcmp(argv[i], "-in") == 0 || strcmp(argv[i], "-ni") == 0) opt = 'b';  // both
        else {
          fprintf(stderr, "Invalid option was entered.\n");
          exit (2);
        } break;
      default:
        break;
    }
  }

  printf("\nSearch string: %s \nFile pathname: %s\n\n", string, file_pathname);

  grep_stream(fpntr, string, file_pathname);

  fclose(fpntr);

  // grep returns 0 if a line is selected, 1 if no lines selected, and 2 if an error occured
  if (errno != 0) return 2;   // check if there was an error first since found will always be either 1 or 0

  return (found == 1) ? 0 : 1;
}


// required functions:
int grep_stream(FILE *fpntr, char *string, char *file_pathname) {
  int count = 1;  // used for -n option to keep track of line number, also start at line 1 since there is no line 0

  while (ch != EOF) {
    char *buff = get_next_line(fpntr);  // store next line in *buff

    if (ferror(fpntr) != 0) {  // if there was an error reading from file
      fprintf(stderr, "%s\n", strerror(errno));
      return found;
    }

    if (opt == '\0' || opt == 'n') {
      if (strstr(buff, string) != NULL) {  // check if string was found in buff
        if (opt == 'n') fprintf(stdout, "%d:%s\n", count, buff);
        else fprintf(stdout, "%s\n", buff);
        found = 1;
      }
    } else if (opt == 'i' || opt == 'b') {
      if (strcasestr(buff, string) != NULL) {  // check if string was found, ignoring case
        if (opt == 'b') fprintf(stdout, "%d:%s\n", count, buff);
        else fprintf(stdout, "%s\n", buff);
        found = 1;
      }
    }
    free(buff);
    ++count;
  }
  return found;
}

char *get_next_line(FILE *fpntr) {
  char *buff = malloc(sizeof(*buff) * BUFF_SIZE); // allocate dynamic memory to buff

  while ((char)(ch = fgetc(fpntr)) != '\n' || ferror(fpntr) != 0) {  // call fgetc() until '\n' is reached OR an error occurs while reading
    if (ch == EOF) return buff; // return when end of file has been reached

    *temp = ch; // store value of ch in value pointed to by temp
    temp++; // increment temp pointer in order to store next value of ch
  }

  ++ch; // move onto next line when finished processing (this skips '\n', which is otherwise stored in the buffer)
  temp[sizeof(temp)] = '\0'; // turn temp into a valid string
  temp = buff; // transfer contents of temp to buff

  return buff;
}


// additional functions:
FILE *open_file(char *file_pathname, FILE *fpntr) {
  fpntr = fopen(file_pathname, "r");

  if (fpntr == NULL) {
    fprintf(stderr, "%s\n", strerror(errno));
    exit (2);
  } return fpntr;
}
