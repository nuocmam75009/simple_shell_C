#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define HISTORY_SIZE 100

extern char **environ;
char *history[HISTORY_SIZE];
int history_count = 0;

void add_to_history(char *command)
{
	if (history_count < HISTORY_SIZE)
	{
		history[history_count] = strdup(command);
	}
	else
	{
		free(history[0]);
		memmove(history, history + 1, (HISTORY_SIZE - 1) * sizeof(char *));
		history[HISTORY_SIZE - 1] = strdup(command);
	}
}

void print_history()
{
	for (int i = 0; i < history_count; i++)
	{
		printf("%d %s\n", i + 1, history[i]);
	}
}

int execute_builtin(char **args)
{
	if (strcmp(args[0], "exit") == 0)
	{
		exit(0);
	}
	else if (strcmp(args[0], "cd") == 0)
	{
		if (args[1] == NULL)
		{
			fprintf(stderr, "cd: expected argument to \"cd\"\n");
		}
		else if (chdir(args[1]) != 0)
		{
			perror("cd");
		}
		return (1);
	}
	else if (strcmp(args[0], "help") == 0)
	{
		printf("Simple Shell\n");
		printf("Type program names and arguments, and hit enter.\n");
		printf("The following are built in:\n");
		printf("cd\n");
		printf("exit\n");
		printf("help\n");
		return (1);
	}
	else if (strcmp(args[0], "history") == 0)
	{
		print_history();
		return (1);
	}
	else if (strcmp(args[0], "setenv") == 0)
	{
		if (args[1] && args[2])
		{
			if (setenv(args[1], args[2], 1) != 0)
			{
				perror("setenv");
			}
		}
		else
		{
			fprintf(stderr, "setenv: expected arguments\n");
		}
		return (1);
	}
	else if (strcmp(args[0], "unsetenv") == 0)
	{
		if (args[1])
		{
			if (unsetenv(args[1]) != 0)
			{
				perror("unsetenv");
			}
		}
		else
		{
			fprintf(stderr, "unsetenv: expected argument\n");
		}
		return (1);
	}
	else if (strcmp(args[0], "printenv") == 0)
	{
		for (char **env = environ; *env; ++env)
		{
			printf("%s\n", *env);
		}
		return (1);
	}
	return (0);
}

char **parse_input(char *input)
{
	int buffsize = 64, position = 0;
	char **tokens = malloc(buffsize * sizeof(char *));
	char *token;

	if (!tokens)
	{
		fprintf(stderr, "simple_shell: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(input, " ");
	while (token != NULL)
	{
		tokens[position++] = token;

		if (position >= buffsize)
		{
			buffsize += 64;
			tokens = realloc(tokens, buffsize * sizeof(char *));
			if (!tokens)
			{
				fprintf(stderr, "simple_shell: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, " ");
	}
	tokens[position] = NULL;
	return (tokens);
}

void execute_commands(char **args)
{
	pid_t pid = fork();
	if (pid == -1)
	{
		perror("fork failed.");
		return;
	}

	if (pid == 0)
	{
		if (execvp(args[0], args) == -1)
		{
			perror("Error.");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		int status;
		waitpid(pid, &status, 0);
	}
}

void simple_shell(void)
{
	char *buffer = NULL;
	size_t buffer_size = 0;
	ssize_t line_size;

	while (1)
	{
		printf("simple_shell$ ");
		fflush(stdout);

		line_size = getline(&buffer, &buffer_size, stdin);
		if (line_size == -1)
		{
			printf("\n");
			break;
		}

		buffer[strcspn(buffer, "\n")] = '\0';

		if (strlen(buffer) == 0)
			continue;

		char **args = parse_input(buffer);

		if (execute_builtin(args))
		{
			free(args);
			continue;
		}

		execute_commands(args);
		free(args);
	}

	free(buffer);
}

int main(void)
{
	simple_shell();
	return (0);
}
