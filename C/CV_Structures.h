/****************************************************************************
*																			*
*  This header file contain all structures used in the CV library	*
*  																			*
*	Author: Petar Nikolov													*
*																			*
*																			*
*****************************************************************************/

/*********************    Main structure in the program    ********************************************************/

//#ifndef STRUKTURA
typedef struct Image
{
    int Width;                // Image Width
    int Height;               // Image Height
    unsigned char *rgbpix;    // Pointer to data of the image
    int Num_channels;         // Number of channels for image. 1 = Grayscale, 3 = Color
    int imageDepth;
    char *Image_FileName;     // Image file name. Currently is not used
    int ColorSpace;           // Color space: 0 - Binary image, 1 -Grayscale, 2 - RGB, 3 - YCbCr, 4 - Lab, 5 - HSL
    int isLoaded;			  // This flag is raised if the image is succefully loaded
}Image;
//#define STRUKTURA

/******************************************************************************************************************/

struct point_xy
{
    float X;				  // x-axis
    float Y;				  // y-axis

}point;

struct ArrPoints
{
    struct point_xy *ArrayOfPoints;

}ArrPoints;

/* White Balance -> L*ab algorithm */
struct WhitePoint
{
    int Temperature;

    float X;
    float Y;
    float Z;

    float u;
    float v;

}WhitePoint;

struct ColorPoint_RGB
{
    int R;
    int G;
    int B;

}ColorPoint_RGB;

struct ColorPoint_Lab
{
    float L;
    float a;
    float b;

}ColorPoint_Lab;

struct ColorPoint_HSV
{
    int H;
    int S;
    int V;

}ColorPoint_HSV;

struct ColorPoint_YCbCr
{
    int Y;
    int Cb;
    int Cr;

}ColorPoint_YCbCr;

struct ColorPoint_XYZ
{
    float X;
    float Y;
    float Z;

}ColorPoint_XYZ;

struct ColorPoint_UV
{
    float u;
    float v;

}ColorPoint_UV;

struct Histogram
{
    int Bins;
    int Size_x;
    int Size_y;
    long int *values;  // the hist image is here
    long int MaxValue;
    short NumberOfLayers;
}Histogram;

//#endif
