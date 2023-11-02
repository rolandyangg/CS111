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
  long cpu_time;
  bool started_exec;
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
      process[i].cpu_time = 0;
      process[i].started_exec = false;
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
      cpu_times[i++] = temp->cpu_time;
    }

    qsort(cpu_times, n, sizeof(long), compare_longs);

    // Go to the middle of the sorted array to find median
    if (n % 2 != 0) {
      median = cpu_times[n / 2];
    } else { // Median is a calculation between two numbers
      median = (cpu_times[(n/2) - 1] + cpu_times[(n/2)]) / 2.0;

      // Round to nearest even number
      long truncated = (long)median;
      if (median - truncated != 0) {
        if (truncated % 2 == 0)
          median = truncated;
        else
          median = truncated + 1;
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
  long curr_quantum = quantum_length;
  bool non_zero_remaining_time = false;
  bool context_switch = false;
  struct process* process_executing = NULL;
  struct process* prev_process = NULL;

  for (;;) {
    // Add on any new arriving processes
    for (int i = 0; i < ps.nprocesses; i++) {
      if (time == ps.process[i].arrival_time) {
        TAILQ_INSERT_TAIL(&list, &ps.process[i], pointers);
        // printf("Process %d arrived at %d matching %d\n", ps.process[i].pid, time, ps.process[i].arrival_time);
      }
    }

    // Handle context switch check
    if (prev_process != NULL && !TAILQ_EMPTY(&list)) {
      if (prev_process->pid != TAILQ_FIRST(&list)->pid) {
        context_switch = true;
      }
    }
    prev_process = NULL; // To avoid having multiple context switches in a row

    // Enqueue the next process
    if (context_switch == false) { // Check context switch first, don't do anything if so
      if (process_executing == NULL) {
        if (!TAILQ_EMPTY(&list)) {
          // Handle quantum length
          if (quantum_length == -1) {
            curr_quantum = compute_median(&list);
          } else {
            curr_quantum = quantum_length;
          }

          // Schedule new process
          process_executing = TAILQ_FIRST(&list);
          TAILQ_REMOVE(&list, process_executing, pointers);
          // printf("NEW CURR EXECUTING PROCESS: %d at TIME %d\n", process_executing->pid, time);

          // Begin running process
          curr_quantum--;
          process_executing->remaining_time--;
          process_executing->cpu_time++;

          // Calculate response time for current process if necessary
          if (process_executing->started_exec == false) {
            process_executing->response_time = time - process_executing->arrival_time;
            process_executing->started_exec = true;
          }
        }
      } else { // There is a process that is already executing
        curr_quantum--;
        process_executing->remaining_time--;
        process_executing->cpu_time++;
      }
    }

    // Add onto the waiting times of every other process in the queue
    struct process* temp;
    TAILQ_FOREACH(temp, &list, pointers) {
      temp->waiting_time++;
    }

    // Skip steps if context switch
    if (context_switch == true) {
      time++;
      context_switch = false;
      continue;
    }

    // Check if process finished executing or quantum time finished
    if (process_executing != NULL) {
      if (process_executing->remaining_time <= 0 || curr_quantum <= 0) {
        if (process_executing->remaining_time > 0) {
          TAILQ_INSERT_TAIL(&list, process_executing, pointers);
        }
        prev_process = process_executing; // For context switch checking
        process_executing = NULL;
      }
    }

    time++;

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
