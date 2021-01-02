#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<string.h>
#include<fcntl.h>
#define HISTORY_COUNT 10
#define MAX_CMD_LEN  128
char*** separate_piped_commands(char **, int *);
int clear_history(char *[]);
void  parse(char *, char **);
void  execute(char **);
int iter_pipes(char*** , int );

char*** separate_piped_commands(char **args, int *no_of_piped_commands)
{

	char*** piped_commands = (char*** )malloc(5 * sizeof(char**));
	int k = 0;	 
	int i,j=0;
	piped_commands[k] = (char** ) malloc(20 * sizeof(char*));

	for (i = 0; args[i] != NULL; i++) 
	{
		if (strcmp(args[i], "|") == 0)
		{
			k++;
			j=0;
			piped_commands[k] = (char** ) malloc(20 * sizeof(char*));
		}
		else
		{
			piped_commands[k][j] = (char* )malloc(100 *sizeof(char));
			strcpy(piped_commands[k][j], args[i]);
			j++;
		}
	}

	*no_of_piped_commands = k;
	return piped_commands;	
}	

int iter_pipes(char*** piped_commands, int n)
{
	int i, in, fd[2], status;
	pid_t pid = fork();
	if (pid < 0) {
		printf("Fork error");
		return -1;
	}
	else if (pid == 0) {
		in = 0;
		for (i = 0; i < n - 1; ++i) {
			pipe(fd);				
			close(fd[1]);	
			in = fd[0];
		}
		if (in != 0) {
			dup2(in, 0);
		}
		execvp (piped_commands[i][0],piped_commands[i]);
		exit(EXIT_FAILURE);
	}
	else {
		do {
			waitpid(pid, &status, WUNTRACED);
		}
		while (!WIFEXITED(status) && !WIFSIGNALED(status));
		return 1;
	}	
	return;
}	

int clear_history(char *hist[])
{
	int i;

	for (i = 0; i < HISTORY_COUNT; i++) {
		free(hist[i]);
		hist[i] = NULL;
	}

	return 0;
}

void  parse(char *line, char **argv)
{
	while (*line != '\0') {       
		while (*line == ' ' || *line == '\t' || *line == '\n')
			*line++ = '\0';     
		*argv++ = line;          
		while (*line != '\0' && *line != ' ' && *line != '\t' && *line != '\n')
			line++;            
	}
	*argv = '\0';
}

void  execute(char **argv)
{
	pid_t  pid;
	int    status;
	pid = fork();
	if (pid < 0) {     
		printf("ERROR: forking child process failed\n");
		return;
	}
	else if (pid == 0) {          
		if (execvp(argv[0], argv) < 0) {     
			printf("ERROR: exec failed\n");
			return;
		}
	}
	else {                                 
		while (wait(&status) != pid);
	}
}

void  main(void)
{
	int saved_stdout = dup(1);	
	dup2(STDOUT_FILENO, 1);
	char  line[1024];             
	char  *argv[64];
	char  *argv1[64];
	char ***temp;
	char *hist_temp;
	char *hist[HISTORY_COUNT];
	int i, current = 0, num_pipes=0;
	int in,out;
	char *input,*output;
	for (i = 0; i < HISTORY_COUNT; i++)
		hist[i] = NULL;   
	char *prev="";           
	while (1) {      
		in=0;  out=0;
		printf("Shell $ ");     
		gets(line);
		free(hist[current]);
		if(line[0]!= '!' && strcmp(prev,line) && line!=NULL)
		{	 hist[current] = strdup(line);
			prev=strdup(line);
			current = (current + 1) % HISTORY_COUNT;     
		}
		parse(line, argv);
		
		if(!strcmp(prev,""))
			continue;
		if (strcmp(argv[0], "exit") == 0)  
			return;
		else if (strcmp(line, "hc") == 0)
			clear_history(hist);    		    
		else
		{
			temp=separate_piped_commands(argv, &num_pipes);
			iter_pipes(temp, num_pipes+1);  
		}
	}
	dup2(saved_stdout, 1);
	close(saved_stdout);

}