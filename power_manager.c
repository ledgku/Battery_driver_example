#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#define DEVICE_FILE_NAME "/dev/my_battery"

void* thresholdBtn(void *arg);

pthread_t ntid;

static char power_save_mode = 0;
static void sig_usr(int);

static char wbuf[10] = "Write";
static char rbuf[10] = "Read";
static char choice = 0;

int main(int argc, char *argv[]) {

	int fd, i, err, status;	

	fd = open("/proc/battery_notify", O_RDWR | O_NDELAY);

	if (fd < 0)
	{
		fprintf(stderr, "/proc/battery_notify open error!!\n");
		return 1;
	}

	pid_t pid = getpid();
	sprintf(wbuf, "%ld", (long)pid);
	write(fd, wbuf, 10);
	close(fd);

	err = pthread_create(&ntid, NULL, thresholdBtn, NULL);

	if (err != 0)
	{
		fprintf(stderr, "pthread_create error!!\n");
		exit(1);
	}

	if (signal(SIGUSR1, sig_usr) == SIG_ERR)
	{
		fprintf(stderr, "can't catch SIGUSR1\n");
		return 1;
	}

	if (signal(SIGUSR2, sig_usr) == SIG_ERR)
	{
		fprintf(stderr, "can't catch SIGUSR2\n");
		return 1;
	}

	while(1)
	{
		system("clear");

		fd = open(DEVICE_FILE_NAME, O_RDWR | O_NDELAY);

		read(fd, rbuf, 4);
		status = atoi(rbuf);

		printf("If you want to change threshold, press y\n");
		printf("battery status : %d%\n", status);
		if (power_save_mode)
			printf("power saving mode\n");

		printf("----------\n");
		for (i=0; i<=status/10; ++i)
			printf("|");
		printf("\n----------\n");

		sleep(1);

		if (choice == 'y')
			sleep(5);

		close(fd);
	}

	return 0;

}


void* thresholdBtn(void *arg)
{
	int fd, threshold;	
	while(1)
	{
		scanf("%c", &choice);

		if (choice == 'y')
		{
			fd = open("/proc/battery_threshold", O_RDWR | O_NDELAY);
			printf("-----\tchange battery threshold\t-----\n");
			read(fd, rbuf, 10);
			threshold = atoi(rbuf);
			printf("Max : 100, Min : 0, Now : %d\n", threshold);
			printf("Enter threshold : ");
			scanf("%d", &threshold);

			if (threshold >= 0 && threshold <= 100)
			{
				sprintf(wbuf, "%d", threshold);
				write(fd, wbuf, 10);
			}

			close(fd);
		}

		choice = 0;
	}
}

static void sig_usr(int signo)
{
	if (signo == SIGUSR1)
	{
		if (signal(SIGUSR1, sig_usr) == SIG_ERR)
		{
			fprintf(stderr, "can't catch SIGUSR1\n");
			return 1;
		}
		power_save_mode = 1;
	}
	
	else if (signo == SIGUSR2)
	{
		if (signal(SIGUSR2, sig_usr) == SIG_ERR)
		{
			fprintf(stderr, "can't catch SIGUSR2\n");
			return 1;
		}
		power_save_mode = 0;
	}
		
	return;
}
