#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct word{
	char *contents;
	int numChars;
	int frequency;
	int orderAppeared;
	struct word *nextWord;
	struct word *prevWord;
} WORD;

/*
Get the number of characters in the file
*/
int getFileLength(FILE *fp){
	rewind(fp);
	int char_count = 0;
	while(fgetc(fp)!= EOF){
		char_count++;
	}
	return char_count;
}


/*
Copies the contents of the file to the string dest
*/
int fileToString(FILE *fp, char* dest){
	rewind(fp);
	char str[50];
	if (fgets(str, 50, fp) == NULL){
		return -1;
	} else {
		strcpy(dest, str);	
	}

	while (fgets(str, 50, fp) != NULL){
		strcat(dest, str);
	}

	return 0;
}

/*
Stores the number of unique characters from the string str and 
the number of occurences in the array freq
*/
void getCharFrequency(int freq[][2], char *str){
	char* ptr = str;
	int pos = 0;
	int charVal;
	while (*ptr != '\0'){
		charVal = *ptr;
		if (freq[charVal][1] == 0){
			freq[charVal][0] = pos;
		}
		freq[charVal][1]++;
		pos++;
		ptr++;
	}
}

/*
prints unique characters, the number of occurence, and the first found position
stored in the array freq. if outputString is null results are printed to stdout, if not 
output is printed to a file with name stored in output string. 
*/

void printCharFrequence(int freq[][2], char* outputString, int newLine){
	int totalChars = 0;
	int uniqueChars = 0;
	for (int i = 0; i < 256; i++){
		if (freq[i][1] > 0){
			totalChars += freq[i][1];
			uniqueChars++;
		}
	}

	if (outputString != NULL){
		FILE *op = fopen(outputString, "a");
		fprintf(op, "Total Number of Chars = %d\n", totalChars);
		fprintf(op, "Total Unique Chars = %d\n", uniqueChars);
		fprintf(op, "\n");

		for (int i = 0; i < 256; i++){
			if (freq[i][1] > 0){
					fprintf(op, "Ascii Value: %d, Char: %c, Count: %d, Initial Position: %d\n", i, i, freq[i][1], freq[i][0]);
			}
		}

		if (newLine == 0){
			fprintf(op, "\n");
		}

		fclose(op);
		
	} else {

		printf("Total Number of Chars = %d\n", totalChars);
		printf("Total Unique Chars = %d\n", uniqueChars);
		printf("\n");
	
		for (int i = 0; i < 256; i++){
			if (freq[i][1] > 0){
				printf("Ascii Value: %d, Char: %c, Count: %d, Initial Position: %d\n", i, i, freq[i][1], freq[i][0]);
			}
		}

		if (newLine == 0){
			printf("\n");
		}

	}

}

/*
Inserts a WORD struct into a linked list *words in ASCII alphabetical order. The variable ptr
points to the first character in the word to be inserted in a string possibly containing more words. 
wordLength is the length of the word to be copied
*/

void insertWord(WORD *words, char* ptr, int wordLength, int location){
		WORD *prev = words;
		WORD *curr = words->nextWord;
		WORD *newWord;
		char *tempStr;

		// copy the first wordLength characters of ptr to tempStr.
		// these are all characters of the word to be inserted into the linked list
		tempStr = (char*)malloc((wordLength+1)*sizeof(char));
		strncpy(tempStr, ptr, wordLength);
		tempStr[wordLength] = '\0';

		// loop through the list until we get to the end or a word that equals or should come after the one 
		// to be insterted
		while (curr != NULL && curr->contents != NULL && strcmp(curr->contents, tempStr) < 0){
			prev = curr;
			curr = curr->nextWord;
		}
		if (curr != NULL && curr->contents != NULL && strcmp(curr->contents, tempStr) == 0){
		// the words are the same. Do no insertion and increment the number of occurences
			curr->frequency++;
		} else {
			if (curr == NULL){
				// we're at the end of the list. allocated space for a new word and add it to the end
				prev->nextWord = (WORD*)malloc(sizeof(WORD));
				if (prev->nextWord == NULL){
					printf("memory not allocated");
					exit(1);
				}
				curr = prev->nextWord;
				curr->contents = (char*)malloc((wordLength+1)*sizeof(char));
				strcpy(curr->contents, tempStr);
				curr->numChars = wordLength;
				curr->frequency = 1;
				curr->orderAppeared = location;
				curr->nextWord = NULL;
				curr->prevWord = prev;
			} else if (curr->contents == NULL){
				curr->contents = (char*)malloc((wordLength+1)*sizeof(char));
				strncpy(curr->contents, ptr, wordLength);
				curr->contents[wordLength] = '\0';
				curr->numChars = wordLength;
				curr->frequency = 1;
				curr->orderAppeared = location;
				curr->prevWord = prev;
			} else {
				// the word needs to be inserted in the middle of the list
				newWord = (WORD*)malloc(sizeof(WORD));
				newWord->contents = (char*)malloc((wordLength+1)*sizeof(char));
				strncpy(newWord->contents, ptr, wordLength);
				newWord->contents[wordLength] = '\0';
				newWord->numChars = wordLength;
				newWord->frequency = 1;
				newWord->orderAppeared = location;
				newWord->nextWord = curr;
				newWord->prevWord = prev;
				curr->prevWord = newWord;
			
				if (prev != NULL){
					prev->nextWord = newWord;
				}

			}
		}
		// free the memory used for the temp string
		free(tempStr);
}

/*
get the number of unique words, number of occurences, and their location in the string str
and add put them in the linked list words
*/
void getWordFrequency(WORD *words, char *str){
	char *ptr1 = str;
	int location = 0;
	size_t wordLength=0;

	// loop through any whitespace at the beginning of the strting
	while(*ptr1 != '\0' &&(*ptr1 == ' ' || *ptr1 == '\n' || *ptr1 == '\t'|| *ptr1 == '\r' || *ptr1 == '\v' || *ptr1 == '\f')){
		ptr1++;
	}

	char *ptr2 = ptr1;

	while (*ptr1!='\0'){
		// loop through the string until we reach new white space or the end
		while(*ptr1 != '\0' && *ptr1 != ' ' && *ptr1 !='\n' && *ptr1 != '\t' && *ptr1 != '\r' && *ptr1 != '\v' && *ptr1 != '\f'){
			ptr1++;
			wordLength++;
		}
		// ptr2 is pointed to the beginning of the word to be inserted
		// insert it into the linked list
		insertWord(words, ptr2, wordLength, location);
		ptr1++;
		location++;
		//loop through any additional white space until the start of the next word
		while(*ptr1 != '\0' &&(*ptr1 == ' ' || *ptr1 == '\n' || *ptr1 == '\t'|| *ptr1 == '\r' || *ptr1 == '\v' || *ptr1 == '\f')){
			ptr1++;
		}
		// point ptr2 to the start of the next word
		ptr2 = ptr1;
		wordLength=0;
	}
}

/*
prints unique words and their frequency to stdout if outputString is null
other wise to file with name stored in outputString
*/
void printWordFrequency(WORD *words, char* outputString, int newLine){
	int totalWords = 0;
	int uniqueWords = 0;
	
	if (words->nextWord == NULL){
		printf("Null pointer in printWordFrequency");
		exit(0);
	}
	WORD *curr = words->nextWord;
	if (curr == NULL){
		printf("null pointer in printWordFrequency");
		exit(0);
	}

	while(curr != NULL){
		totalWords += curr->frequency;
		uniqueWords++;
		curr = curr->nextWord;
	}

	if (outputString != NULL){
		FILE *op = fopen(outputString, "a");
		if (op == NULL){
			printf("cannot open file");
			exit(0);
		}
		fprintf(op, "Total Number of Words: %d\n", totalWords);
		fprintf(op, "Total Unique Words: %d\n", uniqueWords);
		fprintf(op, "\n");

		curr = words->nextWord;

		while(curr != NULL){
			fprintf(op, "Word: %s, Freq: %d, Initial Position: %d\n", curr->contents, curr->frequency, curr->orderAppeared);
			free(curr->contents);
			free(curr->prevWord);
			curr = curr->nextWord;
		}

		if (newLine == 0){
			fprintf(op, "\n");
		}

		fclose(op);
	} else {

		printf("Total Number of Words: %d\n", totalWords);
		printf("Total Unique Words: %d\n", uniqueWords);
		printf("\n");
	
		curr = words->nextWord;
	
		while (curr != NULL){
			printf("Word: %s, Freq: %d, Initial Position: %d\n", curr->contents, curr->frequency, curr->orderAppeared);
			free(curr->contents);
			free(curr->prevWord);
			curr = curr->nextWord;
		}

		if (newLine == 0){
			printf("\n");
		}

	}
}


/*
get unique lines in the string *str and add them to the linked list *lines
*/
void getLineFrequency(WORD *lines, char *str){
	char *ptr1 = str;
	char *ptr2 = str;
	int location = 0;
	int lineLength = 0;
	
	while (*ptr1 != '\0'){
		// loop through until we get to a newline
		while (*ptr1 != '\0' && *ptr1 != '\n'){
			ptr1++;
			lineLength++;
		}
		// insert the line into the linked list
		insertWord(lines, ptr2, lineLength, location);
		ptr1++;
		ptr2 = ptr1;
		location++;
		lineLength = 0;
	}
}

/*
prints the unique lines to stdout or file with name stored in outputString
*/
void printLineFrequency(WORD *lines, char* outputString, int newLine){
	int totalLines = 0;
	int uniqueLines = 0;
	
	WORD* currLine = lines->nextWord;
	if (currLine == NULL){
		printf("Null pointer in printLineFrequency");
		exit(0);
	}

	while(currLine != NULL){
		totalLines+=currLine->frequency;
		uniqueLines++;
		currLine = currLine->nextWord;
	}

	if (outputString != NULL){

		FILE *op = fopen(outputString, "a");

		fprintf(op, "Total Number of Lines: %d\n", totalLines);
		fprintf(op, "Total Unique Lines: %d\n", uniqueLines);
		fprintf(op, "\n");
	
		currLine = lines->nextWord;
		
		while(currLine != NULL){
			fprintf(op, "Line: %s, Freq: %d, Initial Position: %d\n", currLine->contents, currLine->frequency, currLine->orderAppeared);
			free(currLine->contents);
			free(currLine->prevWord);
			currLine = currLine->nextWord;
		}
	
		if (newLine == 0){
			fprintf(op, "\n");
		}
		fclose(op);
	} else {

		printf("Total Number of Lines: %d\n", totalLines);
		printf("Total Unique Lines: %d\n", uniqueLines);
		printf("\n");

		currLine = lines->nextWord;
	
		while(currLine != NULL){
			printf("Line: %s, Freq: %d, Initial Position: %d\n", currLine->contents, currLine->frequency, currLine->orderAppeared);
			free(currLine->contents);
			free(currLine->prevWord);
			currLine = currLine->nextWord;
		}

		if (newLine == 0){
			printf("\n");
		}

	}

}

/*
Find the length of the longest word in str
*/
int getLongestWordLength(char* str){
	char* ptr1 = str;
	int wordLength = 0;
	int maxLength = 0;
	
	while(*ptr1 != '\0'){
			while(*ptr1 != '\0' && *ptr1 != ' ' && *ptr1 != '\n' && *ptr1 != '\t' && *ptr1 != '\r' && *ptr1 != '\v' && *ptr1 != '\f'){
				ptr1++;
				wordLength++;
			}
			if (wordLength > maxLength){
				maxLength = wordLength;
			}
			while(*ptr1 != '\0' && (*ptr1 == ' ' || *ptr1 == '\n' || *ptr1 == '\t'|| *ptr1 == '\r' || *ptr1 == '\v' || *ptr1 == '\f')){
				ptr1++;
			}
			wordLength = 0;
		}

	return maxLength;
}

/*
Prints all words of maximal length to stdout or file outputString
uses a linked list longWords and the function insertWord to alphabetize them
if there are more than one
*/
void printLongestWord(WORD* longWords, char* str, char* outputString, int newLine){
	int wordLength = 0;
	int maxLength = 0;
	char* ptr1 = str;
	char* ptr2 = str;

	maxLength = getLongestWordLength(str);

	while(*ptr1 != '\0'){
		while(*ptr1 != '\0' && *ptr1 != ' ' && *ptr1 != '\n' && *ptr1 != '\t' && *ptr1 != '\r' && *ptr1 != '\v' && *ptr1 != '\f'){
			ptr1++;
			wordLength++;
		}
		if (wordLength ==  maxLength){
			insertWord(longWords, ptr2, maxLength, 0);
		}
		while(*ptr1 !='\0' && (*ptr1 == ' ' || *ptr1 == '\n' || *ptr1 == '\t' || *ptr1 == '\r' || *ptr1 == '\v' || *ptr1 == '\f' )){
			ptr1++;
		}
		ptr2 = ptr1;
		wordLength = 0;
	}

	if (longWords->nextWord==NULL){
		printf("Null pointer in printLongestWord");
		exit(0);
	}

	if (outputString != NULL){
		FILE *op = fopen(outputString, "a");
		fprintf(op, "Longest Word is %d characters long:\n", maxLength);
		
		WORD *curr = longWords->nextWord;
		while(curr != NULL && curr->contents != NULL){
			fprintf(op, "\t%s\n", curr->contents);
			free(curr->contents);
			free(curr->prevWord);
			curr = curr->nextWord;
		}

		if (newLine == 0){
			fprintf(op, "\n");
		}

		fclose(op);
			
	} else {
	
		printf("Longest Word is %d characters long:\n", maxLength);

		WORD *curr = longWords->nextWord;
		while(curr != NULL && curr->contents != NULL){
			printf("\t%s\n", curr->contents);
			free(curr->contents);
			free(curr->prevWord);
			curr = curr->nextWord;
		}

		if (newLine == 0){
			printf("\n");
		}

	}

}
/* 
Find the length of the longest line in str
*/
int getLongestLineLength(char* str){
	char* ptr1 = str;
	int lineLength = 0;
	int maxLength = 0;
	
	while(*ptr1 != '\0'){
			while(*ptr1 != '\0' && *ptr1 != '\n'){
				ptr1++;
				lineLength++;
			}
			if (lineLength > maxLength){
				maxLength = lineLength;
			}
			while(*ptr1 != '\0' && (*ptr1 == ' ' || *ptr1 == '\n')){
				ptr1++;
			}
			lineLength = 0;
		}

	return maxLength;
}

/*
prints the longest line(s) in the string str. Logic is the same as printLongestWord
*/
void printLongestLine(WORD* longLines, char* str, char* outputString, int newLine){
	int lineLength = 0;
	int maxLength = 0;
	char* ptr1 = str;
	char* ptr2 = str;

	maxLength = getLongestLineLength(str);


	while(*ptr1 != '\0'){
		while(*ptr1 != '\0' && *ptr1 != '\n'){
			ptr1++;
			lineLength++;
		}
		if (lineLength ==  maxLength){
			insertWord(longLines, ptr2, maxLength, 0);
		}
		while(*ptr1 != '\0' && *ptr1 == '\n'){
			ptr1++;
		}
		ptr2 = ptr1;
		lineLength = 0;
	}

	if (longLines->nextWord==NULL){
		printf("Null pointer in printLongestLine");
		exit(0);
	}


	if (outputString != NULL){
		FILE *op = fopen(outputString, "a");

		fprintf(op, "Longest Line is %d characters long:\n", maxLength);
		
		WORD *curr = longLines->nextWord;
			
		while(curr != NULL && curr->contents != NULL){
			fprintf(op, "\t%s\n", curr->contents);
			free(curr->contents);
			free(curr->prevWord);
			curr = curr->nextWord;
		}

		if (newLine == 0){
			fprintf(op, "\n");
		}

		fclose(op);
	} else {

		printf("Longest Line is %d characters long:\n", maxLength);

		WORD *curr = longLines->nextWord;
	
		while(curr != NULL && curr->contents != NULL){
			printf("\t%s\n", curr->contents);
			free(curr->contents);
			free(curr->prevWord);
			curr = curr->nextWord;
		}

		if (newLine == 0){
			printf("\n");
		}

	}

}

/*
preforms a sinclge run of the program with argumes args[] 
*/
void singleRun(int numArgs, char* args[], int *errorFlag){
	char *statFlags[5] = {NULL}; // to store statistic flags (there are at most 5)
	int numFlags = 0; 
	int foundFile = 0; // will be set to 1 if an input file is found
	int newLineFlag = 0;// flag to print a new line after output for one statistic flag 
	char* inputFileName; // to store name of input file
	char* outputFileName = NULL; // to store name of ouput file

	// loop through the args[] adding statistic flags to statFlags array and saving the names of input and output files
	for (int i = 0; i < numArgs; i++){
		if (strcmp(args[i], "-c")==0 || strcmp(args[i], "-l") == 0 || strcmp(args[i], "-w")==0 || strcmp(args[i], "-Lw")==0 || strcmp(args[i], "-Ll") == 0){
			statFlags[numFlags] = args[i];
			numFlags++;
		} else if (strcmp(args[i], "-f") == 0){
			// file flag is found
			foundFile = 1;
			size_t nameLength = strlen(args[i+1]);
			inputFileName = (char*)malloc(nameLength*sizeof(char));
			if (inputFileName == NULL){
				printf("Memory not allocated.\n");
				exit(1);
			} else if (i == numArgs - 1 || args[i+1][0] == '-'){
				// -f flag is at the end of the list or no file name provided
				printf("ERROR: No Input File Provided\n");
				*errorFlag = 1;
				return;
			} else {
				// found the file name. copy it to inputFileName
				strcpy(inputFileName, args[i+1]);
			}
		} else if (strcmp(args[i], "-o") == 0){
			// output file flag included
			if (i == numArgs - 1 || args[i+1][0] == '-'){
				// no output file name provided
				printf("ERROR: No Output File Provided\n");
				*errorFlag = 1;
				return;
			}
			outputFileName = (char*)malloc(strlen(args[i+1])*sizeof(char));
			if (outputFileName == NULL){
				printf("Memory not allocated.\n");
				exit(1);
			}
			strcpy(outputFileName, args[i+1]);

			FILE *op = fopen(outputFileName, "r+");
			if (op != NULL){
				// if op is not null we are printing the output of multiple batch runs to the same 
				// file. print a new line before the analysis
				fseek(op, 0, SEEK_END);
				fprintf(op, "\n");
				fclose(op);
			}

		} else if (args[i][0] == '-') {
			// caught an invalid flag
			printf("ERROR: Invalid Flag Types\n");
			*errorFlag = 1;
			return;
		}
	}

	if (foundFile == 0){
		// -f flag was not part of the arguments so no input file provided
		printf("ERROR: No Input File Provided\n");
		*errorFlag = 1;
		return;
	}

	FILE *fp = fopen(inputFileName, "r");
		
	if (fp == NULL) {
		printf("ERROR: Can't open input file\n");
		*errorFlag = 1;
		return;
	}

	// copy the contents of the file to fileString, exiting the single run if 
	// the file is empty
	int fileLength = getFileLength(fp);
	char* fileString;
	fileString = (char*)malloc(fileLength*sizeof(char));
	int fileEmpty = fileToString(fp, fileString);
	if (fileEmpty == -1){
		printf("ERROR: Input File Empty\n");
		*errorFlag = 1;
		return;
	}
	fileString[fileLength] = '\0';

	// loop throug the statistic flags and perform the relevant analysis for each flag found
	for (int i = 0; i < numFlags; i++){
			if (i == numFlags - 1){
				newLineFlag = 1;
			}
			if (strcmp(statFlags[i], "-c")==0){
				int wordFreq[256][2] = {0};
				getCharFrequency(wordFreq, fileString);
				printCharFrequence(wordFreq, outputFileName, newLineFlag);

			} else if (strcmp(statFlags[i], "-w")==0){
				WORD *words;
				words = (WORD*)malloc(sizeof(WORD));
				if (words == NULL){
					printf("memory not allocated.\n");
					exit(1);
				}
				words->contents = NULL;
				words->nextWord = NULL;
				words->prevWord = NULL;
					
				getWordFrequency(words, fileString);
				printWordFrequency(words, outputFileName, newLineFlag);
				
			} else if (strcmp(statFlags[i], "-l")==0){
	
				WORD *lines;
				lines = (WORD*)malloc(sizeof(WORD));
				if (lines == NULL){
					printf("memory not allocated\n");
					exit(1);
				}
				lines->contents = NULL;
				lines->nextWord = NULL;
				lines->prevWord = NULL;
						
				getLineFrequency(lines, fileString);
				printLineFrequency(lines, outputFileName, newLineFlag);

			} else if (strcmp(statFlags[i], "-Lw")==0) {
	
				WORD *longestWords;
				longestWords = (WORD*)malloc(sizeof(WORD));
				if (longestWords == NULL){
					printf("memory not allocated");
					exit(1);
				}
				longestWords->contents = NULL;
				longestWords->nextWord = NULL;
				longestWords->prevWord = NULL;
					
				printLongestWord(longestWords, fileString, outputFileName, newLineFlag);
				
			} else if (strcmp(statFlags[i], "-Ll")==0){
				WORD *longestLines;
				longestLines = (WORD*)malloc(sizeof(WORD));
				if (longestLines == NULL){
					printf("memory not allocated");
					exit(1);
				}
				longestLines->contents = NULL;
				longestLines->nextWord = NULL;
				longestLines->prevWord = NULL;
						
				printLongestLine(longestLines, fileString, outputFileName, newLineFlag);
			
			}
		}

	// close the file and free the strings for the input file name and its contents
	fclose(fp);
	free(inputFileName);
	free(fileString);

	
}


int main(int argc, char *argv[]){
	if (argc < 3){
		printf("USAGE:\n\t./MADCounter -f <input file> -o <output file> -c -w -l -Lw -Ll\n\t\tOR\n\t./MADCounter -B <batch file>\n");
		exit(1);
	}
	int error = 0;
	int *errorFlag = &error;

	int batchMode = 0;

	// loop through argv[] looking for the -B flag for batch mode
	for (int i = 0; i < argc; i++){
		if (strcmp(argv[i], "-B")==0){
			// found the flag. check to see that a file name is provided and it is not empty
			batchMode = 1;
			if (i == argc -1 || argv[i+1][0] == '-'){
				printf("ERROR: Can't open batch file\n");
				exit(1);
			}
			FILE *batchFile = fopen(argv[i+1], "r");
			if (batchFile == NULL){
				printf("ERROR: Can't open batch file\n");
				exit(1);
			}
			
			char* buffer;
			size_t buffsize = 256;

			buffer = (char*)malloc(buffsize*sizeof(char));

			if (fgetc(batchFile) == EOF){
				printf("ERROR: Batch File Empty\n");
				exit(1);
			}

			rewind(batchFile);

			// loop through each line of the file and split the arguments into an array of strings
			while (getline(&buffer, &buffsize, batchFile)> -1){
				buffer[strlen(buffer)-1] = '\0';

				int numArgs = (int)sizeof(buffer);

				char **args = (char**)malloc(numArgs*sizeof(char*));
				char *arg;
				char delim = ' ';

				arg = strtok(buffer, &delim);
				int argCount = 0;
				while ( arg != NULL ){
					args[argCount] = arg;
					arg = strtok(NULL, &delim);
					argCount++;
				}
				// perform a single run with the arguments from the current line of the file
				singleRun(argCount, args, errorFlag);
				free(args);
			}

			free(buffer);
			fclose(batchFile);
		}
		
	}
	
	if(batchMode == 0){
		// we are not in batch mode. do a single run with arguments arg and arv
		singleRun(argc, argv, errorFlag);
		return(error);
	} else {
		return(0);
	}
	
}
