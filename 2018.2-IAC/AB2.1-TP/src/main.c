#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#define ARG_CPU "cpu"
#define ARG_CPU_MEM "cpu-mem"
#define ARG_STORAGE_RESULT "--save"
#define FILE_PATH_FORMAT "%s.txt"
#define FILE_TITLE "Generated file\n"

#define DEFAULT_TIME_TRACKING 10 // 10 seconds
#define DEFAULT_MEM_ALLOCATION 1 // 1 byte

#define CMD_CPU_USAGE_FORMAT "ps u %d | awk '{print $3}' | grep -v %%"
#define CMD_MEM_USAGE_FORMAT "ps u %d | awk '{print $5}' | grep -v VSZ"
#define CMD_KILL_PROCESS_FORMAT "kill %d"

#define DISPLAY_CPU_FORMAT "CPU:  %% %s"
#define DISPLAY_CPU_MEM_FORMAT "CPU:  %% %sMEM: KB %s"

/*
 * get_process_resources_usage - retrieve cpu usage in percentage and memory usage in KB to a process 
 *
 * @pid: process id to be checked
 * @display_mem: boolean to display memory usage too
 *
 * returns a pointer to resources usage string
*/
char* get_process_resources_usage(int pid, int display_mem)
{
	char bash_cmd[256];
	sprintf(bash_cmd, CMD_CPU_USAGE_FORMAT, pid);
	
	FILE* pipe = popen(bash_cmd, "r");

	if (pipe == NULL) {
		perror("Error");
		exit(-1);
	}

	char buffer[64];
	char* cpu_usage = fgets(buffer, sizeof(buffer), pipe);
	
	int result_length = sizeof(char) * (strlen(DISPLAY_CPU_FORMAT) - 1 + strlen(cpu_usage));
	char* result = malloc(result_length);
	sprintf(result, DISPLAY_CPU_FORMAT, cpu_usage);

	if (display_mem) {
		// putting memory usage
		
		sprintf(bash_cmd, CMD_MEM_USAGE_FORMAT, pid);
		pipe = popen(bash_cmd, "r");
		
		char buffer[64];
		char* mem_usage = fgets(buffer, sizeof(buffer), pipe);

		result_length = sizeof(char) * (strlen(DISPLAY_CPU_MEM_FORMAT) - 2 + strlen(cpu_usage) + strlen(mem_usage));
		result = realloc(result, result_length);
		sprintf(result, DISPLAY_CPU_MEM_FORMAT, cpu_usage, mem_usage);
	}
	pclose(pipe);
	
	return result;
}

/*
 * kill_process - kill process
 *
 * @pid: process id to be killed
*/
void kill_process(int pid)
{
	char cmd[32];
	sprintf(cmd, CMD_KILL_PROCESS_FORMAT, pid);
	system(cmd);
}

/*
 * format_date_as_file_name - remove white spaces from date string
 *
 * @string: to be formatted
*/
void format_date_as_file_name(char string[])
{
	int i = strlen(string);
	while (--i >= 0) {
		if (string[i] == ' ') string[i] = '-';
		if (string[i] == '\n') string[i] = '\0';
	}
}

/*
 * create_result_file - create a file with a given date
 *
 * @date: date to be the file name
 *
 * returns a pointer to created file
*/
FILE* create_result_file(char* date)
{
	int date_length = strlen(date);
	char* file_name_date = malloc(sizeof(char) * date_length);
	strncpy(file_name_date, date, date_length);
	format_date_as_file_name(file_name_date);

	int file_name_length = sizeof(char) * (strlen(FILE_PATH_FORMAT) - 1 + strlen(file_name_date));
	char* file_name = malloc(file_name_length);
	sprintf(file_name, FILE_PATH_FORMAT, file_name_date);

	FILE* file = fopen(file_name, "w");

	if (file == NULL) {
		perror("Error");
		exit(-1);
	}
	return file;
}

/*
 * track_process_resources_usage - capture cpu/memory usage for N seconds being able to storage result file
 *
 * @pid: process id that will be tracked
 * @time_in_seconds: for how long the process will be tracked
 * @track_memory: boolean to display memory usage too
 * @storage_result: boolean to storage result file too
*/
void track_process_resources_usage(int pid, int time_in_seconds, int track_memory, int storage_result)
{
	time_t start, end;
	double elapsed = 0.0;
	int elapsed_aux = -1; // auxiliary to detect seconds change
	struct tm* time_info; // struct to set date informations
	FILE* result_file = NULL;
	
	time(&start);

	while (elapsed < time_in_seconds) {
		// updating current time
		time(&end);
		elapsed = difftime(end, start);

		if (elapsed_aux == (int) elapsed) continue;
		
		// the seconds have changed, so display/storage resources usage

		elapsed_aux = elapsed;
		time_info = localtime(&end);
		char* date = asctime(time_info);
		char* resources_usage = get_process_resources_usage(pid, track_memory);
			
		printf("\n%s", date); // printing current time
		printf("%s", resources_usage); // printing resources usage

		if (!storage_result) continue;
		
		if (result_file == NULL) {
			result_file = create_result_file(date); // creating file with current time as name
			fputs(FILE_TITLE, result_file);
		}
		fputs("\n", result_file);
		fputs(date, result_file); // putting current time on result file
		fputs(resources_usage, result_file); // putting resources usage on result file
	}
	printf("\n");
	if (result_file != NULL) fclose(result_file);

	kill_process(pid); // killing the process
}

/**
 * consume_cpu: run a simple infinite loop
*/
void consume_cpu()
{
	while (1) {}
}

/**
 * consume_cpu_and_memory - run a simple infinite loop that allocates memory successively
*/
void consume_cpu_and_memory()
{
	while (1) {
		malloc(DEFAULT_MEM_ALLOCATION);
	}
}

int main (int argc, char *argv[])
{
	// handling wrong input argument
	if (argv[1] == NULL || (strcmp(argv[1], ARG_CPU) != 0 && strcmp(argv[1], ARG_CPU_MEM) != 0)) {
		printf("\nUse %s or %s as argument.\n\n", ARG_CPU, ARG_CPU_MEM);
		return 0;
	}
	// setting boolean variable of result storage option
	int storage_result = 0;
	if (argv[2] != NULL && strcmp(argv[2], ARG_STORAGE_RESULT) == 0) {
		storage_result = 1;
	}

	int pid = fork(); // replicating the current process into a new

	if (pid < 0) {
		perror("An error occurred during fork process");
		exit(-1);
	}
	else if (pid > 0) {
		// handling input arguments running the resources tracking function
		if (strcmp(argv[1], ARG_CPU) == 0) {
			track_process_resources_usage(pid, DEFAULT_TIME_TRACKING, 0, storage_result);
		}
		else if (strcmp(argv[1], ARG_CPU_MEM) == 0) {
			track_process_resources_usage(pid, DEFAULT_TIME_TRACKING, 1, storage_result);
		}
	}
	else {
		// handling input arguments running the resources consumption functions
		if (strcmp(argv[1], ARG_CPU) == 0) {
			consume_cpu();
		}
		else if (strcmp(argv[1], ARG_CPU_MEM) == 0) {
			consume_cpu_and_memory();
		}
	}
	return 0;
}
