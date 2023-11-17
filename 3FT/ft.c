/*-------------------------------------------------------------------*/
/* ft.c                                                              */
/* Author: Hugh Peterson                                             */
/*-------------------------------------------------------------------*/

#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "path.h"
#include "nodeFT.h"
#include "a4def.h"
#include "ft.h"

/*-------------------------------------------------------------------*/

/*
  A File Tree is a representation of a hierarchy of directories and
  files: the File Tree is rooted at a directory, directories
  may be internal nodes or leaves, and files are always leaves. A File 
  Tree is represented as an abstract object with 3 state variables:
*/

/* 1. a flag for being in an initialized state (TRUE) or not (FALSE) */
static boolean isInitialized;
/* 2. a pointer to the root node in the hierarchy */
static Node_T root;
/* 3. a counter of the number of nodes in the hierarchy */
static size_t count;

/*-------------------------------------------------------------------*/

/*
  Traverses the FT starting at the root as far as possible towards
  absolute path oPPath. If able to traverse, returns an int SUCCESS
  status and sets *poNFurthest to the furthest node reached (which may
  be only a prefix of oPPath, or even NULL if the root is NULL).
  Otherwise, sets *poNFurthest to NULL and returns with status:
  * CONFLICTING_PATH if the root's path is not a prefix of oPPath
  * MEMORY_ERROR if memory could not be allocated to complete request
*/
static int FT_traversePath(Path_T oPPath, Node_T *poNFurthest) {
   int status;
   size_t i;
   Path_T prefix;
   Node_T current;
   Node_T child;
   size_t depth;
   size_t childID;

   assert(oPPath != NULL);
   assert(poNFurthest != NULL);

   /* Won't find anything if the root is NULL */
   if (root == NULL) {
      *poNFurthest = NULL;
      return SUCCESS;
   }

   /* check the root */
   status = Path_prefix(oPPath, 1, &prefix);
   if(status != SUCCESS) {
      *poNFurthest = NULL;
      return status;
   }

   /* is the root consistent? */
   if(Path_comparePath(Node_getPath(root), prefix)) {
      Path_free(prefix);
      *poNFurthest = NULL;
      return CONFLICTING_PATH;
   }
   Path_free(prefix);
   prefix = NULL;

   /* traverse the tree */
   current = root;
   depth = Path_getDepth(oPPath);
   for (i = 2; i <= depth; i++) {   
      /* get prefix to depth i */
      status = Path_prefix(oPPath, i, &prefix);
      if(status != SUCCESS) {
         *poNFurthest = NULL;
         return status;
      }

      /* check if current has the child with path prefix */
      if (Node_hasChild(current, prefix, &childID)) {
         /* set current to that child and continue with next prefix */
         Path_free(prefix);
         prefix = NULL;
         
         status = Node_getChild(current, childID, &child);
         if(status != SUCCESS) {
            *poNFurthest = NULL;
            return status;
         }
         current = child;
      }
      else {
         /* current doesn't have child with path oPPrefix:
            this is as far as we can go */
         break;
      }
   }

   /* clean things up and return when finished */
   Path_free(prefix);
   *poNFurthest = current;
   return SUCCESS;
}

/*-------------------------------------------------------------------*/

/*
   Inserts a new directory into the FT with absolute path pcPath.
   Returns SUCCESS if the new directory is inserted successfully.
   Otherwise, returns:
   * INITIALIZATION_ERROR if the FT is not in an initialized state
   * BAD_PATH if pcPath does not represent a well-formatted path
   * CONFLICTING_PATH if the root exists but is not a prefix of pcPath
   * NOT_A_DIRECTORY if a proper prefix of pcPath exists as a file
   * ALREADY_IN_TREE if pcPath is already in the FT (as dir or file)
   * MEMORY_ERROR if memory could not be allocated to complete request
*/
int FT_insertDir(const char *pcPath) {
   Path_T newPath = NULL;
   Node_T ancestorNode = NULL;
   int status;

   assert(pcPath != NULL);

   if (!isInitialized) {
      return INITIALIZATION_ERROR;
   }

   /* make new path */
   status = Path_new(pcPath, &newPath);
   if (status != SUCCESS) {
      return status;
   }

   /* find ancestor node */
   status = FT_traversePath(newPath, &ancestorNode);
   if (status != SUCCESS) {
      Path_free(newPath);
      return status;
   }

   

   
}

/*
  Returns TRUE if the FT contains a directory with absolute path
  pcPath and FALSE if not or if there is an error while checking.
*/
boolean FT_containsDir(const char *pcPath) {

   assert(pcPath != NULL);

   /* Return FALSE if FT is uninitialized */
   if (!isInitialized) {
      return FALSE;
   }
}

/*
  Removes the FT hierarchy (subtree) at the directory with absolute
  path pcPath. Returns SUCCESS if found and removed.
  Otherwise, returns:
  * INITIALIZATION_ERROR if the FT is not in an initialized state
  * BAD_PATH if pcPath does not represent a well-formatted path
  * CONFLICTING_PATH if the root exists but is not a prefix of pcPath
  * NO_SUCH_PATH if absolute path pcPath does not exist in the FT
  * NOT_A_DIRECTORY if pcPath is in the FT as a file not a directory
  * MEMORY_ERROR if memory could not be allocated to complete request
*/
int FT_rmDir(const char *pcPath) {

   assert(pcPath != NULL);
   
   /* Return INITIALIZATION_ERROR if FT is uninitialized */
   if (!isInitialized) {
      return INITIALIZATION_ERROR;
   }
}


/*
   Inserts a new file into the FT with absolute path pcPath, with
   file contents pvContents of size ulLength bytes.
   Returns SUCCESS if the new file is inserted successfully.
   Otherwise, returns:
   * INITIALIZATION_ERROR if the FT is not in an initialized state
   * BAD_PATH if pcPath does not represent a well-formatted path
   * CONFLICTING_PATH if the root exists but is not a prefix of pcPath,
                      or if the new file would be the FT root
   * NOT_A_DIRECTORY if a proper prefix of pcPath exists as a file
   * ALREADY_IN_TREE if pcPath is already in the FT (as dir or file)
   * MEMORY_ERROR if memory could not be allocated to complete request
*/
int FT_insertFile(const char *pcPath, void *pvContents,
                  size_t ulLength) {

   assert(pcPath != NULL);

   /* Return INITIALIZATION_ERROR if FT is uninitialized */
   if (!isInitialized) {
      return INITIALIZATION_ERROR;
   }
}

/*
  Returns TRUE if the FT contains a file with absolute path
  pcPath and FALSE if not or if there is an error while checking.
*/
boolean FT_containsFile(const char *pcPath) {

   assert(pcPath != NULL);
   
   /* Return FALSE if FT is uninitialized */
   if (!isInitialized) {
      return FALSE;
   }
}

/*
  Removes the FT file with absolute path pcPath.
  Returns SUCCESS if found and removed.
  Otherwise, returns:
  * INITIALIZATION_ERROR if the FT is not in an initialized state
  * BAD_PATH if pcPath does not represent a well-formatted path
  * CONFLICTING_PATH if the root exists but is not a prefix of pcPath
  * NO_SUCH_PATH if absolute path pcPath does not exist in the FT
  * NOT_A_FILE if pcPath is in the FT as a directory not a file
  * MEMORY_ERROR if memory could not be allocated to complete request
*/
int FT_rmFile(const char *pcPath) {

   assert(pcPath != NULL);
   
   /* Return INITIALIZATION_ERROR if FT is uninitialized */
   if (!isInitialized) {
      return INITIALIZATION_ERROR;
   }
}

/*
  Returns the contents of the file with absolute path pcPath.
  Returns NULL if unable to complete the request for any reason.

  Note: checking for a non-NULL return is not an appropriate
  contains check, because the contents of a file may be NULL.
*/
void *FT_getFileContents(const char *pcPath);

/*
  Replaces current contents of the file with absolute path pcPath with
  the parameter pvNewContents of size ulNewLength bytes.
  Returns the old contents if successful. (Note: contents may be NULL.)
  Returns NULL if unable to complete the request for any reason.
*/
void *FT_replaceFileContents(const char *pcPath, void *pvNewContents,
                             size_t ulNewLength);

/*
  Returns SUCCESS if pcPath exists in the hierarchy,
  Otherwise, returns:
  * INITIALIZATION_ERROR if the FT is not in an initialized state
  * BAD_PATH if pcPath does not represent a well-formatted path
  * CONFLICTING_PATH if the root's path is not a prefix of pcPath
  * NO_SUCH_PATH if absolute path pcPath does not exist in the FT
  * MEMORY_ERROR if memory could not be allocated to complete request

  When returning SUCCESS,
  if path is a directory: sets *pbIsFile to FALSE, *pulSize unchanged
  if path is a file: sets *pbIsFile to TRUE, and
                     sets *pulSize to the length of file's contents

  When returning another status, *pbIsFile and *pulSize are unchanged.
*/
int FT_stat(const char *pcPath, boolean *pbIsFile, size_t *pulSize);

/*
  Sets the FT data structure to an initialized state.
  The data structure is initially empty.
  Returns INITIALIZATION_ERROR if already initialized,
  and SUCCESS otherwise.
*/
int FT_init(void) {
   /* If FT is already initialized, return INITIALIZATION_ERROR */
   if(isInitialized)
      return INITIALIZATION_ERROR;

   isInitialized = TRUE;
   root = NULL;
   count = 0;

   return SUCCESS;
}

/*
  Removes all contents of the data structure and
  returns it to an uninitialized state.
  Returns INITIALIZATION_ERROR if not already initialized,
  and SUCCESS otherwise.
*/
int FT_destroy(void) {
   /* Return INITIALIZATION_ERROR if FT is uninitialized */
   if (!isInitialized) {
      return INITIALIZATION_ERROR;
   }
}

/*
  Returns a string representation of the
  data structure, or NULL if the structure is
  not initialized or there is an allocation error.

  The representation is depth-first with files
  before directories at any given level, and nodes
  of the same type ordered lexicographically.

  Allocates memory for the returned string,
  which is then owned by client!
*/
char *FT_toString(void) {
   /* Return NULL if FT is uninitialized */
   if (!isInitialized) {
      return NULL;
   }
}

