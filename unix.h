#include "unix-datastructure.h"

void mkfs(Unix *filesystem);
int touch(Unix *filesystem, const char arg[]);
int mkdir(Unix *filesystem, const char arg[]);
int cd(Unix *filesystem, const char arg[]);
int ls(Unix *filesystem, const char arg[]);
void pwd(Unix *filesystem);
