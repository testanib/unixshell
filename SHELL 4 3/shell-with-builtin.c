//Benjamin Testani and Angelina Garguilo
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <glob.h>
#include <sys/wait.h>
#include "sh.h"
#include <utmpx.h>
#include <pthread.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_t p;

void sig_handler(int sig)
{
	fprintf(stdout, "\n>> ");
	fflush(stdout);
}

int toBackGround(char **args)
{
	for (int i = 0; args[i] != NULL; i++)
	{
		if (strstr(args[i], "&") && ((strstr(args[i], "<") || strstr(args[i], ">"))))
		{
			return 0;
		}
		if (strstr(args[i], "&"))
		{
			//args[i] = NULL;
			return 1;
		}
	}
	return 0;
}

void redirectCmd(char **arg, int append, int rstdout, int rstderr,
				 int noclobber, int arg_no, char *ptr)
{
	int fid;
	printf("cmd");
	if (!append && rstdout && !rstderr)
	{ // ">"
		if (noclobber)
		{
			if (access(arg[arg_no - 1], F_OK) != -1)
			{
				printf("File already exists \n");
				return;
			}
			else
			{
				fid = open(arg[arg_no - 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
			}
		}
		else
		{
			//close(STDOUT_FILENO);
			fid = open(arg[arg_no - 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
		}
	}
	else if (append && rstdout && !rstderr)
	{ // ">>"
		if (noclobber)
		{
			if (access(arg[arg_no - 1], F_OK) != -1)
			{
				fid = open(arg[arg_no - 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
			}
			else
			{
				printf("Cannot create a new file\n");
				return;
			}
		}
		else
		{
			fid = open(arg[arg_no - 1], O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP);
		}
	}
	close(1);
	//close(2);
	dup(fid);
	close(fid);
	printf("%s\n", ptr);			  // redirect stdout to file
	fid = open("/dev/tty", O_WRONLY); // redirect stdout
	close(1);
	//close(2);						  // back to terminal
	dup(fid);
	//dup(2);
	close(fid);
	//close(2);  //redirecting stderr doesnt work
	//waitpid(-1, NULL, WNOHANG);
}

void redirectExt(char **arg, int append, int rstdout, int rstderr,
				 int noclobber, int arg_no, char **execargs)
{
	int fid;
	if (!append && rstdout && !rstderr)
	{ // ">"
		if (noclobber)
		{
			if (access(arg[arg_no - 1], F_OK) != -1)
			{
				printf("File already exists \n");
				return;
			}
			else
			{
				fid = open(arg[arg_no - 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
			}
		}
		else
		{
			fid = open(arg[arg_no - 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
		}
		close(1);
		dup(fid);
		close(fid);
	}
	else if (append && rstdout && !rstderr)
	{ // ">>"
		if (noclobber)
		{
			if (access(arg[arg_no - 1], F_OK) != -1)
			{
				fid = open(arg[arg_no - 1], O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP);
			}
			else
			{
				printf("Cannot create a new file\n");
				return;
			}
		}
		else
		{
			//close(STDOUT_FILENO);
			fid = open(arg[arg_no - 1], O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP);
			//fid = open(arg[arg_no - 1], O_APPEND);
		}
		close(1);
		dup(fid);
		close(fid);
	}
	else if (!append && rstdout && rstderr)
	{ // >&
		if (noclobber)
		{
			if (access(arg[arg_no - 1], F_OK) != -1)
			{
				printf("File already exists \n");
				return;
			}
			else
			{
				fid = open(arg[arg_no - 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
			}
		}
		else
		{
			fid = open(arg[arg_no - 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
		}
		close(1);
		dup(fid);
		close(2);
		dup(fid);
		close(fid);
	}
	else if (append && rstdout && rstderr)
	{ // ">>&"
		if (noclobber)
		{
			if (access(arg[arg_no - 1], F_OK) != -1)
			{
				fid = open(arg[arg_no - 1], O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP);
			}
			else
			{
				printf("Cannot create a new file\n");
				return;
			}
		}
		else
		{
			//close(STDOUT_FILENO);
			fid = open(arg[arg_no - 1], O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP);
			//fid = open(arg[arg_no - 1], O_APPEND);
		}
		close(1);
		dup(fid);
		close(2);
		dup(fid);
		close(fid);
	}
}

int pipeCmd(char **args)
{
	int fd[2];

	pipe(fd);

	if (!fork())
	{
		close(1);
		dup(fd[1]);
		close(fd[0]);
		execlp(args[0], args[0], NULL);
	}
	else
	{
		close(0);
		dup(fd[0]);
		close(fd[1]);
		execlp(args[2], args[2], args[3], NULL);
	}
	//waitpid(-1, NULL, WNOHANG);
}

int main(int argc, char **argv, char **envp)
{
	char buf[MAXLINE];
	char *arg[MAXARGS]; // an array of tokens
	char *ptr;
	char *pch;
	pid_t pid;
	int status, i, arg_no, background, noclobber;
	int redirection, append, pipe, rstdin, rstdout, rstderr;
	struct pathelement *dir_list, *tmp;
	char *cmd;

	noclobber = 0; // initially default to 0
	background = 0;
	signal(SIGINT, sig_handler);

	fprintf(stdout, ">> "); /* print prompt (printf requires %% to print %) */
	fflush(stdout);
	while (fgets(buf, MAXLINE, stdin) != NULL)
	{
		if (strlen(buf) == 1 && buf[strlen(buf) - 1] == '\n')
			goto nextprompt; // "empty" command line

		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = 0; /* replace newline with null */

		// no redirection or pipe
		redirection = append = pipe = rstdin = rstdout = rstderr = 0;
		// check for >, >&, >>, >>&, <, |, and |&
		if (strstr(buf, ">>&"))
			redirection = append = rstdout = rstderr = 1;
		else if (strstr(buf, ">>"))
			redirection = append = rstdout = 1;
		else if (strstr(buf, ">&"))
			redirection = rstdout = rstderr = 1;
		else if (strstr(buf, ">"))
			redirection = rstdout = 1;
		else if (strstr(buf, "<"))
			redirection = rstdin = 1;
		else if (strstr(buf, "|&"))
			pipe = rstdout = rstderr = 1;
		else if (strstr(buf, "|"))
			pipe = rstdout = 1;

		/***
                printf("-%d-%d-%d-%d-%d-%d-\n",
		       redirection, append, pipe, rstdin, rstdout, rstderr);
           ***/

		// parse command line into tokens (stored in buf)
		arg_no = 0;
		pch = strtok(buf, " ");
		while (pch != NULL && arg_no < MAXARGS)
		{
			arg[arg_no] = pch;
			arg_no++;
			pch = strtok(NULL, " ");
		}
		arg[arg_no] = (char *)NULL;

		if (arg[0] == NULL) // "blank" command line
			goto nextprompt;

		background = 0; // not background process
		if (toBackGround(arg))
			background = 1; // to background this command
		else
			background = 0;

		// print tokens of this command line
		/***
		for (i = 0; i < arg_no; i++)
		  printf("arg[%d] = %s\n", i, arg[i]);
            ***/

		if (strcmp(arg[0], "exit") == 0)
		{ // built-in command exit
			printf("Executing built-in [exit]\n");
			exit(0);
		}
		if (strcmp(arg[0], "cd") == 0)
		{ // built-in command cd
			printf("Executing built-in [cd]\n");
			char *pa;
			if (arg_no == 1) // cd
				pa = getenv("HOME");
			else
				pa = arg[1]; // cd arg[1]
			if (chdir(pa) < 0)
				printf("%s: No such file or directory.\n", pa);
		}
		else if (strcmp(arg[0], "pwd") == 0)
		{ // built-in command pwd
			printf("Executing built-in [pwd]\n");
			ptr = getcwd(NULL, 0);
			if (redirection)
			{
				redirectCmd(arg, append, rstdout, rstderr, noclobber, arg_no, ptr);
			}
			else
				printf("%s\n", ptr); // no redirection
			free(ptr);
		}
		else if (strcmp(arg[0], "noclobber") == 0)
		{ // built-in command noclobber
			printf("Executing built-in [noclobber]\n");
			noclobber = 1 - noclobber; // switch value
			printf("%d\n", noclobber);
		}
		else if (strcmp(arg[0], "echo") == 0)
		{ // built-in command echo
			printf("Executing built-in [echo]\n");
			ptr = arg[1];
			if (redirection)
			{
				redirectCmd(arg, append, rstdout, rstderr, noclobber, arg_no, ptr);
			}
			else
			{
				for (i = 0; i < arg_no; i++)
					printf("arg[%d] = %s\n", i, arg[i]);
			}
		}
		else if (strcmp(arg[0], "which") == 0)
		{ // built-in command which
			printf("Executing built-in [which]\n");

			if (arg[1] == NULL)
			{ // "empty" which
				printf("which: Too few arguments.\n");
				goto nextprompt;
			}

			dir_list = get_path();

			cmd = which(arg[1], dir_list);
			if (cmd)
			{
				printf("%s\n", cmd);
				free(cmd);
			}
			else // argument not found
				printf("%s: Command not found\n", arg[1]);

			while (dir_list)
			{ // free list of path values
				tmp = dir_list;
				dir_list = dir_list->next;
				free(tmp->element);
				free(tmp);
			}
		}
		else if (strcmp(arg[0], "pid") == 0)
		{
			printf("%d \n", getpid());
		}
		// add other built-in commands here
		else
		{ // external command
			if ((pid = fork()) < 0)
			{
				printf("fork error");
			}
			else if (pid == 0)
			{ /* child */
				// an array of aguments for execve()
				if (arg[1] && strstr(arg[1], "|"))
				{
					printf("pipe");
					pipeCmd(arg);
				}
				char *execargs[MAXARGS];
				glob_t paths;
				int csource, j;
				char **p;

				if (arg[0][0] != '/' && strncmp(arg[0], "./", 2) != 0 && strncmp(arg[0], "../", 3) != 0)
				{ // get absoulte path of command
					dir_list = get_path();
					cmd = which(arg[0], dir_list);
					if (cmd)
					{
						printf("Executing [%s]\n", cmd);
					}
					else
					{ // argument not found
						printf("%s: Command not found\n", arg[1]);
						goto nextprompt;
					}

					while (dir_list)
					{ // free list of path values
						tmp = dir_list;
						dir_list = dir_list->next;
						free(tmp->element);
						free(tmp);
					}
					execargs[0] = malloc(strlen(cmd) + 1);
					strcpy(execargs[0], cmd); // copy "absolute path"
					free(cmd);
				}
				else
				{
					execargs[0] = malloc(strlen(arg[0]) + 1);
					strcpy(execargs[0], arg[0]); // copy "command"
				}

				j = 1;
				for (i = 1; i < arg_no; i++)
				{ // check arguments
					if (strchr(arg[i], '*') != NULL)
					{ // wildcard!
						csource = glob(arg[i], 0, NULL, &paths);
						if (csource == 0)
						{
							for (p = paths.gl_pathv; *p != NULL; ++p)
							{
								execargs[j] = malloc(strlen(*p) + 1);
								strcpy(execargs[j], *p);
								j++;
							}
							globfree(&paths);
						}
						else if (csource == GLOB_NOMATCH)
						{
							execargs[j] = malloc(strlen(arg[i]) + 1);
							strcpy(execargs[j], arg[i]);
							j++;
						}
					}
					else
					{
						execargs[j] = malloc(strlen(arg[i]) + 1);
						strcpy(execargs[j], arg[i]);
						j++;
					}
				}
				execargs[j] = NULL;

				if (background)
				{ // get rid of argument "&"
					j--;
					free(execargs[j]);
					execargs[j] = NULL;
				}

				// print arguments into execve()
				/***
		        for (i = 0; i < j; i++)
		          printf("exec arg[%d] = %s\n", i, execargs[i]);
                     ***/
				if (redirection)
				{
					if (pid == 0)
					{
						redirectExt(arg, append, rstdout, rstderr, noclobber, arg_no, execargs);
						execve(execargs[0], execargs, NULL);
					}
				}
				else
				{
					execve(execargs[0], execargs, NULL);
					printf("couldn't execute: %s", buf);
					exit(127);
				}
			}

			/* parent */
			waitpid(-1, &status, WNOHANG);
			if (!background)
			{ // foreground process
				if ((pid = waitpid(pid, &status, 0)) < 0)
					printf("waitpid error");
			}
			else
			{ // no waitpid if background
				printf("+[%d]\n", pid);
				background = 0;
				if ((pid = waitpid(pid, &status, WNOHANG)) < 0)
					printf("waitpid error");
			}
			/**
                  if (WIFEXITED(status)) S&R p. 239 
                    printf("child terminates with (%d)\n", WEXITSTATUS(status));
**/
		}

	nextprompt:
		fprintf(stdout, ">> ");
		fflush(stdout);
	}
	exit(0);
}
