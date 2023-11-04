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
  long num_in_file;

  TAILQ_ENTRY (process) pointers;

  /* Additional fields here */
  bool has_started;
  long start_time;
  long remaining_time;
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
      process[i].num_in_file = i;
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

int compare_long( const void* a, const void* b) //so i can use qsort for median computation
{
     long long_a = * ( (long*) a );
     long long_b = * ( (long*) b );

     if ( long_a == long_b ) return 0;
     else if ( long_a < long_b ) return -1;
     else return 1;
}

int compare_process( const void* a, const void* b){ //sorting processes by arrival time and order in file
  struct process *procA = (struct process*) a;
  struct process *procB = (struct process*) b;

  long at = procA->arrival_time;
  long bt = procB->arrival_time;

  if (at == bt) {
    long an = procA->num_in_file;
    long bn = procB->num_in_file;
    if (an == bn) return 0;
    else if (an < bn) return -1;
    else return 1;
  }
  else if (at < bt) return -1;
  else return 1;
}

int
main (int argc, char *argv[])
{
  if (argc != 3)
    {
      fprintf (stderr, "%s: usage: %s file quantum\n", argv[0], argv[0]);
      return 1;
    }

  struct process_set ps = init_processes (argv[1]); //read data, file name is in argv[1]
  long quantum_length = (strcmp (argv[2], "median") == 0 ? -1
			 : next_int_from_c_str (argv[2]));
  if (quantum_length == 0)
    {
      fprintf (stderr, "%s: zero quantum length\n", argv[0]);
      return 1;
    }
  
  //TODO: Remove

  struct process_list list;
  TAILQ_INIT (&list);

  long total_wait_time = 0;
  long total_response_time = 0;

  /* Your code here */

  long context_switch = 1;

  //sort the jobs by arrival time (you can do it by pairs, arrival time and position on process list)
  /* begin sort */

  qsort(ps.process, ps.nprocesses, sizeof(*ps.process), compare_process);

  /* end sort */

  //initialize has started and time remaining
  for (int i = 0; i < ps.nprocesses; i++){
    ps.process[i].remaining_time = ps.process[i].burst_time;
    ps.process[i].has_started = false;

    //printf ("ID: %ld \n", ps.process[i].pid);
  }

  long current_time = ps.process[0].arrival_time;
  long finished_jobs = 0;
  long index = 0;

  struct process *previous_process_pointer = NULL;
  struct process *current_process_pointer = NULL;

  while (finished_jobs < ps.nprocesses){
    
    /* update queue before context switch */
    for (; index < ps.nprocesses && ps.process[index].arrival_time < current_time; index++){
      TAILQ_INSERT_TAIL(&list, &ps.process[index], pointers);
    }
    if (previous_process_pointer != NULL && previous_process_pointer->remaining_time != 0){
      TAILQ_INSERT_TAIL(&list, previous_process_pointer, pointers); //only if there is time left over
    }
    for (;index < ps.nprocesses && ps.process[index].arrival_time <= current_time; index++){
      TAILQ_INSERT_TAIL(&list, &ps.process[index], pointers);
    }
    /* */

    //if our queue is not empty
    if (!TAILQ_EMPTY(&list)){
      current_process_pointer = TAILQ_FIRST(&list);
      TAILQ_REMOVE(&list, current_process_pointer, pointers);
      //take and pop first element

      if (previous_process_pointer != NULL && (current_process_pointer != previous_process_pointer)){
        current_time += context_switch; //simulate the context switch time
      }

      //do start time things
      if (!current_process_pointer->has_started){
        current_process_pointer->has_started = true;
        current_process_pointer->start_time = current_time;
      }

      //print start time of quantum
      

      //Determine run time
      long time_to_run;
      if (quantum_length == -1){
        //return 0;
        long num_running_processes = 1;
        struct process *p;
        TAILQ_FOREACH(p, &list, pointers) num_running_processes++;

        //create array of all current CPU times
        long *arr = (long*)malloc(num_running_processes * sizeof(long));
        arr[0] = current_process_pointer->burst_time - current_process_pointer->remaining_time;
        long index = 1;
        TAILQ_FOREACH(p, &list, pointers){
          arr[index] = p->burst_time - p->remaining_time;
          index++;
        }

        //sort the runtimes
        qsort(arr, num_running_processes, sizeof(*arr), compare_long);

        long mid = num_running_processes / 2;

        //find the median of the run times
        if (num_running_processes%2 == 0){
          if ((arr[mid] + arr[mid - 1])%2 == 0) time_to_run = (arr[mid] + arr[mid - 1]) / 2;
          else {
            time_to_run = (arr[mid] + arr[mid - 1] + 1) / 2;
            if (time_to_run%2 == 1) time_to_run--;
          }
          //printf("%ld -- ", arr[mid - 1]);
          //printf("%ld\n", arr[mid]);
        }
        else {
          time_to_run = arr[mid];
          //printf("%ld\n", arr[mid]);
        }
      
        free(arr);

        //if median is 0, set quantum to 1
        if (time_to_run == 0) time_to_run = 1;
      }
      else {
        time_to_run = quantum_length;
      }

      //printf("Time to run: %ld\n", time_to_run);

      //printf("Process %ld: ", current_process_pointer->pid);
      //printf("%ld to ", current_time);

      if (time_to_run > current_process_pointer->remaining_time) {
        time_to_run = current_process_pointer->remaining_time;
      }
      
      //simulate running the program
      current_process_pointer->remaining_time -= time_to_run;
      current_time += time_to_run;

      //update wait and response time if the process is finished
      if (current_process_pointer->remaining_time == 0){
        finished_jobs++;
        total_wait_time += (current_time - current_process_pointer->arrival_time - current_process_pointer->burst_time);
        total_response_time += (current_process_pointer->start_time - current_process_pointer->arrival_time);
      }

      //print end time of quantum
      //printf("%ld\n", current_time);

      previous_process_pointer = current_process_pointer;
    }
    else {
      current_time = ps.process[index].arrival_time;
      previous_process_pointer = NULL;
    }
  }

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
