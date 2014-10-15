#ifndef __LIST_H__
#define __LIST_H__

/* your list data structure declarations */
struct node {
	pid_t pid;
	char cmd[1024];
	char state[8];
	struct node* next;
};

/* your function declarations associated with the list */
void list_insert(pid_t, char *, struct node **);
void list_clear(struct node *);
void list_print(const struct node *);
void change_state(pid_t, char *, struct node *);

#endif // __LIST_H__
