#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#define ARG_CPU "cpu"
#define ARG_CPU_AND_MEMORY "cpu-mem"

#define DEFAULT_TIME_TRACKING 10
#define DEFAULT_MEMORY_ALLOCATION 1000000

#define CMD_CPU_USAGE_FORMAT "ps u %d | awk '{print $3}' | grep -v CPU"
#define CMD_KILL_PROCESS_FORMAT "kill %d"

void display_process_cpu_usage(int pid)
{
	char bash_cmd[256];
	sprintf(bash_cmd, CMD_CPU_USAGE_FORMAT, pid);
	
	FILE* pipe = popen(bash_cmd, "r");

	if (pipe == NULL) {
		printf("Error!");
		exit(-1);
	}

	char buffer[8];
	char* cpu_usage = fgets(buffer, sizeof(buffer), pipe);
	pclose(pipe);

	printf("CPU: %% %s\n", cpu_usage);
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
			printf("%s", asctime(time_info));
			display_process_cpu_usage(pid);
		}
	}
	char cmd_kill_process[32];
	sprintf(cmd_kill_process, CMD_KILL_PROCESS_FORMAT, pid);
	system(cmd_kill_process);
}

void consume_cpu()
{
	for (;;) {}
}

void consume_cpu_and_memory()
{

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
