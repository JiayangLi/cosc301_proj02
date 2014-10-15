#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include "list.h"

/*Adapted from previous lab*/
char** tokenify(const char *s, const char *delimiters) {
    char *copy = strdup(s); //first copy to get the size 
    char *token = strtok(copy, delimiters);

    int counts = 1; //account for NULL

    for (; token != NULL; token = strtok(NULL, delimiters)){
        counts++;
    }

    free(copy);

    char **rv = malloc(counts * sizeof(char *));

    char *copy1 = strdup(s);    //second copy to tokenify
    
    int i = 0;
    token = strtok(copy1, delimiters);

    while (token != NULL){
        *(rv + i) = strdup(token);  //rv[i]
        token = strtok(NULL, delimiters);
        i++;
    }
    *(rv + i) = NULL;

    free(copy1);

    return rv;
}

/*borrowed from previous lab*/
void free_tokens(char **tokens) {
    int i = 0;
    while (tokens[i] != NULL) {
        free(tokens[i]); // free each string
        i++;
    }
    free(tokens); // then free the array
}

/*Remove comment from tokens by replacing the first
comment token by NULL*/
void remove_comment(char **tokens){
	for (int i = 0; tokens[i] != NULL; i++){
		if (tokens[i][0] == '#'){
			tokens[i] = NULL;
			return;
		}
	}
}

char ***get_commands(char **tokens){
    int count = 1;
    for (int i = 0; tokens[i] != NULL; i++){
        count++;
    }

    char ***rv = malloc(count * sizeof(char **));

    int j = 0;
    for (int i = 0; tokens[i] != NULL; i++){
        char **temp = tokenify(tokens[i], " \t\n");
        if (temp[0] != NULL){ //add non-empty input
            rv[j] = temp;
            j++;
        }
        else{
            free_tokens(temp); //free empty input
        }
    }

    rv[j] = NULL;
    return rv;
}

void free_commands(char ***cmd){
    for (int i = 0; cmd[i] != NULL; i++){
        free_tokens(cmd[i]);
    }
    free(cmd);
}

int get_input(char *buffer){
    while (fgets(buffer, 1024, stdin) != NULL){
        return 0; //0-normal input
    }
    return 1; //1-eof or error
}

int check_exit(char **cmd, int *p_done, struct node *head){
    if (strcasecmp(cmd[0], "exit") == 0){
        if (head != NULL){
            fprintf(stderr, "there are jobs running in the background. Can't exit.\n");
        }
        else{
            *p_done = 1; //no background jobs, exit allowed
        }
        return 1; //0-exit command found
    }
    return 0; //1-exit command not found
}

int check_mode(char **cmd, int *p_mode){
    if (strcasecmp(cmd[0], "mode") == 0){
        if (cmd[1] == NULL){
            if (*p_mode == 1){
                printf("The current mode is sequential.\n");
            }
            else{
                printf("The current mode is parallel.\n");
            }
        } //end of cmd[1]==NULL if

        else if (strcasecmp(cmd[1], "sequential") == 0 || strcasecmp(cmd[1], "s") == 0){
            *p_mode = 1;
        }

        else if (strcasecmp(cmd[1], "parallel") == 0 || strcasecmp(cmd[1], "p") == 0){
            *p_mode = 0;
        }

        else{
            fprintf(stderr, "Error: invalid mode!\n");
        }

        return 1; //0-mode command found
    }

    return 0; //1-mode command not found
}

int check_jobs(char **cmd, struct node *head){
    if (strcasecmp(cmd[0], "jobs") == 0){
        if (cmd[1] == NULL){
            list_print(head);
        }
        else{
            fprintf(stderr, "Error: jobs command doesn't have any arguments\n");
        }
        return 1; //jobs command found
    }
    return 0; //jobs command not found
}

int check_pause_resume(char **cmd, struct node *head, char *to_do){
    if (strcasecmp(cmd[0], to_do) == 0){
        if (cmd[2] != NULL){
            fprintf(stderr, "Error: %s command only takes one argument\n", to_do);
            return 1;
        }

        char

        if (strcasecmp(to_do, "resume") == 0){
            to_do = "run";
        }

        int update = change_state(atoi(cmd[1]), to_do, head);

        if (update == 1){
            if (strcasecmp(to_do, "pause") == 0){
                kill(atoi(cmd[1]), SIGSTOP);
            }
            else{
                kill(atoi(cmd[1]), SIGCONT);
            }
        }
        else if (update == -1){
            fprintf(stderr, "Error: attempted to set the same state\n");
        }
        else{
            fprintf(stderr, "Error: given pid doesn't exist\n");
        }
        return 1; //pause/resume command found
    }
    return 0; //pause/resume command not found
}

char ***create_cmd_list(){
    char buffer[1024];
    if (get_input(buffer) == 1){ //eof encountered
        printf("\n");
        exit(EXIT_SUCCESS);
    }

    char **tokens = tokenify(buffer, ";");
    remove_comment(tokens);
    char ***cmd = get_commands(tokens);

    free_tokens(tokens);

    return cmd;
}

void execute_jobs(int *p_done, int *p_mode, struct node **list, int mode){

    char ***cmd = create_cmd_list();

    int cmd_n = 0;

    while (cmd[cmd_n] != NULL){

        if (check_exit(cmd[cmd_n], p_done, *list) || check_mode(cmd[cmd_n], p_mode) ||
            check_jobs(cmd[cmd_n], *list) || check_pause_resume(cmd[cmd_n], *list, "pause") ||
            check_pause_resume(cmd[cmd_n], *list, "resume")){ 
            cmd_n++;
            continue;
        }


        int status;
        pid_t pid = fork();
        
        if (pid == 0){ //child process, executes a command
            if (execv(cmd[cmd_n][0], cmd[cmd_n]) < 0){
                fprintf(stderr, "execv failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        else if (pid > 0){ //parent process, wait in sequential mode
            if (mode == 1){ //sequential mode
                waitpid(pid, &status, 0);
                cmd_n++;
            }
            else{ //parallel mode
                list_insert(pid, cmd[cmd_n][0], list);
                cmd_n++;
            }
        }
        else{   //fork failed, quit
            fprintf(stderr, "fork failed: %s\n", strerror(errno));
            free_commands(cmd);
            exit(EXIT_FAILURE); //need checking again
        }
    } //end of cmd while 

    //printf("after execute_jobs: \n");
    //list_print(list);

    free_commands(cmd);
}




int check_finished_jobs(struct node **list){
    //printf("before check: \n");
    //list_print(*list);
    struct node *temp = *list;
    int rv = 0;  //return 0 if no process is completed
                 //return 1 if at least one process is completed 

    while (temp != NULL){
        int status;
        pid_t wait_result = waitpid(temp->pid, &status, WNOHANG);
        if (wait_result > 0){  //this job completed
            printf("\nProcess %d, %s, is completed.\n", temp->pid, temp->cmd);
            struct node *to_delete = temp;
            temp = temp->next;
            list_delete(to_delete->pid, list);
            rv = 1;
        }
        else if (wait_result == 0){ //this job not completed
            temp = temp->next;
        }
        else{ //wait error
            printf("%d\n", temp->pid);
            fprintf(stderr, "wait error: %s.\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    return rv; 
}


int main(int argc, char **argv) {

    struct node *job_list = NULL;
    char prompt[] = "prompt>>> ";

    struct pollfd pfd;
    pfd.fd = 0; 
    pfd.events = POLLIN;
    pfd.revents = 0;

    int done = 0;
    int mode = 1; //1-sequential, 0-parallel
    int print_prompt = 1; //keep track of when to print prompt

    while (!done){
        if (print_prompt){
            printf("%s", prompt);
            fflush(stdout);
        }

        int rv = poll(&pfd, 1, 100);

        if (rv == 0){ //timeout, no input
            print_prompt = check_finished_jobs(&job_list);
        }
        else if (rv > 0){ //input detected
            print_prompt = 1;

            execute_jobs(&done, &mode, &job_list, mode);
           
        }
        else{ //poll error
            fprintf(stderr, "poll error: %s\n", strerror(errno));
            return 1;
        } 
    } //end of done while

    return 0;
}

