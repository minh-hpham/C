/* This is the main file for the `whoosh` interpreter and the part
   that you modify. */

#include <stdlib.h>
#include <stdio.h>
#include "csapp.h"
#include "ast.h"
#include "fail.h"

static void run_script(script *scr);
static void run_group(script_group *group);
//static void run_command(script_command *command);
static int run_command(script_command *command, int fds_write[], int fds_read[]);
static int run_command1(script_command *command);
static void single_command(script_group *group);
static void and_command(script_group *group);
static void or_command(script_group *group);
static void set_var(script_var *var, int new_value);
static void read_to_var(int fd, script_var *var);
static void write_var_to(int fd, script_var *var);

/* You probably shouldn't change main at all. */

int main(int argc, char **argv) {
  script *scr;
  
  if ((argc != 1) && (argc != 2)) {
    fprintf(stderr, "usage: %s [<script-file>]\n", argv[0]);
    exit(1);
  }

  scr = parse_script_file((argc > 1) ? argv[1] : NULL);

  run_script(scr);

  return 0;
}

static void run_script(script *scr) {
  /* You'll have to make run_script do better than this */
  int i;
  for (i = 0; i < scr->num_groups; i++) {
    run_group(&scr->groups[i]);
  }

}

static void run_group(script_group *group) {
  /* You'll have to make run_group do better than this, too */
  int repeats = group->repeats;
  int i;
  for (i = 0; i < repeats; i++) {
    if (group->mode == 0) {
      single_command(group);
      //single_command(group);
    } else if (group->mode == 1) {
      and_command(group);
    } else if (group->mode == 2) {
      or_command(group);
    } 
  }
}

/* This run_command function is a good start, but note that it runs
   the command as a replacement for the `whoosh` script, instead of
   creating a new process. */

static int run_command1(script_command *command) {
  const char **argv;
  int i;
  int fds_read[2], fds_write[2];
  argv = malloc(sizeof(char *) * (command->num_arguments + 2));
  argv[0] = command->program;
  
  for (i = 0; i < command->num_arguments; i++) {
    if (command->arguments[i].kind == ARGUMENT_LITERAL)
      argv[i+1] = command->arguments[i].u.literal;
    else
      argv[i+1] = command->arguments[i].u.var->value;
  }
  
  argv[command->num_arguments + 1] = NULL;
  
  Pipe(fds_read);
  Pipe(fds_write);
  pid_t pid = Fork();
  // child
  if (pid==0) {
    if (command->input_from != NULL) {
      Dup2(fds_read[0], 0);
      Close(fds_read[1]);
    } else {
      Close(fds_read[0]);
      Close(fds_read[1]);
    }
    if (command->output_to != NULL) {
      Dup2(fds_write[1], 1);
      Close(fds_write[0]);
    } else {
      Close(fds_write[0]);
      Close(fds_write[1]);
    }
    Execve(argv[0], (char * const *)argv, environ); 
  } else {
    // parent
    if (command->input_from != NULL) {
      Close(fds_read[0]); 
      write_var_to(fds_read[1], command->input_from);
      Close(fds_read[1]);    
    } else {
      Close(fds_read[0]);
      Close(fds_read[1]);
    }
    if (command->output_to != NULL) {
      Close(fds_write[1]);
      read_to_var(fds_write[0], command->output_to);   
      Close(fds_write[0]);   
    } else {
      Close(fds_write[0]);
      Close(fds_write[1]);
    }
  }
  if (command->pid_to != NULL) {
    set_var(command->pid_to,pid);
  }

  free(argv);
  return pid;
}
static int run_command(script_command *command, int fds_write[], int fds_read[]){
  const char **argv;
  int i;

  argv = malloc(sizeof(char *) * (command->num_arguments + 2));
  argv[0] = command->program;
  
  for (i = 0; i < command->num_arguments; i++) {
    if (command->arguments[i].kind == ARGUMENT_LITERAL)
      argv[i+1] = command->arguments[i].u.literal;
    else
      argv[i+1] = command->arguments[i].u.var->value;
  }
  
  argv[command->num_arguments + 1] = NULL;
  

  pid_t pid = Fork();
  // child
  if (pid==0) {
    if (command->input_from != NULL) {
      Dup2(fds_read[0], 0);
      Close(fds_read[1]);
    } else {
      Close(fds_read[0]);
      Close(fds_read[1]);
    }
    if (command->output_to != NULL) {
      Dup2(fds_write[1], 1);
      Close(fds_write[0]);
    } else {
      Close(fds_write[0]);
      Close(fds_write[1]);
    }
    Execve(argv[0], (char * const *)argv, environ); 
  } 
  if (command->pid_to != NULL) {
    set_var(command->pid_to,pid);
  }
  free(argv);
  return pid;
}
static void single_command(script_group *group) {
  int status;
  script_command *command = &group->commands[0];
  pid_t pid = run_command1(command);

  Waitpid(pid, &status, 0);
  if (WIFSIGNALED(status) == 1) {
    set_var(command->output_to,(-1)*WTERMSIG(status));
  }
  else if (WIFEXITED(status) && WEXITSTATUS(status) > 0) {
  	set_var(command->output_to,WEXITSTATUS(status));
  }        
}

static void and_command(script_group *group) {
  int num_commands = group->num_commands;
  pid_t command_pid[num_commands];
  int command_i,status;
      
  for (command_i = 0 ; command_i < num_commands; ++command_i) {
    command_pid[command_i] = run_command1(&group->commands[command_i]);
  }

  for (command_i = 0 ; command_i < num_commands; ++command_i) {
    script_command *command = &group->commands[command_i];
    Waitpid(command_pid[command_i], &status, 0);
    if (command->output_to != NULL) {
      if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
		set_var(command->output_to,WEXITSTATUS(status));
      } else if (WIFSIGNALED(status) == 1) {
		set_var(command->output_to,(-1)*WTERMSIG(status));
      }     
    }
  }

}

static void or_command(script_group *group) {
  struct data
  {
    pid_t pid;
    int fds_read[2];
    int fds_write[2];
  };

  int num_commands = group->num_commands;

  struct data all_data[num_commands];
  
  int command_i;
  int i,status;
  int fds_read[2], fds_write[2];

  for (command_i = 0 ; command_i < num_commands; ++command_i) {
    Pipe(fds_read);
    Pipe(fds_write);

    script_command *command = &group->commands[command_i];
    pid_t pid = run_command(command, fds_write, fds_read);
  
    all_data[command_i].pid = pid;
    all_data[command_i].fds_read[0] = fds_read[0];
    all_data[command_i].fds_read[1] = fds_read[1];
    all_data[command_i].fds_write[0] = fds_write[0];
    all_data[command_i].fds_write[1] = fds_write[1];

    for (i = 0 ; i < 2 ; ++i) {
      fds_read[i] = 0;
      fds_write[i] = 0;
    }

  }
  pid_t first = Wait(&status);

  for (command_i = 0 ; command_i < num_commands; ++command_i) {
    pid_t current = all_data[command_i].pid;
    fds_read[0] = all_data[command_i].fds_read[0];
    fds_read[1] = all_data[command_i].fds_read[1];
    fds_write[0] = all_data[command_i].fds_write[0];
    fds_write[1] = all_data[command_i].fds_write[1];    
    if ( current != first) {
      Close(fds_read[0]); 
      Close(fds_read[1]);        

      int exit;
      kill(all_data[command_i].pid, SIGTERM);
      Waitpid(all_data[command_i].pid, &exit, WUNTRACED);
    } else {
      script_command *command = &group->commands[command_i];

      if (command->input_from != NULL) {
        Close(fds_read[0]); 
        write_var_to(fds_read[1], command->input_from);
        Close(fds_read[1]);    
      } else {
        Close(fds_read[0]);
        Close(fds_read[1]);
      }
      if (command->output_to != NULL) {
        Close(fds_write[1]);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
          read_to_var(fds_write[0], command->output_to);
        }else if (WIFSIGNALED(status) == 1) {
          set_var(command->output_to,(-1)*WTERMSIG(status));
        }
        else {
          set_var(command->output_to,WEXITSTATUS(status));
        }      
        
        Close(fds_write[0]);   
      } else {
        Close(fds_write[0]);
        Close(fds_write[1]);
      }      
    }
  }

 
}

/* You'll likely want to use this set_var function for converting a
   numeric value to a string and installing it as a variable's
   value: */
static void set_var(script_var *var, int new_value) {
  char buffer[32];
  free((void*)var->value);
  snprintf(buffer, sizeof(buffer), "%d", new_value);
  var->value = strdup(buffer);
}

/* You'll likely want to use this write_var_to function for writing a
   variable's value to a pipe: */
static void write_var_to(int fd, script_var *var) {
  size_t len = strlen(var->value);
  ssize_t wrote = Write(fd, var->value, len);
  wrote += Write(fd, "\n", 1);
  if (wrote != len + 1)
    app_error("didn't write all expected bytes");
}

/* You'll likely want to use this write_var_to function for reading a
   pipe's content into a variable: */
static void read_to_var(int fd, script_var *var) {
  size_t size = 4097, amt = 0;
  char buffer[size];
  ssize_t got;

  while (1) {
    got = Read(fd, buffer + amt, size - amt);
    if (!got) {
      if (amt && (buffer[amt-1] == '\n'))
        amt--;
      buffer[amt] = 0;
      free((void*)var->value);
      var->value = strdup(buffer);
      return;
    }
    amt += got;
    if (amt > (size - 1))
      app_error("received too much output");
  }
}

