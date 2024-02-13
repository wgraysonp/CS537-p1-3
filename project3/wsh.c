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

typedef struct hist{
	int length;
	char **commands;
} HISTORY;

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

// convert string to integer
int str2Int(char* str){
	int len = strlen(str);
	int strInt = 0;
	for (int i = 0; i < len; i++){
		if (str[i] - 48 < 0 || str[i] - 48 > 9){
			return -1;
		}

		int dec = 1;
		int j = 0;
		while (j < len - i - 1){
			dec = 10*dec;
			j++;
		}

		strInt += (str[i]-48)*dec;
	}

	return strInt;
}

// functions for getting and editing variables

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
		if (curr->isHead == 1){
			curr->nextVar->isHead = 1;
		}
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

// helper functions for history

void historySet(HISTORY *hist, int length){

	if (length == hist->length){
		return;
	} else {
		hist->commands = (char**)realloc(hist->commands, length*sizeof(char*));
		if (hist->commands == NULL){
			printf("ERROR: Memory not allocated\n");
			exit(1);
		}
		for (int i = hist->length; i<length; i++){
			hist->commands[i] = NULL;
		}
		hist->length = length;
	}
}

void updateHistory(HISTORY *hist, char* command){

	int i = hist->length-1;
	free(hist->commands[i]);
	while (i > 0){
		hist->commands[i] = hist->commands[i-1];
		i--;
	}
	//free(hist->commands[0]);
	hist->commands[0] = (char*)malloc((strlen(command) + 1)*sizeof(char));
	strcpy(hist->commands[0], command);

}

void printHistory(HISTORY *hist){

	int i = 0;
	while (i < hist->length && hist->commands[i] != NULL){
		printf("%d) %s\n", i+1, hist->commands[i]);
		i++;
	}
}

// main built-in functions

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

//TODO UPDATE TO ADD HISOTORICAL COMMAND EXECUTION
void history(HISTORY *hist, int argCount, char *args[]){
	if (argCount > 3){
		printf("USAGE: history or history <n> or history set <n>\n");
	} else if (argCount == 3){
		if (strcmp(args[1], "set") != 0){
			printf("USAGE: history or history <n> or history set <n>\n");	
		} else {
			int len = str2Int(args[2]);
			if (len == -1){
				printf("Invalid history length: %s. Must be a positive integer.\n", args[2]);
			} else {
				historySet(hist, len);
			}
		}
	} else if (argCount == 2){
		int len = str2Int(args[1]);
		if (len == -1){
			printf("Invalid command ");
		} else {
			// ADD CODE TO EXECUTE HISTORICAL COMMAND HERE
		}
	} else {
		printHistory(hist);
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

void parseCommand(VAR **vars, HISTORY *hist, int argCount, char*args[]){

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
	} else if (strcmp(args[0], "history") == 0){
		history(hist, argCount, args);
        } else {
                execCommand(args);
        }
}

void runInteractive(){
	VAR *vars = NULL;

	HISTORY *hist = (HISTORY*)malloc(sizeof(HISTORY));
	hist->length = 5;
	hist->commands = (char**)malloc(5*sizeof(char*));
	for (int i = 0; i < 5; i++){
		hist->commands[i] = NULL;
	}

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

		updateHistory(hist, buff);

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
		parseCommand(&vars, hist,  wc, args);
		free(args);
	}

}

int main(int argc, char *argv[]){
	runInteractive();
	
	return 0;
}
