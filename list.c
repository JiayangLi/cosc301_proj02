#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

void list_clear(struct node *list){
	while (list != NULL){
		struct node *temp = list;
		list = list->next;
		free(temp);
	}
}

void list_insert(pid_t pid, char *cmd, struct node **head){
	struct node *addition = malloc(sizeof(struct node));
	addition->pid = pid;
	strcpy(addition->cmd, cmd);
	strcpy(addition->state, "running");
	addition->next = NULL;

	//when given an empty linked list
	if (*head == NULL){	
		*head = addition;
		return;
	}

	struct node *temp = *head;

	while (temp->next != NULL){
		temp = temp->next;
	}

	temp->next = addition;
}

void list_delete(pid_t pid, struct node **head){
	struct node *temp = *head;
	//if the node to delete is the head
    if (temp->pid == pid){
        *head = (*head)->next;
        free(temp);
        return;
    }

    //if the node to delete is not the head
    while (temp->next != NULL){
        if ((temp->next)->pid == pid){
        	struct node *temp2 = temp->next;
            temp->next = temp2->next;
            free(temp2);
            return;
        }
        temp = temp->next;
    }

}

void list_print(const struct node *list){
	printf("pid\tcommand\t\tstate\n");

	while (list != NULL){
		printf("%d\t", list->pid);
		printf("%s\t", list->cmd);
		printf("%s\n", list->state);
		list = list->next;
	}
}

int change_state(pid_t pid, char *new_state, struct node *list){
	while (list != NULL){
		if (list->pid == pid){
			if (strcasecmp(list->state, new_state) == 0)
				return -1; //set the same state
			else{
				strcpy(list->state, new_state);
				return 1;  //successful update 
			}
		}
		list = list->next;
	}
	return 0;  //cannot find the given pid
}



/*
int main(){
	struct node *head = NULL;
	list_insert(123, "a", &head);
	list_insert(124, "b", &head);
	list_insert(125, "c", &head);
	list_insert(126, "d", &head);

	list_print(head);

	list_delete(124, &head);
	printf("\n");

	list_print(head);

	list_clear(head);
	
	return 0;
}*/