#include "stddef.h"
static char* RESCAN_CMD[] = {"/bin/find", "/sys/", "-name", "uevent", "-exec", "sh", "-c", "echo add > \"$1\"", "_", "{}", ";", NULL};
static char* LOGGER_CMD[] = {"/bin/logger", "-i", "-s", "-t", "ndev", NULL};
static const char* LOG_PATH = "/var/log/ndev.log";

typedef enum {
    CREATE_DEV = 1,
    REMOVE_DEV = 2,
} PathRule;
typedef struct {
    const char* action;
    char prefixes[4];
    PathRule pathRule;
} ActionType;

static ActionType actionTypes[] =  {
    // from mdev
    {"add",    {'@', '*'}, CREATE_DEV},
    {"remove", {'$', '*'}, REMOVE_DEV},
    // New
    {"change", {'#'}},
    {"bind",   {'!'}},
    {"unbind", {'%'}},
};

static const struct Rule {
    const char *envVar;
    const char *devRegex;
    const char *user;
    const char *group;
    int mode;
    const char *path;
    const char *cmd;
    const int noEndOnMatch;
} rules[] = {
#ifdef ENV_DUMP
    { NULL,       ".*",          NULL  , NULL ,   0000, "!",       "*echo $(env)", .noEndOnMatch=1},
    { NULL,       ".*",          NULL  , NULL ,   0000, "!",       "#echo $(env)", .noEndOnMatch=1},
#endif
    // load driver
    { "MODALIAS", ".+",          NULL  , NULL ,   0000, "!",       "@modprobe -v -b $MODALIAS", .noEndOnMatch=1},
    { "DEVTYPE", "partition",    NULL  , NULL ,   0000, "!",       "@blkid /dev/$DEVNAME | grep -q TYPE && modprobe -q \"$(blkid /dev/$DEVNAME)\"", .noEndOnMatch=1},
    // libinput-zero uevent support
    { "SUBSYSTEM", "input",      NULL  , "input", 0660, NULL,      "*libudev-zero-helper", .noEndOnMatch=1},
    { "SUBSYSTEM", "drm",        NULL  , "input", 0660, NULL,      "*libudev-zero-helper", .noEndOnMatch=1},
    // change ownership of specified led in /sys/class/leds
    { "SUBSYSTEM", "leds",       NULL , NULL,   0000,  "!",        "@chmod g+w /sys/$DEVPATH/brightness /sys/$DEVPATH/trigger; chown :sys /sys/$DEVPATH/brightness /sys/$DEVPATH/trigger"},
    { "SUBSYSTEM", "backlight",  NULL , NULL,   0000,  "!",        "@chmod g+w /sys/$DEVPATH/brightness; chown :sys /sys/$DEVPATH/brightness"},
    // auto power on devices
    { "DEVNAME", ".*power",      "root", "root",  0660, NULL,      "@printf 1 > /sys/$DEVPATH/device/powered"},
    // Standard permission changes and linking
    { "DEVNAME", "console",      "root", "tty",   0600, NULL,      NULL                           },
    { "DEVNAME", "dri/.*",       "root", "video", 0660, NULL,      NULL                           },
    { "DEVNAME", "fb[0-9]",      "root", "input", 0660, NULL,      NULL                           },
    { "DEVNAME", "full",         "root", "root",  0666, NULL,      NULL                           },
    { "DEVNAME", "fuse",         "root", "root",  0666, NULL,      NULL                           },
    { "DEVNAME", "hidraw[0-9]+", "root", "input", 0660, NULL,      NULL                           },
    { "DEVNAME", "hwrandom",     "root", "root",  0660, NULL,      NULL                           },
    { "DEVNAME", "input/.*",     "root", "input", 0660, NULL,      "@remapkeys /dev/$DEVNAME"     },
    { "DEVNAME", "kmem",         "root", "root",  0640, NULL,      NULL                           },
    { "DEVNAME", "media[0-9]",   "root", "video", 0660, NULL,      NULL                           },
    { "DEVNAME", "mem",          "root", "root",  0640, NULL,      NULL                           },
    { "DEVNAME", "midi.*",       "root", "audio", 0660, "=snd/",   NULL                           },
    { "DEVNAME", "mmcblk[0-9].*","root", "disk",  0660, NULL,      NULL                           },
    { "DEVNAME", "null",         "root", "root",  0666, NULL,      NULL                           },
    { "DEVNAME", "nvme[0-9].*",  "root", "disk",  0660, NULL,      NULL                           },
    { "DEVNAME", "port",         "root", "root",  0640, NULL,      NULL                           },
    { "DEVNAME", "ptmx",         "root", "tty",   0666, NULL,      NULL                           },
    { "DEVNAME", "pty.*",        "root", "tty",   0660, NULL,      NULL                           },
    { "DEVNAME", "random",       "root", "root",  0666, NULL,      NULL                           },
    { "DEVNAME", "rfkill",       "root", "sys",   0666, NULL,      NULL                           },
    { "DEVNAME", "rtc0",         "root", "sys",   0664, NULL,      "@ln -sf $DEVNAME /dev/rtc"    },
    { "DEVNAME", "rtc[0-9]*",    "root", "sys",   0664, NULL,      NULL                           },
    { "DEVNAME", "sd[a-z].*",    "root", "disk",  0660, NULL,      NULL                           },
    { "DEVNAME", "seq",          "root", "audio", 0660, "=snd/",   NULL                           },
    { "DEVNAME", "snd/.*",       "root", "audio", 0660, NULL,      NULL                           },
    { "DEVNAME", "sr[0-9]*",     "root", "disk",  0660, NULL,      "@ln -sf $DEVNAME /dev/cdrom"  },
    { "DEVNAME", "timer",        "root", "audio", 0660, "=snd/",   NULL                           },
    { "DEVNAME", "ts[0-9]+",     "root", "root",  0660, "=input/", NULL                           },
    { "DEVNAME", "tty",          "root", "tty",   0666, NULL,      NULL                           },
    { "DEVNAME", "tty.+",        "root", "tty",   0660, NULL,      NULL                           },
    { "DEVNAME", "tty[0-9]+",    "root", "tty",   0660, NULL,      NULL                           },
    { "DEVNAME", "uinput",       "root", "input", 0660, NULL,      NULL                           },
    { "DEVNAME", "urandom",      "root", "root",  0444, NULL,      NULL                           },
    { "DEVNAME", "vbi[0-9]",     "root", "video", 0660, NULL,      NULL                           },
    { "DEVNAME", "vcs[0-9]*",    "root", "tty",   0660, NULL,      NULL                           },
    { "DEVNAME", "vcsa*[0-9]*",  "root", "tty",   0660, NULL,      NULL                           },
    { "DEVNAME", "video[0-9]",   "root", "video", 0660, NULL,      NULL                           },
    { "DEVNAME", "zero",         "root", "root",  0666, NULL,      NULL                           },
    // Everything else
    { "DEVNAME", ".*",           "root", "root",  0660, NULL,      NULL                           },
};
