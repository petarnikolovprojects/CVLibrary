/************************************************************************************
*																					*
*  This header file contain Image processing functions 								*
*  including reading/writing														*
*  																					*
*																					*
*	The file is limited for the purpose of 	https://github.com/petarnikolovprojects	*	
*																					*
*	Author: Petar Nikolov															*										
*************************************************************************************/

#include <stdio.h>
#include "jpeglib.h"
#include <setjmp.h>
#include <math.h>
#include "CV_Library.h"
#include <string.h>
#include <stdlib.h>



#ifndef MAX
#define MAX(a,b) (a>b?a:b)
#endif

#ifndef MIN
#define MIN(a,b) (a<b?a:b)
#endif


static void put_scanline(unsigned char buffer[], int line, int width,
                         int height, unsigned char *rgbpix);
static int debug = 0;                /* = 1; prints every pixel */


/*
    O P E N   I M A G E
*/
struct Image ReadImage(char *FileName)
{
    FILE *f_ptr = NULL;
    struct Image Img_src;
    int isJpeg = 0;
    int Counter = 0;
    Img_src.isLoaded = 0;
    /* Open the file only with extension JPEG or JPG*/
#pragma warning (disable : 4996)
    f_ptr = fopen(FileName, "rb");
    if (f_ptr == NULL)
    {
        printf("Error opening file. Program will now close\n");
        _getch();
        return Img_src;
    }
    /* Check filename extension */
    for (Counter = 0; Counter < 255; Counter++)
    {
        if (FileName[Counter] == '.' && Counter < 253)
        {
            if (FileName[Counter + 1] == 'J' || FileName[Counter + 1] == 'j')
            {
                if (FileName[Counter + 2] == 'P' || FileName[Counter + 2] == 'p')
                {
                    isJpeg = 1;
                }
            }
        }
    }
    if (isJpeg == 0)
    {
        printf("Only Jpeg file extensions are allowed\n");
        _getch();
        return Img_src;
    }

    Img_src = read_Image_file(f_ptr);
    Img_src.Image_FileName = FileName;
    Img_src.isLoaded = 1;
    return Img_src;
}


/*
    W R I T E   I M A G E
*/
void WriteImage(char * filename, struct Image Img_src, int quality)//uint16 *image_buffer, int image_width, int image_height)
{
  struct jpeg_compress_struct cinfo;

  struct jpeg_error_mgr jerr;
  /* More stuff */
  FILE * outfile;		/* target file */
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
  int row_stride;		/* physical row width in image buffer */

  cinfo.err = jpeg_std_error(&jerr);

  jpeg_create_compress(&cinfo);
  //filename = "C:\\Users\\Petar\\Downloads\\pic.jpg";
  if ((outfile = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    return;
  }
  jpeg_stdio_dest(&cinfo, outfile);

  cinfo.image_width = Img_src.Width; 	/* image width and height, in pixels */
  cinfo.image_height = Img_src.Height;
  if (Img_src.Num_channels == 3)
  {
      cinfo.input_components = 3;		/* # of color components per pixel */
      cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
      row_stride = Img_src.Width * 3;
  }
  else if (Img_src.Num_channels == 1)
  {
      cinfo.input_components = 1;		/* # of color components per pixel */
      cinfo.in_color_space = JCS_GRAYSCALE; 	/* colorspace of input image */
      row_stride = Img_src.Width;
  }
  jpeg_set_defaults(&cinfo);

  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  jpeg_start_compress(&cinfo, TRUE);

    /* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height)
  {
    row_pointer[0] = & Img_src.rgbpix[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  jpeg_finish_compress(&cinfo);

  fclose(outfile);

  jpeg_destroy_compress(&cinfo);

}

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  (*cinfo->err->output_message) (cinfo);

  longjmp(myerr->setjmp_buffer, 1);
}

/*
    R E A D   I M A G E
*/
struct Image read_Image_file(FILE * infile)
{
  struct jpeg_decompress_struct cinfo;

  struct Image Img_src;
  struct my_error_mgr jerr;

  JSAMPARRAY buffer;		/* Output row buffer */
  int row_stride;		/* physical row width in output buffer */

  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return Img_src;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_stdio_src(&cinfo, infile);

  /* Step 3: read file parameters with jpeg_read_header() */

  (void) jpeg_read_header(&cinfo, TRUE);


  (void) jpeg_start_decompress(&cinfo);

  row_stride = cinfo.output_width * cinfo.output_components;

  /* Fill the structure */
  Img_src.Width = cinfo.image_width;
  Img_src.Height = cinfo.image_height;
  Img_src.Num_channels = cinfo.num_components;
  Img_src.ColorSpace = cinfo.jpeg_color_space;
  Img_src.rgbpix = (unsigned char *)calloc(Img_src.Num_channels * Img_src.Width*Img_src.Height, sizeof(unsigned char));
  buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);


  while (cinfo.output_scanline < cinfo.output_height)
  {

    (void) jpeg_read_scanlines(&cinfo, buffer, 1);

    if(debug)printf("cinfo.output_scanline=%d\n", cinfo.output_scanline);
    if(debug)printf("cinfo.output_height=%d\n", cinfo.output_height);
    if(debug)printf("row_stride=%d\n", row_stride);
    put_scanline(buffer[0], cinfo.output_scanline, cinfo.output_width,
                 cinfo.output_height, Img_src.rgbpix);
  }

  (void) jpeg_finish_decompress(&cinfo);

  jpeg_destroy_decompress(&cinfo);

  fclose(infile);

  return Img_src;
}

static void put_scanline(unsigned char buffer[], int line, int width,
                         int height, unsigned char *rgbpix)
{
  int i, k;

    //k = (height-line)*3*width;
    k = (line-1) * 3 * width;
    for(i=0; i<3*width; i+=3)
    {
      rgbpix[k+i]   = buffer[i];
      rgbpix[k+i+1] = buffer[i+1];
      rgbpix[k+i+2] = buffer[i+2];
    }
} /* end put_scanline */



/*
    C R E A T E -  New Image
*/
struct Image CreateNewImage(struct Image *Prototype, struct Image *Img_dst, int NumChannels)
{
    Img_dst->ColorSpace = Prototype->ColorSpace;
    Img_dst->Height = Prototype->Height;
    Img_dst->Width = Prototype->Width;

    if (NumChannels == 0) Img_dst->Num_channels = Prototype->Num_channels;
    else Img_dst->Num_channels = NumChannels;

    Img_dst->rgbpix = (unsigned char *)calloc(Img_dst->Height * Img_dst->Width * Img_dst->Num_channels, sizeof(unsigned char));
    if (Img_dst->rgbpix == NULL)
    {
        printf("cannot allocate memory for the new image\n");
        Img_dst->isLoaded = 0;
    }
    else
        Img_dst->isLoaded = 1;

    return *Img_dst;
}

/*
    S E T   D E S T I N A T I O N  - to match the size of the prototype
*/
struct Image SetDestination(struct Image *Prototype, struct Image *Img_dst)
{
    Img_dst->ColorSpace = Prototype->ColorSpace;
    Img_dst->Height = Prototype->Height;
    Img_dst->Width = Prototype->Width;

    Img_dst->rgbpix = (unsigned char *)realloc(Img_dst->rgbpix, Img_dst->Height * Img_dst->Width * Img_dst->Num_channels* sizeof(unsigned char));
    if (Img_dst->rgbpix == NULL)
    {
        printf("cannot allocate memory for the new image\n");
        Img_dst->isLoaded = 0;
    }
    else
        Img_dst->isLoaded = 1;
}

/*
    D E S T R O Y  image
*/
void DestroyImage(struct Image *Img)
{
    free(Img->rgbpix);
}

/*
    B L U R  image function - around point
*/
struct Image BlurImageAroundPoint(struct Image *Img_src, struct Image *Img_dst, struct point_xy CentralPoint, int BlurPixelRadius, int SizeOfBlur, int BlurOrSharp, int BlurAgression)
{
    double MaxRatio = 0;
    double Distance = 0;
    double DistanceRatio = 0;
    double *matrix = (double *)calloc(Img_src->Width * Img_src->Height, sizeof(double));
    double Chislo = 0, Chislo2 = 0;
    int Sybiraemo = 0;
    int i, j, z, t, l;

    /* Only odd nubers are allowed (bigger than 5*/
    if (BlurPixelRadius % 2 == 0) BlurPixelRadius += 1;
    if (BlurPixelRadius < 5) BlurPixelRadius = 5;

    MaxRatio = (double)MAX(CentralPoint.X - ((double)SizeOfBlur * Img_dst->Width / 100), Img_dst->Width - CentralPoint.X + ((double)SizeOfBlur * Img_dst->Width / 100)) / ((double)SizeOfBlur * Img_dst->Width / 100);
    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            //luma = ui->imageData[row * width + col];
            Distance = sqrt(pow(abs((double)CentralPoint.X - j), 2) + pow(abs((double)CentralPoint.Y - i), 2));
            if (Distance < ((double)SizeOfBlur * Img_dst->Width / 100))
            {
                matrix[i * Img_dst->Width + j] = 1;
            }
            else
            {
                DistanceRatio = Distance / ((double)SizeOfBlur * Img_dst->Width / 100);
                matrix[i * Img_dst->Width + j] = 1 - ((double)BlurAgression / 100 * (DistanceRatio / MaxRatio));
                if (matrix[i * Img_dst->Width + j] < 0) matrix[i * Img_dst->Width + j] = 0;
            }
        }
    }

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {

            if (i < BlurPixelRadius / 2 || j < BlurPixelRadius / 2 || j >= Img_dst->Width - BlurPixelRadius / 2 || i >= Img_dst->Height - BlurPixelRadius / 2)
            {
                for (l = 0; l < Img_dst->Num_channels; l++)
                {
                    Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l];
                }
                continue;
            }
            for (l = 0; l < 3; l++)
            {
                if (Img_src->rgbpix[3 * (i*Img_dst->Width + j) + l] > 255)
                    Chislo = 0;

                Sybiraemo = 0;
                if (BlurOrSharp == 0)
                    Chislo2 = ((double)(matrix[i * Img_dst->Width + j]) / (pow((double)BlurPixelRadius, 2) - 1 - (12 + (2 * (BlurPixelRadius - 5)))));
                else
                    Chislo2 = ((double)(1 - matrix[i * Img_dst->Width + j]) / (pow((double)BlurPixelRadius, 2) - 1 - (12 + (2 * (BlurPixelRadius - 5)))));
                for (z = 0; z < BlurPixelRadius / 2; z++)
                {
                    for (t = 0; t < BlurPixelRadius / 2; t++)
                    {
                        if (z == 0 && t == 0) continue;
                        Sybiraemo += Img_src->rgbpix[3*((i - z)*Img_dst->Width + j - t) + l];
                        Sybiraemo += Img_src->rgbpix[3*((i - z)*Img_dst->Width + j + t) + l];
                        Sybiraemo += Img_src->rgbpix[3*((i + z)*Img_dst->Width + j - t) + l];
                        Sybiraemo += Img_src->rgbpix[3*((i + z)*Img_dst->Width + j + t) + l];
                    }
                }

                Chislo2 *= Sybiraemo;
                Chislo = 0;
                if (BlurOrSharp == 0)
                    Chislo = (1 - matrix[i * Img_dst->Width + j])*Img_src->rgbpix[3*(i*Img_dst->Width + j) + l] + (int)Chislo2;
                else
                    Chislo = (matrix[i * Img_dst->Width + j])*Img_src->rgbpix[3*(i*Img_dst->Width + j) + l] + (int)Chislo2;
                if (Chislo > 255)
                    Chislo = 255;
                if (Chislo < 0)
                    Chislo = 0;
                Img_dst->rgbpix[3 * (i*Img_dst->Width + j) + l] = Chislo;
            }
        }
    }

    return *Img_dst;
}

/*
    B L U R  image function - Gaussian
*/
struct Image BlurImageGussian(struct Image *Img_src, struct Image *Img_dst, int BlurPixelRadius, double NeighborCoefficient)
{
    int i, j, l, z, t;
    int Sybiraemo = 0;
    double Chislo = 0;
    double Chislo2 = 0;
    if (BlurPixelRadius < 5) BlurPixelRadius = 5;
    if (NeighborCoefficient > 100) NeighborCoefficient /= 100;
    if (NeighborCoefficient < 0) NeighborCoefficient *= -1;

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            if (i < BlurPixelRadius / 2 || j < BlurPixelRadius / 2 || j >= Img_dst->Width - BlurPixelRadius / 2 || i >= Img_dst->Height - BlurPixelRadius / 2)
            {
                for (l = 0; l < Img_dst->Num_channels; l++)
                {
                    Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l];
                }
                continue;
            }
            for (l = 0; l < Img_dst->Num_channels; l++)
            {
                Sybiraemo = 0;
                Chislo2 = ((double)(NeighborCoefficient) / (pow((double)BlurPixelRadius, 2) - 1 - (12 + (2 * (BlurPixelRadius - 5)))));

                for (z = 0; z < BlurPixelRadius / 2; z++)
                {
                    for (t = 0; t < BlurPixelRadius / 2; t++)
                    {
                        if (z == 0 && t == 0) continue;
                        Sybiraemo += Img_src->rgbpix[Img_dst->Num_channels * ((i - z)*Img_dst->Width + j - t) + l];
                        Sybiraemo += Img_src->rgbpix[Img_dst->Num_channels * ((i - z)*Img_dst->Width + j + t) + l];
                        Sybiraemo += Img_src->rgbpix[Img_dst->Num_channels * ((i + z)*Img_dst->Width + j - t) + l];
                        Sybiraemo += Img_src->rgbpix[Img_dst->Num_channels * ((i + z)*Img_dst->Width + j + t) + l];
                    }
                }

                Chislo2 *= Sybiraemo;
                Chislo = 0;
                Chislo = (1 - NeighborCoefficient)*Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] + (int)Chislo2;

                if (Chislo > 255)
                    Chislo = 255;
                if (Chislo < 0)
                    Chislo = 0;
                Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = Chislo;
            }
        }
    }
    return *Img_dst;
}

/*
    Correct   B R I G H T N E S S
*/
struct Image BrightnessCorrection(struct Image *Img_src, struct Image *Img_dst, double Algo_paramBrightnessOrEV, int Algotype)
{
    int i, j, l;

    if (Img_src->Num_channels != Img_dst->Num_channels)
        return *Img_dst;

    if (Algotype == 1)
    {
        if (abs(Algo_paramBrightnessOrEV) > 1) Algo_paramBrightnessOrEV /= 100;

        for (i = 0; i < Img_dst->Height; i++)
        {
            for (j = 0; j < Img_dst->Width; j++)
            {
                for (l = 0; l < Img_dst->Num_channels; l++)
                {
                    if (Algo_paramBrightnessOrEV * Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] + Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l]> 255)
                        Img_dst->rgbpix[3 * (i*Img_dst->Width + j) + l] = 255;
                    else
                    {
                        if (Algo_paramBrightnessOrEV * Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] + Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] < 0)
                            Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = 0;
                        else
                            Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = Algo_paramBrightnessOrEV * Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] + Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l];
                    }
                }
            }
        }
    }
    else if (Algotype == 2)
    {
        for (i = 0; i < Img_dst->Height; i++)
        {
            for (j = 0; j < Img_dst->Width; j++)
            {
                for (l = 0; l < Img_dst->Num_channels; l++)
                {
                    if (pow(2, Algo_paramBrightnessOrEV) * Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] > 255)
                        Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = 255;
                    else if (pow(2, Algo_paramBrightnessOrEV) * Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] < 0)
                        Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = 0;
                    else
                        Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = pow(2, Algo_paramBrightnessOrEV) * Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l];
                }
            }
        }
    }

    return *Img_dst;
}

/*
    Correct    C O N T R A S T
*/
struct Image ContrastCorrection(struct Image *Img_src, struct Image *Img_dst, double percentage)
{
    /* The percentage value should be between -100 and 100*/
    double pixel = 0;
    double contrast = 0;
    int i, j, l;

    if (Img_src->Num_channels != Img_dst->Num_channels)
        return *Img_dst;

    if (percentage < -100) percentage = -100;
    if (percentage > 100) percentage = 100;
    contrast = (100.0 + percentage) / 100.0;

    contrast *= contrast;

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            for (l = 0; l < Img_dst->Num_channels; l++)
            {
                pixel = (double)Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] / 255;
                pixel -= 0.5;
                pixel *= contrast;
                pixel += 0.5;
                pixel *= 255;
                if (pixel < 0) pixel = 0;
                if (pixel > 255) pixel = 255;
                Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = pixel;
            }
        }
    }

    return *Img_dst;
}

/*
    Correct     W H I T E  B A L A N C E
*/
struct Image WhiteBalanceCorrection(struct Image *Img_src, struct Image *Img_dst, int Algotype)
{
    int i, j;

    int MaxR = 0, MaxG = 0, MaxB = 0;
    double GtoR_Ratio = 1;
    double GtoB_Ratio = 1;
    long SumG = 0;
    long SumR = 0;
    long SumB = 0;
    double Gto255_Ratio = 1;
    int check3Values = 0;

    if ((Img_src->Num_channels != Img_dst->Num_channels) || (Img_src->Num_channels != 3))
        return *Img_dst;

    if (Algotype == 1)
    {
        //Green world - automatic white detection
        for (i = 0; i < Img_src->Height; i++)
        {
            for (j = 0; j < Img_src->Width; j++)
            {
                check3Values = 0;
                if (Img_src->rgbpix[3 * (i*Img_src->Width + j) ] > MaxR) { MaxR = Img_src->rgbpix[3 * (i*Img_src->Width + j)]; check3Values++; }
                if (Img_src->rgbpix[3 * (i*Img_src->Width + j)  + 1] > MaxG) { MaxG = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1]; check3Values++;}
                if (Img_src->rgbpix[3 * (i*Img_src->Width + j)  + 2] > MaxB) { MaxB = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2]; check3Values++; }

                if (check3Values == 3)
                {
                    //Calculate ratios
                    GtoR_Ratio = (double)Img_src->rgbpix[3 * (i*Img_src->Width + j) +1] / Img_src->rgbpix[3 * (i*Img_src->Width + j) ];
                    GtoB_Ratio = (double)Img_src->rgbpix[3 * (i*Img_src->Width + j) +1] / Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
                }
            }
        }
        /*Calculate new values based on GtoR amd GtoB ratios*/
        for (i = 0; i < Img_src->Height; i++)
        {
            for (j = 0; j < Img_src->Width; j++)
            {
                if (GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j)] <= 255)
                    Img_dst->rgbpix[3 * (i*Img_src->Width + j) ] = GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j)];
                else Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = 255;
                if (GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2] <= 255)
                    Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
                else Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = 255;

                Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 1] = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1];
            }
        }
    }
    else if (Algotype == 2)
    {
        for (i = 0; i < Img_src->Height; i++)
        {
            for (j = 0; j < Img_src->Width; j++)
            {
                if (Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1] > MaxG)
                {
                    MaxG = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1];
                    GtoR_Ratio = (double)MaxG / Img_src->rgbpix[3 * (i*Img_src->Width + j)];
                    GtoB_Ratio = (double)MaxG / Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
                }
            }
        }
        /*Calculate new values based on GtoR and GtoB ratios*/
        for (i = 0; i < Img_src->Height; i++)
        {
            for (j = 0; j < Img_src->Width; j++)
            {
                if (GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j)] <= 255)
                    Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j)];
                else Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = 255;
                if (GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2] <= 255)
                    Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
                else Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = 255;

                Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 1] = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1];
            }
        }

    }
    else if (Algotype == 3)
    {
        //Green world - automatic white detection
        for (i = 0; i < Img_src->Height; i++)
        {
            for (j = 0; j < Img_src->Width; j++)
            {
                check3Values = 0;
                if (Img_src->rgbpix[3 * (i*Img_src->Width + j)] > MaxR) { MaxR = Img_src->rgbpix[3 * (i*Img_src->Width + j)]; check3Values++; }
                if (Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1] > MaxG) { MaxG = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1]; check3Values++; }
                if (Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2] > MaxB) { MaxB = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2]; check3Values++; }

                if (check3Values == 3)
                {
                    //Calculate ratios
                    Gto255_Ratio = 255 / MaxG;
                    GtoR_Ratio = (double)(Gto255_Ratio *(MaxG / MaxR));
                    GtoB_Ratio = (double)(Gto255_Ratio *(MaxG / MaxB));
                }
            }
        }
        /*Calculate new values based on GtoR amd GtoB ratios*/
        for (i = 0; i < Img_src->Height; i++)
        {
            for (j = 0; j < Img_src->Width; j++)
            {
                if (GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j)] <= 255)
                    Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j)];
                else Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = 255;
                if (GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2] <= 255)
                    Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
                else Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = 255;
                if (Gto255_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1] <= 255)
                    Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 1] = Gto255_Ratio* Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1];
                else Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 1] = 255;
            }
        }
    }
    else if (Algotype == 4)
    {
        //Green world - automatic white detection
        for (i = 0; i < Img_src->Height; i++)
        {
            for (j = 0; j < Img_src->Width; j++)
            {
                SumR += Img_src->rgbpix[3 * (i*Img_src->Width + j)    ];
                SumG += Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1];
                SumB += Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
            }
        }
        GtoR_Ratio =   (double)SumG / SumR;
        GtoB_Ratio =   (double)SumG / SumB;
        Gto255_Ratio = (double)SumG / (255 * Img_src->Height * Img_src->Width);
    /*	if (GtoB_Ratio < 0.8 || GtoR_Ratio < 0.8)
        {
            GtoR_Ratio = GtoR_Ratio * ;
            GtoB_Ratio = (double)SumG / SumB;
        }*/
        /*Calculate new values based on GtoR amd GtoB ratios*/
        for (i = 0; i < Img_src->Height; i++)
        {
            for (j = 0; j < Img_src->Width; j++)
            {
                if (GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) ] <= 255)
                    Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j)];
                else Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = 255;
                if (GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2] <= 255)
                    Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
                else Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = 255;

                Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 1] = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1];
            }
        }
    }
    else return *Img_src;

    return *Img_dst;
}

/*
    Correct    N O I S E
*/
struct Image NoiseCorrection(struct Image *Img_src, struct Image *Img_dst, double threshold, int Algotype)
{
    int i, j, z;
    int CurrentValue;
    int ProbablityValue = 0;

    if (Img_src->Num_channels != Img_dst->Num_channels)
        return *Img_dst;

    memcpy(&Img_dst, &Img_src,sizeof(Image));

    /* if the current pixel is X % different from the pixels around -> it is noise*/

    if (Algotype == 1)
    {
        for (i = 1; i < Img_dst->Height - 1; i++)
        {
            for (j = 1; j < Img_dst->Width - 1; j++)
            {
                for (z = 0; z < Img_dst->Num_channels; z++)
                {
                    ProbablityValue = 0;
                    CurrentValue = Img_src->rgbpix[i * Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z];

                    if (threshold * CurrentValue < Img_src->rgbpix[(i - 1) * Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z] || CurrentValue > threshold *  Img_src->rgbpix[(i - 1) * Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z]) ProbablityValue++;
                    if (threshold * CurrentValue < Img_src->rgbpix[(i + 1) * Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z] || CurrentValue > threshold *  Img_src->rgbpix[(i + 1) * Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z]) ProbablityValue++;
                    if (threshold * CurrentValue < Img_src->rgbpix[(i)* Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * (j - 1) + z] || CurrentValue > threshold *  Img_src->rgbpix[(i)* Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * (j - 1) + z]) ProbablityValue++;
                    if (threshold * CurrentValue < Img_src->rgbpix[(i)* Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * (j + 1) + z] || CurrentValue > threshold *  Img_src->rgbpix[(i)* Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * (j + 1) + z]) ProbablityValue++;

                    if (ProbablityValue >= 3)
                    {
                        Img_dst->rgbpix[(i)* Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z] = (Img_src->rgbpix[(i - 1) * Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z] + Img_src->rgbpix[(i + 1) * Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z] + Img_src->rgbpix[(i)* Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * (j - 1) + z] + Img_src->rgbpix[(i)* Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * (j + 1) + z]) / 4;
                    }
                }
            }
        }
    }
    else if (Algotype == 2)
    {
        //future implementation
    }

    return *Img_dst;
}

/*
    Correction   G A M M A
*/
struct Image GammaCorrection(struct Image *Img_src, struct Image *Img_dst, double RedGamma, double GreenGamma, double BlueGamma)
{
    int i, j;

    int redGamma[256];
    int greenGamma[256];
    int blueGamma[256];

    if ((Img_src->Num_channels != Img_dst->Num_channels) || (Img_src->Num_channels != 3))
        return *Img_dst;

    for (i = 0; i < 256; ++i)
    {
        redGamma[i] = MIN(255, (int)((255.0 * pow((double)i / 255.0, 1.0 / RedGamma)) + 0.5));
        greenGamma[i] = MIN(255, (int)((255.0 * pow((double)i / 255.0, 1.0 / GreenGamma)) + 0.5));
        blueGamma[i] = MIN(255, (int)((255.0 * pow((double)i / 255.0, 1.0 / BlueGamma)) + 0.5));
    }

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            Img_dst->rgbpix[(i)* 3 * Img_src->Width + 3 * j] = redGamma[Img_src->rgbpix[(i)* 3 * Img_src->Width + 3 * j]];// *Img_src->rgbpix[(i)* 3 * Img_src->Width + 3 * j];
            Img_dst->rgbpix[(i)* 3 * Img_src->Width + 3 * j + 1] = greenGamma[Img_src->rgbpix[(i)* 3 * Img_src->Width + 3 * j + 1]];
            Img_dst->rgbpix[(i)* 3 * Img_src->Width + 3 * j + 2] = blueGamma[Img_src->rgbpix[(i)* 3 * Img_src->Width + 3 * j + 2]];
        }
    }
    return *Img_dst;
}

/*
    convert  I N D E X t o P O S
*/
void getPositionFromIndex(struct Image *Img_src, int pixIdx, int *red, int *col)
{
    *red = pixIdx / (Img_src->Num_channels * Img_src->Width);
    *col = pixIdx - ((*red)*Img_src->Width * Img_src->Num_channels);
}

/*
    convert   P O S t o I N D E X
*/
int getPixelIndex(struct Image *Img_src, int *pixIdx, int red, int col)
{
    int pixelIndex = 0;

    *pixIdx = (red * Img_src->Width + col)*Img_src->Num_channels;
    pixelIndex = *pixIdx;

    return pixelIndex;
}

/*
    convert to    G R A Y S C A L E  - 3 channels RGB
*/
struct Image ConvertToGrayscale_3Channels(struct Image *Img_src, struct Image *Img_dst)
{
    int i, j;

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = 0.3 * Img_src->rgbpix[3 * (i*Img_src->Width + j)] + 0.59 * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1] + 0.11 * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
            Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 1] = Img_dst->rgbpix[3 * (i*Img_src->Width + j)];
            Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = Img_dst->rgbpix[3 * (i*Img_src->Width + j)];
        }
    }

    return *Img_dst;
}

/*
    convert to    G R A Y S C A L E  - 1 channel
*/
struct Image ConvertToGrayscale_1Channel(struct Image *Img_src, struct Image *Img_dst)
{
    int i, j;

    if (Img_dst->isLoaded != 1) return *Img_dst;

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            Img_dst->rgbpix[(i*Img_src->Width + j)] = 0.3 * Img_src->rgbpix[3 * (i*Img_src->Width + j)] + 0.59 * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1] + 0.11 * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
        }
    }
}

/*
A F F I N E  -  Transformation: Rotation
*/
struct Image RotateImage(struct Image *Img_src, struct Image *Img_dst, double RotationAngle, struct point_xy CentralPoint)
{
    float Sx, Matrix[2][2], Cos, Sin;
    int i, j, z ;
    int x_new = 0, y_new = 0;
    int currentPixel_smallImage = 0;

    Sx = (-RotationAngle * 3.14) / 180;
    Cos = cos(Sx);
    Sin = sin(Sx);
    Matrix[0][0] = Cos;
    Matrix[0][1] = -Sin;
    Matrix[1][0] = Sin;
    Matrix[1][1] = Cos;

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            // za vsqko i,j (ot staoroto izobrajenie se namirat novi x_new  i  y_new na koito s epipisvat stoinostite za RGB
            x_new = Matrix[0][0] * (j - CentralPoint.X) + Matrix[0][1] * (i - CentralPoint.Y) + CentralPoint.X;
            y_new = Matrix[1][0] * (j - CentralPoint.X) + Matrix[1][1] * (i - CentralPoint.Y) + CentralPoint.Y;
            if (x_new > Img_dst->Width) x_new = Img_dst->Width;
            if (x_new < 0) x_new = 0;
            if (y_new > Img_dst->Height) y_new = Img_dst->Height;
            if (y_new < 0) y_new = 0;

            for(z = 0; z < Img_dst->Num_channels; z++)
            {
                Img_dst->rgbpix[y_new * Img_dst->Num_channels * Img_dst->Width + Img_dst->Num_channels * x_new + z] = Img_src->rgbpix[i * Img_dst->Num_channels * Img_dst->Width + Img_dst->Num_channels * j + z];
            }
        }
    }

    return *Img_dst;
}

/*
    A F F I N E  - scale (zoom) image - in/out
*/
struct Image ScaleImage(struct Image *Img_src, struct Image *Img_dst, double ScalePercentage)
{
    int i, j, z;
    int NewX = 0;
    int NewY = 0;

    /* Modify Img_dst */
    Img_dst->Width = Img_src->Width * (1 + (ScalePercentage / 100.0));
    Img_dst->Height = Img_src->Height * (1 + (ScalePercentage / 100.0));
    Img_dst->rgbpix = (unsigned char *)realloc(Img_dst->rgbpix, Img_dst->Height * Img_dst->Width * Img_dst->Num_channels* sizeof(unsigned char));

    if (ScalePercentage < 0)
    {
        for (i = 0; i < Img_dst->Height; i++)
        {
            for (j = 0; j < Img_dst->Width; j++)
            {
                NewX = j / (1 + (ScalePercentage / 100.0)); // Plus Sign because of the negative Scale Value
                NewY = i / (1 + (ScalePercentage / 100.0));

                if (NewX < 0) NewX = 0;
                if (NewY < 0) NewY = 0;
                if (NewX >= Img_src->Width) NewX = Img_src->Width - 1;
                if (NewY >= Img_src->Height) NewY = Img_src->Height - 1;

                for(z = 0; z < Img_dst->Num_channels; z++)
                {
                    Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + z ] = Img_src->rgbpix[Img_dst->Num_channels * ((NewY)*Img_src->Width + (NewX)) + z ];
                }
            }
        }
    }
    else if (ScalePercentage > 0)
    {
        for (i = 0; i < Img_dst->Height; i++)
        {
            for (j = 0; j < Img_dst->Width; j++)
            {
                NewX = j / (1 + (ScalePercentage / 100));
                NewY = i / (1 + (ScalePercentage / 100));

                if (NewX < 0) NewX = 0;
                if (NewY < 0) NewY = 0;
                if (NewX >= Img_src->Width) NewX = Img_src->Width - 1;
                if (NewY >= Img_src->Height) NewY = Img_src->Height - 1;

                for(z = 0; z < Img_dst->Num_channels; z++)
                {
                    Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + z ] = Img_src->rgbpix[Img_dst->Num_channels * ((NewY)*Img_src->Width + (NewX)) + z ];
                }
            }
        }
    }
    else return *Img_src;

    return *Img_dst;
}

/*
    A F F I N E  - Translation
*/
struct Image TranslateImage(struct Image *Img_src, struct Image *Img_dst, struct point_xy ToPoint)
{
    int i, j, z;
    int NewX = 0, NewY = 0;

    int ShiftX = ToPoint.X - Img_dst->Width / 2;
    int ShiftY = ToPoint.Y - Img_dst->Height / 2;


    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            NewX = j + ShiftX;
            NewY = i + ShiftY;
            if (ShiftX < 0 ) if(NewX < 0) continue;
            if (ShiftX > 0)if (NewX >= Img_dst->Width) continue;
            if (ShiftY < 0) if(NewY < 0) continue;
            if (ShiftY > 0)if (NewY >= Img_dst->Height) continue;

            for(z = 0; z < Img_dst->Num_channels; z++)
            {
                Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + z] = Img_src->rgbpix[Img_dst->Num_channels * ((NewY)*Img_src->Width + (NewX)) + z];
            }
        }
    }
    return *Img_dst;
}

/*
    E D G E   Extraction
*/
struct ArrPoints EdgeExtraction(struct Image *Img_src, struct Image *Img_dst, int Algotype, float Algo_param1, float Algo_param2)
{
    int i, j, z, l;
    int NewX = 0, NewY = 0;
    struct ArrPoints ArrPts;
    struct Image DerrivativeX = CreateNewImage(Img_src, &DerrivativeX, 1);
    struct Image DerrivativeY = CreateNewImage(Img_src, &DerrivativeY, 1);
    struct Image Magnitude    = CreateNewImage(Img_src, &Magnitude, 1);
    struct Image Magnitude2 = CreateNewImage(Img_src, &Magnitude2, 1);
    struct Image Magnitude3 = CreateNewImage(Img_src, &Magnitude3, 1);
    struct Image NMS = CreateNewImage(Img_src, &NMS, 1);
    struct Image Hysteresis = CreateNewImage(Img_src, &Hysteresis, 1);

    Img_dst->Width = Img_src->Width;
    Img_dst->Height = Img_src->Height;
    Img_dst->rgbpix = (unsigned char *)realloc(Img_dst->rgbpix, Img_dst->Num_channels * Img_dst->Width * Img_dst->Height * sizeof(unsigned char));
    ArrPts.ArrayOfPoints = (struct point_xy *)calloc(50,sizeof(struct point_xy));

    const float Gx[] =
      { -1, 0, 1,
        -2, 0, 2,
        -1, 0, 1 };
    const float Gy[] =
      { 1, 2, 1,
        0, 0, 0,
       -1, -2, -1 };

    const float HighPass[] =
    { 1 / 9 * (
        -1, -1, -1,
        -1,  8, -1,
        -1, -1, -1) };

     float Laplace[] =
     {  0,  1,  0,
        1, -4,  1,
        0,  1,  0 };

     float Prewitt_X_1[] =
     { -5, -5, -5,
        0,  0,  0,
        5,  5,  5 };
     float Prewitt_Y_1[] =
     { -5,  0,  5,
       -5,  0,  5,
       -5,  0,  5 };

     float Prewitt_X_2[] =
     { 5,  5,  5,
       0,  0,  0,
      -5, -5, -5  };
     float Prewitt_Y_2[] =
     { 5,  0, -5,
       5,  0, -5,
       5,  0, -5 };

     float Sobel_X_1[] =
    { -1, -2, -1,
       0,  0,  0,
       1,  2,  1 };
     float Sobel_Y_1[] =
    { -1,  0,  1,
      -2,  0,  2,
      -1,  0,  1 };

     float Sobel_X_2[] =
    {  1,  2,  1,
       0,  0,  0,
      -1, -2, -1 };
     float Sobel_Y_2[] =
    {  1,  0, -1,
       2,  0, -2,
       1,  0, -1 };

    if (Algotype < 1 || Algotype > 3)
    {
        printf("Non existing Algo for Edge extraction\n");
        return ArrPts;
    }
    /* Canny */
    if (Algotype == 1)
    {
        // Step 1: Perfrom Gaussian Blur
        BlurImageGussian(Img_src, Img_dst, (0.5 * Img_src->Width) / 100, 0.6);
        FindDerrivative_XY(Img_dst, &DerrivativeX, &DerrivativeY);
        FindMagnitudeOfGradient(&DerrivativeX, &DerrivativeY, &Magnitude);
        FindNonMaximumSupp(&Magnitude, &DerrivativeX, &DerrivativeY, &NMS);

        FindHysteresis(&Magnitude, &NMS, &Hysteresis, Algo_param1, Algo_param2);
        memcpy(Img_dst->rgbpix, Hysteresis.rgbpix, Hysteresis.Width* Hysteresis.Height * sizeof(unsigned char));
    }
    /* Sobel */
    else if (Algotype == 2)
    {
        BlurImageGussian(Img_src, Img_dst, (0.5 * Img_src->Width) / 100, 0.6);
        Convolution(Img_dst->rgbpix, DerrivativeX.rgbpix, DerrivativeX.Height, DerrivativeX.Width, Sobel_X_1, 3);
        Convolution(Img_dst->rgbpix, DerrivativeY.rgbpix, DerrivativeY.Height, DerrivativeY.Width, Sobel_Y_1, 3);
        FindMagnitudeOfGradient(&DerrivativeX, &DerrivativeY, &Magnitude);

        Convolution(Img_dst->rgbpix, DerrivativeX.rgbpix, DerrivativeX.Height, DerrivativeX.Width, Sobel_X_2, 3);
        Convolution(Img_dst->rgbpix, DerrivativeY.rgbpix, DerrivativeY.Height, DerrivativeY.Width, Sobel_Y_2, 3);
        if (Algo_param1 == 0)
        {
            FindMagnitudeOfGradient(&DerrivativeX, &DerrivativeY, &Magnitude2);
            memcpy(Img_dst->rgbpix, Magnitude2.rgbpix, Magnitude2.Width* Magnitude2.Height * sizeof(unsigned char));
        }
        else
        {
            memcpy(Img_dst->rgbpix, DerrivativeX.rgbpix, DerrivativeX.Width* DerrivativeX.Height * sizeof(unsigned char));
        }
    }
    /* Prewitt */
    else if (Algotype == 3)
    {
        BlurImageGussian(Img_src, Img_dst, (0.5 * Img_src->Width) / 100, 0.6);
        Convolution(Img_dst->rgbpix, DerrivativeX.rgbpix, DerrivativeX.Height, DerrivativeX.Width, Prewitt_X_1, 3);
        Convolution(Img_dst->rgbpix, DerrivativeY.rgbpix, DerrivativeY.Height, DerrivativeY.Width, Prewitt_Y_1, 3);
        FindMagnitudeOfGradient(&DerrivativeX, &DerrivativeY, &Magnitude);

        Convolution(Img_dst->rgbpix, DerrivativeX.rgbpix, DerrivativeX.Height, DerrivativeX.Width, Prewitt_X_2, 3);
        Convolution(Img_dst->rgbpix, DerrivativeY.rgbpix, DerrivativeY.Height, DerrivativeY.Width, Prewitt_Y_2, 3);
        FindMagnitudeOfGradient(&DerrivativeX, &DerrivativeY, &Magnitude2);
        FindMagnitudeOfGradient(&Magnitude, &Magnitude2, &Magnitude3);
        if (Algo_param1 == 0)
        {
            FindNonMaximumSupp(&Magnitude3, &Magnitude, &Magnitude2, &NMS);
            FindHysteresis(&Magnitude3, &NMS, &Hysteresis, Algo_param1, Algo_param2);

            memcpy(Img_dst->rgbpix, Hysteresis.rgbpix, Hysteresis.Width* Hysteresis.Height * sizeof(unsigned char));
        }
        else
            memcpy(Img_dst->rgbpix, Magnitude3.rgbpix, Magnitude3.Width* Magnitude3.Height * sizeof(unsigned char));
    }

    //DestroyImage(&Hysteresis);
    DestroyImage(&DerrivativeX);
    DestroyImage(&DerrivativeY);
    DestroyImage(&Magnitude);
    DestroyImage(&Magnitude2);
    DestroyImage(&Magnitude3);
    DestroyImage(&NMS);

    return ArrPts;
}

/*
    D E R R I V A T I V E    calculation
*/
void FindDerrivative_XY(struct Image *Img_src, struct Image *DerrivativeX_image, struct Image *DerrivativeY_image)
{
    int r, c, pos;
    int rows = Img_src->Height;
    int cols = Img_src->Width;

    /* Calculate X - derrivative image */
    for (r = 0; r < rows-1; r++)
    {
        pos = r * cols;
        //DerrivativeX_image->rgbpix[pos] = Img_src->rgbpix[pos + 1] - Img_src->rgbpix[pos];
        //pos++;
        for (c = 0; c < (cols - 1); c++, pos++)
        {
            DerrivativeX_image->rgbpix[pos] = abs((Img_src->rgbpix[pos] - Img_src->rgbpix[pos + cols + 1]));
        }
        DerrivativeX_image->rgbpix[pos] = abs(Img_src->rgbpix[pos] - Img_src->rgbpix[pos - 1]);
    }


    /* Calculate Y - derrivative image */
    for (c = 0; c < cols-1; c++)
    {
        pos = c;
        //DerrivativeY_image->rgbpix[pos] = Img_src->rgbpix[pos + cols] - Img_src->rgbpix[pos];
        //pos += cols;
        for (r = 0; r < (rows - 1); r++, pos += cols)
        {
            DerrivativeY_image->rgbpix[pos] = abs(Img_src->rgbpix[pos + 1] - Img_src->rgbpix[pos + cols]);
        }
        DerrivativeY_image->rgbpix[pos] = abs(Img_src->rgbpix[pos] - Img_src->rgbpix[pos - cols]);
    }

    //int kn = 3;
    //const int khalf = kn / 2;
    //float min = 35000, max = -3000;
    //const float Gx[] = { -1, 0, 1,
    //	-2, 0, 2,
    //	-1, 0, 1 };
    //const float Gy[] = { 1, 2, 1,
    //	0, 0, 0,
    //	-1, -2, -1 };

    //for (int m = khalf; m < Img_src->Width - khalf; m++)
    //for (int n = khalf; n < Img_src->Height - khalf; n++) {
    //	float pixel = 0.0;
    //	float pixel2 = 0.0;
    //	size_t c = 0;
    //	for (int j = -khalf; j <= khalf; j++)
    //	for (int i = -khalf; i <= khalf; i++) {
    //		pixel += Img_src->rgbpix[(n - j) * Img_src->Width + m - i] * Gx[c];
    //		pixel2 += Img_src->rgbpix[(n - j) * Img_src->Width + m - i] * Gy[c];
    //		c++;
    //	}

    //	//pixel = 255 * (pixel - min) / (max - min);
    //	DerrivativeX_image->rgbpix[n * Img_src->Width + m] = pixel;
    //	DerrivativeY_image->rgbpix[n * Img_src->Width + m] = pixel2;
    //}
}

/*
    Find   M A G N I T U D E  of Gradient
*/
void FindMagnitudeOfGradient(struct Image *DerrivativeX_image, struct Image *DerrivativeY_image, struct Image *Magnitude)
{
    int r, c, pos, sq1, sq2;
    int rows = DerrivativeX_image->Height;
    int cols = DerrivativeX_image->Width;

    for (r = 0, pos = 0; r < rows; r++)
    {
        for (c = 0; c < cols; c++, pos++)
        {
            sq1 = DerrivativeX_image->rgbpix[pos] * DerrivativeX_image->rgbpix[pos];
            sq2 = DerrivativeY_image->rgbpix[pos] * DerrivativeY_image->rgbpix[pos];
            Magnitude->rgbpix[pos] = (int)(0.5 + sqrt((float)sq1 + (float)sq2));
        }
    }
}

/*
    Find  N O N - M A X - S U P P
*/
void FindNonMaximumSupp(struct Image *Magnitude, struct Image *DerrivativeX, struct Image *DerrivativeY, struct Image *NMS)
{
    int rowcount, colcount, count;
    unsigned char *magrowptr, *magptr;
    unsigned char *gxrowptr, *gxptr;
    unsigned char *gyrowptr, *gyptr, z1, z2;
    unsigned char m00, gx = 0, gy = 0;
    float mag1 = 0, mag2 = 0, xperp = 0, yperp = 0;
    unsigned char *resultrowptr, *resultptr;
    int nrows = DerrivativeX->Height;
    int ncols = DerrivativeX->Width;

    unsigned char *result = NMS->rgbpix;
    unsigned char *mag = Magnitude->rgbpix;
    unsigned char *gradx = DerrivativeX->rgbpix;
    unsigned char *grady = DerrivativeY->rgbpix;


    /****************************************************************************
    * Zero the edges of the result image.
    ****************************************************************************/
    for (count = 0, resultrowptr = NMS->rgbpix, resultptr = NMS->rgbpix + ncols*(nrows - 1);
        count<ncols; resultptr++, resultrowptr++, count++){
        *resultrowptr = *resultptr = (unsigned char)0;
    }

    for (count = 0, resultptr = NMS->rgbpix, resultrowptr = NMS->rgbpix + ncols - 1;
        count<nrows; count++, resultptr += ncols, resultrowptr += ncols){
        *resultptr = *resultrowptr = (unsigned char)0;
    }

    /****************************************************************************
    * Suppress non-maximum points.
    ****************************************************************************/
    for (rowcount = 1, magrowptr = mag + ncols + 1, gxrowptr = gradx + ncols + 1,
        gyrowptr = grady + ncols + 1, resultrowptr = result + ncols + 1;
        rowcount<nrows - 2; rowcount++, magrowptr += ncols, gyrowptr += ncols, gxrowptr += ncols,
        resultrowptr += ncols)
    {
        for (colcount = 1, magptr = magrowptr, gxptr = gxrowptr, gyptr = gyrowptr,
            resultptr = resultrowptr; colcount<ncols - 2;
            colcount++, magptr++, gxptr++, gyptr++, resultptr++)
        {
            m00 = *magptr;
            if (m00 == 0){
                *resultptr = (unsigned char)NOEDGE;
            }
            else{
                xperp = -(gx = *gxptr) / ((float)m00);
                yperp = (gy = *gyptr) / ((float)m00);
            }

            if (gx >= 0){
                if (gy >= 0){
                    if (gx >= gy)
                    {
                        /* 111 */
                        /* Left point */
                        z1 = *(magptr - 1);
                        z2 = *(magptr - ncols - 1);

                        mag1 = (m00 - z1)*xperp + (z2 - z1)*yperp;

                        /* Right point */
                        z1 = *(magptr + 1);
                        z2 = *(magptr + ncols + 1);

                        mag2 = (m00 - z1)*xperp + (z2 - z1)*yperp;
                    }
                    else
                    {
                        /* 110 */
                        /* Left point */
                        z1 = *(magptr - ncols);
                        z2 = *(magptr - ncols - 1);

                        mag1 = (z1 - z2)*xperp + (z1 - m00)*yperp;

                        /* Right point */
                        z1 = *(magptr + ncols);
                        z2 = *(magptr + ncols + 1);

                        mag2 = (z1 - z2)*xperp + (z1 - m00)*yperp;
                    }
                }
                else
                {
                    if (gx >= -gy)
                    {
                        /* 101 */
                        /* Left point */
                        z1 = *(magptr - 1);
                        z2 = *(magptr + ncols - 1);

                        mag1 = (m00 - z1)*xperp + (z1 - z2)*yperp;

                        /* Right point */
                        z1 = *(magptr + 1);
                        z2 = *(magptr - ncols + 1);

                        mag2 = (m00 - z1)*xperp + (z1 - z2)*yperp;
                    }
                    else
                    {
                        /* 100 */
                        /* Left point */
                        z1 = *(magptr + ncols);
                        z2 = *(magptr + ncols - 1);

                        mag1 = (z1 - z2)*xperp + (m00 - z1)*yperp;

                        /* Right point */
                        z1 = *(magptr - ncols);
                        z2 = *(magptr - ncols + 1);

                        mag2 = (z1 - z2)*xperp + (m00 - z1)*yperp;
                    }
                }
            }
            else
            {
                if ((gy = *gyptr) >= 0)
                {
                    if (-gx >= gy)
                    {
                        /* 011 */
                        /* Left point */
                        z1 = *(magptr + 1);
                        z2 = *(magptr - ncols + 1);

                        mag1 = (z1 - m00)*xperp + (z2 - z1)*yperp;

                        /* Right point */
                        z1 = *(magptr - 1);
                        z2 = *(magptr + ncols - 1);

                        mag2 = (z1 - m00)*xperp + (z2 - z1)*yperp;
                    }
                    else
                    {
                        /* 010 */
                        /* Left point */
                        z1 = *(magptr - ncols);
                        z2 = *(magptr - ncols + 1);

                        mag1 = (z2 - z1)*xperp + (z1 - m00)*yperp;

                        /* Right point */
                        z1 = *(magptr + ncols);
                        z2 = *(magptr + ncols - 1);

                        mag2 = (z2 - z1)*xperp + (z1 - m00)*yperp;
                    }
                }
                else
                {
                    if (-gx > -gy)
                    {
                        /* 001 */
                        /* Left point */
                        z1 = *(magptr + 1);
                        z2 = *(magptr + ncols + 1);

                        mag1 = (z1 - m00)*xperp + (z1 - z2)*yperp;

                        /* Right point */
                        z1 = *(magptr - 1);
                        z2 = *(magptr - ncols - 1);

                        mag2 = (z1 - m00)*xperp + (z1 - z2)*yperp;
                    }
                    else
                    {
                        /* 000 */
                        /* Left point */
                        z1 = *(magptr + ncols);
                        z2 = *(magptr + ncols + 1);

                        mag1 = (z2 - z1)*xperp + (m00 - z1)*yperp;

                        /* Right point */
                        z1 = *(magptr - ncols);
                        z2 = *(magptr - ncols - 1);

                        mag2 = (z2 - z1)*xperp + (m00 - z1)*yperp;
                    }
                }
            }

            /* Now determine if the current point is a maximum point */

            if ((mag1 > 0.0) || (mag2 > 0.0))
            {
                *resultptr = (unsigned char)NOEDGE;
            }
            else
            {
                if (mag2 == 0.0)
                    *resultptr = (unsigned char)NOEDGE;
                else
                    *resultptr = (unsigned char)POSSIBLE_EDGE;
            }
        }
    }
}

/*
    Find     H Y S T E R E S I S
*/
void FindHysteresis(struct Image *Magnitude, struct Image *NMS, struct Image *Img_dst, float Algo_param1, float Algo_param2)
{
    int r, c, pos, edges, highcount, lowthreshold, highthreshold,
        i, hist[32768], rr, cc;
    unsigned char maximum_mag, sumpix;

    int rows = Img_dst->Height;
    int cols = Img_dst->Width;

    /****************************************************************************
    * Initialize the Img_dst->rgbpix map to possible Img_dst->rgbpixs everywhere the non-maximal
    * suppression suggested there could be an Img_dst->rgbpix except for the border. At
    * the border we say there can not be an Img_dst->rgbpix because it makes the
    * follow_Img_dst->rgbpixs algorithm more efficient to not worry about tracking an
    * Img_dst->rgbpix off the side of the image.
    ****************************************************************************/
    for (r = 0, pos = 0; r<rows; r++){
        for (c = 0; c<cols; c++, pos++){
            if (NMS->rgbpix[pos] == POSSIBLE_EDGE) Img_dst->rgbpix[pos] = POSSIBLE_EDGE;
            else Img_dst->rgbpix[pos] = NOEDGE;
        }
    }

    for (r = 0, pos = 0; r<rows; r++, pos += cols){
        Img_dst->rgbpix[pos] = NOEDGE;
        Img_dst->rgbpix[pos + cols - 1] = NOEDGE;
    }
    pos = (rows - 1) * cols;
    for (c = 0; c<cols; c++, pos++){
        Img_dst->rgbpix[c] = NOEDGE;
        Img_dst->rgbpix[pos] = NOEDGE;
    }

    /****************************************************************************
    * Compute the histogram of the magnitude image. Then use the histogram to
    * compute hysteresis thresholds.
    ****************************************************************************/
    for (r = 0; r<32768; r++) hist[r] = 0;
    for (r = 0, pos = 0; r<rows; r++){
        for (c = 0; c<cols; c++, pos++){
            if (Img_dst->rgbpix[pos] == POSSIBLE_EDGE) hist[Magnitude->rgbpix[pos]]++;
        }
    }

    /****************************************************************************
    * Compute the number of pixels that passed the nonmaximal suppression.
    ****************************************************************************/
    for (r = 1, edges = 0; r<32768; r++){
        if (hist[r] != 0) maximum_mag = r;
        edges += hist[r];
    }

    highcount = (int)(edges * Algo_param2 + 0.5);

    /****************************************************************************
    * Compute the high threshold value as the (100 * Algo_param2) percentage point
    * in the magnitude of the gradient histogram of all the pixels that passes
    * non-maximal suppression. Then calculate the low threshold as a fraction
    * of the computed high threshold value. John Canny said in his paper
    * "A Computational Approach to Img_dst->rgbpix Detection" that "The ratio of the
    * high to low threshold in the implementation is in the range two or three
    * to one." That means that in terms of this implementation, we should
    * choose Algo_param1 ~= 0.5 or 0.33333.
    ****************************************************************************/
    r = 1;
    edges = hist[1];
    while ((r<(maximum_mag - 1)) && (edges < highcount)){
        r++;
        edges += hist[r];
    }
    highthreshold = r;
    lowthreshold = (int)(highthreshold * Algo_param1 + 0.5);


    /****************************************************************************
    * This loop looks for pixels above the highthreshold to locate Img_dst->rgbpixs and
    * then calls follow_Img_dst->rgbpixs to continue the Img_dst->rgbpix.
    ****************************************************************************/
    for (r = 0, pos = 0; r<rows; r++){
        for (c = 0; c<cols; c++, pos++){
            if ((Img_dst->rgbpix[pos] == POSSIBLE_EDGE) && (Magnitude->rgbpix[pos] >= highthreshold)){
                Img_dst->rgbpix[pos] = EDGE;
                Follow_edges((Img_dst->rgbpix + pos), (Magnitude->rgbpix + pos), lowthreshold, cols);
            }
        }
    }

    /****************************************************************************
    * Set all the remaining possible Img_dst->rgbpixs to non-Img_dst->rgbpixs.
    ****************************************************************************/
    for (r = 0, pos = 0; r<rows; r++)
    {
        for (c = 0; c<cols; c++, pos++) if (Img_dst->rgbpix[pos] != EDGE) Img_dst->rgbpix[pos] = NOEDGE;
    }
}

/*
    Follow  E D G E S
*/
void Follow_edges(unsigned char *edgemapptr, unsigned char *edgemagptr, unsigned char lowval, int cols)
{
    unsigned char *tempmagptr;
    unsigned char *tempmapptr;
    int i;
    float thethresh;
    int x[8] = { 1, 1, 0, -1, -1, -1, 0, 1 },
        y[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };

    for (i = 0; i<8; i++){
        tempmapptr = edgemapptr - y[i] * cols + x[i];
        tempmagptr = edgemagptr - y[i] * cols + x[i];

        if ((*tempmapptr == POSSIBLE_EDGE) && (*tempmagptr > lowval)){
            *tempmapptr = (unsigned char)EDGE;
            Follow_edges(tempmapptr, tempmagptr, lowval, cols);
        }
    }
}

/*
    C O N V O L U T I O N
*/
void Convolution(unsigned char *InputArray, unsigned char *OutputArray, int rows, int cols, float *Kernel, int KernelSize)
{
    int i, j, n, m;
    int FinalNum;
    int DevideNumber = pow((double)KernelSize, 2) - 1;

    for (i = KernelSize / 2; i < cols - KernelSize / 2; i++)
    {
        for (j = KernelSize / 2; j < rows - KernelSize / 2; j++)
        {
            //OutputArray[i * cols + j] = OutputArray[i * cols + j];
            size_t c = 0;
            FinalNum = 0;
            for (int n = -KernelSize / 2; n <= KernelSize / 2; n++)
            for (int m = -KernelSize / 2; m <= KernelSize / 2; m++)
            {
                FinalNum += InputArray[(j - n) * cols + i - m] * Kernel[c];
                c++;
            }
            FinalNum = (double)FinalNum / DevideNumber;
            if (FinalNum < 0) FinalNum = 0;
            else if (FinalNum > 255) FinalNum = 255;

            OutputArray[j * cols + i] = FinalNum;
        }
    }
}

/*
    M I R R O R  image - horizontal
*/
struct Image MirrorImageHorizontal(struct Image *Img_src, struct Image *Img_dst)
{
    int i, j, l, k;

    for (i = Img_dst->Height-1; i >= 0; i--)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            for (l = 0; l < Img_dst->Num_channels; l++)
            {
                Img_dst->rgbpix[((Img_dst->Height - 1 - i) * Img_src->Num_channels * Img_src->Width) + (Img_src->Num_channels * j) + l] = Img_src->rgbpix[(i * Img_src->Num_channels * Img_src->Width) + (Img_src->Num_channels * j) + l];
            }
        }
    }

    return *Img_dst;
}

/*
    M I R R O R  image - vertical
*/
struct Image MirrorImageVertical(struct Image *Img_src, struct Image *Img_dst)
{
    int i, j, l;

    if (Img_src->Num_channels != Img_dst->Num_channels)
        return *Img_dst;

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            for (l = 0; l < Img_dst->Num_channels; l++)
            {
                Img_dst->rgbpix[(i * Img_src->Num_channels * Img_src->Width) + (Img_src->Num_channels * (Img_dst->Width - 1 - j)) + l] = Img_src->rgbpix[(i * Img_src->Num_channels * Img_src->Width) + (Img_src->Num_channels * j) + l];
            }
        }
    }

    return *Img_dst;
}

/*
    C R O P  image - around given point and given new dimensions
*/
struct Image CropImage(struct Image *Img_src, struct Image *Img_dst, struct point_xy CentralPoint, int NewWidth, int NewHeight)
{
    int i, j, l, k, z;

    if (Img_src->Num_channels != Img_dst->Num_channels)
        return *Img_dst;

    if (NewWidth >= Img_src->Width || NewHeight >= Img_src->Height)
        return *Img_dst;

    /* Modify Img_dst */
    Img_dst->Width = NewWidth;
    Img_dst->Height = NewHeight;
    Img_dst->rgbpix = (unsigned char *)realloc(Img_dst->rgbpix, Img_dst->Width * Img_dst->Height * Img_dst->Num_channels * sizeof(unsigned char));

    /* Modify Central point - shift x and y*/
    if (CentralPoint.X > Img_src->Width - 1) CentralPoint.X = Img_src->Width - 1;
    if (CentralPoint.Y > Img_src->Height - 1) CentralPoint.X = Img_src->Height - 1;
    if (CentralPoint.X < 0) CentralPoint.X = 0;
    if (CentralPoint.Y < 0) CentralPoint.X = 0;

    /* for x - too right*/
    if (Img_src->Width < (NewWidth / 2) + CentralPoint.X) CentralPoint.X = Img_src->Width - (NewWidth / 2) - 1;
    /* for y - too down */
    if (Img_src->Height < (NewHeight / 2) + CentralPoint.Y) CentralPoint.Y = Img_src->Height - (NewHeight / 2) - 1;
    /* for x - too left*/
    if (CentralPoint.X - (NewWidth / 2 ) < 0) CentralPoint.X = (NewWidth / 2) + 1;
    /* for y - too up*/
    if (CentralPoint.Y - (NewHeight / 2) < 0) CentralPoint.Y = (NewHeight / 2) + 1;

    k = CentralPoint.Y - (NewHeight / 2);
    for (i = 0; i < Img_dst->Height; i++)
    {
        k++;
        z = CentralPoint.X - (NewWidth / 2);
        for (j = 0; j < Img_dst->Width; j++)
        {
            z++;
            for (l = 0; l < Img_dst->Num_channels; l++)
            {
                Img_dst->rgbpix[(i * Img_dst->Num_channels * Img_dst->Width) + (Img_dst->Num_channels * j) + l] = Img_src->rgbpix[(k * Img_src->Num_channels * Img_src->Width) + (Img_src->Num_channels * z) + l];
            }
        }
    }

    return *Img_dst;
}

/*
    M O R P H O L O G Y  -  Dilation
*/
struct Image MorphDilate(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberOfIterations)
{
    int i, j, l, k, z;

    /* Only Grayscale is currently supported */
    if ((Img_src->Num_channels != Img_dst->Num_channels) && Img_dst->Num_channels != 1)
        return *Img_dst;
    if (ElementSize < 3) ElementSize = 3;
    if (NumberOfIterations < 0) NumberOfIterations = 0;
    if ((Img_src->Width != Img_dst->Width) || (Img_src->Height != Img_dst->Height)) SetDestination(Img_src, Img_dst);

    float StructureElement[9] = { 0, 1, 0, 1, 1, 1, 0, 1, 0 };
    Convolution(Img_src->rgbpix, Img_dst->rgbpix, Img_src->Height, Img_src->Width, StructureElement, ElementSize);

    if (NumberOfIterations % 2 == 0)
    {
        if (NumberOfIterations > 0 && NumberOfIterations % 2 == 0)
        {
            NumberOfIterations -= 1;
            MorphDilate(Img_src, Img_dst, ElementSize, NumberOfIterations);
            return *Img_dst;
        }
        if (NumberOfIterations > 0 && NumberOfIterations % 2 == 1)
        {
            NumberOfIterations -= 1;
            MorphDilate(Img_dst, Img_src, ElementSize, NumberOfIterations);
            return *Img_src;
        }
    }
    else
    {
        if (NumberOfIterations > 0 && NumberOfIterations % 2 == 0)
        {
            NumberOfIterations -= 1;
            MorphDilate(Img_dst, Img_src, ElementSize, NumberOfIterations);
            return *Img_src;
        }
        if (NumberOfIterations > 0 && NumberOfIterations % 2 == 1)
        {
            NumberOfIterations -= 1;
            MorphDilate(Img_src, Img_dst, ElementSize, NumberOfIterations);
            return *Img_dst;
        }
    }
}


/*
    M O R P H O L O G Y  -  Erosion
*/
struct Image MorphErode(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberOfIterations)
{
    int i, j, l, k, z;

    if (Img_src->Num_channels != Img_dst->Num_channels)
        return *Img_dst;
    if (ElementSize < 3) ElementSize = 3;
    if (NumberOfIterations < 0) NumberOfIterations = 0;
    if ((Img_src->Width != Img_dst->Width) || (Img_src->Height != Img_dst->Height)) SetDestination(Img_src, Img_dst);

    float StructureElement[9] = { 1, 0, 1, 0, 0, 0, 1, 0, 1 };
    Convolution(Img_src->rgbpix, Img_dst->rgbpix, Img_src->Height, Img_src->Width, StructureElement, ElementSize);

    if (NumberOfIterations % 2 == 0)
    {
        if (NumberOfIterations > 0 && NumberOfIterations % 2 == 0)
        {
            NumberOfIterations -= 1;
            MorphErode(Img_src, Img_dst, ElementSize, NumberOfIterations);
            return *Img_dst;
        }
        if (NumberOfIterations > 0 && NumberOfIterations % 2 == 1)
        {
            NumberOfIterations -= 1;
            MorphErode(Img_dst, Img_src, ElementSize, NumberOfIterations);
            return *Img_src;
        }
    }
    else
    {
        if (NumberOfIterations > 0 && NumberOfIterations % 2 == 0)
        {
            NumberOfIterations -= 1;
            MorphErode(Img_dst, Img_src, ElementSize, NumberOfIterations);
            return *Img_src;
        }
        if (NumberOfIterations > 0 && NumberOfIterations % 2 == 1)
        {
            NumberOfIterations -= 1;
            MorphErode(Img_src, Img_dst, ElementSize, NumberOfIterations);
            return *Img_dst;
        }
    }
}


/*
    M O R P H O L O G Y  -  Opening
*/
struct Image MorphOpen(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberOfIterations)
{
    int i, j, l, k, z;


}
/*
    M O R P H O L O G Y  -  Closing
*/
struct Image MorphClose(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberOfIterations)
{
    int i, j, l, k, z;


}

/*
    S H A R P   image - using EdgeExtraction function
*/
struct Image SharpImageContours(struct Image *Img_src, struct Image *Img_dst, int Percentage)
{
    int i, j, l;

    Image Img_dst_Grayscale = CreateNewImage(Img_src, &Img_dst_Grayscale, 1);
    Image Img_src_Grayscale = CreateNewImage(Img_src, &Img_src_Grayscale, 1);

    if (abs(Percentage) > 1) Percentage /= 100;
    if ((Img_src->Width != Img_dst->Width) || (Img_src->Height != Img_dst->Height)) SetDestination(Img_src, Img_dst);

    ConvertToGrayscale_1Channel(Img_src, &Img_src_Grayscale);

    EdgeExtraction(&Img_src_Grayscale, &Img_dst_Grayscale, EDGES_PREWITT, 1, 0.9);

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            for (l = 0; l < Img_dst->Num_channels; l++)
            {
                if (Img_dst_Grayscale.rgbpix[(i*Img_src->Width + j)] >= POSSIBLE_EDGE)
                {
                    if (Percentage * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] > 255)
                        Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 255;
                    else if (Percentage * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] < 0)
                        Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 0;
                    else
                        Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = Percentage * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
                }
                else
                    Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
            }
        }
    }

    DestroyImage(&Img_dst_Grayscale);
    DestroyImage(&Img_src_Grayscale);
    return *Img_dst;
}

/*
    S H A R P   image - using Binary mask
*/
struct Image SharpImageBinary(struct Image *Img_src, struct Image *Img_dst, struct Image *Img_Binary, int Percentage)
{
    int i, j, l;

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            for (l = 0; l < Img_dst->Num_channels; l++)
            {
                if (Img_Binary->rgbpix[(i*Img_src->Width + j)] == 1)
                {
                    if (Percentage * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] > 255)
                        Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 255;
                    else if (Percentage * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] < 0)
                        Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 0;
                    else Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = Percentage * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
                }
                else
                    Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
            }
        }
    }

    return *Img_dst;
}
