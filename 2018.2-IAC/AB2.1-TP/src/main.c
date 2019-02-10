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

void displayProcessCpuUsage(int pid)
{
	char bash_cmd[256];
	sprintf(bash_cmd, CMD_CPU_USAGE_FORMAT, pid);
	
	FILE* pipe = popen(bash_cmd, "r");

	if (pipe == NULL)
	{
		printf("Error!");
		exit(-1);
	}

	char buffer[8];
	char* cpuUsage = fgets(buffer, sizeof(buffer), pipe);
	pclose(pipe);

	printf("CPU: %% %s\n", cpuUsage);
}

void trackProcessResourcesUsage(int pid, int timeInSeconds, int trackMemory)
{
	time_t currentTime = time(NULL);
	time_t finalTime = currentTime + timeInSeconds + 1;
	time_t seconds = currentTime;
	struct tm* timeInfo;

	while (currentTime < finalTime)
	{
		currentTime = time(NULL);
		if (currentTime != seconds)
		{
			seconds = currentTime;
			timeInfo = localtime(&seconds);
			printf("%s", asctime(timeInfo));
			displayProcessCpuUsage(pid);
		}
	}
	char cmd_kill_process[32];
	sprintf(cmd_kill_process, CMD_KILL_PROCESS_FORMAT, pid);
	system(cmd_kill_process);
}

void consumeCPU()
{
	for (;;) {}
}

void consumeCPUAndMemory()
{

}

int main (int argc, char *argv[])
{
	int pid = fork();

	if (pid < 0)
	{
		printf("An error occurred during fork process!");
		exit(-1);
	}
	else if (pid > 0)
	{
		if (strcmp(argv[1], ARG_CPU) == 0)
		{
			trackProcessResourcesUsage(pid, DEFAULT_TIME_TRACKING, 0);
		}
		else if (strcmp(argv[1], ARG_CPU_AND_MEMORY) == 0)
		{
			trackProcessResourcesUsage(pid, DEFAULT_TIME_TRACKING, 1);
		}
	}
	else
	{
		if (strcmp(argv[1], ARG_CPU) == 0)
		{
			consumeCPU();
		}
		else if (strcmp(argv[1], ARG_CPU_AND_MEMORY) == 0)
		{
			consumeCPUAndMemory();
		}
	}
	return 0;
}
