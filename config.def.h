#include "stddef.h"
const char* RESCAN_CMD[] = {"/bin/find", "/sys/", "-name", "uevent", "-exec", "sh", "-c", "echo add > '{}'", ";", NULL};
const char* LOG_PATH = "/var/log/ndev.log";
struct Rule {
    const char *envVar;
    const char *devRegex;
    const char *user;
    const char *group;
    int mode;
    const char *path;
    const char *cmd;
    const int noEndOnMatch;
} rules[] = {
#ifdef DEBUG
    { NULL,       ".*",          NULL  , NULL ,   0000, "!",       "*echo $(env)", .noEndOnMatch=1},
#endif
    { "DEVNAME", "null",         "root", "root",  0666, NULL,      NULL                           },
    { "DEVNAME", "zero",         "root", "root",  0666, NULL,      NULL                           },
    { "DEVNAME", "full",         "root", "root",  0666, NULL,      NULL                           },
    { "DEVNAME", "random",       "root", "root",  0666, NULL,      NULL                           },
    { "DEVNAME", "urandom",      "root", "root",  0444, NULL,      NULL                           },
    { "DEVNAME", "hwrandom",     "root", "root",  0660, NULL,      NULL                           },
    { "DEVNAME", "mem",          "root", "root",  0640, NULL,      NULL                           },
    { "DEVNAME", "kmem",         "root", "root",  0640, NULL,      NULL                           },
    { "DEVNAME", "port",         "root", "root",  0640, NULL,      NULL                           },
    { "DEVNAME", "console",      "root", "tty",   0600, NULL,      NULL                           },
    { "DEVNAME", "ptmx",         "root", "tty",   0666, NULL,      NULL                           },
    { "DEVNAME", "tty",          "root", "tty",   0666, NULL,      NULL                           },
    { "DEVNAME", "tty[0-9]+",    "root", "tty",   0660, NULL,      NULL                           },
    { "DEVNAME", "tty.+",        "root", "tty",   0660, NULL,      NULL                           },
    { "DEVNAME", "pty.*",        "root", "tty",   0660, NULL,      NULL                           },
    // load driver
    { "MODALIAS", ".+",          NULL  , NULL ,   0000, "!",       "@modprobe -v -b $MODALIAS", .noEndOnMatch=1},
    { "DEVTYPE", "partition",    NULL  , NULL ,   0000, "!",       "@modprobe -q \"$(lsblk -no FSTYPE /dev/$DEVNAME | grep -v linux_raid_member)\"", .noEndOnMatch=1},
    // libinput-zero uevent support
    { "SUBSYSTEM", "(input|drm)", NULL  , NULL ,   0000, "!",      "*env > /tmp/.libudev-zero/uevent.$$", .noEndOnMatch=1},
    // auto power on devices
    { "DEVNAME", ".*power",      "root", "root",  0660, NULL,      "@printf 1 > /sys/$DEVPATH/device/powered"},
    { "DEVNAME", "vcs[0-9]*",    "root", "tty",   0660, NULL,      NULL                           },
    { "DEVNAME", "vcsa*[0-9]*",  "root", "tty",   0660, NULL,      NULL                           },
    { "DEVNAME", "sd[a-z].*",    "root", "disk",  0660, NULL,      NULL                           },
    { "DEVNAME", "sr[0-9]*",     "root", "cdrom", 0660, NULL,      "@ln -sf $DEVNAME /dev/cdrom"  },
    { "DEVNAME", "ts[0-9]+",     "root", "root",  0660, "=input/", NULL                           },
    { "DEVNAME", "input/.*",     "root", "input", 0660, NULL,      NULL                           },
    { "DEVNAME", "dri/.*",       "root", "video", 0660, NULL,      NULL                           },
    { "DEVNAME", "snd/.*",       "root", "audio", 0660, NULL,      NULL                           },
    { "DEVNAME", "midi.*",       "root", "audio", 0660, "=snd/",   NULL                           },
    { "DEVNAME", "seq",          "root", "audio", 0660, "=snd/",   NULL                           },
    { "DEVNAME", "timer",        "root", "audio", 0660, "=snd/",   NULL                           },
    { "DEVNAME", "rtc[0-9]*",    "root", "root",  0664, NULL,      NULL                           },
    { "DEVNAME", "vbi[0-9]",     "root", "video", 0660, NULL,      NULL                           },
    { "DEVNAME", "video[0-9]",   "root", "video", 0660, NULL,      NULL                           },
    { "DEVNAME", "fuse",         "root", "root",  0666, NULL,      NULL                           },
    { "DEVNAME", ".*",           "root", "root",  0660, NULL,      NULL                           },
};

