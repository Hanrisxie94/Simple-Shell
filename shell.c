// Shell starter file
// You may make any changes to any part of this file.
//
// name: Hongyi Jin
// ID: 301277714
// email: hongyij@sfu.ca
// date: Oct.9 2017

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>


#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)
#define HISTORY_DEPTH 10

char history[HISTORY_DEPTH][COMMAND_LENGTH]; // record last 10 history commands.
int history_counter = 0; // number of total history commands

// record_history: put current command into history[] array
// remove the oldest command if array is full
void record_history(char* buff){
	if (history_counter < 10){
		strcpy(history[history_counter], buff); // record 'buff' into history[]
	}
	else {
		for (int i = 0; i < 9 ; i++){
			strcpy(history[i], history[i+1]); // shift one position for previous commands, delete the ordest one.
		}
		strcpy(history[9], buff); // copy 'buff' into the last position of history[]
	}
	history_counter++; //increment count
}

// print_history: print the last 10 history commands
// with the n_th position with them.
void print_history(){
	char history_index[COMMAND_LENGTH]; // char element used to 'write'
	int temp_counter = history_counter-HISTORY_DEPTH+1; // get the position of the first command that will be printed.
	if (history_counter <= HISTORY_DEPTH){ // if history[] is not full,
		for (int i = 0; i < history_counter; i++){ // print each command
			sprintf(history_index, "%i", i+1);
			write(STDOUT_FILENO, history_index, strlen(history_index));
			write(STDOUT_FILENO, "\t", strlen("\t"));
			write(STDOUT_FILENO, history[i], strlen(history[i]));
			write(STDOUT_FILENO, "\n", strlen("\n"));
		}
	}
	else{ // if history commands is more than HISTORY_DEPTH
		for (int i = 0; i < HISTORY_DEPTH; i++){ // must print 10 commands
			sprintf(history_index, "%i", temp_counter);
			write(STDOUT_FILENO, history_index, strlen(history_index));
			write(STDOUT_FILENO, "\t", strlen("\t"));
			write(STDOUT_FILENO, history[i], strlen(history[i]));
			write(STDOUT_FILENO, "\n", strlen("\n"));
			temp_counter++;
		}
	}
}


// return 1 if cannot recall the command, 0 if something be modified
// and history command will be successfully Executed.
int command_recall(char *buff){
	if (history_counter == 0){ // if there is not history command
		write(STDOUT_FILENO, "Error: no previous history!\n", strlen("Error: no previous history!\n"));
		return 1;
	}
	else{

		//handler for !!
		if (buff[1] == '!'){
			if (strlen(buff) == 2){ //check command is exactly '!!'
				if (history_counter <= 10){ // if history[] is not full
					strcpy(buff, history[history_counter-1]); // replace '!!' command with the last history command
				}
				else{ // if history[] is full, and the newest command must sit at the last postion of history[]
					strcpy(buff, history[9]); // replace '!!' command with the last history command
				}
			} // something is followed by '!!'
			else{
				write(STDOUT_FILENO, "Error: Unknown History Command!\n", strlen("Error: Unknown History Command!\n"));
				return 1;
			}
		}

		//handler for !
		// return 1 and exit if 'buff' is exactly '!'
		else if (buff[1] == '\0'){
			write(STDOUT_FILENO, "Error: Unknown History Command!\n", strlen("Error: Unknown History Command!\n"));
			return 1;
		}

		//handler for !n
		else if (buff[1] != '!'){
			int i,j,length;
			char str[40]; // create a str that will copy buff
			length = strlen(buff);

			for (i = 0 ; i <= length ; i++){ // copy buff
				str[i] = buff[i];
			}
			for (i = 0, j = 0; i < length; i++){ //delete '!' from str
				if (str[i] != '!'){
					str[j] = str[i];
					j++;
				}
			}
			str[j] = '\0'; // set end of str

			int num = atoi(str); // convert str to int
			if (num == 0){ // if str cannot convert to int
				write(STDOUT_FILENO, "Error: Unknown History Command!\n", strlen("Error: Unknown History Command!\n"));
				return 1;
			}
			else if (num <= history_counter && num >= history_counter-HISTORY_DEPTH+1){ //num locate at correct with last 10 history commands
				if (history_counter <= HISTORY_DEPTH){ // if history[] is not full
					strcpy(buff, history[num-1]);
				}
				else{
					strcpy(buff, history[HISTORY_DEPTH-1+num-history_counter]); // copy the history with corresponding index(9-(history_counter-num)) to buff
				}
			}
			else {
				write(STDOUT_FILENO, "Error: Unknown History Command!\n", strlen("Error: Unknown History Command!\n"));
				return 1;
			}
		}

	}
	return 0;
}




/**
 * Command Input and Processing
 */

/*
 * Tokenize the string in 'buff' into 'tokens'.
 * buff: Character array containing string to tokenize.
 *       Will be modified: all whitespace replaced with '\0'
 * tokens: array of pointers of size at least COMMAND_LENGTH/2 + 1.
 *       Will be modified so tokens[i] points to the i'th token
 *       in the string buff. All returned tokens will be non-empty.
 *       NOTE: pointers in tokens[] will all point into buff!
 *       Ends with a null pointer.
 * returns: number of tokens.
 */
int tokenize_command(char *buff, char *tokens[])
{
	int add_history = 0;
	if (buff[0] == '!'){
		add_history = command_recall(buff);
	}
	else if (buff[0] == '\0'){
		add_history = 1;
	}

	if (add_history != 1){
		record_history(buff);
	}


	int token_count = 0;
	_Bool in_token = false;
	int num_chars = strnlen(buff, COMMAND_LENGTH);
	for (int i = 0; i < num_chars; i++) {
		switch (buff[i]) {
		// Handle token delimiters (ends):
		case ' ':
		case '\t':
		case '\n':
			buff[i] = '\0';
			in_token = false;
			break;

		// Handle other characters (may be start)
		default:
			if (!in_token) {
				tokens[token_count] = &buff[i];
				token_count++;
				in_token = true;
			}
		}
	}
	tokens[token_count] = NULL;
	return token_count;
}

/**
 * Read a command from the keyboard into the buffer 'buff' and tokenize it
 * such that 'tokens[i]' points into 'buff' to the i'th token in the command.
 * buff: Buffer allocated by the calling code. Must be at least
 *       COMMAND_LENGTH bytes long.
 * tokens[]: Array of character pointers which point into 'buff'. Must be at
 *       least NUM_TOKENS long. Will strip out up to one final '&' token.
 *       tokens will be NULL terminated (a NULL pointer indicates end of tokens).
 * in_background: pointer to a boolean variable. Set to true if user entered
 *       an & as their last token; otherwise set to false.
 */
void read_command(char *buff, char *tokens[], _Bool *in_background)
{
	*in_background = false;

	// Read input
	int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);
	if (length < 0 && (errno !=EINTR) ) {
		perror("Unable to read command from keyboard. Terminating.\n");
		exit(-1);
	}

	// Null terminate and strip \n.
	buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n') {
		buff[strlen(buff) - 1] = '\0';
	}

	// Tokenize (saving original command string)
	int token_count = tokenize_command(buff, tokens);
	if (token_count == 0) {
		return;
	}

	// Extract if running in background:
	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
		*in_background = true;
		tokens[token_count - 1] = 0;
	}
}


//handler of SIGINT
void handle_SIGINT(){
	write(STDOUT_FILENO, "\n" ,strlen("\n"));
	print_history();
	signal(SIGINT, handle_SIGINT);
}

/**
 * Main and Execute Commands
 */
int main(int argc, char* argv[])
{
	char input_buffer[COMMAND_LENGTH];
	char *tokens[NUM_TOKENS];
	char curr_path[COMMAND_LENGTH];


	while (true) {
		// print current path:
		int no_fork = 0;

		getcwd(curr_path, sizeof(curr_path));
		write(STDOUT_FILENO, curr_path, strlen(curr_path));

		// Get command
		// Use write because we need to use read() to work with
		// signals, and read() is incompatible with printf().
		write(STDOUT_FILENO, "> ", strlen("> "));
		_Bool in_background = false;
		signal(SIGINT, handle_SIGINT);
		// struct sigaction handler;
		// handler.sa_handler = handle_SIGINT;
		// handler.sa_flags = 0;
		// sigemptyset(&handler.sa_mask);
		// sigaction(SIGINT, &handler, NULL);
		read_command(input_buffer, tokens, &in_background);



		// check if tokens was successfully created
		if (tokens[0]){
			// check those cases that will not create child precess
			if (strcmp(tokens[0], "exit") == 0){ // if user type exit
				exit(0);
			}
			else if (strcmp(tokens[0], "pwd") == 0){ // if "pwd" entered
				write(STDOUT_FILENO, curr_path, strlen(curr_path));
				write(STDOUT_FILENO, "\n", strlen("\n"));
				no_fork = 1;
			}
			else if (strcmp(tokens[0], "cd") == 0){ // if "cd" to go next directory
				int next_DICTIONARY = chdir(tokens[1]);
				if (next_DICTIONARY == -1){ // if failed to find such directory
					write(STDOUT_FILENO, "No such file or directory!\n", strlen("No such file or directory!\n"));
				}
				no_fork = 1;
			}
			else if (strcmp(tokens[0], "history") == 0){ // if "history" entered
				print_history(); //
				no_fork = 1;
			}
			else if (strncmp(tokens[0],"!",1) == 0){
				no_fork = 1;
			}
		}


		/**
		 * Steps For Basic Shell:
		 * 1. Fork a child process
		 * 2. Child process invokes execvp() using results in token array.
		 * 3. If in_background is false, parent waits for
		 *    child to finish. Otherwise, parent loops back to
		 *    read_command() again immediately.
		 */


		//3. Creating Child Process
		if (no_fork != 1){

			pid_t pid = fork();
			if (pid < 0){
				perror("fork() failed!");
			}
			else if (pid == 0){
					// Child:
					int execvp_success = execvp(tokens[0], tokens);
					if (execvp_success == -1){ //if execvp failed
						write(STDOUT_FILENO, tokens[0], strlen(tokens[0]));
						write(STDOUT_FILENO, ": \t", strlen(": \t"));
						write(STDOUT_FILENO, "Invalid Command!\n", strlen("Invalid Command!\n"));
						exit(0);
					}
			}

			//parent:
			if (!in_background) {
					// Cleanup any previously exited background child processes
					// (The zombies)
					while (waitpid(-1, NULL, WNOHANG) > 0){
						;	// do nothing.
					}
					waitpid(pid, NULL, 0);
			}
		}


	}
	return 0;
}
