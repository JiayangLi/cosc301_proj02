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

int check_exit(char **cmd, int *p_done){
    if (strcasecmp(cmd[0], "exit") == 0){
        *p_done = 1;
        return 0; //0-exit command found
    }
    return 1; //1-exit command not found
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
            fprintf(stderr, "Invalid mode!\n");
        }

        return 0; //0-mode command found
    }

    return 1; //1-mode command not found
}


int main(int argc, char **argv) {

	const char prompt[] = "prompt>>> ";

    int done = 0;
    int mode = 1; //1-sequential, 0-parallel
    int new_mode = 1; //keep track of changes in mode

    while (!done){
    	printf("%s", prompt);
    	fflush(stdout);

    	char buffer[1024];
    	if (get_input(buffer) == 1){ //eof encountered
            printf("\n");
            break;
        }

        char **tokens = tokenify(buffer, ";");
        remove_comment(tokens);
        char ***cmd = get_commands(tokens);

        free_tokens(tokens);

        int cmd_n = 0;
        while (cmd[cmd_n] != NULL){

            if (check_exit(cmd[cmd_n], &done) == 0){ //exit found
                cmd_n++;
                continue;
            }

            if (check_mode(cmd[cmd_n], &new_mode) == 0){ //mode found
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
                // add if..else here for different modes
                if (mode == 1){
                    waitpid(pid, &status, 0);
                    cmd_n++;
                }
                
            }
            else{   //fork failed, quit
                fprintf(stderr, "fork failed: %s\n", strerror(errno));
                free_commands(cmd);
                return 1;
            }
        } //end of cmd while 

        free_commands(cmd);
    } //end of done while

    return 0;
}

