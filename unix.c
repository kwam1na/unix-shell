/*
 * unix.c
 *
 * This file contains the functions that can be performed on a simulated
 * version of a Unix system.
 *
 * (c) Ernest Essuah Mensah
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unix.h"

#define CD "."
#define PARENT ".."
#define ROOT "/"

static int non_error_arg(const char arg[]);
static int invalid_arg(const char arg[]);
static Container * name_exists(Unix *fs, const char arg[],
			       int *exists, int should_assign);
static int add_container_to_filesystem(Unix *fs, const char arg[],
				       enum Type type);
static void print_elements(Container *container, int is_curr_dir);
static void pwd_helper(Container *dir, Unix *filesystem);
static void delete(Container *dir, int delete_all);


/*
 * This function initializes the Unix parameter to be a filesystem that has
 * only one container that is the root and current directory of the system
 */
void mkfs(Unix *filesystem) {

  /* Only perform initialization on non-NULL value */
  if (filesystem != NULL) {

    Container *root = malloc(sizeof(*root));

    /* Allocate enough memory and verify memmory was allocated */
    if (root == NULL) {
      printf("Not enough memory for allocation. Terminating program.\n");
      exit(1);
    }

    /* Set the members for this unix variable */
    filesystem->root = root;
    filesystem->curr_dir = root;

    /* Initialize the root directory for this unix variable */
    root->prev = NULL;
    root->parent = root;
    root->type = U_ROOT;
    root->next = NULL;
    root->name = malloc(sizeof(char) + 1);

    if (root->name == NULL) {
      printf("Not enough memory for allocation. Terminating program.\n");
      exit(1);
    }
    
    strcpy(root->name, "/");
  }
}


/*
 * Adds a file with the name arg to the passed in unix variable
 *
 * Returns 1 if file was added successfully, 0 if there was an error
 * or invalid parameter
 */
int touch(Unix *filesystem, const char arg[]) {

  int exists = 0;

  if (filesystem == NULL || arg == NULL)
    return 0;
   
  name_exists(filesystem, arg, &exists, 0);

  if (non_error_arg(arg) || exists) 
    return 1;

  if (invalid_arg(arg))
    return 0;
  
  return add_container_to_filesystem(filesystem, arg, U_FILE);
}


/*
 * Adds a directory with the name arg to the passed in unix variable
 *
 * Returns 1 if directory was added successfully, 0 if there was an error
 * or invalid parameter
 */
int mkdir(Unix *filesystem, const char arg[]) {

  int exists = 0;

  if (filesystem == NULL || arg == NULL)
    return 0;
   
  name_exists(filesystem, arg, &exists, 0);
  
  if (exists || non_error_arg(arg))
    return 0;

  if (invalid_arg(arg))
    return 0;
  
  return add_container_to_filesystem(filesystem, arg, U_DIR);
}



/*
 * Changes the current directory of the unix variable sent in
 * to the parameter arg
 *
 * Returns 1 if successful, 0 otherwise
 */
int cd(Unix *filesystem, const char arg[]) {

  Container * position = NULL;
  int exists = 0;

  if (filesystem == NULL || arg == NULL)
    return 0;

  /* Case when current directory is the arg sent in */
  if (strcmp(arg, CD) == 0 || (int)strlen(arg) == 0)
    return 1;

  /* Changing cd to the parent of the current directory */
  if (strcmp(arg, PARENT) == 0) {

    /* Check if this is the root */
    if (filesystem->curr_dir->type != U_ROOT)
      filesystem->curr_dir = filesystem->curr_dir->parent;
    
    return 1;
  }

  /* Changing cd to root */
  if (strcmp(arg, ROOT) == 0) {
    filesystem->curr_dir = filesystem->root;
    return 1;
  }

  /* Changing cd to a different directory other than the ROOT */
  position = name_exists(filesystem, arg, &exists, 1);

  if (!exists && non_error_arg(arg) == 0)
    return 0;

  if (position != NULL) {
    if (position->type == U_FILE)
      return 0;
  }
  
  /* Changing cd to valid subdirectory */
  filesystem->curr_dir = position;
  
  return 1;
}


/*
 * Prints depending on arg sent in: the elements in the
 * current directory, elements in the parent directory,
 * elements in the root directory, name of a file in the
 * current directory, or elements of a subdirectory in 
 * the current directory
 */
int ls(Unix *filesystem, const char arg[]) {

  Container * position = NULL;
  int exists = 0;

  if (filesystem == NULL || arg == NULL)
    return 0;
  
  position = name_exists(filesystem, arg, &exists, 1);

  /* Printing contents of the current directory */
  if (strcmp(arg, CD) == 0 || (int)strlen(arg) == 0) {
    print_elements(filesystem->curr_dir, 1);
    return 1;
  }

  /* Argument sent in doesn't exist in the current directroy */
  if (!exists && non_error_arg(arg) == 0)
    return 0;

  /* Printing the contents of the parent directory */
  if (strcmp(arg, PARENT) == 0)
    print_elements(filesystem->curr_dir->parent, 0);

  /* Printing the contents of the root directory */
  if (strcmp(arg, ROOT) == 0)
    print_elements(filesystem->root, 0);

  /* Either printing a file or directory */
  if (position != NULL) {

    if (position->type == U_FILE)
      printf("%s\n", position->name);
    else /* Printing elements of a subdirectory */
      print_elements(position->sub_dir, 0);
  }

  return 1;
}


/*
 * Prints the current directory of the unix variable
 * passed in listing out it's entire path from the 
 * root directory
 */
void pwd(Unix *filesystem) {
  pwd_helper(filesystem->curr_dir, filesystem);
}

/*
 * Removes all containers created in the Unix filesystem
 * passed in.
 */
void rmfs(Unix *filesystem) {
  delete(filesystem->root, 1);
}


/*
 * Removes the container named arg from the Unix
 * variable sent in.
 *
 * Returns 1 if successful, 0 if an error was encountered
 */
int rm(Unix *filesystem, const char arg[]) {

  Container * position = NULL;
  int exists = 0;

  position = name_exists(filesystem, arg, &exists, 1);

  if (filesystem == NULL || arg == NULL)
    return 0;

  if (invalid_arg(arg) || non_error_arg(arg) || !exists)
    return 0;

  if (position != NULL) {
    delete(position, 0);
    return 1;
  }

  return 0;
}


/*
 * Private functions
 */


/*
 * Prints out all the entire path of the current directory
 * all the way up to the ROOT directory
 */
static void pwd_helper(Container *dir, Unix *filesystem) {

  if (dir->type == U_ROOT) {
    /* Check if the ROOT is the current directroy */
    if (dir == filesystem->curr_dir)
      printf("%s\n", dir->name);
    else
      printf("%s", dir->name);
    
    return;
  }

  /* Print the parent subdirectory of the current directory recursively */
  pwd_helper(dir->parent, filesystem);

  /* Print the name of this directory */
  if (dir != filesystem->curr_dir)
    printf("%s/", dir->name);
  else
    printf("%s\n", dir->name);
}

/*
 * Checks if the name parameter is invalid for the filesystem.
 * Returns a non-zero value if invalid, zero otherwise
 */
static int invalid_arg(const char arg[]) {
  return ((int)strlen(arg) == 0 || strstr(arg, ROOT) != NULL);
}

/*
 * Checks if the name parameter does not cause an error but cannot
 * be the name of a file or directory
 * Returns a non-zero value is true, zero otherwise
 */
static int non_error_arg(const char arg[]) {
  return (strcmp(arg, CD) == 0 || strcmp(arg, PARENT) == 0 ||
	  strcmp(arg, ROOT) == 0);
}

/*
 * Checks if the name parameter exists in the filesystem sent in
 * If should_assign is a non-zero value, this function returns a pointer
 * to the element in the list if was found. Returns NULL otherwise
 *
 * Assigns the value of exists to 1 if the parameter was found, 0 otherwise
 */
static Container *name_exists(Unix *filesystem, const char arg[],
			      int *exists, int should_assign) {

  Container *curr = filesystem->curr_dir;

  /* Check if the current directory is the root or a directory*/
  if (curr->type == U_ROOT || curr->type == U_DIR) {
    
    if (curr->type == U_ROOT)
      curr = curr->next;
    else
      curr = curr->sub_dir;
  }
  
  while (curr != NULL) {

    if (strcmp(curr->name, arg) == 0) {

      /* Set the value to be found and return the pointer to the element */
      *exists = 1;
      if (should_assign) { 
	return curr;
      }
      return NULL;
    }

    /* Name doesn't match, check the next element */
    curr = curr->next;
  }

  /* Name wasn't found in filesystem */
  *exists = 0;
  return NULL;
}


/*
 * Adds a container with the name arg to the unix parameter sent in
 * Sorts the files alphabetically as it adds to make printing 
 * the elements easier.
 *
 * Returns 1 if successful, 0 if there was an error.
 */
static int add_container_to_filesystem(Unix *filesystem,
				       const char arg[], enum Type type) {

  Container *curr_dir = filesystem->curr_dir, *curr, *new_container;
  Container *prev = curr_dir;
  char *container_name;
  curr = curr_dir;

  /* Check if the current directory is a directory other than ROOT */
  if (curr_dir->type == U_DIR)
    curr = curr_dir->sub_dir;

  /* Find the right place to insert this container */
  while (curr != NULL && (curr->type == U_ROOT
			  || strcmp(curr->name, arg) < 0)) {
    prev = curr;
    curr = curr->next;
  }
  

  /* Allocate enough space for the new file container and verify success */
  new_container = malloc(sizeof(*new_container));

  if (new_container == NULL) {
    printf("Not enough memory for allocation. Terminating program.\n");
    return 0;
  }

  /* Successfully allocated memory, initialize the container
     with the right data */
  new_container->parent = curr_dir;
  new_container->next = curr;
  new_container->type = type;
  new_container->prev = prev;

  /* Switch the previous of curr */
  if (curr != NULL)
    curr->prev = new_container;
  
  if (type == U_DIR) 
    new_container->sub_dir = NULL;

  /* Allocate enough memory for the name of a file container */
  container_name = malloc(strlen(arg) + 1);

  if (container_name == NULL) {
    printf("Not enough memory for allocation. Terminating program.\n");

    free(new_container);
    new_container = NULL;
    return 0;
  }
  
  /* Set the name of this container */
  strcpy(container_name, arg);
  new_container->name = container_name;

  /* Check if the current directory is a directory other than ROOT 
     and if this is the first element added to it */
  if (curr_dir->type == U_DIR && curr_dir->sub_dir == NULL) {

    curr_dir->sub_dir = new_container;
    
  } else {

    if (prev == curr_dir && prev->type == U_DIR)
      prev->sub_dir = new_container;
    else
      prev->next = new_container;
  }
  
  return 1;
}
  


/*
 * Prints out the elements in the linked list representing the 
 * files and directories in the Unix filesystem
 */
static void print_elements(Container *container, int is_current_dir) {

  Container *curr = container;

  if (curr == NULL)
    return;

  /* Check if printing root directory with no elements */
  if (curr->type == U_ROOT && curr->next == NULL)
    return;

  if (curr->type == U_ROOT || is_current_dir) {

    if (curr->type == U_ROOT)
      curr = curr->next;
    else
      curr = curr->sub_dir;
  }

  if (curr == NULL)
    return;
  
  while (curr->next != NULL) {

    if (curr->type == U_DIR)
      printf("%s/\n", curr->name);
    else
      printf("%s\n", curr->name);;
    
    curr = curr->next;
  }

  /* Print the last element before returning */
  if (curr->type == U_DIR)
    printf("%s/\n", curr->name);
  else
    printf("%s\n", curr->name);
}


/*
 * Deletes the container sent in from the current Unix variable.
 * If the dir is a directory, this function deletes all the contents
 * of dir, otherwise it just deletes the container.
 *
 * If delete_all is non-zero, this function deletes everything in the
 * current unix variable.
 */
void delete(Container * dir, int delete_all) {


  /* Check if deleting a non-empty directory */
  if (dir->type == U_DIR && dir->sub_dir != NULL)
    delete(dir->sub_dir, 1);
 

  if (delete_all) { /* Delete all connections recursively */

   if (dir->next != NULL)
     delete(dir->next, delete_all);
  } 

  /* Adjust the previous pointer of this container */
  if (dir->prev != NULL) {

    /* Case where the previous container is the first container
       of a subdirectoryy */
    if ((dir->prev == dir->parent) && dir->prev->type != U_ROOT)
      dir->parent->sub_dir = dir->next;
    else 
      dir->prev->next = dir->next;
  }

  /* Container to be removed is between two containers */
  if (dir->next != NULL)
    dir->next->prev = dir->prev;
  
  /* Free all dynamically allocated memory and clear dangling pointers */
  free(dir->name);
  free(dir);
  dir->name = NULL;
  dir->parent = NULL;
  dir->prev = NULL;
  dir = NULL;
}
