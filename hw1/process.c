#include "process.h"
#include "shell.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <termios.h>
#include <stdbool.h>

// return false if the no process with pid can be found or if pid is 0
bool mark_status(pid_t pid, int status) {
  if (pid == 0) return false;
  process *p = first_process;
  while (p && p->pid != pid) p = p->next;
  if (p) {
    p->status = status;
    if (WIFSTOPPED(p->status)) {
      p->stopped = true;
    } else {
      p->completed = true;
    }
    return true;
  }
  return false;
}

// wait for process p to finish and also mark other processes that finish before p
void wait_for_process(process *p) {
  int status;
  pid_t pid;
  do {
    // while waiting for p to stop/complete, also update other finished processes
    // once p actually finishes, while loop will stop running
    pid = waitpid(WAIT_ANY, &status, WUNTRACED);
  } while(mark_status(pid, status) && !p->stopped && !p->completed);
  if (p->stopped) {
    printf("PID: %d stopped.\n", p->pid);
  }
}

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
  tcsetpgrp(shell_terminal, p->pid);
  if (cont) {
    tcsetattr(shell_terminal, TCSADRAIN, &p->tmodes);
    if (kill(- p->pid, SIGCONT) < 0) {
      perror("kill (SIGCONT)");
    }
  }
  // wait for process to finish
  wait_for_process(p);
  // put shell back in foreground
  tcsetpgrp(shell_terminal, shell_pgid);
  // save process's terminal modes
  tcgetattr(shell_terminal, &p->tmodes);
  // restore shell's terminal modes
  tcsetattr(shell_terminal, TCSADRAIN, &shell_tmodes);
}

/* Put a process in the background. If the cont argument is true, send
 * the process group a SIGCONT signal to wake it up. */
void put_process_in_background (process *p, int cont) {
  printf("PID: %d in background.\n", p->pid);
  if (cont) {
    if (kill(- p->pid, SIGCONT) < 0) {
      perror("kill (SIGCONT)");
    }
  }
}
