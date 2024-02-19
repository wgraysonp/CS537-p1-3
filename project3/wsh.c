#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

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

typedef struct commands{
	int numCommands;
	int *argsPerCommand;
	char ***commList;
} COMMAND;

void parseCommand(VAR **vars, HISTORY *hist, COMMAND *commands, char *histStr);
COMMAND *parseArgs(VAR *vars, char *buff);

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
	curr->contents = (char*)realloc(curr->contents, (strlen(varVal)+1)*sizeof(char));
	strcpy(curr->contents, varVal);

}

void deleteVar(VAR **vars, char*varName){
	VAR *curr = *vars;
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
			// we are deleting the head of the list
			curr->nextVar->isHead = 1;
			*vars = curr->nextVar;
		}
		free(curr->contents);
		free(curr->name);
		free(curr);
	} else {
		curr->nextVar->prevVar = curr->prevVar;
		curr->prevVar->nextVar = curr->nextVar;
		if (curr->isHead == 1){
			curr->nextVar->isHead = 1;
			*vars = curr->nextVar;
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
		if (hist->commands == NULL && length > 0){
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
	if (hist->length == 0){
		return;
	}
	if (hist->commands[0] != NULL){
		if (strcmp(command, hist->commands[0]) == 0){
			return;
		}
	}

	int i = hist->length-1;
	free(hist->commands[i]);
	while (i > 0){
		hist->commands[i] = hist->commands[i-1];
		i--;
	}
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
	} else if(getVar(*vars, varName) != NULL){
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
		if (setenv(varName, varVal, 1) == -1){
			printf("ERROR: setenv() failed\n");
			exit(1);
		}
	}
}

//TODO UPDATE TO ADD HISOTORICAL COMMAND EXECUTION
void history(HISTORY *hist, VAR **vars,  int argCount, char *args[], char *histStr){
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
		int commandNum = str2Int(args[1]);
		if (commandNum < 1 || commandNum > hist->length){
			return;
		} else {
			if (hist->commands[commandNum-1] == NULL){
				return;
			}
			char *buff = NULL;
			buff = (char*)malloc((strlen(hist->commands[commandNum-1]) + 1)*sizeof(char));
			strcpy(buff, hist->commands[commandNum-1]);
			char *buffcopy = buff;
			
                	COMMAND *commands = parseArgs(*vars, buffcopy);

                	parseCommand(vars, hist, commands, histStr);
                	free(buff);

                	for (int i = 0; i < commands->numCommands; i++){
                        	free(commands->commList[i]);
                	}

                	free(commands->argsPerCommand);
                	free(commands->commList);

                	free(commands);
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


// resource: https://stackoverflow.com/questions/33884290/pipes-dup2-and-exec
void runPipe(int infd, int totalCommands,  int currCommand,  char **pipeArgs[], int terminal){
		
	if (currCommand + 1 == totalCommands){
		if(dup2(infd, 0) == -1){
			dprintf(terminal, "ERROR: dup2() failed");
			exit(1);
		}
		close(infd);
		execvp(pipeArgs[currCommand][0], pipeArgs[currCommand]);
		if (errno == 2){
			dprintf(terminal, "execvp: No such file or directory\n");
		} else {
			dprintf(terminal, "ERROR: exec failed\n");
		}
		exit(1);
	} else {

		int newfd[2];
		if (pipe(newfd) == -1){
			printf("ERROR: pipe() failed");
			exit(1);
		}
		
		int rc = fork();

		if (rc < 0){
			dprintf(terminal, "ERROR: fork failed\n");
			exit(1);
		} else if (rc == 0){
			close(newfd[0]);
			int dupOut;
			if((dupOut = dup2(newfd[1], 1)) == -1){
				dprintf(terminal, "ERROR: dup2() failed in child");
			}
			if(dup2(infd, 0) == -1){
						dprintf(terminal, "ERROR: dup2() failed");
						exit(1);
			}
			close(infd);
			close(newfd[1]);
			execvp(pipeArgs[currCommand][0], pipeArgs[currCommand]);
			if (errno == 2){
                        	dprintf(terminal, "execvp: No such file or directory\n");
                	} else {
                        	dprintf(terminal, "ERROR: exec failed\n");
               		}
			exit(1);
		} else {
			close(newfd[1]);
			wait(NULL);
			runPipe(newfd[0], totalCommands, currCommand + 1, pipeArgs, terminal);
		}

	}

}

void execNonBuiltin(int totalCommands, char **args[]){
	fflush(0);
	int rc = fork();
	if (rc < 0){
		printf("ERROR: fork failed\n");
		exit(1);
	} else if (rc == 0){
		int terminal = dup(1);
		if (terminal == -1){
			printf("ERROR: dup() failed");
		}
		runPipe(0, totalCommands, 0, args, terminal);
		printf("ERROR: child return");
		exit(1);
	} else {
		 wait(NULL);
	}

}

void parseCommand(VAR **vars, HISTORY *hist, COMMAND *commands, char *histStr){
	char **args = commands->commList[0];
	int argCount = commands->argsPerCommand[0];

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
			history(hist, vars, argCount, args, histStr);
        } else {
			updateHistory(hist, histStr);
            execNonBuiltin(commands->numCommands, commands->commList);
        }
}

COMMAND *parseArgs(VAR *vars, char *buff){
	int commandNum = 0;
	char *ptr = buff;
	while (*ptr != '\0'){
		if (*ptr == '|'){
			commandNum++;
		}
		ptr++;
	}
	commandNum++;
	
	// MAYBE CHECK FOR || or | at end of line
	COMMAND *commands = (COMMAND*)malloc(sizeof(COMMAND));
	commands->commList = NULL;
	commands->argsPerCommand = NULL;

	commands->numCommands = commandNum;
	commands->commList = (char***)malloc(commandNum*sizeof(char**));
	commands->argsPerCommand = (int*)malloc(commandNum*sizeof(int));

	if (commands->argsPerCommand == NULL || commands->commList == NULL){
		printf("ERROR: Memory not allocated.\n");
		exit(1);
	}

	char *commandbuff = NULL;
	char *tempArg = NULL;
	int j = 0;

	while((commandbuff = strsep(&buff, "|")) != NULL){
		int wc = wordCount(commandbuff);
		char *argsbuff = commandbuff;
		if (wc == 0){
			continue;
		} else {
			commands->commList[j] = NULL;
			commands->commList[j]  = (char**)malloc((wc+1)*sizeof(char*));
			int i = 0;
			while((tempArg = strsep(&argsbuff, " ")) != NULL && i < wc){
				if (tempArg[0]	== '\0'){
					continue;
				} else if (tempArg[0] == '$') {
					tempArg++;
					char *varVal = getenv(tempArg);
					if (varVal == NULL){
						varVal = getVar(vars, tempArg);
					}
					if (varVal == NULL){
						continue;
					} else {
						commands->commList[j][i] = varVal;
						i++;
					}

				} else {
					commands->commList[j][i] = tempArg;
					i++;
				}

			}
			commands->argsPerCommand[j] = i;
			while (i < wc + 1){
				commands->commList[j][i] = NULL;
				i++;
			}
			j++;
			
		}
	}

	return commands;
}

void runCommand(char *buff, VAR **vars, HISTORY *hist){
	// remove ending new line character
		if (buff[strlen(buff) - 1] == '\n'){
			buff[strlen(buff) - 1] = '\0';
		}
	
		char *buffcopy = buff;
		char *histStr = (char*)malloc((strlen(buff) + 1)*sizeof(char));
		strcpy(histStr, buff);
	
	
		COMMAND *commands = parseArgs(*vars, buffcopy);
	
		parseCommand(vars, hist, commands, histStr);
	
		free(histStr);
	
		for (int i = 0; i < commands->numCommands; i++){
			free(commands->commList[i]);
		}
	
		free(commands->argsPerCommand);
		free(commands->commList);
			
		free(commands);


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

         runCommand(buff, &vars, hist);		
         free(buff);
		// remove ending new line character
		//if (buff[strlen(buff) - 1] == '\n'){
		//	buff[strlen(buff) - 1] = '\0';
		//}

		//char *buffcopy = buff;
		//char *histStr = (char*)malloc((strlen(buff) + 1)*sizeof(char));
		//strcpy(histStr, buff);


		//COMMAND *commands = parseArgs(vars, buffcopy);

		//parseCommand(&vars, hist, commands, histStr);

		//if (strcmp(buff, "history") != 0){
		//	updateHistory(hist, histStr);
		//}
		//free(histStr);
		//free(buff);

		//for (int i = 0; i < commands->numCommands; i++){
		//	free(commands->commList[i]);
		//}

		//free(commands->argsPerCommand);
		//free(commands->commList);
		
		//free(commands);
	}

}

void runBatch(FILE *batchFile){

	VAR *vars = NULL;

	HISTORY *hist = (HISTORY*)malloc(sizeof(HISTORY));
	hist->length = 5;
	hist->commands = (char**)malloc(5*sizeof(char*));
	for (int i = 0; i < 5; i++){
		hist->commands[i] = NULL;
	}

	char *buff;
	size_t buffsize = 256;
	buff = (char*)malloc(256*sizeof(char));

	if (fgetc(batchFile) == EOF){
		printf("ERROR: Batch File Empty\n");
		exit(1);
	}

	rewind(batchFile);

	while(getline(&buff,&buffsize, batchFile) > -1){
		runCommand(buff, &vars, hist);
	}
	free(buff);	
}

int main(int argc, char *argv[]){
	
	if (argc > 1){
		FILE *batchFile = fopen(argv[1], "r");
		if (batchFile == NULL){
			printf("ERROR: Can't open batch file\n");
			exit(1);
		}

		runBatch(batchFile);
	} else {
		runInteractive();
	}
	
	return 0;
}
