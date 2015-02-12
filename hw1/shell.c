#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>

#define FALSE 0
#define TRUE 1
#define INPUT_STRING_SIZE 80

#include "io.h"
#include "parse.h"
#include "process.h"
#include "shell.h"

int asprintf(char **strp, const char *fmt, ...);
int cmd_help(tok_t arg[]);
int cmd_change_dir(tok_t arg[]);
int cmd_fg(tok_t arg[]);
int cmd_bg(tok_t arg[]);
int cmd_wait(tok_t arg[]);
process *find_process(pid_t pid);


int cmd_quit(tok_t arg[]) {
  if (!first_process) {
    printf("Bye\n");
    exit(0);
  }
  return 1;
}

/* Command Lookup table */
typedef int cmd_fun_t (tok_t args[]); /* cmd functions take token array and return int */
typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
  {cmd_help, "?", "show this help menu"},
  {cmd_quit, "quit", "quit the command shell"},
  {cmd_change_dir, "cd", "change the current working directory"},
  {cmd_fg, "fg", "move the process with id pid to the foreground"},
  {cmd_bg, "bg", "move the process with id pid to the background"},
  {cmd_wait, "wait", "wait until all backgrounded jobs have terminated before returning to the prompt."},
};

int cmd_help(tok_t arg[]) {
  int i;
  for (i=0; i < (sizeof(cmd_table)/sizeof(fun_desc_t)); i++) {
    printf("%s - %s\n",cmd_table[i].cmd, cmd_table[i].doc);
  }
  return 1;
}

int cmd_change_dir(tok_t arg[]) {
  chdir(*arg);
  return 1;
}

int manage_process(tok_t arg[], bool foreground) {
  process *p;
  pid_t pid = 0;
  if (arg[0]) pid = (pid_t)atoi(arg[0]);
  p = find_process(pid);  
  if (!p) {
    printf("No such process: ");
    if (pid == 0) printf("most recently launched.\n");
    else printf("%d.\n", pid);
  } else {
    if (foreground) {
      p->background = false;
      p->stopped = false;
      put_process_in_foreground(p, true);
    } else {
      p->background = true;
      put_process_in_background(p, false);
    }
  }
  return 1;
}

int cmd_fg(tok_t arg[]) {
  return manage_process(arg, true);
}

int cmd_bg(tok_t arg[]) {
  return manage_process(arg, false);
}

int cmd_wait(tok_t arg[]) {
  wait_all();
  return 1;
}

int lookup(char cmd[]) {
  int i;
  for (i=0; i < (sizeof(cmd_table)/sizeof(fun_desc_t)); i++) {
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0)) return i;
  }
  return -1;
}

void init_shell()
{
  /* Check if we are running interactively */
  shell_terminal = STDIN_FILENO;

  /** Note that we cannot take control of the terminal if the shell
      is not interactive */
  shell_is_interactive = isatty(shell_terminal);

  if(shell_is_interactive){

    /* force into foreground */
    while(tcgetpgrp (shell_terminal) != (shell_pgid = getpgrp()))
      kill( - shell_pgid, SIGTTIN);

    shell_pgid = getpid();
    /* Put shell in its own process group */
    if(setpgid(shell_pgid, shell_pgid) < 0){
      perror("Couldn't put the shell in its own process group");
      exit(1);
    }

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);
    tcgetattr(shell_terminal, &shell_tmodes);
  }
  /** YOUR CODE HERE */
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
  signal(SIGCHLD, SIG_IGN);
}

char* resolve_path(char *fname) {
  if (!fname) return NULL;
  char *ORIGINAL_PATH = getenv("PATH");
  char *PATH;
  asprintf(&PATH, "%s", ORIGINAL_PATH);
  tok_t *t = getToks(PATH);
  int i = 0;
  for (i=0; t[i]; i++) {
    char *full_path;
    asprintf(&full_path, "%s/%s", t[i], fname);
    if (access(full_path, F_OK) != -1) {
      // file exists
      return full_path;
    }
  }
  printf("No command '%s' found.\n", fname);
  return NULL;
}

/**
 * Add a process to our process list
 */
 void add_process(process* p) {
  if (!first_process) {
    first_process = p;
  } else {
    process *last_process = first_process;
    while (last_process->next && (last_process = last_process->next));
    last_process->next = p;
    p->prev = last_process;
    p->next = NULL;  
  }
}

process *find_process(pid_t pid) {
  process *last_process = first_process;
  if (last_process) {
    if (pid) {
      while (last_process->pid != pid && (last_process = last_process->next));   
    } else {
      while (last_process->next) last_process = last_process->next;
    }
  }
  return last_process;
}

/**
 * Creates a process given the inputString from stdin
 */
 process* create_process(char* inputString) {

  // create process on heap
  process *p = malloc(sizeof(process));
  // look for background flag
  char *amp = strchr(inputString, '&');
  if (amp) {
    int i = (int)(amp - inputString);
    p->background = true;
    printf("%s\n", "Process marked background");
    char *old_string = inputString;
    inputString = malloc(sizeof(char)*(i+1));
    inputString[i] = '\0';
    strncpy(inputString, old_string, sizeof(char)*i);
  }
  // tokenize command line input
  tok_t *t = getToks(inputString); /* break the line into tokens */
  // figure out argv, argc and io
  int argc;
  bool io_found = false;
  p->argv = t;
  p->stdin = STDIN_FILENO;
  p->stdout = STDOUT_FILENO;
  p->stderr = STDERR_FILENO;

  for (argc=0; t[argc]; ++argc) {
    if (!io_found) {
      // figure out argv
      if (strcmp(t[argc], ">")==0 || strcmp(t[argc], "<")==0) {
        // copy every token before the pipe sign
        p->argv = malloc(sizeof(tok_t)*argc);
        memcpy(p->argv, t, sizeof(tok_t)*argc);
        io_found = true;
      }
      // figure out io
      if (strcmp(t[argc], ">")==0) {
        tok_t outfile = t[argc+1];
        p->stdout = fileno(fopen(outfile, "w"));
      } else if (strcmp(t[argc], "<")==0) {
        tok_t infile = t[argc+1];
        p->stdin = fileno(fopen(infile, "r"));
      }
    }
  }
  // set argument count
  p->argc = argc;
  // resolve file path
  bool access_ok = (access(p->argv[0], F_OK) != -1);
  if (!access_ok) {
    p->argv[0] = resolve_path(p->argv[0]);
    if (!p->argv[0]) {
      return NULL;
    }
  }
  return p;
}

void run_program(tok_t *t, char *s) {
  // execute another program specified in command
  process* proc;
  if ((proc = create_process(s)) != NULL) {
    add_process(proc);
    pid_t pid = fork();
    // both parent and child should set put the child into its own process group
    if (pid == 0) {
      proc->pid = getpid();
      launch_process(proc);
    } else {
      proc->pid = pid;
      if (shell_is_interactive) {
        setpgid(proc->pid, proc->pid);
      }
      if (proc->background) {
        put_process_in_background(proc, false);
      } else {
        put_process_in_foreground(proc, false);
      }
    }
  }
}

int shell (int argc, char *argv[]) {
  char *s = malloc(INPUT_STRING_SIZE+1);			/* user input string */
  tok_t *t;			/* tokens parsed from input */
  int lineNum = 0;
  int fundex = -1;
  pid_t pid = getpid();		/* get current processes PID */
  pid_t ppid = getppid();	/* get parents PID */
  // pid_t cpid, tcpid, cpgid;

  init_shell();

  printf("%s running as PID %d under %d\n",argv[0],pid,ppid);

  lineNum=0;
  char *cwd = getcwd(NULL, 0);
  fprintf(stdout, "%d [%s]:", lineNum, cwd);
  free(cwd);
  while ((s = freadln(stdin))){
    // printf("%s\n", s);
    char *s_copy;
    asprintf(&s_copy, "%s", s);

    t = getToks(s); /* break the line into tokens */
    fundex = lookup(t[0]); /* Is first token a shell literal */
    if(fundex >= 0) cmd_table[fundex].fun(&t[1]);
    else {
      run_program(t, s_copy);
    }
    ++lineNum;
    cwd = getcwd(NULL, 0);
    fprintf(stdout, "%d [%s]:", lineNum, cwd);
    free(cwd);
  }
  return 0;
}
