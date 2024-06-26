#include <fcntl.h>
#include <stdbool.h>
#include <stdckdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <unistd.h>

/* A process table entry.  */
struct process
{
  long pid;
  long arrival_time;
  long burst_time;

  TAILQ_ENTRY (process) pointers;

  /* Additional fields here */
  long remaining_time;
  long waiting_time;
  long response_time;
  /* End of "Additional fields here" */
};

TAILQ_HEAD (process_list, process);

/* Skip past initial nondigits in *DATA, then scan an unsigned decimal
   integer and return its value.  Do not scan past DATA_END.  Return
   the integer’s value.  Report an error and exit if no integer is
   found, or if the integer overflows.  */
static long
next_int (char const **data, char const *data_end)
{
  long current = 0;
  bool int_start = false;
  char const *d;

  for (d = *data; d < data_end; d++)
    {
      char c = *d;
      if ('0' <= c && c <= '9')
	{
	  int_start = true;
	  if (ckd_mul (&current, current, 10)
	      || ckd_add (&current, current, c - '0'))
	    {
	      fprintf (stderr, "integer overflow\n");
	      exit (1);
	    }
	}
      else if (int_start)
	break;
    }

  if (!int_start)
    {
      fprintf (stderr, "missing integer\n");
      exit (1);
    }

  *data = d;
  return current;
}

/* Return the first unsigned decimal integer scanned from DATA.
   Report an error and exit if no integer is found, or if it overflows.  */
static long
next_int_from_c_str (char const *data)
{
  return next_int (&data, strchr (data, 0));
}

/* A vector of processes of length NPROCESSES; the vector consists of
   PROCESS[0], ..., PROCESS[NPROCESSES - 1].  */
struct process_set
{
  long nprocesses;
  struct process *process;
};

/* Return a vector of processes scanned from the file named FILENAME.
   Report an error and exit on failure.  */
static struct process_set
init_processes (char const *filename)
{
  int fd = open (filename, O_RDONLY);
  if (fd < 0)
    {
      perror ("open");
      exit (1);
    }

  struct stat st;
  if (fstat (fd, &st) < 0)
    {
      perror ("stat");
      exit (1);
    }

  size_t size;
  if (ckd_add (&size, st.st_size, 0))
    {
      fprintf (stderr, "%s: file size out of range\n", filename);
      exit (1);
    }

  char *data_start = mmap (NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data_start == MAP_FAILED)
    {
      perror ("mmap");
      exit (1);
    }

  char const *data_end = data_start + size;
  char const *data = data_start;

  long nprocesses = next_int (&data, data_end);
  if (nprocesses <= 0)
    {
      fprintf (stderr, "no processes\n");
      exit (1);
    }

  struct process *process = calloc (sizeof *process, nprocesses);
  if (!process)
    {
      perror ("calloc");
      exit (1);
    }

  for (long i = 0; i < nprocesses; i++)
    {
      process[i].pid = next_int (&data, data_end);
      process[i].arrival_time = next_int (&data, data_end);
      process[i].burst_time = next_int (&data, data_end);
      /* START of user added code */
      process[i].remaining_time = process[i].burst_time;
      process[i].waiting_time = 0;
      process[i].response_time = 0;
      /* END of user added code */

      if (process[i].burst_time == 0)
	{
	  fprintf (stderr, "process %ld has zero burst time\n",
		   process[i].pid);
	  exit (1);
	}
    }

  if (munmap (data_start, size) < 0)
    {
      perror ("munmap");
      exit (1);
    }
  if (close (fd) < 0)
    {
      perror ("close");
      exit (1);
    }
  return (struct process_set) {nprocesses, process};
}

long
compare_longs(const void *a, const void *b) {
  return (*(long *)a - *(long *)b);
}

long
compute_median(struct process_list *pl) {
  double median = 0;

  if (!TAILQ_EMPTY(pl)) {
    // Populate array with CPU Times
    long n = 0;
    struct process *temp;
    TAILQ_FOREACH(temp, pl, pointers) {
      n++;
    }

    long* cpu_times = malloc(sizeof(long) * n);
    int i = 0;
    TAILQ_FOREACH(temp, pl, pointers) {
      cpu_times[i++] = temp->burst_time - temp->remaining_time;
    }

    qsort(cpu_times, n, sizeof(long), compare_longs);

    // Go to the middle of the sorted array to find median
    // if (n % 2 != 0) {
    //   median = cpu_times[n / 2];
    // } else { // Median is a calculation between two numbers
    //   median = (cpu_times[(n/2) - 1] + cpu_times[(n/2)]) / 2.0;

    //   // Round to nearest even number
    //   long truncated = (long)median;
    //   if (median - truncated != 0) {
    //     if (truncated % 2 == 0)
    //       median = truncated;
    //     else
    //       median = truncated + 1;
    //   }
    // }
    if (n % 2 != 0) {
      median = cpu_times[n / 2];
    } else { // Median is a calculation between two numbers
      bool is_even = (cpu_times[n / 2] + cpu_times[(n / 2) - 1]) % 2 == 0;
      if (is_even)
        median = ((cpu_times[n / 2] + cpu_times[(n / 2) - 1]) / 2);
      else {
        median = ((cpu_times[n / 2] + cpu_times[(n / 2) - 1] + 1) / 2);
        if ((long)median % 2 == 1)
          median--;
      }
    }

    free(cpu_times);
  }

  if ((long)median == 0)
    return 1;
  return (long)median;
}

int
main (int argc, char *argv[])
{
  if (argc != 3)
    {
      fprintf (stderr, "%s: usage: %s file quantum\n", argv[0], argv[0]);
      return 1;
    }

  struct process_set ps = init_processes (argv[1]);
  long quantum_length = (strcmp (argv[2], "median") == 0 ? -1
			 : next_int_from_c_str (argv[2]));
  if (quantum_length == 0)
    {
      fprintf (stderr, "%s: zero quantum length\n", argv[0]);
      return 1;
    }

  struct process_list list;
  TAILQ_INIT (&list);

  long total_wait_time = 0;
  long total_response_time = 0;

  /* Your code here */
  long time = 0;
  long new_processes = 0;
  long curr_quantum = 1; // 1 by default for median
  bool non_zero_remaining_time = false;
  bool context_switch = false;
  struct process* curr_process = NULL;
  struct process* prev_process = NULL;
  struct process* first_new = NULL;

  struct process_list temp_list;
  TAILQ_INIT (&temp_list);

  for (;;) {
    // Add on any new arriving processes
    new_processes = 0;
    first_new = NULL;
    for (int i = 0; i < ps.nprocesses; i++) {
      if (time == ps.process[i].arrival_time) {
        if (first_new == NULL)
          first_new = &ps.process[i];
        TAILQ_INSERT_TAIL(&list, &ps.process[i], pointers);
        // printf("New arrival: %d at t=%d", (long)ps.process[i].pid, time);
        // new_processes += 1;
      }
    }

    // Schedule new process!
    if (!TAILQ_EMPTY(&list) && curr_process == NULL) {
        // Context switches
        if (prev_process != NULL && prev_process->pid != TAILQ_FIRST(&list)->pid) {
            time++;
            prev_process = NULL;
            context_switch = true;
            continue;
        }

        // Handle quantum length
        if (quantum_length == -1)
          curr_quantum = compute_median(&list);
        else
          curr_quantum = quantum_length;

        // Schedule new process
        curr_process = TAILQ_FIRST(&list);
        TAILQ_REMOVE(&list, curr_process, pointers);

        // if (context_switch == true) {
        //   context_switch = false;
        //   continue;
        // }

        if (curr_process->arrival_time == time)
          new_processes--;
    } else
      prev_process = NULL; // Make sure a context switch is not done after an empty interval

    // Calculate response time for current process if necessary
    if (curr_process != NULL && curr_process->burst_time == curr_process->remaining_time)
        curr_process->response_time = time - curr_process->arrival_time;

    if (curr_process != NULL) {
        curr_quantum--;
        curr_process->remaining_time--;
    }

    time++;

    // Check if process finished executing or quantum time finished
    if (curr_process != NULL) {
      if (curr_process->remaining_time <= 0 || curr_quantum <= 0) {
        if (curr_process->remaining_time == 0)
          curr_process->waiting_time = time - curr_process->arrival_time - curr_process->burst_time;
        else {
            // printf("|%d|", new_processes);
            // for (int i = 0; i < new_processes; i++) {
            //   struct process* last_item = TAILQ_LAST(&list, process_list);
            //   TAILQ_REMOVE(&list, last_item, pointers);
            //   // printf("Popped\n");
            //   printf("%d", last_item->pid);
            //   TAILQ_INSERT_TAIL(&temp_list, last_item, pointers);
            // }
            // printf("Reinserted: %d at t=%d\n", (int)curr_process->pid, time - 1);
            // TAILQ_INSERT_TAIL(&list, curr_process, pointers);
            // while (!TAILQ_EMPTY(&temp_list)) {
            //   struct process* first_item = TAILQ_FIRST(&temp_list);
            //   TAILQ_REMOVE(&temp_list, TAILQ_FIRST(&temp_list), pointers);
            //   TAILQ_INSERT_TAIL(&list, first_item, pointers);
            //   // printf("Replaced\n");
            // }
            if (first_new != NULL) {
              if (first_new == curr_process)
                TAILQ_INSERT_TAIL(&list, curr_process, pointers);
              else
                TAILQ_INSERT_BEFORE(first_new, curr_process, pointers);
            } else
              TAILQ_INSERT_TAIL(&list, curr_process, pointers);
        }
        prev_process = curr_process;
        curr_process = NULL;
      }
    }

    // Check to see if all processes have finished
    for (int i = 0; i < ps.nprocesses; i++) {
      if (ps.process[i].remaining_time != 0) {
        non_zero_remaining_time = true;
        break;
      }
    }
    if (non_zero_remaining_time == false) break;
    non_zero_remaining_time = false;
  }

  // Do final totals and calculations
  for (int i = 0; i < ps.nprocesses; i++) {
    total_wait_time += ps.process[i].waiting_time;
    total_response_time += ps.process[i].response_time;
    // printf("%d %d %d\n", ps.process[i].pid, ps.process[i].waiting_time, ps.process[i].response_time);
  }
  // printf("Final time: %d\n", time); // remove later
  /* End of "Your code here" */

  printf ("Average wait time: %.2f\n",
	  total_wait_time / (double) ps.nprocesses);
  printf ("Average response time: %.2f\n",
	  total_response_time / (double) ps.nprocesses);

  if (fflush (stdout) < 0 || ferror (stdout))
    {
      perror ("stdout");
      return 1;
    }

  free (ps.process);
  return 0;
}