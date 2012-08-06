#include <signal.h>
#include <unistd.h>
#include "printsig.h"

void print_sig_desc(int sig)
{
  switch (sig) {
  case SIGBUS:
    write(STDOUT_FILENO, "\nSIGBUS:  access to undefined portion of a memory object\n", 57);
    break;
  case SIGFPE:
    write(STDOUT_FILENO, "\nSIGFPE:  erroneous arithmetic operation\n", 41);
    break;
  case SIGILL:
    write(STDOUT_FILENO, "\nSIGILL:  illegal instruction\n", 30);
    break;
  case SIGPIPE:
    write(STDOUT_FILENO, "\nSIGPIPE:  write on a pipe with no reader\n", 42);
    break;
  case SIGSEGV:
    write(STDOUT_FILENO, "\nSIGSEGV:  invalid memory reference\n", 36);
    break;
  case SIGSYS:
    write(STDOUT_FILENO, "\nSIGSYS:  bad system call\n", 26);
    break;
  case SIGXCPU:
    write(STDOUT_FILENO, "\nSIGXCPU:  CPU-time limit exceeded\n", 35);
    break;
  case SIGXFSZ:
    write(STDOUT_FILENO, "\nSIGXFSZ:  file-size limit exceeded\n", 36);
    break;
  case SIGABRT:
    write(STDOUT_FILENO, "\nSIGABRT:  call to abort\n", 25);
    break;
  case SIGHUP:
    write(STDOUT_FILENO, "\nSIGHUP:  hangup\n", 17);
    break;
  case SIGINT:
    write(STDOUT_FILENO, "\nSIGINT:  interrupt (from keyboard)\n", 36);
    break;
  case SIGKILL:
    write(STDOUT_FILENO, "\nSIGKILL:  kill; synthetic only\n", 32);
    break;
  case SIGQUIT:
    write(STDOUT_FILENO, "\nSIGQUIT:  quit (from keyboard)\n", 32);
    break;
  case SIGTERM:
    write(STDOUT_FILENO, "\nSIGTERM:  termination; synthetic only\n", 39);
    break;
  case SIGUSR1:
    write(STDOUT_FILENO, "\nSIGUSR1:  user signal 1; synthetic only\n", 41);
    break;
  case SIGUSR2:
    write(STDOUT_FILENO, "\nSIGUSR2:  user signal 2; synthetic only\n", 41);
    break;
  case SIGCHLD:
    write(STDOUT_FILENO, "\nSIGCHLD:  child process terminated or stopped\n", 47);
    break;
  case SIGCONT:
    write(STDOUT_FILENO, "\nSIGCONT:  continue executing (from keyboard)\n", 46);
    break;
  case SIGSTOP:
    write(STDOUT_FILENO, "\nSIGSTOP:  stop executing (from keyboard)\n", 42);
    break;
  case SIGTSTP:
    write(STDOUT_FILENO, "\nSIGTSTP:  terminal stop signal (from keyboard)\n", 48);
    break;
  case SIGTTIN:
    write(STDOUT_FILENO, "\nSIGTTIN:  background process attempting read\n", 46);
    break;
  case SIGTTOU:
    write(STDOUT_FILENO, "\nSIGTTOU:  background process attemping write\n", 46);
    break;
  case SIGALRM:
    write(STDOUT_FILENO, "\nSIGALRM:  alarm clock expired\n", 31);
    break;
  case SIGVTALRM:
    write(STDOUT_FILENO, "\nSIGVTALRM:  virtual timer expired\n", 35);
    break;
  case SIGPROF:
    write(STDOUT_FILENO, "\nSIGPROF:  profiling timer expired\n", 35);
    break;
  case SIGPOLL:
    write(STDOUT_FILENO, "\nSIGPOLL:  pollable event\n", 26);
    break;
  case SIGTRAP:
    write(STDOUT_FILENO, "\nSIGTRAP:  trace/breakpoint trap\n", 33);
    break;
  case SIGURG:
    write(STDOUT_FILENO, "\nSIGURG:  out-of-band data available at socket\n", 47);
    break;
  default:
    write(STDOUT_FILENO, "\nUnknown signal\n", 16);
  }
}

