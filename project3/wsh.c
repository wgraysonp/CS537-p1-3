#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

typedef struct var{
	char *name;
	char *contents;
	struct var *nextVar;
	struct var *prevVar;
	int isHead;
} VAR;

int wordCount(char* str){
	char *ptr = str;
	int count = 0;

	if (str == NULL){
		return 0;
	}

	while(*ptr == 32){
		ptr++;
	}

	while (*ptr != '\0'){
		while(*ptr != '\0' && *ptr != 32){
			ptr++;
		}
		count++;
		while(*ptr != '\0' && *ptr == 32){
			ptr++;
		}
	}
	return count;
}

char *getVar(VAR *head, char* varName){
	VAR *curr = head;
	while (curr != NULL && strcmp(varName, curr->name) != 0){
		if (curr->nextVar == NULL || curr->nextVar->isHead == 1){
			return NULL;
		}
		curr = curr->nextVar;
	}

	if (curr == NULL){
		return NULL;
	} else {
		return curr->contents;
	}
}


//TODO: FIX ME
//everything is still inserted into the head
void insertVar(VAR **vars, char* varName, char *varVal){

	char *test = getVar(*vars, varName);

	if (test != NULL){
		printf("ERROR: Variable already assigned");
		return;
	} 
	
	if (*vars==NULL){
	
		*vars = (VAR*)malloc(sizeof(VAR));
		VAR *head = *vars;
		head->name = (char*)malloc((strlen(varName)+1)*sizeof(char));
		strcpy(head->name, varName);
		head->contents = (char*)malloc((strlen(varVal)+1)*sizeof(char));
		strcpy(head->contents, varVal);
		printf("head contents: %s\n", head->contents);
		head->nextVar = NULL;
		head->prevVar = NULL;
		head->isHead = 1;
		
	} else if ((*vars)->prevVar == NULL){
		
		VAR *head = *vars;
		head->prevVar = (VAR*)malloc(sizeof(VAR));
		VAR *temp = head->prevVar;
		temp->isHead = 0;
		temp->name = (char*)malloc((strlen(varName)+1)*sizeof(char));
		strcpy(temp->name, varName);
		temp->contents = (char*)malloc((strlen(varVal)+1)*sizeof(char));
		strcpy(temp->contents, varVal);
		temp->nextVar = head;
		temp->prevVar = head;
		head->nextVar = temp;
		
	} else {

		VAR *head = *vars;
		VAR *oldPrev  = head->prevVar;
		head->prevVar = (VAR*)malloc(sizeof(VAR));
		VAR *temp = head->prevVar;
		temp->isHead = 0;
		temp->name = (char*)malloc((strlen(varName)+1)*sizeof(char));
		strcpy(temp->name, varName);
		temp->contents = (char*)malloc((strlen(varVal)+1)*sizeof(char));
		strcpy(temp->contents, varVal);
		oldPrev->nextVar = temp;
		temp->nextVar = head;
		temp->prevVar = oldPrev;
		
	}
	
}

void exitShell(int argCount){

        if (argCount > 1){
                exit(1);
        } else {
                exit(0);
        }
}

void changeDir(int argCount, char *args[]){
        if (argCount == 1 || argCount > 2){
                exit(1);
        }

        int suc = chdir(args[1]);
        if (suc == -1){
                exit(1);
        }
}

void local(VAR *vars, int argCount, char *args[]){
	char delim = '=';
	if (argCount != 2){
		printf("USAGE: local VAR=<value>\n");
		return;
	}
	char *varName = args[1];
	char *varVal = args[1];

	while(*varVal != '\0' && *varVal != delim){
		varVal++;
	}
	
	if (*varVal == '\0'){
		printf("USAGE: local VAR=<value>\n");
		return;
	}

	*varVal = '\0';
	varVal++;

	insertVar(&vars, varName, varVal);

	printf("varVal: %s\n", varVal);
	printf("varName: %s\n", varName);
}

void execCommand(char *args[]){

	int rc = fork();
	if (rc < 0){
		printf("ERROR: fork failed\n");
		exit(1);
	} else if (rc == 0){
		execvp(args[0], args);
	} else {
		 wait(NULL);	
	}

}

void parseCommand(VAR* vars, int argCount, char*args[]){

        if (strcmp(args[0], "exit") == 0){
        	exitShell(argCount);
        } else if (strcmp(args[0], "cd") == 0){
        	changeDir(argCount, args);
        } else if (strcmp(args[0], "local") == 0){
        	local(vars, argCount, args);
        } else {
                execCommand(args);
        }
}

void runInteractive(){
	VAR *vars = NULL;
	
	while(1){
		printf("wsh> ");
		size_t buffsize = 256;
        char *buff = (char*)malloc(buffsize*sizeof(char));
                        
        
        if(getline(&buff,&buffsize, stdin) == -1){
			//hit EOF marker
        	exit(0);
         }
		
		// remove ending new line character
		if (buff[strlen(buff) - 1] == '\n'){
			buff[strlen(buff) - 1] = '\0';
		}

		int wc = wordCount(buff);

		if (wc == 0){
			continue;
		}
		char **args = (char**)malloc((wc+1)*sizeof(char*));
		int i = 0;
		char delim = ' ';
		while((args[i] = strsep(&buff, &delim)) != NULL && i < wc){
			if (args[i][0] == ' '){
				continue;
			} else {
				i++;
			}
		}

		args[i] = NULL;

		if (buff != NULL){
			printf("ERROR: args not fully parsed");
			exit(1);
		}
		
        	free(buff);
		parseCommand(vars, wc, args);
		free(args);
	}

}

int main(int argc, char *argv[]){
	runInteractive();
	
	return 0;
}
