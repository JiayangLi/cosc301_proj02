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

void list_print(const struct node *list){
	while (list != NULL){
		printf("%d     ", list->pid);
		printf("%s     \n", list->cmd);
		list = list->next;
	}
}

void change_state(pid_t pid, char *new_cmd, struct node *list){
	while (list != NULL){
		if (list->pid == pid){
			strcpy(list->cmd, new_cmd);
			return;
		}
		list = list->next;
	}
}


/*

int main(){
	struct node *head = NULL;
	list_insert(123, "a", &head);
	list_insert(123, "b", &head);
	list_insert(125, "c", &head);
	list_insert(126, "d", &head);

	list_print(head);

	change_state(123, "e", head);

	list_print(head);


	
	return 0;
}*/