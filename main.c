#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h> // for fork()
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */

#define MAX_PRESSES 1
static int pressCount = 0;
void catchCtrlZ(int signalNbr)
{
    pressCount++; // Global variable
}

struct back { // to hold background process
    int id;
    struct back *next;
};

void insert(struct back ** head_ref,int id){
    struct back* new_node = (struct back*) malloc(sizeof(struct back));
    new_node->id=id;
    new_node->next = (*head_ref);
    (*head_ref) = new_node;
}

void deleteback(struct back **head_ref, int id){
// Store head node
struct back* temp = *head_ref, *prev;

// If head node itself holds the key to be deleted
if (temp != NULL && temp->id == id)
{
*head_ref = temp->next; // Changed head
free(temp);			 // free old head
return;
}

// Search for the key to be deleted, keep track of the
// previous node as we need to change 'prev->next'
while (temp != NULL && temp->id != id)
{
prev = temp;
temp = temp->next;
}

// If key was not present in linked list
if (temp == NULL) return;

// Unlink the node from linked list
prev->next = temp->next;

free(temp); // Free memory

}

void printback(struct back *back1)
{
    while (back1 != NULL)
    {
        printf(" %d ", back1->id);
        back1 = back1->next;
    }
}




//----- A linked list node
struct Node
{
    //int data;
    char key[100];
    char value[100];
    struct Node *next;
};

/*----- Given a reference (pointer to pointer) to the head of a list
and an int, inserts a new node on the front of the list. */
void push(struct Node** head_ref, char key[],char value[])
{
    struct Node* new_node = (struct Node*) malloc(sizeof(struct Node));
    strcpy(new_node->key,key);
    strcpy(new_node->value,value);
    new_node->next = (*head_ref);
    (*head_ref) = new_node;
}

/*------- Given a reference (pointer to pointer) to the head of a list
and a key, deletes the first occurrence of key in linked list */
void deleteNode(struct Node **head_ref, char alias[])
{
    // Store head node
    struct Node* temp = *head_ref, *prev;
    char h[100];
    strcpy(h, alias);
    // If head node itself holds the key to be deleted
    if (temp != NULL && strcmp(temp->key,h) == 0 )
    {
        *head_ref = temp->next; // Changed head
        free(temp);			 // free old head
        return;
    }

    // Search for the key to be deleted, keep track of the
    // previous node as we need to change 'prev->next'
    while (temp != NULL && strcmp(temp->key,h) != 0)
    {
        prev = temp;
        temp = temp->next;
    }

    // If key was not present in linked list
    if (temp == NULL) {
        return;
    }
    // Unlink the node from linked list
    prev->next = temp->next;

    free(temp); // Free memory

}

//------ This function prints contents of linked list starting from
// the given node
void printList(struct Node *node)
{
    while (node != NULL)
    {
        printf("%s %s\n", node->key,node->value);
        node = node->next;
    }
}

//----findNode in Linked list
void findNode(struct Node *node,char alias[],char command[] ){
    char h[200];
    strcpy(h, alias);
    while (node != NULL)
    {
        if(strcmp(node->key,h) == 0){
            strcpy(command,node->value );
            return;
        }
        node = node->next;
    }
}



/* The setup function below will not return any value, but it will just: read
in the next command line; separate it into distinct arguments (using blanks as
delimiters), and set the args array entries to point to the beginning of what
will become null-terminated, C-style strings. */

void setup(char inputBuffer[], char *args[],int *background,int *argnum)
{
    int length, /* # of characters in the command line */
            i,      /* loop index for accessing inputBuffer array */
            start,  /* index where beginning of next command parameter is */
            ct;     /* index of where to place the next parameter into args[] */

    ct = 0;

    /* read what the user enters on the command line */
    length = read(STDIN_FILENO,inputBuffer,MAX_LINE);

    /* 0 is the system predefined file descriptor for stdin (standard input),
       which is the user's screen in this case. inputBuffer by itself is the
       same as &inputBuffer[0], i.e. the starting address of where to store
       the command that is read, and length holds the number of characters
       read in. inputBuffer is not a null terminated C-string. */

    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */

/* the signal interrupted the read system call */
/* if the process is in the read() system call, read returns -1
  However, if this occurs, errno is set to EINTR. We can check this  value
  and disregard the -1 value */
    if ( (length < 0) && (errno != EINTR) ) {
        perror("error reading the command");
        exit(-1);           /* terminate with error code of -1 */
    }

    printf(">>%s<<",inputBuffer);
    for (i=0;i<length;i++){ /* examine every character in the inputBuffer */

        switch (inputBuffer[i]){
            case ' ':
            case '\t' :               /* argument separators */
                if(start != -1){
                    args[ct] = &inputBuffer[start];    /* set up pointer */
                    ct++;
                }
                inputBuffer[i] = '\0'; /* add a null char; make a C string */
                start = -1;
                break;

            case '\n':                 /* should be the final char examined */
                if (start != -1){
                    args[ct] = &inputBuffer[start];
                    ct++;
                }
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
                break;

            default :             /* some other character */
                if (start == -1)
                    start = i;
                if (inputBuffer[i] == '&'){
                    *background  = 1;
                    inputBuffer[i-1] = '\0';
                    start=-1; // this is for & doesn't be included args[] to execute command correctly
                }
        } /* end of switch */
    }    /* end of for */
    args[ct] = NULL; /* just in case the input line was > 80 */
    *argnum=ct;
    for (i = 0; i <= ct; i++)
        printf("args %d = %s\n",i,args[i]);
} /* end of setup routine */





int main(void){
    // to execute alias ,hold alias
    char poppy = '"';
    int oki=0; // check to alias "ls -l" list ,check list index
    char str[1000]; // to hold command concat with ","
    struct Node* head = NULL;
    struct back* backproc = NULL; // to hold background process
    pid_t childpid;
    char inputBuffer[MAX_LINE]; /*buffer to hold command entered */
    int background; /* equals 1 if a command is followed by '&' */
    int argnum;
    char *args[MAX_LINE/2 + 1]; /*command line arguments */

    struct sigaction action;
    int status;


    while (1){ //---1---
        // signal handling for ctrlz
        pressCount=0;
        action.sa_handler = catchCtrlZ;
        action.sa_flags = 0;
        status = sigemptyset(&action.sa_mask);
        if (status == -1)
        {
            perror("Failed to initialize signal set");
            exit(1);
        } // End if
        status = sigaction(SIGTSTP, &action, NULL);
        if (status == -1)
        {
            perror("Failed to set signal handler for SIGINT");
            exit(1);
        } // End if

        char x[200]="\0";
        char com[100]="\0";
        int err;
        int flag = 6; // flag for alias =1 , unalias =0
        char *ret; // for quatation marks check in each args elements
        int countt=0; // for take begin end args index for concat the command
        int begin,end;
        argnum=0; // argument number that entered from command line
        background = 0;
        printf("myshell: ");
        fflush(stdout);
        setup(inputBuffer, args, &background,&argnum);
        /*setup() calls exit() when Control-D is entered */
        printf("----%d----\n",background);

        char *paths = getenv("PATH");
        char *dup =strdup(getenv("PATH"));
        char *s=dup;
        char *p=NULL;
//--------------------------------------------------------
        childpid = fork();
        if (childpid == -1) {
            perror("Failed to fork");
            return 1;
        }
        if(childpid !=0){
            printf("I am parent %d, my child is %d\n",getpid(),childpid);
            if(background == 0) { // this is foreground process
                waitpid(childpid, NULL, 0); // WAİT FOR CHİLD PROCESS TO JOİN WİTH THİS PARENT
                //while(wait(NULL)>0);
                if(pressCount)
                    kill(childpid,SIGKILL);
            }else{ // this is background process
                printback(backproc);
                printf("\n");
                insert(&backproc,childpid);
                printf("back process list:  ");
                printback(backproc);
                printf("\n");

            }
            if (strcmp(args[0], "fg") == 0) {
                while (backproc != NULL) { // baktığım node u siliyorum
                    //printf(" %d ", node->data);
                    printback(backproc);
                    printf("will be waited %d ", backproc->id);
                    int bbb = backproc->id;
                    waitpid(bbb, NULL, 0);
                    deleteback(&backproc, backproc->id);
                    printf("after  ");
                    printback(backproc);
                    //back = back->next;
                    //while(wait(NULL)>0);
                }

            }

            if (strcmp(args[0], "exit") == 0){
                if(backproc != NULL){
                    printf("There are background processes . You should terminate all process... \n");
                    while (backproc != NULL) { // baktığım node u siliyorum
                        //printf(" %d ", node->data);
                        printback(backproc);
                        printf("will be waited %d ", backproc->id);
                        int bbb = backproc->id;
                        waitpid(bbb, NULL, 0);
                        deleteback(&backproc, backproc->id);
                        printf("after  ");
                        printback(backproc);
                        //back = back->next;
                        //while(wait(NULL)>0);
                    }


                }
                exit(EXIT_SUCCESS);
            }
            if (strcmp(args[0], "alias") == 0 ){
                    kill(childpid,SIGKILL);

            }
            if (strcmp(args[0], "unalias") == 0){
                kill(childpid,SIGKILL);

            }

        }else{ //---

                //printf("I'm child %d, my parent is %d\n",getpid(),getppid());
                // alias,unalias, alias -l
                if (strcmp(args[0], "alias") == 0 && argnum > 2) { //----alias check----
                    flag = 1;
                    //printf("%d----",flag);
                    for (int j = 1; j < argnum; j++) {
                        ret = strchr(args[j], poppy);
                        //printf("%s----",ret);
                        if (ret != NULL) {
                            countt++;
                            if (countt == 1) {
                                begin = j;
                            } else if (countt == 2) {
                                end = j;
                            } else {
                                printf("You entered command with incorrect syntax");
                            }
                        }
                    }//--for
                    // command concat
                    printf("helooo");
                    printf("%d %d  %d----", begin, end, countt);
                    if (countt == 2) {
                        strcpy(str, args[begin]);
                        for (int w = begin + 1; w <= end; w++) {
                            strcat(str, " ");
                            strcat(str, args[w]);
                            printf("\n%s*****\n", str);
                        }
                        // get alias name
                        if (countt == 2 && argnum - end - 1 == 1){ // everyting is okay commands are like "ls -l" list
                            oki = 1;
                        }

                    }
                    //----alias check if end----
                } else if (strcmp(args[0], "unalias") == 0) { //---unalias check---
                    flag = 2;
                } else if (strcmp(args[0], "alias") == 0 && strcmp(args[1], "-l") == 0) { //--alias -l check---
                    flag = 3;
                }else if(strcmp(args[0], "fg") == 0){
                    break;
                }else if (strcmp(args[0], "clr") == 0) {
                    flag = 5;
                }else if (strcmp(args[0], "exit") == 0){
                    break;
                }

                switch (flag) {

                    case 1: // varsa sokma test et , ---alias--> alias "ls -l" list
                        if (oki) { push(&head, args[argnum - 1], str); } // oki nonzero if everything is ok
                        printf("Im here \n");
                        //printList(head);
                        break;
                    case 2: // unalias , unalias list--> daha fazla input girerse kontrol lazım *argnum check yap
                        deleteNode(&head, args[1]);
                        break;
                    case 3: // alias -l
                        printList(head);
                        break;
                    case 5:
                        system("clear");
                        break;
                    case 6:
                        printf("I m here---");
                        // ------------if user enter built in command we will find path for command en "execl" execute it
                        //case 7: // background processsler linked listte
                        //waitpid linked list elemanlarını bekle
                        //waitpid (chilpid,null,0)
                        do {
                            p = strchr(s, ':');
                            if (p != NULL) {
                                p[0] = 0;
                            }
                            //printf("Path is $PATH:%s\n",s);
                            char *ppath = malloc(1000);
                            strcat(ppath, s);
                            strcat(ppath, "/");
                            strcat(ppath, args[0]);
                                while(pressCount<1) {
                                    err = execl(ppath, args[0], args[1], args[2], args[3], args[4], NULL);
                                    break;
                                }
                                if (err == -1) {
                                    s = p + 1;
                                } else {
                                    break;
                                }

                        } while (p != NULL);//-----------------
                        if (err == -1) {  // command sistemde yok demek ,
                            //printf("Command is not found\n"); //kendi listemiste arayacaz
                            findNode(head, args[0], x); // x: command to execute with system call
                            printf("\n%s\n", x);
                            // char command[100];
                            //strcpy( command,x );
                            printf("%s\n", x);
                            printf("%s\n", com);
                            char *l = "\"";
                            strcpy(com, l);
                            strcat(com, x);
                            strcat(com, l);
                            int b = system(com);
                            if (b != 0) { printf("Command is not found!"); }
                        }


                } // end switch


            }

        }//-----
    }//---



/* * the steps are:
            (1) fork a child process using fork()
            (2) the child process will invoke execv()
            (3) if background == 0, the parent will wait,
            otherwise it will invoke the setup() function again. */





