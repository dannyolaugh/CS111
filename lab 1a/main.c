#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>


int main(int argc, char * const argv[])
{
  static struct option longopts[]= {
    {"rdonly", required_argument, NULL, 'r'},
    {"wronly", required_argument, NULL, 'w'},
    {"verbose", no_argument, NULL, 'v'},
    {"command", required_argument, NULL, 'c'},
    {0,0,0,0}
  };
  int longindex=0;
  int * fdarr = (int*)malloc(20*sizeof(int));
  if(fdarr == NULL)
    {
      printf("Error Allocating Memory");
      exit(0);
    }
  
  int pos = 0;
  int arrsize = 20;
  int verbose = 0;
  int opt;
  while((opt=getopt_long(argc, argv, "", longopts, &longindex)) != -1)
    {
      if(opt == 'r')
	{
	  if(verbose)
	    {
	      printf("%s", argv[optind-2]);
	      printf(" %s\n", argv[optind-1]); 
	    }
	  fdarr[pos] = open(argv[optind-1],O_RDONLY);
	  
	  if(fdarr[pos] == -1)
	    exit(1);
	  
	  pos++;	  
	}
	
      if(opt == 'w')
	{
	  if(verbose)
	    {
	      printf("%s", argv[optind-2]);
	      printf(" %s\n",argv[optind-1]); 
	    }
	  fdarr[pos] = open(argv[optind-1],O_WRONLY);
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

	  pid_t  process;
	  int    status;
	  process = fork();
	  if (process < 0)
	    {
	      printf("ERROR: forking child process failed\n");
	      exit(1);
	    }
	  else if (process == 0)
	    {
	      dup2(fdarr[input], 0);
	      dup2(fdarr[output], 1);
	      dup2(fdarr[error], 2);
	      if (execvp(options[0], options) < 0)
		{
		  printf("ERROR: exec failed\n");
		  exit(1);
		}
	    }
	  else
	    { 
	      while (wait(&status) != process)
		{
		  continue;
		}
	    }
	  /*
	  int a;
	  for(a = 0; a < i-1; a++)
	    {
	      free(options[a]);
	    }
	  free(options);
	  free(command);
	  */
	  printf("%d", status);
	  for(i=0; i <= (lastArg-firstArg-2); i++)
	    {
	      printf(" %s", options[i]);
	    }
	  printf("\n");
	}
      
      if(pos == arrsize)
	{
	  fdarr = (int*)realloc(fdarr,arrsize*2*sizeof(int));
	  if(fdarr == NULL)
	    {
	      printf("Error Reallocating Memory");
	      exit(0);
	    }
	  arrsize *= 2;
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
}
