#include "component.h"
#include "cimage.h"
#include "bmplib.h"
#include <deque>
#include <iomanip>
#include <iostream>
#include <cmath>
// You shouldn't need other #include's - Ask permission before adding

using namespace std;

// TO DO: Complete this function
CImage::CImage(const char* bmp_filename)
{
  //  Note: readRGBBMP dynamically allocates a 3D array
  //    (i.e. array of pointers to pointers (1 per row/height) where each
  //    point to an array of pointers (1 per col/width) where each
  //    point to an array of 3 unsigned char (uint8_t) pixels [R,G,B values])

  // ================================================
  // TO DO: call readRGBBMP to initialize img_, h_, and w_;
  img_ = readRGBBMP(bmp_filename,h_,w_);

  // Leave this check
  if(img_ == NULL) 
  {
    throw std::logic_error("Could not read input file");
  }

  // Set the background RGB color using the upper-left pixel
  for(int i=0; i < 3; i++) 
  {
    bgColor_[i] = img_[0][0][i];
  }

  // ======== This value should work - do not alter it =======
  // RGB "distance" threshold to continue a BFS from neighboring pixels
  bfsBgrdThresh_ = 60;

  // ================================================
  // TO DO: Initialize the vector of vectors of labels to -1
  for (int i = 0; i < h_; i++)
  {
    vector<int> row; //create a new vector to pushback all at once into labels_
    for (int j = 0; j < w_; j++)
    {
      row.push_back(-1); 
    }
    labels_.push_back(row);
  }
  // ================================================
  // TO DO: Initialize any other data members
  components = 0; 
}

// TO DO: Complete this function
CImage::~CImage()
{
  // Add code here if necessary
  deallocateImage(img_);
}

// Complete - Do not alter
bool CImage::isCloseToBground(uint8_t p1[3], double within) 
{
  // Computes "RGB" (3D Cartesian distance)
  double dist = sqrt( pow(p1[0]-bgColor_[0],2) +
                      pow(p1[1]-bgColor_[1],2) +
                      pow(p1[2]-bgColor_[2],2) );
  return dist <= within;
} //returns true if it's the background

// TO DO: Complete this function
size_t CImage::findComponents()
{
  int label_no = 0;
  for (int i = 0; i < h_; i++) 
  {
    for (int j = 0; j < w_; j++)
    {
      if (labels_[i][j] == -1 && !isCloseToBground(img_[i][j],bfsBgrdThresh_)) //check if it's labeled and if it's close to the background
      {
        components_.push_back(bfsComponent(i, j, label_no));
        label_no++; //to create unique labels
      }
    }
  }
  components = label_no+1; 
  return label_no+1; //add one since label_no starts from 0
}

// Complete - Do not alter
void CImage::printComponents() const
{
  cout << "Height and width of image: " << h_ << "," << w_ << endl;
  cout << setw(4) << "Ord" << setw(4) << "Lbl" << setw(6) << "ULRow" << setw(6) << "ULCol" << setw(4) << "Ht." << setw(4) << "Wi." << endl;
  for(size_t i = 0; i < components_.size(); i++) 
  {
    const Component& c = components_[i];
    cout << setw(4) << i << setw(4) << c.label << setw(6) << c.ulNew.row << setw(6) << c.ulNew.col
          << setw(4) << c.height << setw(4) << c.width << endl;
  }
}


// TODO: Complete this function
int CImage::getComponentIndex(int mylabel) const
{
  for (int i = 0; i < components; i++)
  {
    if (components_[i].label == mylabel)
    {
      return i;
    }
  }
}


// Nearly complete - TO DO:
//   Add checks to ensure the new location still keeps
//   the entire component in the legal image boundaries
void CImage::translate(int mylabel, int nr, int nc)
{
  // Get the index of specified component
  int cid = getComponentIndex(mylabel);
  if(cid < 0) 
  {
    return;
  }
  int h = components_[cid].height;
  int w = components_[cid].width;

  // ==========================================================
  // ADD CODE TO CHECK IF THE COMPONENT WILL STILL BE IN BOUNDS
  // IF NOT:  JUST RETURN.
  if (nr < 0 || nr+h > h_ || nc < 0 || nc+w > w_)
  {
    return;
  }

  // ==========================================================

  // If we reach here we assume the component will still be in bounds
  // so we update its location.
  Location nl(nr, nc);
  components_[cid].ulNew = nl;
}

// TO DO: Complete this function
void CImage::forward(int mylabel, int delta)
{
  int cid = getComponentIndex(mylabel);
  if(cid < 0 || delta <= 0) 
  {
    return;
  }
  // Add your code here
  int new_position = cid + delta;
  if (new_position < 0) //if and else if checking if the new_position is in bounds
  {
    new_position = 0;
  }
  else if (new_position >= components_.size()) 
  {
    new_position = components_.size() - 1;
  }

  Component c = components_[cid];
  for (int i = cid; i < new_position; i++) 
  {
    components_[i] = components_[i+1]; //pushing the components foward
  }
  components_[new_position] = c; //push the value to the new position
}

// TO DO: Complete this function
void CImage::backward(int mylabel, int delta)
{
  int cid = getComponentIndex(mylabel);
  if(cid < 0 || delta <= 0) 
  {
    return;
  }
  // Add your code here
  int new_position = cid - delta;
  if (new_position < 0)
  {
    new_position = 0;
  }

  Component c = components_[cid];
  for (int i = cid; i > new_position; i--)
  {
    components_[i] = components_[i-1]; //pushing the components backward
  }
  components_[new_position] = c; //push the value to the new position
}

// TODO: complete this function
void CImage::save(const char* filename)
{
  // Create another image filled in with the background color
  uint8_t*** out = newImage(bgColor_);

  // Add your code here
  for (int i = 0; i < components_.size(); i++) //going through each components
  {
    Component& c = components_[i]; //referencing
    for (int j = 0; j < c.height; j++) //accessing each row and col
    {
      for (int k = 0; k < c.width; k++)
      {
        if (c.label == labels_[c.ulOrig.row+j][c.ulOrig.col+k]) //if the label is the one you need to copy
        {
          out[c.ulNew.row+j][c.ulNew.col+k][0] = img_[c.ulOrig.row+j][c.ulOrig.col+k][0]; //0,1,2 is the RGB
          out[c.ulNew.row+j][c.ulNew.col+k][1] = img_[c.ulOrig.row+j][c.ulOrig.col+k][1]; //copying the old one to the new one
          out[c.ulNew.row+j][c.ulNew.col+k][2] = img_[c.ulOrig.row+j][c.ulOrig.col+k][2];
        }
      }
    }
  }
  writeRGBBMP(filename, out, h_, w_); 
  // Add any other code you need after this
  deallocateImage(out);
}

// Complete - Do not alter - Creates a blank image with the background color
uint8_t*** CImage::newImage(uint8_t bground[3]) const
{
  uint8_t*** img = new uint8_t**[h_];
  for(int r=0; r < h_; r++) 
  {
    img[r] = new uint8_t*[w_];
    for(int c = 0; c < w_; c++) 
    {
      img[r][c] = new uint8_t[3];
      img[r][c][0] = bground[0];
      img[r][c][1] = bground[1];
      img[r][c][2] = bground[2];
    }
  }
  return img;
}

// To be completed
void CImage::deallocateImage(uint8_t*** img) const
{
  // Add your code here
  for (int i = 0; i < h_; i++)
  {
    for (int j = 0; j < w_; j++)
    {
      delete [] img[i][j];
    }
    delete [] img[i];
  }
  delete[] img;
}

// TODO: Complete the following function or delete this if
//       you do not wish to use it.
Component CImage::bfsComponent(int pr, int pc, int mylabel)
{
  // Arrays to help produce neighbors easily in a loop
  // by encoding the **change** to the current location.
  // Goes in order N, NW, W, SW, S, SE, E, NE
  int neighbor_row[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
  int neighbor_col[8] = {0, -1, -1, -1, 0, 1, 1, 1};
  deque<Location> queue;
  int min_row = h_, max_row = 0, min_col = w_, max_col = 0;
  Location loc;
  loc.row = pr; 
  loc.col = pc;
  queue.push_back(loc);
  while (!queue.empty())
  {
    loc = queue.front();
    queue.pop_front();

    if (loc.row > max_row)
    {
      max_row = loc.row;
    }
    if (loc.col > max_col)
    {
      max_col = loc.col;
    }
    if (loc.row < min_row)
    {
      min_row = loc.row;
    }
    if (loc.col < min_col)
    {
      min_col = loc.col;
    }

    for (int i = 0; i < 8; i++) //going through each neighbor
    {
      Location curr;
      curr.row = loc.row + neighbor_row[i]; //updateing the current low
      curr.col = loc.col + neighbor_col[i];
      if(curr.row < h_ && curr.row >= 0 && curr.col >= 0 && curr.col < w_)  //checking if curr is in bound
      {
        if (!isCloseToBground(img_[curr.row][curr.col],bfsBgrdThresh_) && labels_[curr.row][curr.col] == -1) //checking if it's labeled or the background
        {
          labels_[curr.row][curr.col] = mylabel; //label it
          queue.push_back(curr); //push it back to the queue
        }
      }
    }
  }
  Location UL;
  UL.row = min_row; //set the upperleft to the indicated minrow and mincol
  UL.col = min_col;
  Component c = Component(UL, max_row-min_row+1, max_col-min_col+1, mylabel); //create a new component with the new upperleft value
  return c;
}

// Complete - Debugging function to save a new image
void CImage::labelToRGB(const char* filename)
{
  //multiple ways to do this -- this is one way
  vector<uint8_t[3]> colors(components_.size());
  for(unsigned int i=0; i<components_.size(); i++) 
  {
    colors[i][0] = rand() % 256;
    colors[i][1] = rand() % 256;
    colors[i][2] = rand() % 256;
  }

  for(int i=0; i<h_; i++) 
  {
    for(int j=0; j<w_; j++) 
    {
      int mylabel = labels_[i][j];
      if(mylabel >= 0) 
      {
        img_[i][j][0] =  colors[mylabel][0];
        img_[i][j][1] =  colors[mylabel][1];
        img_[i][j][2] =  colors[mylabel][2];
      } 
      else 
      {
        img_[i][j][0] = 0;
        img_[i][j][1] = 0;
        img_[i][j][2] = 0;
      }
    }
  }
  writeRGBBMP(filename, img_, h_, w_);
}

// Complete - Do not alter
const Component& CImage::getComponent(size_t i) const
{
  if(i >= components_.size()) 
  {
    throw std::out_of_range("Index to getComponent is out of range");
  }
  return components_[i];
}

// Complete - Do not alter
size_t CImage::numComponents() const
{
  return components_.size();
}

// Complete - Do not alter
void CImage::drawBoundingBoxesAndSave(const char* filename)
{
  for(size_t i=0; i < components_.size(); i++)
  {
    Location ul = components_[i].ulOrig;
    int h = components_[i].height;
    int w = components_[i].width;
    for(int i = ul.row; i < ul.row + h; i++)
    {
      for(int k = 0; k < 3; k++)
      {
        img_[i][ul.col][k] = 255-bgColor_[k];
        img_[i][ul.col+w-1][k] = 255-bgColor_[k];
      }
      // cout << "bb " << i << " " << ul.col << " and " << i << " " << ul.col+w-1 << endl; 
    }
    for(int j = ul.col; j < ul.col + w ; j++)
    {
      for(int k = 0; k < 3; k++)
      {
        img_[ul.row][j][k] = 255-bgColor_[k];
        img_[ul.row+h-1][j][k] = 255-bgColor_[k];
      }
      // cout << "bb2 " << ul.row << " " << j << " and " << ul.row+h-1 << " " << j << endl; 
    }
  }
  writeRGBBMP(filename, img_, h_, w_);
}
// Add other (helper) function definitions here