#include <linux/input.h>
typedef struct {
    unsigned int original_keycode;
    unsigned int new_keycode;
    unsigned int scancode;
} KeyMappings;
KeyMappings mappings[] = {
    {KEY_KEYBOARD, KEY_PLAYPAUSE},
    {KEY_MICMUTE, KEY_F20},
};
