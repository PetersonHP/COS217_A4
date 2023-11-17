/*-------------------------------------------------------------------*/
/* nodeFT.c                                                          */
/* Author: Hugh Peterson                                             */
/*-------------------------------------------------------------------*/

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "a4def.h"
#include "dynarray.h"
#include "nodeFT.h"

/*-------------------------------------------------------------------*/

/* A node in a FT */
struct node {
   /* the object corresponding to the node's absolute path */
   Path_T oPPath;
   /* this node's parent */
   Node_T oNParent;
   /* the object containing links to this node's children */
   DynArray_T oDChildren;
   /* Flag to indicate whether this node is a file or a directory. 
      TRUE if it is a file. FALSE otherwise. */
   boolean isFile;
   /* this string contains the contents of the node if it is a file */
   const char *contents;
};

/*-------------------------------------------------------------------*/

/*
  Compares the string representation of oNfirst with a string
  pcSecond representing a node's path.
  Returns <0, 0, or >0 if oNFirst is "less than", "equal to", or
  "greater than" pcSecond, respectively.
*/
static int Node_compareString(const Node_T oNFirst,
                                 const char *pcSecond) {
   assert(oNFirst != NULL);
   assert(pcSecond != NULL);

   return Path_compareString(oNFirst->oPPath, pcSecond);
}

/*
  Links new child oNChild into oNParent's children array at index
  ulIndex. Returns SUCCESS if the new child was added successfully,
  MEMORY_ERROR if allocation fails adding oNChild to the array, or 
  NO_SUCH_PATH if oNParent is a file.
*/
static int Node_addChild(Node_T oNParent, Node_T oNChild,
                         size_t ulIndex) {
   assert(oNParent != NULL);
   assert(oNChild != NULL);

   if (Node_isFile(oNParent)) {
      return NO_SUCH_PATH;
   }

   if(DynArray_addAt(oNParent->oDChildren, ulIndex, oNChild))
      return SUCCESS;
   else
      return MEMORY_ERROR;
}

/* 
   Tries to link a newly created node newNode to its parent oNParent, 
   assuming its path has already been set. Returns SUCCESS if there 
   are no issues. Otherwise, does nothing and returns status:
   * CONFLICTING_PATH if oNParent's path is not an ancestor of oPPath
   * NO_SUCH_PATH if oNParent's path is not oPPath's direct parent
                 or oNParent is NULL but oPPath is not of depth 1
                 or oNParent is a file
   * ALREADY_IN_TREE if oNParent already has a child with this path
*/
static int Node_linkToParent(Node_T newNode, Node_T oNParent) {
   Path_T parentPath;
   size_t parentDepth;
   int status;
   size_t index;

   assert(newNode != NULL);
   assert(newNode->oPPath != NULL);

   /* ensure oNParent is not a file if it isn't NULL */
   if(oNParent != NULL && Node_isFile(oNParent)) {
      return NO_SUCH_PATH;
   }
   
   /* validate and set the new node's parent */
   if(oNParent != NULL) {
      size_t sharedDepth;

      parentPath = oNParent->oPPath;
      parentDepth = Path_getDepth(parentPath);
      sharedDepth = Path_getSharedPrefixDepth(newNode->oPPath,
                                                parentPath);

      /* parent must be an ancestor of child */
      if(sharedDepth < parentDepth) {
         return CONFLICTING_PATH;
      }

      /* parent must be exactly one level up from child */
      if(Path_getDepth(newNode->oPPath) != parentDepth + 1) {
         return NO_SUCH_PATH;
      }

      /* parent must not already have child with this path */
      if(Node_hasChild(oNParent, newNode->oPPath, &index)) {
         return ALREADY_IN_TREE;
      }
   }
   else {
      /* new node must be root */
      /* can only create one "level" at a time */
      if(Path_getDepth(newNode->oPPath) != 1) {
         return NO_SUCH_PATH;
      }
   }
   newNode->oNParent = oNParent;

   /* Link into parent's children list */
   if(oNParent != NULL) {
      status = Node_addChild(oNParent, newNode, index);
      if(status != SUCCESS) {
         return status;
      }
   }
   
   return SUCCESS;
}

/*-------------------------------------------------------------------*/

/*
  Creates a new directory node in the File Tree, with path oPPath and
  parent oNParent. Returns an int SUCCESS status and sets *poNResult 
  to be the new node if successful. Otherwise, sets *poNResult to NULL 
  and returns status:
  * MEMORY_ERROR if memory could not be allocated to complete request
  * CONFLICTING_PATH if oNParent's path is not an ancestor of oPPath
  * NO_SUCH_PATH if oPPath is of depth 0
                 or oNParent's path is not oPPath's direct parent
                 or oNParent is NULL but oPPath is not of depth 1
                 or oNParent is a file
  * ALREADY_IN_TREE if oNParent already has a child with this path
*/
int Node_newDir(Path_T oPPath, Node_T oNParent, Node_T *poNResult) {
   struct node *newNode;
   int status;
   Path_T newPath;

   assert(oPPath != NULL);

   /* make sure parent isn't a file if it isn't NULL */
   if(oNParent != NULL && Node_isFile(oNParent)) {
      return NO_SUCH_PATH;
   }

   /* allocate space for a new node */
   newNode = malloc(sizeof(struct node));
   if(newNode == NULL) {
      *poNResult = NULL;
      return MEMORY_ERROR;
   }

   /* set the new node's path */
   status = Path_dup(oPPath, &newPath);
   if(status != SUCCESS) {
      free(newNode);
      *poNResult = NULL;
      return status;
   }
   newNode->oPPath = newPath;

   /* link node into tree */
   status = Node_linkToParent(newNode, oNParent);
   if(status != SUCCESS) {
      free(newNode);
      *poNResult = NULL;
      return status;
   }

   /* set up the new node as a directory */
   newNode->oDChildren = DynArray_new(0);
   if(newNode->oDChildren == NULL) {
      Path_free(newNode->oPPath);
      free(newNode);
      *poNResult = NULL;
      return MEMORY_ERROR;
   }
   newNode->isFile = FALSE;
   newNode->contents = NULL;

   *poNResult = newNode;
   return SUCCESS;
}

/*
  Creates a new file node in the File Tree, with path oPPath, parent 
  oNParent, and contents contents. Returns an int SUCCESS status and 
  sets *poNResult to be the new node if successful. Otherwise, sets 
  *poNResult to NULL and returns status:
  * MEMORY_ERROR if memory could not be allocated to complete request
  * CONFLICTING_PATH if oNParent's path is not an ancestor of oPPath
  * NO_SUCH_PATH if oPPath is of depth 0
                 or oNParent's path is not oPPath's direct parent
                 or oNParent is NULL but oPPath is not of depth 1
                 or oNParent is a file
  * ALREADY_IN_TREE if oNParent already has a child with this path
*/
int Node_newFile(Path_T oPPath, Node_T oNParent, const char *contents,
                 Node_T *poNResult) {
   struct node *newNode;
   int status;
   Path_T newPath;

   assert(oPPath != NULL);

   /* make sure parent isn't a file if it isn't NULL */
   if(oNParent != NULL && Node_isFile(oNParent)) {
      return NO_SUCH_PATH;
   }

   /* allocate space for a new node */
   newNode = malloc(sizeof(struct node));
   if(newNode == NULL) {
      *poNResult = NULL;
      return MEMORY_ERROR;
   }

   /* set the new node's path */
   status = Path_dup(oPPath, &newPath);
   if(status != SUCCESS) {
      free(newNode);
      *poNResult = NULL;
      return status;
   }
   newNode->oPPath = newPath;

   /* link node into tree */
   status = Node_linkToParent(newNode, oNParent);
   if(status != SUCCESS) {
      Path_free(newNode->oPPath);
      free(newNode);
      *poNResult = NULL;
      return status;
   }

   /* set up the new node as a file, duplicating contents */
   newNode->contents = malloc(strlen(contents) + 1);
   if(newNode->contents == NULL) {
      Path_free(newNode->oPPath);
      free(newNode);
      *poNResult = NULL;
      return MEMORY_ERROR;
   }
   strcpy((char *)newNode->contents, contents);
   newNode->oDChildren = NULL;
   newNode->isFile = TRUE;

   *poNResult = newNode;
   return SUCCESS;
}

/*
  Destroys and frees all memory allocated for the subtree rooted at
  oNNode, i.e., deletes this node and all its descendents. Returns the
  number of nodes deleted.
*/
size_t Node_free(Node_T oNNode) {
   size_t index;
   size_t count = 0;
   
   assert(oNNode != NULL);
   
   /* remove from parent's list */
   if(oNNode->oNParent != NULL) {
      if(DynArray_bsearch(
            oNNode->oNParent->oDChildren,
            oNNode, &index,
            (int (*)(const void *, const void *)) Node_compare)
        )
         (void) DynArray_removeAt(oNNode->oNParent->oDChildren,
                                  index);
   }

   /* recursively remove children if oNNode is a directory */
   if (!Node_isFile(oNNode)) {
      /* remove children */
      while(DynArray_getLength(oNNode->oDChildren) != 0) {
         count += Node_free(DynArray_get(oNNode->oDChildren, 0));
      }
      DynArray_free(oNNode->oDChildren);
   }
   else {
      /* free contents if oNNode is a file */
      free((char *)oNNode->contents);
   }

   /* remove path */
   Path_free(oNNode->oPPath);

   /* finally, free the struct node */
   free(oNNode);
   count++;
   return count;
}

/* Returns the path object representing oNNode's absolute path. */
Path_T Node_getPath(Node_T oNNode) {
   assert(oNNode != NULL);

   return oNNode->oPPath;
}

/*
  Returns TRUE if oNParent has a child with path oPPath. Returns
  FALSE if it does not, or if oNParent is a file.

  If oNParent has such a child, stores in *pulChildID the child's
  identifier (as used in Node_getChild). If oNParent does not have
  such a child, stores in *pulChildID the identifier that such a
  child _would_ have if inserted.
*/
boolean Node_hasChild(Node_T oNParent, Path_T oPPath,
                         size_t *pulChildID) {
   assert(oNParent != NULL);
   assert(oPPath != NULL);
   assert(pulChildID != NULL);

   if(Node_isFile(oNParent)) {
      return FALSE;
   }

   /* *pulChildID is the index into oNParent->oDChildren */
   return DynArray_bsearch(oNParent->oDChildren,
            (char*) Path_getPathname(oPPath), pulChildID,
            (int (*)(const void*,const void*)) Node_compareString);
}

/* 
  Returns the number of children that oNParent has 
  (always 0 if oNParent is a file).
*/
size_t Node_getNumChildren(Node_T oNParent) {
   assert(oNParent != NULL);

   if(Node_isFile(oNParent)) {
      return 0;
   }
   return DynArray_getLength(oNParent->oDChildren);
}

/*
  Returns an int SUCCESS status and sets *poNResult to be the child
  node of oNParent with identifier ulChildID, if one exists.
  Otherwise, sets *poNResult to NULL and returns status:
  * NO_SUCH_PATH if ulChildID is not a valid child for oNParent
*/
int Node_getChild(Node_T oNParent, size_t ulChildID,
                  Node_T *poNResult) {
   assert(oNParent != NULL);
   assert(poNResult != NULL);

   /* ensure oNParent is not a file */
   if (Node_isFile(oNParent)) {
      *poNResult = NULL;
      return NO_SUCH_PATH;
   }

   /* ulChildID is the index into oNParent->oDChildren */
   if(ulChildID >= Node_getNumChildren(oNParent)) {
      *poNResult = NULL;
      return NO_SUCH_PATH;
   }
   else {
      *poNResult = DynArray_get(oNParent->oDChildren, ulChildID);
      return SUCCESS;
   }
}

/*
  Returns a the parent node of oNNode.
  Returns NULL if oNNode is the root and thus has no parent.
*/
Node_T Node_getParent(Node_T oNNode) {
   assert(oNNode != NULL);

   return oNNode->oNParent;
}

/*
  Compares oNFirst and oNSecond lexicographically based on their paths.
  Returns <0, 0, or >0 if onFirst is "less than", "equal to", or
  "greater than" oNSecond, respectively.
*/
int Node_compare(Node_T oNFirst, Node_T oNSecond) {
   assert(oNFirst != NULL);
   assert(oNSecond != NULL);

   return Path_comparePath(oNFirst->oPPath, oNSecond->oPPath);
}

/*
  Returns a string representation for oNNode, or NULL if
  there is an allocation error.

  Allocates memory for the returned string, which is then owned by
  the caller!
*/
char *Node_toString(Node_T oNNode) {
   char *copyPath;

   assert(oNNode != NULL);

   copyPath = malloc(Path_getStrLength(Node_getPath(oNNode))+1);
   if(copyPath == NULL)
      return NULL;
   else
      return strcpy(copyPath, Path_getPathname(Node_getPath(oNNode)));
}

/* 
  Returns TRUE if oNNode is a file node and FALSE if it is a 
  directory node.
*/
boolean Node_isFile(Node_T oNNode) {
   assert(oNNode != NULL);

   return oNNode->isFile;
}

/* 
  If oNNode is a file node, returns its contents as a string.
  Otherwise, returns NULL.

  Allocates mmory for the returned string, which is then owned by 
  the caller!
 */
char *Node_getContents(Node_T oNNode) {
   char *extracted;
   
   assert(oNNode != NULL);

   if (!Node_isFile(oNNode)) {
      return NULL;
   }

   extracted = malloc(strlen(oNNode->contents) + 1);
   if (extracted == NULL) {
      return NULL;
   }
   return strcpy(extracted, oNNode->contents);
}

/*-------------------------------------------------------------------*/
#ifdef DEBUG

#include <stdio.h>

int main(void) {
   Node_T rootNode = NULL;
   Node_T childDir = NULL;
   Node_T helloWorldFile = NULL;
   Node_T current = NULL;
   Path_T path = NULL;
   int status;
   size_t index;
   const char *temp;
   const char *helloWorld = "Hello World!";
   
   /* test creation of a root directory */
   Path_new("~", &path);
   status = Node_newDir(path, NULL, &rootNode);
   assert(status == SUCCESS);
   assert(rootNode != NULL);
   assert(rootNode->contents == NULL);
   assert(!Node_isFile(rootNode));
   assert(!strcmp("~", Path_getPathname(Node_getPath(rootNode))));

   /* test addition of a child directory */
   Path_new("~/COS217_A4", &path);
   status = Node_newDir(path, rootNode, &childDir);
   assert(status == SUCCESS);
   assert(childDir != NULL);
   assert(!strcmp("~/COS217_A4",
                  Path_getPathname(Node_getPath(childDir))));

   /* test addition of a file with contents */
   Path_new("~/COS217_A4/hello_world.txt", &path);
   status = Node_newFile(path, childDir, helloWorld, &helloWorldFile);
   assert(status == SUCCESS);
   assert(helloWorldFile != NULL);
   assert(helloWorldFile->oDChildren == NULL);
   assert(!strcmp("~/COS217_A4/hello_world.txt",
                  Path_getPathname(Node_getPath(helloWorldFile))));
   assert(!strcmp(helloWorldFile->contents, helloWorld));

   /* test has child (existing child) */
   Path_new("~/COS217_A4", &path);
   status = Node_hasChild(rootNode, path, &index);
   assert(status == TRUE);
   assert(index == 0);

   /* test has child (nonexisting child) */
   Path_new("~/A", &path);
   status = Node_hasChild(rootNode, path, &index);
   assert(status == FALSE);
   assert(index == 0);
   Path_new("~/D", &path);
   status = Node_hasChild(rootNode, path, &index);
   assert(status == FALSE);
   assert(index == 1);

   /* test get child (existing child) */
   status = Node_getChild(childDir, 0, &current);
   assert(status == SUCCESS);
   assert(!Node_compare(current, helloWorldFile));

   /* test get child (nonexisting child) */
   status = Node_getChild(childDir, 3, &current);
   assert(status == NO_SUCH_PATH);
   assert(current == NULL);

   /* test toString */
   printf("%s\n", temp = Node_toString(rootNode));
   free((char *) temp);
   printf("%s\n", temp = Node_toString(childDir));
   free((char *) temp);
   printf("%s\n", temp = Node_toString(helloWorldFile));
   free((char *) temp);

   /* test get contents of a file */
   printf("%s\n", temp = Node_getContents(helloWorldFile));
   free((char *) temp);

   /* test get contents from a directory */
   assert(Node_getContents(rootNode) == NULL);
   
   /* test free */
   status = Node_free(rootNode);
   assert(status == 3);
   
   return SUCCESS;
}

#endif
/*-------------------------------------------------------------------*/
