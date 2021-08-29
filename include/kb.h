#ifndef KB_H
#define KB_H

// kbhit
#include <stdio.h>
#include <sys/ioctl.h> // For FIONREAD
#include <termios.h>
#include <stdbool.h>

int kbhit(void);

#endif //KB_H