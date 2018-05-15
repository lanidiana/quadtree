/**
 * @file quadtree.cpp
 * Quadtree class implementation.
 */

#include <cstddef>
#include <cstdlib>
#include <iostream>

using namespace std;
using std::cout;
using std::endl;

#include "quadtree.h"
#include "png.h"


// Quadtree
//   - parameters: none
//   - constructor for the Quadtree class; makes an empty tree
Quadtree::Quadtree() 
{
	root = NULL;
	res = 0;
}

// Quadtree
//   - parameters: PNG const & source - reference to a const PNG
//                    object, from which the Quadtree will be built
//                 int resolution - resolution of the portion of source
//                    from which this tree will be built
//   - constructor for the Quadtree class; creates a Quadtree representing
//        the resolution by resolution block in the upper-left corner of
//        source
Quadtree::Quadtree(PNG const& source, int setresolution)
{
	root = NULL;
	res = 0;

	buildTree(source,setresolution);
}

// Quadtree
//   - parameters: Quadtree const & other - reference to a const Quadtree
//                    object, which the current Quadtree will be a copy of
//   - copy constructor for the Quadtree class
Quadtree::Quadtree(Quadtree const& other) 
{
	res = other.res;
	root = deepCopyHelper(other.root);
}

// ~Quadtree
//   - parameters: none
//   - destructor for the Quadtree class
Quadtree::~Quadtree()
{
	deleteTreeHelper(root);
	root = NULL;
	res = 0;
}

// operator=
//   - parameters: Quadtree const & other - reference to a const Quadtree
//                    object, which the current Quadtree will be a copy of
//   - return value: a const reference to the current Quadtree
//   - assignment operator for the Quadtree class
Quadtree const& Quadtree::operator=(Quadtree const& other)
{
	if(this != &other)
	{
		res = other.res;
		deleteTreeHelper(root);
		root = NULL;

		root = deepCopyHelper(other.root);
	}
	return *this;
}

// buildTree (public interface)
//   - parameters: PNG const & source - reference to a const PNG
//                    object, from which the Quadtree will be built
//                 int resolution - resolution of the portion of source
//                    from which this tree will be built
//   - transforms the current Quadtree into a Quadtree representing
//        the resolution by resolution block in the upper-left corner of
//        source
void Quadtree::buildTree(PNG const& source, int setresolution)
{
	//cout<<"Entering buildTree"<<endl;
	//cout<<"Printing out root"<< root<< endl;
	deleteTreeHelper(root);
	root = NULL;

	res = setresolution;
	int imageWidth = source.width();
	int imageHeight = source.height();
	int x = 0;
	int y = 0;

	root = helperbuildTree(root, res, source, x, y);

}


// helper for buildTree
//   - parameters: QuadtreeNode, resolution, source of PNG and x and y coordinates 
//   - recursively builds the tree from the source file
Quadtree::QuadtreeNode* Quadtree::helperbuildTree(QuadtreeNode* nd, int resolution, PNG const& source, int x, int y)
{
	nd = new QuadtreeNode();
	if(resolution == 1)
	{
		nd->element = *source(x,y);
	}
	if(resolution > 1)
	{
		nd->nwChild = helperbuildTree(nd->nwChild, resolution/2, source, x, y);
		
		nd->neChild = helperbuildTree(nd->neChild, resolution/2, source, x+resolution/2, y);
		
		nd->swChild = helperbuildTree(nd->swChild, resolution/2, source, x, y+resolution/2);
		
		nd->seChild = helperbuildTree(nd->seChild, resolution/2, source, x+resolution/2, y+resolution/2);
		
		nd->element = calculatingAverageRGBA(nd);
	}
	return nd;
}


// helper for calculating the RGBA colour
//    - parameter: current node
//    - takes the average of the four pixel colours, red, blue, green and alpha
//    - returns the PRGBAPixel
RGBAPixel Quadtree::calculatingAverageRGBA(const QuadtreeNode* nd)
{
	int avred = averageRed(nd);
	int avblue = averageBlue(nd);
	int avgreen = averageGreen(nd);
	int avalpha = averageAlpha(nd);

	return RGBAPixel(avred,avgreen,avblue,avalpha);
}


// helper for taking the average of the red elements in a QuadtreeNode
//   - parameter: QuadtreeNode
//   - returns the int value of the colour
int Quadtree::averageRed(const QuadtreeNode* nd)
{
	int total = (int)((nd->nwChild)->element.red + (int)(nd->neChild)->element.red+ (int)(nd->swChild)->element.red + (int)(nd->seChild)->element.red);
	return total/4;
}

// helper for taking the average of the blue elements in a QuadtreeNode
//   - parameter: QuadtreeNode
//   - returns the int value of the colour
int Quadtree::averageBlue(const QuadtreeNode* nd)
{
	int total = (int)(nd->nwChild)->element.blue + (int)(nd->neChild)->element.blue+ (int)(nd->swChild)->element.blue + (int)(nd->seChild)->element.blue;
	return total/4;
}

// helper for taking the average of the green elements in a QuadtreeNode
//   - parameter: QuadtreeNode
//   - returns the int value of the colour
int Quadtree::averageGreen(const QuadtreeNode* nd)
{
	int total = (int)((nd->nwChild)->element.green + (int)(nd->neChild)->element.green+ (int)(nd->swChild)->element.green + (int)(nd->seChild)->element.green);
	return total/4;
}

// helper for taking the average of the alpha elements in a QuadtreeNode
//   - parameter: QuadtreeNode
//   - returns the int value of the colour
int Quadtree::averageAlpha(const QuadtreeNode* nd)
{
	int total = (int)((nd->nwChild)->element.alpha + (int)(nd->neChild)->element.alpha+(int) (nd->swChild)->element.alpha +(int) (nd->seChild)->element.alpha);
	return total/4;
}

// getPixel (public interface)
//   - parameters: int x, int y - coordinates of the pixel to be retrieved
//   - return value: an RGBAPixel representing the desired pixel of the
//        underlying bitmap
//   - retrieves and returns the pixel at coordinates (x, y) in the
//        underlying bitmap
RGBAPixel Quadtree::getPixel(int x, int y) const
{
	if(root == NULL || x >= res || y >= res || x < 0 || y < 0)
		return RGBAPixel();
	return getPixelHelper(root, x, y, res);
}

// helper to get the pixel from given coordinates
//    - parameters: QuadtreeNode, x and y coordinates and the resolution
//    - returns the pixel at the given coordinate
RGBAPixel Quadtree::getPixelHelper(QuadtreeNode* nd,int x, int y, int resolution) const
{
	RGBAPixel p;
	if(nd == NULL)
		return RGBAPixel();
	if(resolution != 1 && nd->nwChild == NULL && nd->neChild == NULL && nd->swChild == NULL && nd->seChild == NULL)
		return nd->element;
	if(resolution==1)
		p=nd->element;
	if(resolution>1)
	{
		if(x<resolution/2 && y<resolution/2)
			p=  getPixelHelper(nd->nwChild , x, y, resolution/2);
		if(x>=resolution/2 && y<resolution/2)
			p=  getPixelHelper(nd->neChild , x-(resolution/2), y, resolution/2);
		if(x<resolution/2 && y>=resolution/2)
			p=  getPixelHelper(nd->swChild , x, y-(resolution/2), resolution/2);
		if(x>=resolution/2 && y>=resolution/2)
			p=  getPixelHelper(nd->seChild , x-(resolution/2), y-(resolution/2), resolution/2);
	}
	return p;

}

// decompress (public interface)
//   - parameters: none
//   - return value: a PNG object representing this quadtree's underlying
//        bitmap
//   - constructs and returns this quadtree's underlying bitmap
PNG Quadtree::decompress() const
{	
	PNG picture(res,res);
	int width = picture.width();
	int height = picture.height();
	
	for (int x=0; x<width; x++)
	{
		for(int y=0; y<height;y++)
		{
			if(root==NULL)
				cout<<"Root is NULL"<<endl;
			*picture(x,y) = getPixel(x,y);
		}
	}
	return picture;
}

// helper method for decompress
//     - parameters: takes in QuadtreeNode, x, y coordinates, and resolution
//     - converts the given Quadtree and turns it into a PNG.
void Quadtree:: helperDecompress(PNG& png, QuadtreeNode* curr, int x, int y, int resolution) const
{
	
	if(resolution!=1 && curr->nwChild==NULL && curr->neChild==NULL && curr->swChild==NULL && curr->seChild==NULL)
		*png(x,y) = curr->element;
    
    if(resolution==1)
		*png(x,y) = curr->element; 

	if(resolution>1)
		{
			if(x<resolution/2 && y<resolution/2)
			helperDecompress(png, curr->nwChild, x, y, resolution/2);
			if(x>=resolution/2 && y<resolution/2)
			helperDecompress(png, curr->neChild, x-(resolution/2), y, resolution/2);
			if(x<resolution/2 && y>=resolution/2)
			helperDecompress(png, curr->swChild, x, y-(resolution/2), resolution/2);
			if(x>=resolution/2 && y>=resolution/2)
			helperDecompress(png, curr->seChild, x-(resolution/2), y-(resolution/2), resolution/2);
		}
		
} 


// clockwiseRotate (public interface)
//   - parameters: none
//   - transforms this quadtree into a quadtree representing the same
//        bitmap, rotated 90 degrees clockwise
void Quadtree::clockwiseRotate()
{
	clockwiseRotateHelper(root);
}

// helper function to clockwiseRotate function
//    - parameters: QuadtreeNode
//    - redirects the pointers in the tree structure to rotate the
//      image 90 degrees clockwise
void Quadtree:: clockwiseRotateHelper( QuadtreeNode* nd)
{
	if(nd==NULL)
		return;

	QuadtreeNode* temp1 = nd->nwChild;
	QuadtreeNode* temp2 = nd->neChild;

	nd->nwChild = nd->swChild;
	nd->swChild = nd->seChild;
	nd->neChild = temp1;
	nd->seChild = temp2;


	temp1=NULL;
	temp2=NULL;

	clockwiseRotateHelper(nd->nwChild);
	clockwiseRotateHelper(nd->neChild);
	clockwiseRotateHelper(nd->swChild);
	clockwiseRotateHelper(nd->seChild);

}

// prune (public interface)
//   - parameters: int tolerance - an integer representing the maximum
//                    "distance" which we will permit between a node's color
//                    (i.e. the average of its descendant leaves' colors)
//                    and the color of each of that node's descendant leaves
//   - for each node in the quadtree, if the "distance" between the average
//        of that node's descendant leaves' colors and the color of each of
//        that node's descendant leaves is at most tolerance, this function
//        deletes the subtrees beneath that node; we will let the node's
//        color "stand in for" the colors of all (deleted) leaves beneath it
void Quadtree::prune(int tolerance)
{
	
	helpPrune(root, tolerance);
}

// helper to prune method
//    - parameters: QuadtreeNode and tolerance
//    - recursively checks to see if a node is prunable and if so
//      delete all its childNodes
void Quadtree::helpPrune(QuadtreeNode* nd, int tolerance)
{
	
	if(nd==NULL)
		return;
    if (prunable (nd, nd, tolerance))
    {
    	
		deleteTreeHelper(nd->nwChild);
		deleteTreeHelper(nd->neChild);
		deleteTreeHelper(nd->swChild);
		deleteTreeHelper(nd->seChild);

		nd->nwChild=NULL;
		nd->neChild=NULL;
		nd->swChild=NULL;
		nd->seChild=NULL;

    }
    else
    {

    
	helpPrune(nd->nwChild, tolerance);
	
	helpPrune(nd->neChild, tolerance);
	
	helpPrune(nd->swChild, tolerance);
	
	helpPrune(nd->seChild, tolerance);
    }

}


// helper to check if a node is question is prunable with leaf
//   - parameters: one QuadtreeNode in question, and the other QuadtreeNode to
//    		   keep track of current Node as well as the tolerance
//   - returns true if the node in question is prunable with its leaves, else false
bool Quadtree::prunable(QuadtreeNode* nd, QuadtreeNode* curr, int tolerance)
{
	if(curr->nwChild!=NULL)
	{
		if( prunable(nd, curr->nwChild, tolerance) &&
			prunable(nd, curr->neChild, tolerance) &&
			prunable(nd, curr->swChild, tolerance) &&
			prunable(nd, curr->seChild, tolerance))
			return true;
		return false;
	}
	return diffPrune(nd, curr,tolerance);
}



// helper function to determine if colour value of no leaf in Tn differs from
// the component-wise average colour value, a, by no more than tolerance
bool Quadtree::diffPrune(QuadtreeNode* parent, QuadtreeNode* child, int tolerance)
{

    int diffRed = child->element.red - parent->element.red;
    int diffBlue = child->element.blue - parent->element.blue;
    int diffGreen = child->element.green - parent->element.green;
    int d = pow(diffRed,2)+pow(diffBlue,2)+pow(diffGreen,2);
    if (d <= tolerance)
    {
		return true;
    }
    else return false;
}

// pruneSize (public interface)
//   - parameters: int tolerance - an integer representing the maximum
//                    "distance" which we will permit between a node's color
//                    (i.e. the average of its descendant leaves' colors)
//                    and the color of each of that node's descendant leaves
//   - returns the number of leaves which this quadtree would contain if it
//        was pruned using the given tolerance; does not actually modify the
//        tree
int Quadtree::pruneSize(int tolerance) const
{
	
	return helpPruneSize(root, tolerance);
	
}


// helper to pruneSize method
//    - parameter: takes in QuadtreeNode and the tolerance
//    - returns the number of leaves that would be left after pruning
int Quadtree:: helpPruneSize(QuadtreeNode* nd, int tolerance) const
{
	int count=pow(4,treeHeight(nd));
	if(nd==NULL)
		return 0;
    if (prunableSize (nd, nd, tolerance))
    {
    	return count-pow(4,treeHeight(nd))+1;
    }
    else
    {
	count= helpPruneSize(nd->nwChild, tolerance)+ helpPruneSize(nd->neChild, tolerance)+ helpPruneSize(nd->swChild, tolerance)+helpPruneSize(nd->seChild, tolerance);
    }

    return count;

}

// helper to determine if the node in question is prunable
//    - parameters: QuadtreeNode in question and the other to keep track of
// 		    current node, and the tolerance
//    - returns true if the node in question is prunable with the leaves else return false
bool Quadtree::prunableSize(QuadtreeNode* nd, QuadtreeNode* curr, int tolerance) const
{
	if(curr->nwChild!=NULL)
	{
		if( prunableSize(nd, curr->nwChild, tolerance) &&
			prunableSize(nd, curr->neChild, tolerance) &&
			prunableSize(nd, curr->swChild, tolerance) &&
			prunableSize(nd, curr->seChild, tolerance))
			return true;
		return false;
	}
	return diffPruneSize(nd, curr,tolerance);
}



// helper function to determine if colour value of no leaf in Tn differs from
// the component-wise average colour value, a, by no more than tolerance
bool Quadtree::diffPruneSize(QuadtreeNode* parent, QuadtreeNode* child, int tolerance) const
{
    int diffRed = child->element.red - parent->element.red;
    int diffBlue = child->element.blue - parent->element.blue;
    int diffGreen = child->element.green - parent->element.green;
    int d = pow(diffRed,2)+pow(diffBlue,2)+pow(diffGreen,2);
    if (d <= tolerance)
    {
		return true;
    }
    else return false;
}

// idealPrune (public interface)
//   - parameters: int numLeaves - an integer representing the number of
//                    leaves we wish the quadtree to have, after pruning
//   - returns the minimum tolerance such that pruning with that tolerance
//        would yield a tree with at most numLeaves leaves
int Quadtree::idealPrune(int numLeaves) const
{

	int min = 0;
	int max = 3*255*255;
	int i=0;

	i = binarySearch(min, max, numLeaves);
	
	return i;
}


// helper function to search for idealPrune
//     - parameters: min Tolerance, max Tolerance, and the numLeaves
//		     requested by the caller method
//     - returns the minimum tolerance required to get at most numLeaves
int Quadtree:: binarySearch(int min, int max, int numLeaves) const
{
	int average = min+ (max-min)/2;
	int idealLeaves = pruneSize(average);

	if(min>max)
		return average;
	if(idealLeaves==numLeaves)
		return binarySearchMinimumTolerance(min, max, numLeaves, average);
	else
		if(numLeaves<idealLeaves)
			return binarySearch(average+1, max,numLeaves);
		return binarySearch(min, average-1,numLeaves);
	
}

// helper function to keep track of the minimum tolerance value
//    - parameter: minimum tolerance, maximum tolerance, numLeaves, and similar is the 
//     		   first number of idealLeaves found, before entering this method to search for the
//    		   absolute minimum tolerance
//    - returns the absolute minimum (closest) tolerance 
int Quadtree:: binarySearchMinimumTolerance(int min, int max, int numLeaves, int similar) const
{
	int average = min+ (max-min)/2;
	int idealLeaves = pruneSize(average);

	if(min>max)
		return -1;
	if(min==max)
		return average;
	if(idealLeaves==numLeaves)
		{
			if (similar>average)
				similar = average;
		}
	else
		if(numLeaves<idealLeaves)
			return binarySearch(average+1, max,numLeaves);
		return binarySearch(min, average-1,numLeaves);
}

// QuadtreeNode
//   - parameters: none
//   - constructor for the QuadtreeNode class; creates an empty
//        QuadtreeNode, with all child pointers NULL
Quadtree::QuadtreeNode::QuadtreeNode()
{
    neChild = seChild = nwChild = swChild = NULL;
}

// QuadtreeNode
//   - parameters: RGBAPixel const & elem - reference to a const
//        RGBAPixel which we want to store in this node
//   - constructor for the QuadtreeNode class; creates a QuadtreeNode
//        with element elem and all child pointers NULL
Quadtree::QuadtreeNode::QuadtreeNode(RGBAPixel const& elem)
{
    element = elem;
    neChild = seChild = nwChild = swChild = NULL;
}

// helper to copy constructor
//    - parameters: the QuadtreeNode to be deep copied
//    - returns a deep copied QuadtreeNode
Quadtree:: QuadtreeNode* Quadtree:: deepCopyHelper(const QuadtreeNode* other)
{
	if(other==NULL)
	{
		
		return NULL;
	}

	QuadtreeNode* nd = new QuadtreeNode(other->element);
	
	nd->nwChild = deepCopyHelper(other->nwChild);
	nd->neChild = deepCopyHelper(other->neChild);
	nd->swChild = deepCopyHelper(other->swChild);
	nd->seChild = deepCopyHelper(other->seChild);
	return nd;
}


// helper to delete the QuadtreeNode recursively
//    - parameters: the subtree node to be deleted
void Quadtree:: deleteTreeHelper(QuadtreeNode* subtrees)
{
	
	if(subtrees==NULL)
		return;
	
	deleteTreeHelper(subtrees->nwChild);
	deleteTreeHelper(subtrees->neChild);
	deleteTreeHelper(subtrees->swChild);
	deleteTreeHelper(subtrees->seChild);
	delete subtrees;

}


// helper to determine tree height, used for testing
//    - parameters: QuadtreeNode to determine the height of
//    - returns the height of the given tree
int Quadtree:: treeHeight(QuadtreeNode* nd) const
{
	if(nd==NULL)
		return -1;
	return 1+max(max(treeHeight(nd->nwChild), treeHeight(nd->neChild)), max(treeHeight(nd->swChild), treeHeight(nd->seChild)));
}


// helper to determine the max number between two givn values
//     - parameters: two int values to be compared
//     - returns the higher value of the two, if equal, return the
//   	 first parameter in question.
int Quadtree:: max(int a, int b) const
{
	if(a>=b)
		return a;
	return b;
}

