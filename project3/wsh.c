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

void editVar(VAR **vars, char *varName, char *varVal){
	VAR *curr = *vars;
	while (curr != NULL && strcmp(varName, curr->name) != 0){
		if (curr->nextVar == NULL || curr->nextVar->isHead == 1){
			//this function should only be called after checking of varName exists
			//so if variable is not found its an error
			printf("ERROR: Var not found");
			exit(1);
		} else {
			curr = curr->nextVar;
		}

	}

	curr->contents = varVal;

}

void deleteVar(VAR **vars, char*varName){
	VAR *curr = *vars;
	printf("Deleting %s\n", varName);
	if (curr == NULL){
		return;
	}
	while (curr != NULL && strcmp(varName, curr->name) != 0){
		if(curr->nextVar == NULL || curr->nextVar->isHead == 1){
			return;
		}
		curr = curr->nextVar;
	}

	if (curr->nextVar == NULL){
		//only one variable in the list
		free(curr->contents);
		free(curr->name);
		free(curr);
		*vars = NULL;
	} else if (curr->nextVar == curr->prevVar){
		// only two variables in the list
		curr->nextVar->nextVar = NULL;
		curr->nextVar->prevVar = NULL;
		if (curr->isHead == 1){
			curr->nextVar->isHead = 1;
		}
		free(curr->contents);
		free(curr->name);
		free(curr);
	} else {
		curr->nextVar->prevVar = curr->prevVar;
		curr->prevVar->nextVar = curr->nextVar;
		free(curr->contents);
		free(curr->name);
		free(curr);
	}

}

void insertVar(VAR **vars, char* varName, char *varVal){

	char *test = getVar(*vars, varName);

	if (test != NULL){
		printf("ERROR: Variable already assigned\n");
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

void local(VAR **vars, int argCount, char *args[]){
	char delim = '=';
	if (argCount != 2){
		printf("USAGE: local VAR=<value>\n");
		return;
	}
	char *varName = args[1];
	char *varVal = args[1];

	if (*varName == '$'){
		printf("Variable name cannot start with '$'");
		return;
	}

	while(*varVal != '\0' && *varVal != delim){
		varVal++;
	}
	
	if (*varVal == '\0'){
		printf("USAGE: local VAR=<value>\n");
		return;
	}

	*varVal = '\0';
	varVal++;

	if (*varVal=='\0'){
		deleteVar(vars, varName);
	} else if(getVar(*vars, varName)!=NULL){
		editVar(vars, varName, varVal);
	} else {
		insertVar(vars, varName, varVal);
	}
}

void export(int argCount, char *args[]){
	if (argCount != 2){
		printf("USAGE: export VAR=<value>\n");
		return;
	}
	
	char delim = '=';
	char *varName = args[1];
	char *varVal = args[1];

	if (*varName == '$'){
		printf("Variable name cannot start with '$'");
		return;
	}

	while(*varVal != '\0' && *varVal != delim){
		varVal++;
	}

	if (*varVal == '\0'){
		printf("USAGE: loval VAR=<value>\n");
		return;
	}

	*varVal = '\0';
	varVal++;

	if(*varVal=='\0'){
		if(unsetenv(varName) == -1){
			printf("ERROR: unsetenv() failed\n");
			exit(1);
		}
	} else {
		if (setenv(varName, varVal, 0) == -1){
			printf("ERROR: setenv() failed\n");
			exit(1);
		}
	}
}

void printVars(VAR **vars){
	VAR *curr = *vars;
	while (curr != NULL){
		printf("%s=%s\n", curr->name, curr->contents);
		if (curr->nextVar == NULL || curr->nextVar->isHead == 1){
			break;
		} else {
			curr = curr->nextVar;
		}
	}
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

void parseCommand(VAR **vars, int argCount, char*args[]){

        if (strcmp(args[0], "exit") == 0){
        	exitShell(argCount);
        } else if (strcmp(args[0], "cd") == 0){
        	changeDir(argCount, args);
        } else if (strcmp(args[0], "local") == 0){
        	local(vars, argCount, args);
	} else if (strcmp(args[0], "vars") == 0){
		printVars(vars);
	} else if (strcmp(args[0], "export") == 0){
		export(argCount, args);
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
		parseCommand(&vars, wc, args);
		free(args);
	}

}

int main(int argc, char *argv[]){
	runInteractive();
	
	return 0;
}
