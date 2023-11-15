/*--------------------------------------------------------------------*/
/* checkerDT.c                                                        */
/* Author:                                                            */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "checkerDT.h"
#include "dynarray.h"
#include "path.h"



/* see checkerDT.h for specification */
boolean CheckerDT_Node_isValid(Node_T oNNode) {
   Node_T oNParent;
   Path_T oPNPath;
   Path_T oPPPath;
   size_t parentDepth;

   /* DEBUG */
   printf("Checking Node %s\n", Path_getPathname(Node_getPath(oNNode)));
   fflush(stdout);
   
   /* Sample check: a NULL pointer is not a valid node */
   if(oNNode == NULL) {
      fprintf(stderr, "A node is a NULL pointer\n");
      return FALSE;
   }

   /* Sample check: parent's path must be the longest possible
      proper prefix of the node's path */
   oNParent = Node_getParent(oNNode);
   if(oNParent != NULL) {
      oPNPath = Node_getPath(oNNode);
      oPPPath = Node_getPath(oNParent);

      if(Path_getSharedPrefixDepth(oPNPath, oPPPath) !=
         Path_getDepth(oPNPath) - 1) {
         fprintf(stderr, "P-C nodes don't have P-C paths: (%s) (%s)\n",
                 Path_getPathname(oPPPath), Path_getPathname(oPNPath));
         return FALSE;
      }
   }

   /* Actual structure must match path */
   if (oNParent != NULL) {
      oPNPath = Node_getPath(oNNode);
      parentDepth = Path_getDepth(oPNPath) - 1;
      Path_prefix(oPNPath, parentDepth, &oPPPath);

      /* DEBUG */
      printf("%s\n", Path_getPathname(oPPPath));
      printf("%s\n", Path_getPathname(Node_getPath(oNParent)));
      
      if (Path_comparePath(oPPPath, Node_getPath(oNParent))) {
         fprintf(stderr, "Tree structure doesn't match node paths\n");
         return FALSE;
      }
   }

   return TRUE;
}

/*
   Performs a pre-order traversal of the tree rooted at oNNode.
   Returns FALSE if a broken invariant is found and
   returns TRUE otherwise. Takes a DynArray_T seenPaths to store paths 
   seen so far for tracking duplicates.

   You may want to change this function's return type or
   parameter list to facilitate constructing your checks.
   If you do, you should update this function comment.
*/
static boolean CheckerDT_treeCheck(Node_T oNNode) {
   size_t ulIndex;
   Path_T currentPath;

   /* /\* DEBUG *\/ */
   /* printf("I AM HERE AT %d\n", __LINE__); */
   /* fflush(stdout); */

   if(oNNode!= NULL) {
      
      /* Sample check on each node: node must be valid */
      /* If not, pass that failure back up immediately */
      if(!CheckerDT_Node_isValid(oNNode))
         return FALSE;      

      /* Recur on every child of oNNode */
      /* nodeList = DynArray_new(Node_getNumChildren(oNNode)); */
      for(ulIndex = 0; ulIndex < Node_getNumChildren(oNNode); ulIndex++)
      {
         Node_T oNChild = NULL;
         Node_T otherNode = NULL;
         size_t otherIdx;
         int iStatus = Node_getChild(oNNode, ulIndex, &oNChild);

         if(iStatus != SUCCESS) {
            fprintf(stderr, "getNumChildren claims more children than getChild returns\n");
            return FALSE;
         }

         /* /\* Duplicate Nodes cannot exist in tree *\/ */
         /* for (otherIdx = ulIndex + 1; */
         /*      otherIdx < Node_getNumChildren(oNNode); */
         /*      otherIdx++) { */

         /*    /\* DEBUG *\/ */
         /*    printf("Checking for dupes\n"); */
            
         /*    if (Node_getChild(oNNode, otherIdx, &otherNode) && */
         /*        Node_compare(oNChild, otherNode)) { */

         /*       fprintf(stderr, "Duplicate nodes found\n"); */
         /*       return FALSE; */
         /*    } */
         /* } */

         /* /\* DEBUG *\/ */
         /* printf("I AM HERE AT %d", __LINE__); */

         /* /\* /\\* DEBUG *\\/ *\/ */
         /* /\* DynArray_add(nodeList, oNChild); *\/ */
         
         /* /\* Duplicate nodes cannot exist *\/ */
         /* if (ulIndex > 1 && */
         /*     DynArray_search(nodeList, oNChild, (size_t*)&iStatus, */
         /*           (int (*)(const void *, const void *))Node_compare)) { */
            
         /*    fprintf(stderr, "Duplicate nodes found\n"); */
         /*    return FALSE; */
         /* } */
         /* else { */
         /*    /\* DEBUG *\/ */
         /*    printf("ADDING NODE"); */
            
         /*    DynArray_add(nodeList, oNChild); */
         /* } */

         /* if recurring down one subtree results in a failed check
            farther down, passes the failure back up immediately */
         if(!CheckerDT_treeCheck(oNChild, seenPaths))
            return FALSE;
      }
      /* DynArray_free(nodeList); */
   }
   return TRUE;
}

/* see checkerDT.h for specification */
boolean CheckerDT_isValid(boolean bIsInitialized, Node_T oNRoot,
                          size_t ulCount) {
   DynArray_T seenPaths;
   boolean result;

   /* /\* DEBUG *\/ */
   /* printf("I AM HERE AT %d\n", __LINE__); */
   /* fflush(stdout); */

   /* Sample check on a top-level data structure invariant:
      if the DT is not initialized, its count should be 0. */
   if(!bIsInitialized)
      if(ulCount != 0) {
         fprintf(stderr, "Not initialized, but count is not 0\n");
         return FALSE;
      }

   /* Now checks invariants recursively at each node from the root. */
   seenPaths = DynArray_new(ulCount);
   result = CheckerDT_treeCheck(oNRoot, seenPaths);
   DynArray_free(seenPaths);
   
   return result;
}
