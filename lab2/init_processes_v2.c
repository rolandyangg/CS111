static struct process_set
init_processes_v2 (char const *filename)
{
  if (strcmp(filename, "-") != 0)
    {
      return init_processes(filename);
    }
  char *buf = NULL;
  size_t n = 0;
  if (getline(&buf, &n, stdin) == -1)
    {
      perror("getline");
      exit(1);
    }
  const char *data = buf;
  const char *data_end = data + n + 1;

  long nprocesses = next_int(&data, data_end);
  if (nprocesses <= 0)
    {
      fprintf(stderr, "no processes\n");
      exit(1);
    }

  struct process *process = calloc(sizeof *process, nprocesses);
  if (!process)
    {
      perror("calloc");
      exit(1);
    }

  for (long i = 0; i < nprocesses; i++)
    {
      if (getline(&buf, &n, stdin) == -1)
        {
          perror("getline");
          exit(1);
        }
      data = buf;
      data_end = data + n + 1;
      process[i].pid = next_int(&data, data_end);
      process[i].arrival_time = next_int(&data, data_end);
      process[i].burst_time = next_int(&data, data_end);
      if (process[i].burst_time == 0)
    {
      fprintf(stderr, "process %ld has zero burst time\n",
           process[i].pid);
      exit(1);
    }
    }

  free(buf);
  return (struct process_set) {nprocesses, process};
}
