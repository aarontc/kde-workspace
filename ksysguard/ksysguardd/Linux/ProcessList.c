/*
    KTop, the KDE Task Manager
   
	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	$Id$
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/user.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include <signal.h>
#include <errno.h>

#include "ccont.h"
#include "Command.h"
#include "ProcessList.h"

#ifndef PAGE_SIZE /* Needed for SPARC */
#include <asm/page.h>
#endif

static CONTAINER ProcessList = 0;
static int TimeOut = 0;

#define BUFSIZE 1024
#define KDEINITLEN strlen("kdeinit: ")

typedef struct
{
	/* This flag is set for all found processes at the beginning of the
	 * process list update. Processes that do not have this flag set will
	 * be assumed dead and removed from the list. The flag is cleared after
	 * each list update. */
	int alive;

	/* the process ID */
	pid_t pid;

	/* the parent process ID */
	pid_t ppid;

	/* the real user ID */
	uid_t uid;

	/* the real group ID */
	gid_t gid;

	/* a character description of the process status */
    char status;

	/* the number of the tty the process owns */
	int ttyNo;

	/*
	 * The nice level. The range should be -20 to 20. I'm not sure
	 * whether this is true for all platforms.
	 */
	int niceLevel;

	/*
	 * The scheduling priority.
	 */
	int priority;

	/*
	 * The total amount of memory the process uses. This includes shared and
	 * swapped memory.
	 */
	unsigned int vmSize;

	/*
	 * The amount of physical memory the process currently uses.
	 */
	unsigned int vmRss;

	/*
	 * The amount of memory (shared/swapped/etc) the process shares with
	 * other processes.
	 */
	unsigned int vmLib;

	/*
	 * The number of 1/100 of a second the process has spend in user space.
	 * If a machine has an uptime of 1 1/2 years or longer this is not a
	 * good idea. I never thought that the stability of UNIX could get me
	 * into trouble! ;)
	 */
	unsigned int userTime;

	/*
	 * The number of 1/100 of a second the process has spend in system space.
	 * If a machine has an uptime of 1 1/2 years or longer this is not a
	 * good idea. I never thought that the stability of UNIX could get me
	 * into trouble! ;)
	 */
	unsigned int sysTime;

	/* system time as multime of 100ms */
	int centStamp;

	/* the current CPU load (in %) from user space */
	double userLoad;

	/* the current CPU load (in %) from system space */
	double sysLoad;

	/* the name of the process */
	char name[64];

	/* the command used to start the process */
	char cmdline[256];

	/* the login name of the user that owns this process */
	char userName[32];
} ProcessInfo;

static unsigned ProcessCount;

static void
validateStr(char* str)
{
	char* s = str;

	/* All characters that could screw up the communication will be
	 * removed. */
	while (*s)
	{
		if (*s == '\t' || *s == '\n' || *s == '\r')
			*s = ' ';
		++s;
	}
	/* Make sure that string contains at least one character (blank). */
	if (str[0] == '\0')
		strcpy(str, " ");
}

static int 
processCmp(void* p1, void* p2)
{
	return (((ProcessInfo*) p1)->pid - ((ProcessInfo*) p2)->pid);
}

static ProcessInfo*
findProcessInList(int pid)
{
	ProcessInfo key;
	long index;

	key.pid = pid;
	if ((index = search_ctnr(ProcessList, processCmp, &key)) < 0)
		return (0);

	return (get_ctnr(ProcessList, index));
}

static int
updateProcess(int pid)
{
	ProcessInfo* ps;
	FILE* fd;
	char buf[BUFSIZE];
	int userTime, sysTime;
	struct passwd* pwent;

	if ((ps = findProcessInList(pid)) == 0)
	{
		struct timeval tv;

		ps = (ProcessInfo*) malloc(sizeof(ProcessInfo));
		ps->pid = pid;

		gettimeofday(&tv, 0);
		ps->centStamp = tv.tv_sec * 100 + tv.tv_usec / 10000;

		push_ctnr(ProcessList, ps);
		bsort_ctnr(ProcessList, processCmp, 0);
	}

	snprintf(buf, BUFSIZE - 1, "/proc/%d/status", pid);
	if((fd = fopen(buf, "r")) == 0)
	{
		/* process has terminated in the mean time */
		return (-1);
	}

	fscanf(fd, "%*s %63s", ps->name);
	validateStr(ps->name);
	fscanf(fd, "%*s %*c %*s");
	fscanf(fd, "%*s %*d");
	fscanf(fd, "%*s %*d");
	fscanf(fd, "%*s %d %*d %*d %*d", (int*) &ps->uid);
	fscanf(fd, "%*s %*d %*d %*d %*d");
	fscanf(fd, "%*s %*d %*d %*d %*d");
	fscanf(fd, "%*s %*d %*s");	/* VmSize */
	fscanf(fd, "%*s %*d %*s");	/* VmLck */
	fscanf(fd, "%*s %*d %*s");	/* VmRSS */
	fscanf(fd, "%*s %*d %*s");	/* VmData */
	fscanf(fd, "%*s %*d %*s");	/* VmStk */
	fscanf(fd, "%*s %*d %*s");	/* VmExe */
	if (fscanf(fd, "%8s %d %*s", buf, &ps->vmLib) != 2) 	/* VmLib */
		return (-1);
	buf[7] = '\0';
	if (strcmp(buf, "VmLib:") != 0)
		ps->vmLib = 0;
	else
		ps->vmLib *= 1024;

	fclose(fd);

    snprintf(buf, BUFSIZE - 1, "/proc/%d/stat", pid);
	buf[BUFSIZE - 1] = '\0';
	if ((fd = fopen(buf, "r")) == 0)
		return (-1);

	if (fscanf(fd, "%*d %*s %c %d %d %*d %d %*d %*u %*u %*u %*u %*u %d %d"
			   "%*d %*d %*d %d %*u %*u %*d %u %u",
			   &ps->status, (int*) &ps->ppid, (int*) &ps->gid, &ps->ttyNo,
			   &userTime, &sysTime, &ps->niceLevel, &ps->vmSize,
			   &ps->vmRss) != 9)
		return (-1);
	if (!strchr("SRZTD", ps->status))
	{
		/* fprintf(stderr, "Unknown status %c\n", ps->status); */
		ps->status = '-';
	}

	ps->vmRss = (ps->vmRss + 3) * PAGE_SIZE;

	{
		int newCentStamp;
		int timeDiff, userDiff, sysDiff;
		struct timeval tv;

		gettimeofday(&tv, 0);
		newCentStamp = tv.tv_sec * 100 + tv.tv_usec / 10000;

		timeDiff = newCentStamp - ps->centStamp;
		userDiff = userTime - ps->userTime;
		sysDiff = sysTime - ps->sysTime;

		if ((timeDiff > 0) && (userDiff > 0) && (sysDiff > 0))
		{
			ps->userLoad = ((double) userDiff / timeDiff) * 100.0;
			ps->sysLoad = ((double) sysDiff / timeDiff) * 100.0;
			/* During startup we get bigger loads since the time diff
			 * cannot be correct. So we force it to 0. */
			if (ps->userLoad > 100.0)
				ps->userLoad = 0.0;
			if (ps->sysLoad > 100.0)
				ps->sysLoad = 0.0;
		}
		else
			ps->sysLoad = ps->userLoad = 0.0;

		ps->centStamp = newCentStamp;
		ps->userTime = userTime;
		ps->sysTime = sysTime;
	}

	fclose(fd);

	snprintf(buf, BUFSIZE - 1, "/proc/%d/cmdline", pid);
	if ((fd = fopen(buf, "r")) == 0)
		return (-1);

	ps->cmdline[0] = '\0';
	sprintf(buf, "%%%d[^\n]", sizeof(ps->cmdline) - 1);
	fscanf(fd, buf, ps->cmdline);
	ps->cmdline[sizeof(ps->cmdline) - 1] = '\0';
	validateStr(ps->cmdline);
	fclose(fd);

	/* Ugly hack to "fix" program name for kdeinit lauched programs. */
	if (strcmp(ps->name, "kdeinit") == 0 &&
		strncmp(ps->cmdline, "kdeinit: ", KDEINITLEN) == 0)
	{
		size_t len;
		char* end = strchr(ps->cmdline + KDEINITLEN, ' ');
		if (end)
			len = end - ps->cmdline;
		else
			len = strlen(ps->cmdline + KDEINITLEN);
		if (len > 0)
		{
			if (len > sizeof(ps->name) - 1)
				len = sizeof(ps->name) - 1;
			strncpy(ps->name, ps->cmdline + KDEINITLEN, len);
			ps->name[len] = '\0';
		}
	}

	/* find out user name with the process uid */
	pwent = getpwuid(ps->uid);
	if (pwent)
	{
		strncpy(ps->userName, pwent->pw_name, sizeof(ps->userName) - 1);
		ps->userName[sizeof(ps->userName) - 1] = '\0';
		validateStr(ps->userName);
	}

	ps->alive = 1;

	return (0);
}

static void
cleanupProcessList(void)
{
	int i;

	ProcessCount = 0;
	/* All processes that do not have the active flag set are assumed dead
	 * and will be removed from the list. The alive flag is cleared. */
	for (i = 0; i < level_ctnr(ProcessList); i++)
	{
		ProcessInfo* ps = get_ctnr(ProcessList, i);
		if (ps->alive)
		{
			/* Process is still alive. Just clear flag. */
			ps->alive = 0;
			ProcessCount++;
		}
		else
		{
			/* Process has probably died. We remove it from the list and
			 * destruct the data structure. i needs to be decremented so
			 * that after i++ the next list element will be inspected. */
			free(remove_ctnr(ProcessList, i--));
		}
	}
}

/*
================================ public part =================================
*/

void
initProcessList(void)
{
	ProcessList = new_ctnr(CT_DLL);

	registerMonitor("pscount", "integer", printProcessCount,
					printProcessCountInfo);
	registerMonitor("ps", "table", printProcessList, printProcessListInfo);

	registerCommand("kill", killProcess);
	registerCommand("setpriority", setPriority);
}

void
exitProcessList(void)
{
	if (ProcessList)
		free (ProcessList);
}

int
updateProcessList(void)
{
	DIR* dir;
	struct dirent* entry;

	/* If the process information has not been requested for 10 intervals
	 * we save CPU time by not updating the list. The next request will
	 * re-enable the updates again. */
	if (TimeOut > 10)
		return (0);
	TimeOut++;

	/* read in current process list via the /proc filesystem entry */
	if ((dir = opendir("/proc")) == NULL)
	{
		fprintf(stderr, "ERROR: Cannot open directory \'/proc\'!\n"
				"The kernel needs to be compiled with support\n"
				"for /proc filesystem enabled!");
		return (-1);
	}
	while ((entry = readdir(dir))) 
	{
		if (isdigit(entry->d_name[0]))
		{
			int pid;

			pid = atoi(entry->d_name);
			updateProcess(pid);
		}
	}
	closedir(dir);

	cleanupProcessList();

	return (0);
}

void
printProcessListInfo(const char* cmd)
{
	printf("Name\tPID\tPPID\tUID\tGID\tStatus\tUser%%\tSystem%%\tNice\tVmSize"
		   "\tVmRss\tVmLib\tLogin\tCommand\n");
	printf("s\td\td\td\td\ts\td\td\td\td\tf\tf\ts\ts\n");
}

void
printProcessList(const char* cmd)
{
	int i;

	TimeOut = 0;
	for (i = 0; i < level_ctnr(ProcessList); i++)
	{
		ProcessInfo* ps = get_ctnr(ProcessList, i);

		printf("%s\t%ld\t%ld\t%ld\t%ld\t%c\t%.2f\t%.2f\t%d\t%d\t%d\t%d"
			   "\t%s\t%s\n",
			   ps->name, (long) ps->pid, (long) ps->ppid,
			   (long) ps->uid, (long) ps->gid, ps->status,
			   ps->userLoad, ps->sysLoad, ps->niceLevel, 
			   ps->vmSize / 1024, ps->vmRss / 1024, ps->vmLib / 1024,
			   ps->userName, ps->cmdline);
	}
}

void
printProcessCount(const char* cmd)
{
	TimeOut = 0;
	printf("%d\n", ProcessCount);
}

void
printProcessCountInfo(const char* cmd)
{
	printf("Number of Processes\t1\t65535\t\n");
}

void
killProcess(const char* cmd)
{
	int sig, pid;

	sscanf(cmd, "%*s %d %d", &pid, &sig);
	if (kill((pid_t) pid, sig))
	{
		switch(errno)
		{
		case EINVAL:
			printf("4\n");
			break;
		case ESRCH:
			printf("3\n");
			break;
		case EPERM:
			printf("2\n");
			break;
		default:
			printf("1\n");	/* unkown error */
			break;
		}
	}
	else
		printf("0\n");
}

void
setPriority(const char* cmd)
{
	int pid, prio;

	sscanf(cmd, "%*s %d %d", &pid, &prio);
	if (setpriority(PRIO_PROCESS, pid, prio))
	{
		switch(errno)
		{
		case EINVAL:
			printf("4\n");
			break;
		case ESRCH:
			printf("3\n");
			break;
		case EPERM:
		case EACCES:
			printf("2\n");
			break;
		default:
			printf("1\n");	/* unkown error */
			break;
		}
	}
	else
		printf("0\n");
}
