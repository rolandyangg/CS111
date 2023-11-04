
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
  long median; 
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
    long sum; 
    if (n % 2 == 0) {
        sum = cpu_times[n / 2 - 1] + cpu_times[n / 2]; 
        if(sum % 4 == 1){
          sum--; 
        }
        else if(sum % 4 == 3){
          sum++; 
        }
        median = sum / 2; 
    } 
    else {
        median = cpu_times[n / 2];
    }

 
  //   printf("CPU TIMES [");
  // for (int i = 0; i < n; i++)
  //   printf("%ld ", cpu_times[i]);
 
    free(cpu_times);
  }
 
  if (median == 0)
    median = 1;
 
  // printf("| Median calculated: %d [", (int)median);
  //       struct process* p2;
  //       TAILQ_FOREACH(p2, pl, pointers) {
  //           printf("%d ", p2->pid);
  //       }
  //       printf("\n");
 
  return median;
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
  bool time_to_compute_median = true; 
  bool context_switch = false;
  bool median_mode = false;
  struct process* process_executing = NULL;
  struct process* prev_process = NULL;
  struct process* first_new = NULL;
 
  if (quantum_length == -1) {
    median_mode = true;
    quantum_length = 1;
  }
 
  for (;;) {
    // first_new = NULL;
 
    // Add on any new arriving processes
    for (int i = 0; i < ps.nprocesses; i++) {
      if (time == ps.process[i].arrival_time) {
        TAILQ_INSERT_TAIL(&list, &ps.process[i], pointers);
        // if (first_new == NULL)
        //   first_new = &ps.process[i];
        // printf("Process %d arrived at %d matching %d\n", ps.process[i].pid, time, ps.process[i].arrival_time);
      }
    }
    
    if (median_mode == true && time_to_compute_median == true)
        quantum_length = compute_median(&list);
    median_mode = false; 
    // Schedule new process if necessary
    if (!TAILQ_EMPTY(&list) && process_executing == NULL) {
      // Check for context switch
      if (prev_process != NULL && prev_process->pid != TAILQ_FIRST(&list)->pid) {
        context_switch = true;
	// printf("We context switching, time is currently %ld\n", time);
        time++;
        prev_process = NULL;
        continue;
      }
 
      // Schedule new process
      process_executing = TAILQ_FIRST(&list);
      TAILQ_REMOVE(&list, process_executing, pointers);
      // ("Executing %d at time: %ld\n", process_executing->pid, time);
 
      // Handle quantum length
      curr_quantum = quantum_length;
 
      // Calculate response time for current process if necessary
      if (process_executing->started_exec == false) {
        process_executing->response_time = time - process_executing->arrival_time;
        process_executing->started_exec = true;
      }
    }
 
    // Execute the process
    if (process_executing != NULL) {
        if(process_executing->remaining_time == process_executing->burst_time){
		// printf("Process %d starts running at time %ld\n: ", process_executing->pid, time); 
	}
	curr_quantum--;
        process_executing->remaining_time--;
    }
 
    // Check if process finished executing or quantum time finished
    if (process_executing != NULL) {
      if (process_executing->remaining_time <= 0 || curr_quantum <= 0) {
	time_to_compute_median = true; 
	// printf("Process %d stops running at time %ld\n ", process_executing->pid, time); 
        if (process_executing->remaining_time > 0) {
		// gonna fucking kill myself dude
          // if (first_new != NULL && !TAILQ_EMPTY(&list)) {
          //   printf("NEW FIRST ARRIVAL: %d, putting %d before\n", first_new->pid, process_executing->pid);
          //   TAILQ_INSERT_BEFORE(first_new, process_executing, pointers);
          //   first_new = NULL;
          // }
          // else
            TAILQ_INSERT_TAIL(&list, process_executing, pointers);
        }
        // if (median_mode == true)
        //   compute_median(&list);
        prev_process = process_executing; // For context switch checking
        process_executing = NULL;
      }
    }
 
    time++;
 
    // If process finished executing then set its waiting time
    if (prev_process != NULL)
      prev_process->waiting_time = time - prev_process->arrival_time - prev_process->burst_time;
 
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
