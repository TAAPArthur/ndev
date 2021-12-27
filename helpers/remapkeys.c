#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>

#include "remapkeys.h"

void remap(int fd, int scancode, int keycode){
    int codes[2] = {scancode, keycode};
    if (ioctl(fd, EVIOCSKEYCODE, codes)) {
        fprintf(stderr, "Setting scancode 0x%04x with 0x%04x via ", codes[0], codes[1]);
        perror("EVIOCSKEYCODE");
    }
}

static void remap_all_keys(int fd)
{
  struct input_keymap_entry entry = {0};
  int codes[2];
  int n = 0;
  while(1) {
        entry.flags = INPUT_KEYMAP_BY_INDEX;
        entry.index = n++;
        entry.len = sizeof(u_int32_t);
        if (ioctl(fd, EVIOCGKEYCODE_V2, &entry) == -1)
            break;
        for(int i = 0; i < sizeof(mappings)/sizeof(mappings[0]); i++) {
            if(mappings[i].original_keycode == entry.keycode && (!mappings[i].scancode || mappings[i].scancode == *(int*)entry.scancode)) {
                printf("Attempting to remap scan code 0x%x to %d from %d\n", *(int*)entry.scancode, mappings[i].new_keycode, mappings[i].original_keycode);
                remap(fd, *(int*)entry.scancode, mappings[i].new_keycode);
            }
        }
    }
}

int main(int argc, char** argv) {
    for(int i = 1; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY);
        remap_all_keys(fd);
    }
}
