#include "process.h"
#include "shell.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <termios.h>

/**
 * Executes the process p.
 * If the shell is in interactive mode and the process is a foreground process,
 * then p should take control of the terminal.
 */
 void launch_process(process *p) {
  if (shell_is_interactive) {
    // put this process into its own process group
    pid_t pid = p->pid;
    pid_t pgid = pid;
    if (setpgid(pid, pgid) < 0) {
      perror("Couldn't put the program in its own process group");
      exit(1);
    }
    // foreground processes get control of the terminal
    if (!p->background) {
      tcsetpgrp(shell_terminal, pgid);
    }
    // restore signal handling for this process
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
  }
  // redirect in >
  if (p->stdin != STDIN_FILENO) {
    dup2(p->stdin, STDIN_FILENO);
    close(p->stdin);
  }
  if (p->stdout != STDOUT_FILENO) {
    dup2(p->stdout, STDOUT_FILENO);
    close(p->stdout);
  }
  if (p->stderr != STDERR_FILENO) {
    dup2(p->stderr, STDERR_FILENO);
    close(p->stderr);
  }
  // replace the process, make sure we exit if something is wrong
  execv(p->argv[0], p->argv);
  perror ("execv");
  exit (1);
}

/* Put a process in the foreground. This function assumes that the shell
 * is in interactive mode. If the cont argument is true, send the process
 * group a SIGCONT signal to wake it up.
 */
 void put_process_in_foreground (process *p, int cont) {
  /** YOUR CODE HERE */
 }

/* Put a process in the background. If the cont argument is true, send
 * the process group a SIGCONT signal to wake it up. */
 void put_process_in_background (process *p, int cont) {
  /** YOUR CODE HERE */
 }
