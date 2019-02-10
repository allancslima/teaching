#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#define ARG_CPU "cpu"
#define ARG_CPU_AND_MEMORY "cpu-mem"

#define DEFAULT_TIME_TRACKING 10
#define DEFAULT_MEM_ALLOCATION 1

#define CMD_CPU_MEM_USAGE_FORMAT "ps u %d | awk '{print $3, $4}' | grep -v %%"
#define CMD_KILL_PROCESS_FORMAT "kill %d"

void display_process_resources_usage(int pid, int display_mem)
{
	char bash_cmd[256];
	sprintf(bash_cmd, CMD_CPU_MEM_USAGE_FORMAT, pid);
	
	FILE* pipe = popen(bash_cmd, "r");

	if (pipe == NULL) {
		printf("Error!");
		exit(-1);
	}

	char buffer[16];
	char* resources_usage = fgets(buffer, sizeof(buffer), pipe);
	pclose(pipe);

	char *cpu_usage = strtok(resources_usage, " ");
	printf("CPU: %% %s\n", cpu_usage);
	
	if (!display_mem) return;

	char *mem_usage = strtok(NULL, " ");
	printf("MEM: %% %s", mem_usage);
}

void track_process_resources_usage(int pid, int time_in_seconds, int track_memory)
{
	time_t current_time = time(NULL);
	time_t final_time = current_time + time_in_seconds + 1;
	time_t seconds = current_time;
	struct tm* time_info;

	while (current_time < final_time) {
		current_time = time(NULL);
		if (current_time != seconds) {
			seconds = current_time;
			time_info = localtime(&seconds);
			printf("\n%s", asctime(time_info));
			display_process_resources_usage(pid, track_memory);
		}
	}
	printf("\n");

	char cmd_kill_process[32];
	sprintf(cmd_kill_process, CMD_KILL_PROCESS_FORMAT, pid);
	system(cmd_kill_process);
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
	int pid = fork();

	if (pid < 0) {
		printf("An error occurred during fork process!");
		exit(-1);
	}
	else if (pid > 0) {
		if (strcmp(argv[1], ARG_CPU) == 0) {
			track_process_resources_usage(pid, DEFAULT_TIME_TRACKING, 0);
		}
		else if (strcmp(argv[1], ARG_CPU_AND_MEMORY) == 0) {
			track_process_resources_usage(pid, DEFAULT_TIME_TRACKING, 1);
		}
	}
	else {
		if (strcmp(argv[1], ARG_CPU) == 0) {
			consume_cpu();
		}
		else if (strcmp(argv[1], ARG_CPU_AND_MEMORY) == 0) {
			consume_cpu_and_memory();
		}
	}
	return 0;
}
