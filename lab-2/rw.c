#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>
#include "rw.h"   // include the rw custom header file

#define SLOWNESS 30000
#define INVALID_ACCNO -99999

// global shared data structure: readers and writers will be accessing concurrently.
account account_list[SIZE];

pthread_mutex_t r_lock = PTHREAD_MUTEX_INITIALIZER;   // read lock: shared only between readers
pthread_mutex_t rw_lock = PTHREAD_MUTEX_INITIALIZER;  // read-write lock: shared between readers and writers
int read_count = 0;                                   // keeps track of number of readers within the critical section

// sleep function
void rest() {
  usleep(SLOWNESS * (rand() % 1));
}


// writer thread - will update the account_list data structure.
// takes as argument the seed for the srand() function.

void * writer_thr(void * arg) {

  printf("Writer thread ID %ld\n", pthread_self());
	srand((unsigned int)(intptr_t) &arg);		// set random number seed for this writer

	int i, j;
	int r_idx;
	unsigned char found;   // for every update_acc[j], set to TRUE if found in account_list, else set to FALSE
	account update_acc[WRITE_ITR];

	// first create a random data set of account updates
	for (i = 0; i < WRITE_ITR; i++) {

    // a random  number in the range [0, SIZE)
    r_idx = rand() % SIZE;

		update_acc[i].accno = account_list[r_idx].accno;
		update_acc[i].balance = 1000.0 + (float) (rand() % MAX_BALANCE);
	}

	// open a writer thread log file
	char thr_fname[64];
	snprintf(thr_fname, 64, "writer_%ld_thr.log", pthread_self());
	FILE* fd = fopen(thr_fname, "w");

	if (!fd) {
		fprintf(stderr,"Failed to open writer log file %s\n", thr_fname);
		pthread_exit(&errno);
	}

  /* The writer thread will now to update the shared account_list data structure
     for each entry 'j' in the update_acc[] array, it will find the corresponding
     account number in the account_list array and update the balance of that account
     number with the value stored in update_acc[j]. */

  // used for storing the accno of account_list[] before invalidating it
  int temp_accno = 0;

  // used for storing previous balance of account_list[], which is used when printing to the writer log file
  float temp_balance = 0;

  // the writer thread will now update the shared account_list data structure
  for (j = 0; j < WRITE_ITR;j++) {
      found = FALSE;

      // writer locks readers and other writers out of critical section
      pthread_mutex_lock(&rw_lock);

      for (i = 0; i < SIZE; i++) {

        // lock and update location - remember, you MUST always lock if you are going to read/write on a variable
        // that can change! Here account_list[i].accno can be changed by another writer.

        if (account_list[i].accno == update_acc[j].accno) {

          /* You MUST FIRST TEMPORARILY INVALIDATE the accno by setting account_list[i] = INVALID_ACCNO;
          before making any updates to the account_list[i].balance. Once the account balance is updated,
          you MUST put the rest() call in the appropriate place before going for update_acc[j+1].

          This is to enable detecting race condition with reader threads violating CS entry criteria.
          For every successful update to the account_list, you must write a log entry using the following format string:
          fprintf(fd, "Account number = %d [%d]: old balance = %6.2f, new balance = %6.2f\n",
                            				account_list[i].accno, update_acc[j].accno, account_list[i].balance, update_acc[j].balance);

          additionally, your code must also introduce checks/test to detect possible
          corruption due to race condition from CS violations. */

          temp_balance = account_list[i].balance; // store previous account balance
          temp_accno = account_list[i].accno; // save current accno before invalidation

          account_list[i].accno = INVALID_ACCNO;  // before we can update the balance, we must temporarily invalidate the accno
          account_list[i].balance = update_acc[j].balance; // update account_list[i].balance

          // rest to prevent a race condition before invalidating the account number
          rest();

          // restore account number once balance is updated
          account_list[i].accno = temp_accno;

          // write log entry for the acccount_list[i]'s updated balance:
          fprintf(fd, "Account number = %d [%d]: old balance = %6.2f, new balance = %6.2f\n",
                          account_list[i].accno, update_acc[j].accno, temp_balance, update_acc[j].balance);

          found = TRUE;
      }
    }

    pthread_mutex_unlock(&rw_lock);  // finally, unlock rw lock
    if (!found) fprintf(fd, "Failed to find account number %d!\n", update_acc[j].accno);

  }

  fclose(fd);
  return NULL;
}


// reader thread - will read the account_list data structure.
// takes as argument the seed for the srand() function.

void * reader_thr(void *arg) {

  printf("Reader thread ID %ld\n", pthread_self());
  srand((unsigned int)(intptr_t) &arg);   // set random number seed for this reader

  int i, j;
  int r_idx;

  // for every read_acc[j], set to TRUE if found in account_list, else set to FALSE
  unsigned char found;
  account read_acc[READ_ITR];

  // first create a random data set of account updates
  for (i = 0; i < READ_ITR; i++) {
    r_idx = rand() % SIZE;      // a random number in the range [0, SIZE)
    read_acc[i].accno = account_list[r_idx].accno;
    read_acc[i].balance = 0.0;		// we are going to read in the value
  }

  // open a reader thread log file
  char thr_fname[64];
  snprintf(thr_fname, 64, "reader_%ld_thr.log", pthread_self());
  FILE *fd = fopen(thr_fname, "w");

  if (!fd) {
    fprintf(stderr,"Failed to reader log file %s\n", thr_fname);
    pthread_exit(&errno);
  }


  /*  The reader thread will now to read the shared account_list data structure.
  For each entry 'j' in the read_acc[] array, the reader will fetch the
  corresponding balance from the account_list[] array and store in
  read_acc[j].balance. The reader will then make a log entry in the log file
  for every successful read from the account_list using the format:

  fprintf(fd, "Account number = %d [%d], balance read = %6.2f\n",
                    account_list[i].accno, read_acc[j].accno, read_acc[j].balance);

  If it is unable to find a account number, then the following log entry must be
  made:

    fprintf(fd, "Failed to find account number %d!\n", read_acc[j].accno);

  Additionally, your code must also introduce checks/test to detect possible
  corruption due to race condition from CS violations.  */


  // the reader thread will now to read the shared account_list data structure
  for (j = 0; j < READ_ITR; j++) {

    // now read the shared data structure
    found = FALSE;
    pthread_mutex_lock(&r_lock);  // lock read critical section

    // increase number of readers, keep writers from entering the rear/write critical section
    if (++read_count == 1) pthread_mutex_lock(&rw_lock);

    pthread_mutex_unlock(&r_lock);  // finally, unlock read lock

    for (i = 0; i < SIZE; i++) {

      rest(); // rest before locking the r mutex to avoid a race condition

      // now lock and update - remember that since account_list[i].accno can be updated by a writer,
      // it MUST be protected by locking for ANY read!

      if (account_list[i].accno == read_acc[j].accno) {

        // update read_acc[j] balance and print to the reader's log file
        read_acc[j].balance = account_list[i].balance;
        fprintf(fd, "Account number = %d [%d], balance read = %6.2f\n", account_list[i].accno, read_acc[j].accno, read_acc[j].balance);

        found = TRUE;
      }
    }

    pthread_mutex_lock(&r_lock);
    if (--read_count == 0) pthread_mutex_unlock(&rw_lock);

    pthread_mutex_unlock(&r_lock);  // finally, unlock read lock
    if (!found) fprintf(fd, "Failed to find account number %d!\n", read_acc[j].accno);
  }

  fclose(fd);
  return NULL;
}


// populate the shared account_list data structure

void create_testset() {
  time_t t;
  srand(time(&t));
  int i;

  for (i = 0;i < SIZE; i++) {
    account_list[i].accno = 1000 + rand() % RAND_MAX;
    account_list[i].balance = 100 + rand() % MAX_BALANCE;
  }
  return;
}


// if either or both the command line arguments are missing or not integers > 0, then the program should print
// the usage message and abort

void usage(char *str) {
  printf("Usage: %s -r <NUM_READERS> -w <NUM_WRITERS>\n", str);
  abort();
}


// checks if option argument is an integer; returns 0 if it not an integer, 1 if it is

int is_integer(char *str) {
  while (*str != '\0') {

    // numbers 48 through 57 in ASCII represent 0 through 9. the below statement checks if the current character is not a digit.
    if (*str <= 47 || *str >= 58) return 0;
    ++str;
  }
  return 1;
}


int main(int argc, char *argv[]) {
  void *result;
  time_t t;
  unsigned int seed;
  int i;

  int READ_THREADS = 0;     // number of readers to create
  int WRITE_THREADS = 0;    // number of writers to create

  create_testset();         // Generate a list of account informations. This will be used as the input to the Reader/Writer threads.


	/*   Use getopt() to read as command line arguments the number of readers (READ_THREADS)
     and number of writers (WRITE_THREADS). If any of the arguments are missing, then print
     the usage message using the usage() function defined above and exit.

     For reference on getopt(), see "man getopt(3)"   */


  // determines if correct number of arguments were entered
  if (argc != 5) usage(*argv);

  // keeps track of options entered
  int opt = 0;

  while ((opt = getopt(argc, argv, "r:w:")) != -1) {  // assign args until no more options are present
    switch (opt) {
      case 'r' :
        // checks if read option is an integer, if so convert option arguments to integer number which represents number of reader threads
        if (is_integer(optarg) == 1) READ_THREADS = atoi(optarg);
        else usage(*argv);
        break;
      case 'w' :
        // checks if write option is an integer, if so convert option arguments to integer number which represents number of writer threads
        if (is_integer(optarg) == 1) WRITE_THREADS = atoi(optarg);
        else usage(*argv);
        break;
      case '?' :       // checks if invalid option charactera were entered
        usage(*argv);
        break;
      default :
        usage(*argv);
        break;
    }
  }

  pthread_t* reader_idx = (pthread_t *) malloc(sizeof(pthread_t) * READ_THREADS);		// holds thread IDs of readers
  pthread_t* writer_idx  = (pthread_t *) malloc(sizeof(pthread_t) * WRITE_THREADS);	// holds thread IDs of writers

	// create readers
  for (i = 0; i < READ_THREADS; ++i) {
    seed = (unsigned int) time(&t);

    if (pthread_create(reader_idx + i, NULL, reader_thr, (void *) (intptr_t) seed) != 0) {  // if pthread creation was unsuccessful
      perror("pthread create error:");
      exit(2);
    }
  }
  printf("\nDone creating reader threads!\n");

	// create writers
  for (i = 0; i < WRITE_THREADS; ++i) {
    seed = (unsigned int) time(&t);

    if (pthread_create(writer_idx + i, NULL, writer_thr, (void *) (intptr_t) seed) != 0) {  // if pthread creation was unsuccessful
      perror("pthread create error:");
      exit(2);
    }
  }
  printf("\nDone creating writer threads!\n\n");

  // join reader threads
  for(i = 0; i < READ_THREADS; ++i) {
    if (pthread_join(reader_idx[i], &result) != 0) {
      perror("pthread join error:");
      exit(2);
    }
    printf("\nJoined reader thread %d with status: %ld", i, (intptr_t) result);
  }
  printf("\n\nReader threads joined.\n\n");

  // join writer threads
  for(i = 0; i < WRITE_THREADS; ++i) {
    if (pthread_join(writer_idx[i], &result) != 0) {
      perror("pthread join error:");
      exit(2);
    }
    printf("Joined writer thread %d with status: %ld\n", i, (intptr_t) result);
  }
	printf("\nWriter threads joined.\n\n");

  // clean-up
  pthread_mutex_destroy(&r_lock);
  pthread_mutex_destroy(&rw_lock);

  free(reader_idx);
  free(writer_idx);

  return 0;
}
