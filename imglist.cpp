// File:        imglist.cpp
// Date:        2022-01-27 10:21
// Description: Contains partial implementation of ImgList class
//              for CPSC 221 2021W2 PA1
//              Function bodies to be completed by yourselves
//
// ADD YOUR FUNCTION IMPLEMENTATIONS IN THIS FILE
//

#include "imglist.h"

#include <math.h> // provides fmin and fabs functions

/**************************
* MISCELLANEOUS FUNCTIONS *
**************************/

/*
* This function is NOT part of the ImgList class,
* but will be useful for one of the ImgList functions.
* Returns the "difference" between two hue values.
* PRE: hue1 is a double between [0,360).
* PRE: hue2 is a double between [0,360).
* 
* The hue difference is the absolute difference between two hues,
* but takes into account differences spanning the 360 value.
* e.g. Two pixels with hues 90 and 110 differ by 20, but
*      two pixels with hues 5 and 355 differ by 10.
*/
double HueDiff(double hue1, double hue2) {
  return fmin(fabs(hue1 - hue2), fabs(360 + fmin(hue1, hue2) - fmax(hue1, hue2)));
}

/*********************
* CONSTRUCTORS, ETC. *
*********************/

/*
* Default constructor. Makes an empty list
*/
ImgList::ImgList() {
  // set appropriate values for all member attributes here
  northwest = NULL;
  southeast = NULL;
}

/*
* Creates a list from image data
* PRE: img has dimensions of at least 1x1
*/
ImgList::ImgList(PNG& img) {
  // build the linked node structure and set the member attributes appropriately
  ImgNode* lastX;
  ImgNode* lastY;

  for (unsigned int x = 0; x < img.width(); x++) {
    if (x > 0) {
      lastX = northwest;
      for (unsigned int i = 1; i < x; i++) {
        lastX = lastX->east;
      }
    }
    for (unsigned int y = 0; y < img.height(); y++) {
      if (x == 0 && y == 0) { 
        northwest = new ImgNode();
        northwest->colour = *img.getPixel(0, 0);
        lastX = northwest;
        lastY = northwest;
      } else if (x == 0) {
        ImgNode* curr = new ImgNode();
        curr->colour = *img.getPixel(x, y);
        lastY->south = curr;
        curr->north = lastY;
        lastY = curr;
      } else if (y == 0) {
        ImgNode* curr = new ImgNode();
        curr->colour = *img.getPixel(x, y);
        lastX->east = curr;
        curr->west = lastX;
        lastX = lastX->south;
        lastY = curr;
      } else {
        ImgNode* curr = new ImgNode();
        curr->colour = *img.getPixel(x, y);
        lastX->east = curr;
        curr->west = lastX;
        lastY->south = curr;
        curr->north = lastY;
        lastX = lastX->south;
        lastY = curr;
        southeast = curr;
      }
    }
  }
}

/*
* Copy constructor.
* Creates this this to become a separate copy of the data in otherlist
*/
ImgList::ImgList(const ImgList& otherlist) {
  // build the linked node structure using otherlist as a template
  Copy(otherlist);
}

/*
* Assignment operator. Enables statements such as list1 = list2;
*   where list1 and list2 are both variables of ImgList type.
* POST: the contents of this list will be a physically separate copy of rhs
*/
ImgList& ImgList::operator=(const ImgList& rhs) {
  // Re-build any existing structure using rhs as a template
  
  if (this != &rhs) { // if this list and rhs are different lists in memory
    // release all existing heap memory of this list
    Clear();    
    
    // and then rebuild this list using rhs as a template
    Copy(rhs);
  }
  
  return *this;
}

/*
* Destructor.
* Releases any heap memory associated with this list.
*/
ImgList::~ImgList() {
  // Ensure that any existing heap memory is deallocated
  Clear();
}

/************
* ACCESSORS *
************/

/*
* Returns the horizontal dimension of this list (counted in nodes)
* Note that every row will contain the same number of nodes, whether or not
*   the list has been carved.
* We expect your solution to take linear time in the number of nodes in the
*   x dimension.
*/
unsigned int ImgList::GetDimensionX() const {
  int count = 0;
  ImgNode* curr = northwest;

  while (curr != NULL) {
    count++;
    curr = curr->east;
  }

  return count;
}

/*
* Returns the vertical dimension of the list (counted in nodes)
* It is useful to know/assume that the grid will never have nodes removed
*   from the first or last columns. The returned value will thus correspond
*   to the height of the PNG image from which this list was constructed.
* We expect your solution to take linear time in the number of nodes in the
*   y dimension.
*/
unsigned int ImgList::GetDimensionY() const {
  int count = 0;
  ImgNode* curr = northwest;

  while (curr != NULL) {
    count += 1 + curr->skipdown;
    curr = curr->south;
  }

  return count;
}

/*
* Returns the horizontal dimension of the list (counted in original pixels, pre-carving)
* The returned value will thus correspond to the width of the PNG image from
*   which this list was constructed.
* We expect your solution to take linear time in the number of nodes in the
*   x dimension.
*/
unsigned int ImgList::GetDimensionFullX() const {
  int count = 0;
  ImgNode* curr = northwest;

  while (curr != NULL) {
    count += 1 + curr->skipright;
    curr = curr->east;
  }

  return count;
}

/*
* Returns a pointer to the node which best satisfies the specified selection criteria.
* The first and last nodes in the row cannot be returned.
* PRE: rowstart points to a row with at least 3 physical nodes
* PRE: selectionmode is an integer in the range [0,1]
* PARAM: rowstart - pointer to the first node in a row
* PARAM: selectionmode - criterion used for choosing the node to return
*          0: minimum luminance across row, not including extreme left or right nodes
*          1: node with minimum total of "hue difference" with its left neighbour and with its right neighbour.
*        In the (likely) case of multiple candidates that best match the criterion,
*        the left-most node satisfying the criterion (excluding the row's starting node)
*        will be returned.
* A note about "hue difference": For PA1, consider the hue value to be a double in the range [0, 360).
* That is, a hue value of exactly 360 should be converted to 0.
* The hue difference is the absolute difference between two hues,
* but be careful about differences spanning the 360 value.
* e.g. Two pixels with hues 90 and 110 differ by 20, but
*      two pixels with hues 5 and 355 differ by 10.
*/
ImgNode* ImgList::SelectNode(ImgNode* rowstart, int selectionmode) {
  // add your implementation below
  switch (selectionmode) {
    case 0: {
      ImgNode* curr = rowstart->east;
      ImgNode* minLum = curr;
      while (curr != NULL && curr->east != NULL) {
        if (curr->colour.l < minLum->colour.l) {
          minLum = curr;
        }
        curr = curr->east;
      }
      return minLum;
    }

    case 1: {
      ImgNode* curr = rowstart->east;
      ImgNode* minHueDiff = curr;
      double minHueDiffValue = HueDiff(minHueDiff->colour.h, minHueDiff->west->colour.h) + HueDiff(minHueDiff->colour.h, minHueDiff->east->colour.h);
      while (curr != NULL && curr->east != NULL) {
        double currHueDiffValue = HueDiff(curr->colour.h, curr->west->colour.h) + HueDiff(curr->colour.h, curr->east->colour.h);
        if (currHueDiffValue < minHueDiffValue) {
          minHueDiff = curr;
          minHueDiffValue = currHueDiffValue;
        }
        curr = curr->east;
      }
      return minHueDiff;
    }

    default: {
      return NULL;
    }
  }
}

/*
* Renders this list's pixel data to a PNG, with or without filling gaps caused by carving.
* PRE: fillmode is an integer in the range of [0,2]
* PARAM: fillgaps - whether or not to fill gaps caused by carving
*          false: render one pixel per node, ignores fillmode
*          true: render the full width of the original image,
*                filling in missing nodes using fillmode
* PARAM: fillmode - specifies how to fill gaps
*          0: solid, uses the same colour as the node at the left of the gap
*          1: solid, using the averaged values (all channels) of the nodes at the left and right of the gap
*             Note that "average" for hue will use the closer of the angular distances,
*             e.g. average of 10 and 350 will be 0, instead of 180.
*             Average of diametric hue values will use the smaller of the two averages
*             e.g. average of 30 and 210 will be 120, and not 300
*                  average of 170 and 350 will be 80, and not 260
*          2: *** OPTIONAL - FOR BONUS ***
*             linear gradient between the colour (all channels) of the nodes at the left and right of the gap
*             e.g. a gap of width 1 will be coloured with 1/2 of the difference between the left and right nodes
*             a gap of width 2 will be coloured with 1/3 and 2/3 of the difference
*             a gap of width 3 will be coloured with 1/4, 2/4, 3/4 of the difference, etc.
*             Like fillmode 1, use the smaller difference interval for hue,
*             and the smaller-valued average for diametric hues
*/
PNG ImgList::Render(bool fillgaps, int fillmode) const {
  // Add/complete your implementation below
  PNG outpng; //this will be returned later. Might be a good idea to resize it at some point.

  if (fillgaps) {
    outpng = PNG(GetDimensionFullX(), GetDimensionY());
    switch (fillmode) {
      case 0: {
        ImgNode* currNode = northwest;
        ImgNode* nextY = northwest;
        int count = currNode->skipright;

        for (unsigned int y = 0; y < GetDimensionY(); y++) {
          if (y > 0) {
            nextY = nextY->south;
            currNode = nextY;
          }
          for (unsigned int x = 0; x < GetDimensionFullX(); x++) {
            HSLAPixel* currPixel = outpng.getPixel(x, y);
            *currPixel = currNode->colour;
            if (count <= 0 && currNode->east != NULL) {
              currNode = currNode->east;
              count = currNode->skipright;
            }
            count--;
          }
        }
        break;
      }

      case 1: {
        ImgNode* currNode = northwest;
        ImgNode* nextY = northwest;
        int countX = currNode->skipright;
        
        for (unsigned int y = 0; y < GetDimensionFullX(); y++) {
          if (y > 0) {
            nextY = nextY->south;
            currNode = nextY;
          }
          for (unsigned int x = 0; x < GetDimensionY(); x++) {
            HSLAPixel* currPixel = outpng.getPixel(x, y);
            ImgNode* nextNode = currNode->east;
            currPixel->h = fmod(((fmin(currNode->colour.h, nextNode->colour.h) - (HueDiff(currNode->colour.h, nextNode->colour.h))) / 2), 360);
            
            if (countX == 0) {
              currNode = currNode->east;
              countX = currNode->skipright;
            }
            countX--;
          }
        }
        break;
      }

      default: {
        return outpng;
      }
    }
  } else {
    outpng = PNG(GetDimensionX(), GetDimensionY());
    ImgNode* currNode = northwest;
    ImgNode* nextY = northwest;

    for (unsigned int y = 0; y < GetDimensionY(); y++) {
      if (y > 0) {
        nextY = nextY->south;
        currNode = nextY;
      }
      for (unsigned int x = 0; x < GetDimensionX(); x++) {
        HSLAPixel* currPixel = outpng.getPixel(x, y);
        *currPixel = currNode->colour;
        currNode = currNode->east;
      }
    }
  }
  
  return outpng;
}

/************
* MODIFIERS *
************/

/*
* Removes exactly one node from each row in this list, according to specified criteria.
* The first and last nodes in any row cannot be carved.
* PRE: this list has at least 3 nodes in each row
* PRE: selectionmode is an integer in the range [0,1]
* PARAM: selectionmode - see the documentation for the SelectNode function.
* POST: this list has had one node removed from each row. Neighbours of the created
*       gaps are linked appropriately, and their skip values are updated to reflect
*       the size of the gap.
*/
void ImgList::Carve(int selectionmode) {
  ImgNode* rowStart = this->northwest;
  while (rowStart != NULL) {
    ImgNode* curr = SelectNode(rowStart, selectionmode);
    ImgNode* east = curr->east;
    ImgNode* west = curr->west;
    ImgNode* north = curr->north;
    ImgNode* south = curr->south;

    east->west = west;
    east->skipleft += curr->skipleft + 1;

    west->east = east;
    west->skipright += curr->skipright + 1;

    if (north != NULL) {
      north->south = south;
      north->skipdown += curr->skipdown + 1;
    }

    if (south != NULL) {
      south->north = north;
      south->skipup += curr->skipup + 1;
    }

    delete curr;
    curr = NULL;
    rowStart = rowStart->south;
  }
}

// note that a node on the boundary will never be selected for removal
/*
* Removes "rounds" number of nodes (up to a maximum of node width - 2) from each row,
* based on specific selection criteria.
* Note that this should remove one node from every row, repeated "rounds" times,
* and NOT remove "rounds" nodes from one row before processing the next row.
* PRE: selectionmode is an integer in the range [0,1]
* PARAM: rounds - number of nodes to remove from each row
*        If rounds exceeds node width - 2, then remove only node width - 2 nodes from each row.
*        i.e. Ensure that the final list has at least two nodes in each row.
* POST: this list has had "rounds" nodes removed from each row. Neighbours of the created
*       gaps are linked appropriately, and their skip values are updated to reflect
*       the size of the gap.
*/
void ImgList::Carve(unsigned int rounds, int selectionmode) {
  unsigned int roundsRemaining;
  if (rounds > this->GetDimensionX() - 2) {
    roundsRemaining = this->GetDimensionX() - 2;
  } else {
    roundsRemaining = rounds;
  }
  
  for (unsigned int x = 0; x < roundsRemaining; x++) {
    Carve(selectionmode);
  }
}


/*
* Helper function deallocates all heap memory associated with this list,
* puts this list into an "empty" state. Don't forget to set your member attributes!
* POST: this list has no currently allocated nor leaking heap memory,
*       member attributes have values consistent with an empty list.
*/
void ImgList::Clear() {
  ImgNode* curr = northwest;
  ImgNode* nextX = northwest->east;
  unsigned int width = this->GetDimensionX();

  for (unsigned int x = 0; x < width; x++) {
    if (x > 0) {
      curr = nextX;
      nextX = nextX->east;
    }
    while (curr != NULL) {
        ImgNode* next = curr->south;
        delete curr;
        curr = next;
    }
  }

  northwest = NULL;
  southeast = NULL;
}

/* ************************
*  * OPTIONAL - FOR BONUS *
** ************************
* Helper function copies the contents of otherlist and sets this list's attributes appropriately
* PRE: this list is empty
* PARAM: otherlist - list whose contents will be copied
* POST: this list has contents copied from by physically separate from otherlist
*/
void ImgList::Copy(const ImgList& otherlist) {
  // add your implementation here
  
}

/*************************************************************************************************
* IF YOU DEFINED YOUR OWN PRIVATE FUNCTIONS IN imglist.h, YOU MAY ADD YOUR IMPLEMENTATIONS BELOW *
*************************************************************************************************/

