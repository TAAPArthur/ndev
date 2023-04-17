#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/wait.h>
#include <unistd.h>

#include "config.h"

#ifdef VERBOSE
#define DEBUG(X...) printf(X)
#else
#define DEBUG(X...)
#endif
#define LOG(X...) printf(X)

#define LEN(A) sizeof(A)/sizeof(A[0])
int matches(const char*devRegex, const char* str) {
    regex_t regex;
    if(regcomp(&regex, devRegex, REG_EXTENDED) < 0)
        perror("Failed to compile regex");
    regmatch_t match;
    int ret = regexec(&regex, str, 1, &match, 0);
    regfree(&regex);
    return ret != REG_NOMATCH && match.rm_so == 0 && match.rm_eo == strlen(str);
}
int devtype(const char* major, const char *minor) {
	char path[255];
	snprintf(path, sizeof(path), "/sys/dev/block/%s:%s", major, minor);
	if (!access(path, F_OK))
		return S_IFBLK;
	snprintf(path, sizeof(path), "/sys/dev/char/%s:%s", major, minor);
	if (!access(path, F_OK))
		return S_IFCHR;
	return -1;
}
void ownFile(const struct Rule* rule, const char*path) {
	struct passwd *pw;
	struct group *gr;
	pw = getpwnam(rule->user);
    int uid=-1, gid=-1;
    if(pw)
        uid=pw->pw_uid;
    else
        LOG("No such user %s\n", rule->user);
	gr = getgrnam(rule->group);
    if(gr)
        gid=gr->gr_gid;
    else
        LOG("No such group %s\n", rule->group);

	if (chown(path, uid, gid) < 0)
		perror("Couldn't chown the device path");
}
void addDevice(const struct Rule* rule, const char*path) {
    const char* major=getenv("MAJOR"), *minor = getenv("MINOR");
    int type = devtype(major, minor);
    if (type == -1) {
        perror("Failed to find device");
        return;
    }
    if (mknod(path, rule->mode | type, makedev(atoi(major), atoi(minor))) < 0) {
       if(errno == EEXIST) {
           LOG("Device already exists; Updating permissions to %o\n", rule->mode);
           if(chmod(path, rule->mode))
              perror("Could not change device mode");
       } else {
          perror("Could not create device");
          return;
       }
    }
    ownFile(rule, path);
}

void createRemoveDevice(const struct Rule* rule, int add) {
    char path[1024]="/dev/";
    const char* devname = getenv("DEVNAME");
    if(!devname) {
        LOG("Could not %s device because devname is null\n", add ? "add" : "remove", devname);
        return;
    }
    if(!rule->path)
        strcat(path, devname);
    else if(rule->path[strlen(rule->path)-1] == '/'){
        strcpy(path, rule->path+1);
        if(add)
            mkdir(path, 755);
        strcat(path, devname);
    }
    else
        strcpy(path, rule->path+1);

    LOG("%s device: %s\n", add?"Adding":"Removing", path);
    if(add)
        addDevice(rule, path);
    else
        unlink(path);

    if(rule->path && rule->path[0] == '<') {
        char linkPath[255]="/dev/";
        strcat(linkPath, getenv("DEVNAME"));
        LOG("%s link: %s", add?"Adding":"Removing", path);
        if(add)
            symlink(path, linkPath);
        else
            unlink(linkPath);
    }
}

void pipeToSysLogger() {
    int fds[2];
    pipe(fds);
    if(!fork()) {
        dup2(fds[0], STDIN_FILENO);
        close(fds[0]);
        close(fds[1]);
        execv(*LOGGER_CMD, LOGGER_CMD);
        perror("Failed to exec");
        exit(1);
    }
    dup2(fds[1], STDOUT_FILENO);
    dup2(fds[1], STDERR_FILENO);
}

int main(int argc, char *argv[]) {
    if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 's')
        execv(RESCAN_CMD[0], RESCAN_CMD);
    else {

        int fd = open(LOG_PATH, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 644);
        if(fd != -1) {
            if(dup2(fd, STDOUT_FILENO) == -1 || dup2(fd, STDERR_FILENO) == -1 ){
                perror("Failed to dup fd");
            }
        } else {
            perror("Failed to open log file");
        }
        pipeToSysLogger();
        DEBUG("Received event ACTION: %s DEVNAME: %s \n", getenv("ACTION"), getenv("DEVNAME"));
        if(!getenv("ACTION")) {
            LOG("Env isn't setup for hotplug\n");
            exit(1);
        }
        int add = strcmp(getenv("ACTION"), "add") == 0;
        int remove = strcmp(getenv("ACTION"), "remove") == 0;
        int change = strcmp(getenv("ACTION"), "change") == 0;
        if(!add && !remove && !change) {
            LOG("unknown action %s", getenv("ACTION"));
            return 0;
        }
        for(int i=0; i < LEN(rules); i++){
            if(!rules[i].envVar || getenv(rules[i].envVar) && matches(rules[i].devRegex, getenv(rules[i].envVar))) {
                DEBUG("Rule %d matched: '%s' '%s' CMD: %s\n", i, rules[i].envVar, rules[i].devRegex, rules[i].cmd);
                if(!change && (!rules[i].path || rules[i].path && rules[i].path[0] != '!'))
                    createRemoveDevice(rules + i, add);
                if(rules[i].cmd) {
                    if( rules[i].cmd[0] == '*' && (add || remove) || rules[i].cmd[0] == '@' && add || rules[i].cmd[0] == '$' && remove || rules[i].cmd[0] == '#' && change) {
                        LOG("Running cmd %s\n", &rules[i].cmd[1]);
                        system(&rules[i].cmd[1]);
                    }
                }
                if(!rules[i].noEndOnMatch)
                    break;
            }
        }
    }
}
