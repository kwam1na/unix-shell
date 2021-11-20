/*
 * unix-datastructure.h
 *
 * Header file that contains definitions for the Unix
 * filesystem
 *
 * (c) Ernest Essuah Mensah
 * 
 * eessuahm 115784345 Section 0102
 */

struct Container;

enum Type {U_ROOT, U_FILE, U_DIR};

/* A container is the "superclass" of what an element
 * of the Unix filesystem can contain.
 */
typedef struct container {
  char * name;
  struct container * parent;
  struct container * next;
  enum Type type;
  struct container * sub_dir;
} Container;

/* Definition for a Unix filesystem variable */
typedef struct unix {
  struct container * root;
  struct container * curr_dir;
} Unix;
