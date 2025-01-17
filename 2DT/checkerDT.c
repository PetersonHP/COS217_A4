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



/* see checkerDT.h for specification  */
boolean CheckerDT_Node_isValid(Node_T oNNode) {
   Node_T oNParent;
   Path_T oPNPath;
   Path_T oPPPath;
   
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

   return TRUE;
}

/*
   Performs a pre-order traversal of the tree rooted at oNNode.
   Returns FALSE if a broken invariant is found and
   returns TRUE otherwise.

   You may want to change this function's return type or
   parameter list to facilitate constructing your checks.
   If you do, you should update this function comment.
*/
static boolean CheckerDT_treeCheck(Node_T oNNode, size_t *nodeCount) {
   size_t ulIndex;

   if(oNNode!= NULL) {

      /* update node count */
      *nodeCount = *nodeCount + 1;
      
      /* Sample check on each node: node must be valid */
      /* If not, pass that failure back up immediately */
      if(!CheckerDT_Node_isValid(oNNode))
         return FALSE;

      /* Recur on every child of oNNode */
      for(ulIndex = 0; ulIndex < Node_getNumChildren(oNNode); ulIndex++)
      {
         Node_T oNChild = NULL;
         Node_T next = NULL;
         int iStatus = Node_getChild(oNNode, ulIndex, &oNChild);
         int comp;

         if(iStatus != SUCCESS) {
            fprintf(stderr, "getNumChildren claims more children than getChild returns\n");
            return FALSE;
         }

         /* checks for next child node */
         if (ulIndex + 1 < Node_getNumChildren(oNNode)) {
            Node_getChild(oNNode, ulIndex + 1, &next);
            
            /* check if there is a duplicate node next */
            comp = Node_compare(oNChild, next);
            if (comp == 0) {
               fprintf(stderr, "Duplicate node detected (%s)\n",
                       Path_getPathname(Node_getPath(oNChild)));
               return FALSE;
            }

            /* check if the next node follows lexicographically */
            comp = strcmp(Path_getPathname(Node_getPath(oNChild)),
                          Path_getPathname(Node_getPath(next)));
            if (comp > 0) {
               fprintf(stderr, "Children are not in lexicographic order (%s > %s)\n"
                       , Path_getPathname(Node_getPath(oNChild)),
                          Path_getPathname(Node_getPath(next)));
               return FALSE;
            }
         }

         /* if recurring down one subtree results in a failed check
            farther down, passes the failure back up immediately */
         if(!CheckerDT_treeCheck(oNChild, nodeCount))
            return FALSE;
      }
   }
   return TRUE;
}

/* see checkerDT.h for specification */
boolean CheckerDT_isValid(boolean bIsInitialized, Node_T oNRoot,
                          size_t ulCount) {
   size_t nodeCount = 0;
   boolean result;

   /* Sample check on a top-level data structure invariant:
      if the DT is not initialized, its count should be 0. */
   if(!bIsInitialized)
      if(ulCount != 0) {
         fprintf(stderr, "Not initialized, but count is not 0\n");
         return FALSE;
      }

   /* Now checks invariants recursively at each node from the root. */
   result = CheckerDT_treeCheck(oNRoot, &nodeCount);
   if(result == FALSE) {
      return result;
   }

   /* Check if ulCount != nodes reached during tree traversal */
   if (nodeCount != ulCount) {
      fprintf(stderr, "Tree size (%ld) does not match number of nodes reachable (%ld)\n"
              , ulCount, nodeCount);
      return FALSE;
   }
   
   return result;
}
