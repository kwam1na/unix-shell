#include "unix-datastructure.h"

/* (c) Larry Herman, 2019.  You are allowed to use this code yourself, but
   not to provide it to anyone else. */

void mkfs(Unix *filesystem);
int touch(Unix *filesystem, const char arg[]);
int mkdir(Unix *filesystem, const char arg[]);
int cd(Unix *filesystem, const char arg[]);
int ls(Unix *filesystem, const char arg[]);
void pwd(Unix *filesystem);
