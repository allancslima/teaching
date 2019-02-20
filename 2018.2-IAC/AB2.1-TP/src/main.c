#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#define ARG_CPU "ucp"
#define ARG_CPU_MEM "ucp-mem"
#define ARG_STORAGE_RESULT "--save"
#define FILE_PATH_FORMAT "%s.txt"
#define FILE_TITLE "Generated file\n"

#define DEFAULT_TIME_TRACKING 10
#define DEFAULT_MEM_ALLOCATION 1

#define CMD_CPU_USAGE_FORMAT "ps u %d | awk '{print $3}' | grep -v %%"
#define CMD_MEM_USAGE_FORMAT "ps u %d | awk '{print $5}' | grep -v VSZ"
#define CMD_KILL_PROCESS_FORMAT "kill %d"

#define DISPLAY_CPU_FORMAT "CPU:  %% %s"
#define DISPLAY_CPU_MEM_FORMAT "CPU:  %% %sMEM: KB %s"

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

void kill_process(int pid)
{
	char cmd[32];
	sprintf(cmd, CMD_KILL_PROCESS_FORMAT, pid);
	system(cmd);
}

void format_file_name_date(char string[])
{
	int i = strlen(string);
	while (--i >= 0) {
		if (string[i] == ' ') string[i] = '-';
		if (string[i] == '\n') string[i] = '\0';
	}
}

FILE* create_result_file(char* date)
{
	int date_length = strlen(date);
	char* file_name_date = malloc(sizeof(char) * date_length);
	strncpy(file_name_date, date, date_length);
	format_file_name_date(file_name_date);

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

void track_process_resources_usage(int pid, int time_in_seconds, int track_memory, int storageResult)
{
	time_t start, end;
	double elapsed = 0.0;
	int elapsed_aux = -1;
	struct tm* time_info;
	FILE* result_file = NULL;
	
	time(&start);

	while (elapsed < time_in_seconds) {
		time(&end);
		elapsed = difftime(end, start);

		if (elapsed_aux == (int) elapsed) continue;
		
		elapsed_aux = elapsed;
		time_info = localtime(&end);
		char* date = asctime(time_info);
		char* resources_usage = get_process_resources_usage(pid, track_memory);
			
		printf("\n%s", date);
		printf("%s", resources_usage);

		if (!storageResult) continue;
		
		if (result_file == NULL) {
			result_file = create_result_file(date);
			fputs(FILE_TITLE, result_file);
		}
		fputs("\n", result_file);
		fputs(date, result_file);
		fputs(resources_usage, result_file);
	}
	printf("\n");
	if (result_file != NULL) fclose(result_file);

	kill_process(pid);
}

void consume_cpu()
{
	while (1) {}
}

void consume_cpu_and_memory()
{
	while (1) {
		malloc(DEFAULT_MEM_ALLOCATION);
	}
}

int main (int argc, char *argv[])
{
	if (argv[1] == NULL || (strcmp(argv[1], ARG_CPU) != 0 && strcmp(argv[1], ARG_CPU_MEM) != 0)) {
		printf("\nUse %s or %s as argument.\n\n", ARG_CPU, ARG_CPU_MEM);
		return 0;
	}
	int storageResult = 0;
	if (argv[2] != NULL && strcmp(argv[2], ARG_STORAGE_RESULT) == 0) {
		storageResult = 1;
	}

	int pid = fork();

	if (pid < 0) {
		printf("An error occurred during fork process!");
		exit(-1);
	}
	else if (pid > 0) {
		if (strcmp(argv[1], ARG_CPU) == 0) {
			track_process_resources_usage(pid, DEFAULT_TIME_TRACKING, 0, storageResult);
		}
		else if (strcmp(argv[1], ARG_CPU_MEM) == 0) {
			track_process_resources_usage(pid, DEFAULT_TIME_TRACKING, 1, storageResult);
		}
	}
	else {
		if (strcmp(argv[1], ARG_CPU) == 0) {
			consume_cpu();
		}
		else if (strcmp(argv[1], ARG_CPU_MEM) == 0) {
			consume_cpu_and_memory();
		}
	}
	return 0;
}
