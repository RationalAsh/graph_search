///Standard template for getting a video feed from a webcam.
#include <stdio.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <math.h>
#include <SerialStream.h>
#include "string.h"
//#include <system>
//#include <math.h>
//#include <complex>

using namespace std;
using namespace cv;

///Global variables
Point clickpoint(1,1);
Point initialPos(1,1);
Point goalPoint(1,1);
int homeFlag = 0;
int destinationFlag = 0;

class imageCell
{
    public:
    ///constructor
    imageCell(Mat &image);
    ///Set image
    void setImage(Mat &image);
    ///convert from pixel to grid coordinates
    Point pixToGrid(int pix_row, int pix_col);
    ///Set the grid location
    void setGridLoc(int row, int col);
    void setGridLoc(Point gridLoc);

    ///Fill a cell with specified color
    void fillCell(int R, int G, int B);

    ///Check for obstacles
    bool isCellBlocked(Point gridLoc);
    ///Check is destination reached
    bool isCellGoal(Point gridLoc);
    ///Get distance between two grid points
    int getDirectDistance(Point gridPoint1, Point gridPoint2);

    private:
    Mat img;
    int cell_length;
    int rows, cols;
    Point gridPoint;
    Point cell_start;
    Point cell_end;

};

imageCell::imageCell(Mat &image)
{
    img = image;
    cell_length = 20;
    rows = (img.rows)/cell_length; cols = (img.cols)/cell_length;
    gridPoint.x = 1; gridPoint.y = 1;
    cout<<"\nThe size of the image is: "<<img.rows<<"x"<<img.cols<<"\n";
}

void imageCell::setImage(Mat &image)
{
    img = image;
}

Point imageCell::pixToGrid(int pix_row, int pix_col)
{
    Point pt((int)(pix_col/cell_length)+1,(int)(pix_row/cell_length)+1);
    return pt;
}

void imageCell::setGridLoc(int row, int col)
{
    gridPoint.x  = col;
    gridPoint.y  = row;
    cell_start.y = (gridPoint.y - 1)*cell_length;
    cell_start.x = (gridPoint.x - 1)*cell_length;
    cell_end.y   = (gridPoint.y)*cell_length;
    cell_end.x   = (gridPoint.x)*cell_length;
}

void imageCell::setGridLoc(Point gridLoc)
{
    gridPoint = gridLoc;
    cell_start.y = (gridPoint.y - 1)*cell_length;
    cell_start.x = (gridPoint.x - 1)*cell_length;
    cell_end.y   = (gridPoint.y)*cell_length;
    cell_end.x   = (gridPoint.x)*cell_length;
}

void imageCell::fillCell(int R, int G, int B)
{
    int i=0; int j=0;
    for(i=cell_start.y+1; i<cell_end.y; i++)
    {
        for(j=cell_start.x+1; j<cell_end.x; j++)
        {
            img.at<Vec3b>(i,j)[0] = B;
            img.at<Vec3b>(i,j)[1] = G;
            img.at<Vec3b>(i,j)[2] = R;
        }
    }
}

///These functions need to be modified. Take care.
bool imageCell::isCellBlocked(Point gridLoc)
{
    if((img.at<Vec3b>((gridLoc.y*cell_length)+6,(gridLoc.x*cell_length)+6)[0]==0)&&
        (img.at<Vec3b>((gridLoc.y*cell_length)+6,(gridLoc.x*cell_length)+6)[1]==0)&&
        (img.at<Vec3b>((gridLoc.y*cell_length)+6,(gridLoc.x*cell_length)+6)[2]==255)) return true;
        else return false;
}

bool imageCell::isCellGoal(Point gridLoc)
{
    if((img.at<Vec3b>((gridLoc.y*cell_length)+6,(gridLoc.x*cell_length)+6)[0]==0)&&
        (img.at<Vec3b>((gridLoc.y*cell_length)+6,(gridLoc.x*cell_length)+6)[1]==255)&&
        (img.at<Vec3b>((gridLoc.y*cell_length)+6,(gridLoc.x*cell_length)+6)[2]==0)) return true;
        else return false;
}

int imageCell::getDirectDistance(Point gridPoint1, Point gridPoint2)
{
    Point delta = gridPoint2 - gridPoint1;
    return (int)sqrt(pow(delta.x, 2) + pow(delta.y, 2));
}

///Pathfinder class
class Pathfinder
{
    public:

    private:
    Point currentPos;
    std::vector<Point> frontier;
    std::vector<Point> explored;
    int estim_goal; int path_cost;
    int heuristic;

};


///The mousecallback function
void mouseEvent(int event, int x, int y, int flags, void *param)
{
    imageCell *cellptr = (imageCell*) param;

    if(event==EVENT_LBUTTONDOWN )
    {
        clickpoint.x = x;
        clickpoint.y = y;
        cellptr->setGridLoc(cellptr->pixToGrid(y,x));
        cellptr->fillCell(0,0,255);
        cout<<"\nX: "<<x<<" Y: "<<y;
    }

    if((event==EVENT_LBUTTONDOWN) && (homeFlag==1))
    {
        cellptr->setGridLoc(initialPos.y, initialPos.x);
        cellptr->fillCell(255, 255, 255);
        cellptr->setGridLoc(cellptr->pixToGrid(y,x));
        cellptr->fillCell(255, 0, 0);
        initialPos = cellptr->pixToGrid(y,x);
        homeFlag = 0;
    }

    if((event==EVENT_LBUTTONDOWN)&&(destinationFlag==1))
    {
        cellptr->setGridLoc(goalPoint.y, goalPoint.x);
        cellptr->fillCell(255, 255, 255);
        cellptr->setGridLoc(cellptr->pixToGrid(y,x));
        cellptr->fillCell(0, 255, 0);
        goalPoint = cellptr->pixToGrid(y,x);
        destinationFlag = 0;
    }
}




int main(int argc, char** argv)
{
    ///Variables
    char keypress;
    Mat tempp(900, 1600, CV_8UC3, Scalar(0));
    ///Read image
    Mat maze = tempp;
    if(maze.empty()) return -1;


    ///Img processing class
    imageCell cells(maze);
    cells.setGridLoc(1,1);
    namedWindow("maze");
    setMouseCallback("maze", mouseEvent,&cells);
    int i,j;
    for(i=1; i<=45; i++)
    {
        for(j=1; j<=80; j++)
        {
            cells.setGridLoc(i,j);
            cells.fillCell(255, 255, 255);
        }
    }
    while(true)
    {
        imshow("maze", maze);
        keypress = waitKey(30);

        if(keypress==27) break;
        if(keypress=='h') homeFlag = 1;
        if(keypress=='d') destinationFlag = 1;
    }


    return 0;
}

