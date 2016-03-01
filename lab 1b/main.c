#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

void CATCH_SIG(int signum)
{
  fprintf(stderr, "%d caught\n", signum);
  exit(signum);
}


int main(int argc, char * const argv[])
{
  static struct option longopts[]= {
    {"rdonly", required_argument, NULL, 'r'},
    {"wronly", required_argument, NULL, 'w'},
    {"verbose", no_argument, NULL, 'v'},
    {"command", required_argument, NULL, 'c'},
    {"append", no_argument, NULL, 'a'},
    {"cloexez", no_argument, NULL, 'C'},
    {"creat", no_argument, NULL, 'x'},
    {"directory", no_argument, NULL, 'd'},
    {"dsync", no_argument, NULL, 'D'},
    {"excl", no_argument, NULL, 'e'},
    {"nofollow", no_argument, NULL, 'n'},
    {"nonblock", no_argument, NULL, 'N'},
    {"rsync", no_argument, NULL, 'R'},
    {"sync", no_argument, NULL, 's'},
    {"trunc", no_argument, NULL, 't'},
    {"rdwr", required_argument, NULL, 'q'},
    {"pipe", no_argument, NULL, 'p'},
    {"wait", no_argument, NULL, 'W'},
    {"close", required_argument, NULL, 'X'},
    {"abort", no_argument, NULL, 'A'},
    {"catch", required_argument, NULL, 'V'},
    {"ignore", required_argument, NULL, 'i'},
    {"default", no_argument, NULL, 'f'},
    {"pause", no_argument, NULL, 'P'},
    {0,0,0,0}
  };
  int longindex=0;
  int arrsize = 20;
  int pos = 0;
  int * fdarr = (int*)malloc(20*sizeof(int));
  if(fdarr == NULL)
    {
      printf("Error Allocating Memory");
      exit(1);
    }

  int subopts_size = 100;
  int pos1 = 0;
  char** subopts = malloc(100*sizeof(char));
  if(subopts == NULL)
    {
      printf("Error Allocating Memory");
      exit(1);
    }

  int process_size = 20;
  int pos2 = 0;
  pid_t* process = (pid_t*)malloc(20*sizeof(pid_t));
  if(process == NULL)
    {
      printf("Error Allocating Memory");
      exit(1);
    }
 
  int verbose = 0;
  int opt;
  int fdopt = 0;
  while((opt=getopt_long(argc, argv, "", longopts, &longindex)) != -1)
    {
      int sig_num;
      if(opt == 'r')
	{
	  if(verbose)
	    {
	      printf("%s", argv[optind-2]);
	      printf(" %s\n", argv[optind-1]); 
	    }
	  fdarr[pos] = open(argv[optind-1], fdopt | O_RDONLY, 0400);
	  if(fdarr[pos] <0)
	    {
	      printf("%s", "Error opening file");
	      exit(1);
	    }
	  fdopt = 0;
	  pos++;	  
	}
	
      if(opt == 'w')
	{
	  if(verbose)
	    {
	      printf("%s", argv[optind-2]);
	      printf(" %s\n",argv[optind-1]); 
	    }
	  fdarr[pos] = open(argv[optind-1],fdopt | O_WRONLY, 0200);
	  if(fdarr[pos] <0)
	    {
	      printf("%s", "Error opening file");
	      exit(1);
	    }
	  fdopt = 0;

	  if(fdarr[pos] == -1)
	    exit(1);
	  
	  pos++;
	}
      
      if(opt == 'v')
	{
	  verbose = 1;
	}
      
      if(opt == 'c')
	{
	  int firstArg = optind;
	  int lastArg;
	  int loc = firstArg;
	  for(;loc < argc; loc++)
	    {
	      if(argv[loc][0] == '-')
		{
		  if(argv[loc][1] == '-')
		    {
		      lastArg = loc-1;
		      break;
		    }
		}
	    }
	  
	  if(loc == argc)
	    lastArg = argc-1;

	  if(lastArg <= firstArg+1)
	    exit(1);
	  
	  int input = atoi(argv[firstArg-1]);
	  int output = atoi(argv[firstArg]);
	  int error = atoi(argv[firstArg+1]);
	  char* command = argv[firstArg+2];
	  char** options = malloc(sizeof(char*)*(lastArg-firstArg-2));
	  if(options == NULL)
	    {
	      printf("Error Allocating Memory");
	      exit(errno);
	    }
	  int i=1;
	  options[0]= command;
	  for(;i <= (lastArg-firstArg-2);i++)
	    {
	      options[i] = argv[firstArg+2+i];
	    }
	  options[i] = NULL;

	    
	  if(input > pos || output > pos || error > pos)
	    exit(1);
	  
	  if(input < 0 || output < 0 || error < 0)
	    exit(1);
	  
	  if(verbose)
	    {
	      printf("%s", argv[firstArg-2]);
	      printf(" %d", input);
	      printf(" %d", output);
	      printf(" %d", error);
	      int x;
	      for(x=0;x < i;x++)
		{
		  printf(" %s", options[x]);
		}
	      printf("\n");
	    }

	  process[pos1] = fork();
	  if (process[pos1] < 0)
	    {
	      printf("ERROR: forking child process failed\n");
	      exit(1);
	    }
	  else if (process[pos1] == 0)
	    {
	      dup2(fdarr[input], 0);
	      dup2(fdarr[output], 1);
	      dup2(fdarr[error], 2);
	      int x;
	      for(x=0; x < pos;x++)
		close(fdarr[x]);
		
	      if (execvp(options[0], options) < 0)
		{
		  printf("ERROR: exec failed\n");
		  exit(1);
		}
	    }
	  pos1++;
	  for(i=0; i <= (lastArg-firstArg-2); i++)
	    {
	      if(pos1 == subopts_size)
		{
		  subopts = realloc(subopts,subopts_size*2*sizeof(char*));
		  if(subopts == NULL)
		    {
		      printf("Error Reallocating Memory");
		      exit(1);
		    }
		  subopts_size *= 2;
		}
	      subopts[pos2] = options[i];
	      pos2++;
	    }
	  subopts[pos2++] = '\0';
	}

      if(opt == 'a')
	{
	  if(verbose)
	    {
	      printf("%s ", argv[optind-1]);
	    }
	   
	  fdopt |= O_APPEND;
	}

      if(opt == 'C')
	{
	  if(verbose)
	    {
	      printf("%s ", argv[optind-1]);
	    }
	  
	  fdopt |= O_CLOEXEC ;
	}

      if(opt == 'x')
	{
	  if(verbose)
	    {
	      printf("%s ", argv[optind-1]);
	    }
	  
	  fdopt |= O_CREAT;
	}
      
      if(opt == 'd')
	{
	  if(verbose)
	    {
	      printf("%s ", argv[optind-1]);
	    }
	  
	  fdopt |= O_DIRECTORY;
	}
      
      if(opt == 'D')
	{
	  if(verbose)
	    {
	      printf("%s ", argv[optind-1]);
	    }
	  
	  fdopt |= O_DSYNC;
	}

      if(opt == 'e')
	{
	  if(verbose)
	    {
	      printf("%s ", argv[optind-1]);
	    }
	  
	  fdopt |= O_EXCL;
	}

      if(opt == 'n')
	{
	  if(verbose)
	    {
	      printf("%s ", argv[optind-1]);
	    }
	  
	  fdopt |= O_NOFOLLOW;
	}

      if(opt == 'N')
	{
	  if(verbose)
	    {
	      printf("%s ", argv[optind-1]);
	    }
	  
	  fdopt |= O_NONBLOCK;
	}
	   
      if(opt == 'R')
	{
	  if(verbose)
	    {
	      printf("%s ", argv[optind-1]);
	    }
	  
	  fdopt |= O_RSYNC;
	}

      if(opt == 's')
	{
	  if(verbose)
	    {
	      printf("%s ", argv[optind-1]);
	    }
	  
	  fdopt |= O_SYNC;
	}

      if(opt == 't')
	{
	  if(verbose)
	    {
	      printf("%s ", argv[optind-1]);
	    }
	  
	  fdopt |= O_TRUNC;
	}

      if(opt == 'q')
	{
	  if(verbose)
	    {
	      printf("%s", argv[optind-2]);
	      printf(" %s\n", argv[optind-1]); 
	    }
	  fdarr[pos] = open(argv[optind-1], fdopt | O_RDWR, 0600);
	  fdopt = 0;
	  
	  if(fdarr[pos] == -1)
	    exit(1);
	  
	  pos++;	
	}

      if(opt == 'p')
	{
	  if(verbose)
	    {
	      printf("%s\n", argv[optind-1]);
	    }
	  
	  int fd[2];
	  if(pipe(fd) == -1)
	    exit(1);
	  
	  fdarr[pos++] = fd[0];
	  fdarr[pos++] = fd[1];
	}

      if(opt == 'W')
	{
	  if(verbose)
	    {
	      printf("%s\n", argv[optind-1]);
	    }
	  int x;
	  for(x=0; x < pos;x++)
	    {
	      if(fdarr[x] == -1)
		continue;
	      close(fdarr[x]);
	    }

	  int i = 0;
	  x = 0;
	  for(; i < pos1; i++)
	    {
	      int status;
	      wait(&status);
	      printf("%d", status);
	
	      for(; x < pos2; x++)
		{
		  if(subopts[x] == '\0')
		    break;
		  printf(" %s", subopts[x]);
		}
	      x++;
	      printf("\n");
	    }
	}

      if(opt == 'X')
	{
	  if(verbose)
	    {
	      printf("%s", argv[optind-2]);
	      printf(" %s\n", argv[optind-1]);
	    }
	  int x = atoi(argv[optind-1]);
	  close(fdarr[x]);
	  fdarr[x] = -1;
	}

      if(opt == 'A')
	{
	  if(verbose)
	    printf("%s\n", argv[optind-2]); 
	  abort();
	}

      if(opt == 'V')
	{
	  sig_num = atoi(argv[optind-1]);
	  if(verbose)
	    {
	      printf( "%s", argv[optind-2]);
	      printf(" %s\n", argv[optind-1]);
	    }
	  if(signal(sig_num, CATCH_SIG) ==SIG_ERR)
	    {
	      fprintf(stderr, "signal error\n");
	      exit(1);
	    }
	}

      if(opt == 'i')
	{
	  if(verbose)
	    {
	      sig_num = atoi(argv[optind-1]);
	      printf("%s", argv[optind-2]);
	      printf(" %s\n", argv[optind-1]);
	    }
	  if(signal(sig_num, SIG_IGN) == SIG_ERR)
	    {
	      fprintf(stderr, "ERROR ignoring signal\n");
	      exit(1);
	    }
	}

      if(opt == 'f')
	{
	  sig_num = atoi(argv[optind-1]);
	  if(verbose)
	    {
	      printf( "%s", argv[optind-2]);
	      printf(" %s\n", argv[optind-1]);
	    }
	  if(signal(sig_num, SIG_DFL) ==SIG_ERR)
	    {
	      fprintf(stderr, "signal error\n");
	      exit(1);
	    }
	}

      if(opt == 'P')
	{
	  if(verbose)
	    {
	      printf("%s\n", argv[optind-1]);
	    }
	  
	  pause();
	}
      
      if(pos == arrsize)
	{
	  fdarr = (int*)realloc(fdarr,arrsize*2*sizeof(int));
	  if(fdarr == NULL)
	    {
	      printf("Error Reallocating Memory");
	      exit(1);
	    }
	  arrsize *= 2;
	}
      
      if(pos2 == process_size)
	{
	  process = (int*)realloc(process,process_size*2*sizeof(int));
	  if(process == NULL)
	    {
	      printf("Error Reallocating Memory");
	      exit(1);
	    }
	  process_size *= 2;
	}
      
      int x;
      for(x=optind; x<argc; x++)
	{
	  if(argv[optind][0] == '-')
	    {
	      if(argv[optind][1] == '-')
		 break;
	    }
	    optind++;
	}
    }
  free(fdarr);
  free(process);
  
  exit(0);
}
