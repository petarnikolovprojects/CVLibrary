using System;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Drawing.Imaging;
using System.Diagnostics;
using System.Collections.Generic;

using Emgu.CV;
using Emgu.CV.Structure;
using Emgu.CV.CvEnum;
using Emgu.CV.Util;

namespace WpfApplication2.Algorithms
{
    public static class Histogram
    {

        /*G E T  image values R A N G E*/
        public static int GetRange(this Image<Gray, Byte> imgSrc, out int minVal)
        {
            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgData = imgSrc.Data;

            minVal = 256;
            int maxVal = -1;
            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    if (imgData[i, j, 0] > maxVal)
                    {
                        maxVal = imgData[i, j, 0];
                    }
                    if (imgData[i, j, 0] < minVal)
                    {
                        minVal = imgData[i, j, 0];
                    }
                }
            }
            int range = maxVal - minVal;
            return range;
        }


        /* H I S T  -  T H R E S H*/
        public static Image<Gray, Byte> HistThreshold<TColor, TDepth>(this Image<TColor, TDepth> img, int bars, int offset)
            where TColor : struct, IColor
            where TDepth : new()
        {
            Image<Gray, Byte> imgThresh = null;

            using (var imgHsv = img.Convert<Hsv, Byte>())
            {
                // do histogram on Value channel with N bars
                DenseHistogram dhist = new DenseHistogram(bars, new RangeF(0, 255));
                dhist.Calculate(new IImage[] { imgHsv[2] }, true, null);

                int threshold = 0;
                bool isMaxFound = false;

                // find the first local maximum and return the hist bar after the maximum
                var histArr = dhist.MatND.ManagedArray;
                for (int i = 0; i < histArr.Length; i++)
                {
                    if ((i > 1) && (i < histArr.Length - 1))
                    {
                        int curr = (int)(float)histArr.GetValue(i);
                        int prev = (int)(float)histArr.GetValue(i - 1);
                        int next = (int)(float)histArr.GetValue(i + 1);

                        if ((curr > prev) && (curr > next))
                        {
                            isMaxFound = true;
                        }

                        if ((isMaxFound) && (curr < prev) && (curr < next))
                        {
                            int barWeight = 255 / bars;
                            threshold = (i + offset) * barWeight;

                            break;
                        }
                    }
                }

                imgThresh = imgHsv.InRange(new Hsv(0, 0, 0), new Hsv(180, 255, threshold));
            }

            return imgThresh;
        }


        /*G E T  -  H I S T O G R A M  -  and  Max Brightness*/
        public static int[,] GetHistogram(this Image<Bgr, Byte> img, out int maxBrightnessPos)
        {
            maxBrightnessPos = 0;
            var hist = new int[3, 256];
            int maxVal = 0;

            int height = img.Height;
            int width = img.Width;
            var data = img.Data;

            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    var b = data[y, x, 0];

                    var bCurr = ++hist[0, b];
                    if (bCurr > maxVal)
                    {
                        maxVal = bCurr;
                        maxBrightnessPos = b;
                    }

                    var g = data[y, x, 1];

                    var gCurr = ++hist[1, g];
                    if (gCurr > maxVal)
                    {
                        maxVal = gCurr;
                        maxBrightnessPos = g;
                    }

                    var r = data[y, x, 2];

                    var rCurr = ++hist[2, r];
                    if (rCurr > maxVal)
                    {
                        maxVal = rCurr;
                        maxBrightnessPos = r;
                    }
                }
            }

            return hist;
        }


        /*G E T  -  H I S T O G R A M*/
        public static int[] GetHistogram(this Image<Gray, Byte> img)
        {
            var hist = new int[256];

            int height = img.Height;
            int width = img.Width;
            var data = img.Data;

            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    var b = data[y, x, 0];

                    var bCurr = ++hist[b];
                }
            }

            return hist;
        }

        
        /*H I S T   to   I M A G E - GRAY*/
        public static Image<Gray, Byte> GetHistImageGray(this Image<Bgr,Byte> imgSrc)
        {
            int imgHistHeight = 400;
            int imgHistWidth = 256;
            int maxBrightPos = 0;
            var hist = imgSrc.GetHistogram(out maxBrightPos);

            var imgHist = new Image<Gray, Byte>(imgHistWidth, imgHistHeight);
            var histData = imgHist.Data;

            var heightParam = 1.0;
            if(maxBrightPos > 400)
            {
                heightParam = maxBrightPos / 400;
            }

            return imgHist;
        }

        private static int GetMaxNumber(int [,] hist)
        {
            int maxNumber = 0;

            for(int i = 0; i < 3; i++)
            {
                for(int j = 0; j < 256; j++)
                {
                    if(hist[i, j] > maxNumber)
                    {
                        maxNumber = hist[i, j];
                    }
                }
            }

            return maxNumber;
        }

        /*H I S T   to   I M A G E - BGR*/
        public static Image<Bgr, Byte> GetHistImageBgr(this Image<Bgr, Byte> imgSrc)
        {
            int imgHistHeight = 350;
            int imgHistWidth = 256;
            int maxBrightPos = 0;
            var hist = imgSrc.GetHistogram(out maxBrightPos);
            maxBrightPos = Histogram.GetMaxNumber(hist);

            var imgHist = new Image<Bgr, Byte>(imgHistWidth, imgHistHeight);
            var histData = imgHist.Data;

            var heightParam = maxBrightPos / 400.0;
            
            for (int j = 0; j < imgHistWidth; j++)
            {
                int limitY_Blue = (int)(hist[0, j] / heightParam);
                int limitY_Green = (int)(hist[1, j] / heightParam);
                int limitY_Red = (int)(hist[2, j] / heightParam);

                for (int i = imgHistHeight - 1; i >= 0; i--)
                {
                    for(int k = 0; k < 3; k++)
                    {
                        if(k == 0)
                        {
                            if (i > imgHistHeight - limitY_Blue)
                            {
                                histData[i, j, 0] = 255;
                            }
                            else
                            {
                                histData[i, j, 0] = 0;
                            }
                        }
                        if (k == 1)
                        {
                            if (i > imgHistHeight - limitY_Green)
                            {
                                histData[i, j, 1] = 255;
                            }
                            else
                            {
                                histData[i, j, 1] = 0;
                            }
                        }
                        if (k == 2)
                        {
                            if (i > imgHistHeight - limitY_Red)
                            {
                                histData[i, j, 2] = 255;
                            }
                            else
                            {
                                histData[i, j, 2] = 0;
                            }
                        }
                    }
                }
            }

            imgHist.Save("D:\\histogram.jpg");
            return imgHist;
        }

        /*F I N D  -  M I N  -  P E A K*/
        public static int FindMinPeakFromHistogram(int[,] histArr)
        {
            int minVal = 0;
            int numberOfPoistions = 13;

            int[] matchArray = new int[128];

            //scan only the first half of histogram
            for (int i = numberOfPoistions; i < 128; i++)
            {
                if (histArr[1, i] > histArr[1, i - 1] && histArr[1, i] > histArr[1, i + 1])
                {
                    for (int j = numberOfPoistions; j > 0; j--)
                    {
                        if (histArr[1, i] > histArr[1, i - j] && histArr[1, i] > histArr[1, i + j])
                        {
                            matchArray[i]++;
                        }
                    }
                }
            }

            int maxVal = 0;
            for (int i = 0; i < 128; i++)
            {
                if (matchArray[i] > maxVal)
                {
                    minVal = i;
                    maxVal = matchArray[i];
                }
            }

            return minVal;
        }


        /*F I N D  -  M A X  -  P E A K*/
        public static int FindMaxPeakFromHistogram(int[,] histArr, int maxGreenBrightness)
        {
            // search around maxGreenBrightness - get maxGreenBrightness /2
            int maxVal = histArr[1, maxGreenBrightness] / 2;

            int sideA = 0;
            int sideB = 255;

            for (int pos = 2; pos < maxGreenBrightness; pos++)
            {
                if ((histArr[1, pos] <= maxVal) && (histArr[1, pos + 1] >= maxVal))
                {
                    sideA = pos;
                    break;
                }
            }

            for (int pos = maxGreenBrightness; pos < 253; pos++)
            {
                if ((histArr[1, pos] >= maxVal) && (histArr[1, pos + 1] <= maxVal))
                {
                    sideB = pos;
                    break;
                }
            }

            int average = (sideB + sideA) / 2;

            return average;
        }


        ///*Convert   H I S TO G R A M   to   I M A G E*/
        //public static Image<Bgr, Byte> ConvertHistToImage(Image<Bgr, Byte> imgSrc)
        //{
        //	int i, j, k, z;
        //	int write = 1;
        //	float ScaleNumber = 0;
        //	long int MaxValue = hist->MaxValue;
        //	int Char_array[81];
        //	int NumberofCalcs = 0;
        //	int average;
        //
        //    Histogram hist;
        //
        //	/* Set size for the Hist Image */
        //	if (hist->Bins == 256)
        //	{
        //		hist->Size_x = 2 * hist->Bins + 30;
        //		if (MaxValue > 600) // we have to scale if it is too big
        //		{
        //			ScaleNumber = MaxValue / 600;
        //			hist->Size_y = (MaxValue / ScaleNumber + 40);
        //		}
        //		else {
        //			hist->Size_y = MaxValue + 40;
        //			ScaleNumber = ((MaxValue + 40) / 600.0);
        //		}
        //	}
        //
        //	/* Set the destination image (Img_src) to meet the requirements from the hist*/
        //	Img_src->Num_channels = hist->NumberOfLayers;
        //	if (hist->NumberOfLayers == 3) Img_src->ColorSpace = 2;
        //	else Img_src->ColorSpace = 1;
        //
        //	Img_src->rgbpix = (unsigned char *)realloc(Img_src->rgbpix, hist->NumberOfLayers * hist->Size_y * hist->Size_x * sizeof(unsigned char));
        //	Img_src->Width = hist->Size_x;
        //	Img_src->Height = hist->Size_y;
        //
        //	for (j = 0; j < hist->Size_x; j++)
        //	{
        //		for (i = hist->Size_y - 1;  i >= 0; i--)
        //		{
        //			for (k = 0; k < hist->NumberOfLayers; k++)
        //			{
        //				//if (Img_src->rgbpix[hist->NumberOfLayers * (i * hist->Size_x + j) + k] != 255) 
        //				Img_src->rgbpix[hist->NumberOfLayers * (i * hist->Size_x + j) + k] = 0;
        //
        //				if (i == 0 || j == 0 || j == hist->Size_x - 1 || i == hist->Size_y - 1)
        //				{
        //					Img_src->rgbpix[hist->NumberOfLayers * (i * hist->Size_x + j) + k] = 255;
        //					continue;
        //				}
        //
        //				if (j % 2 == 0)
        //				{
        //					if ((j <= 2 * hist->Bins + 20) && j > 20) // ako e chetno
        //					{
        //						if (i > (hist->Size_y - 20 - (hist->values[hist->NumberOfLayers * ((j - 20) / 2) + k] / ScaleNumber)) && (i < hist->Size_y - 20))
        //						{
        //							Img_src->rgbpix[hist->NumberOfLayers * (i * hist->Size_x + j) + k] = 255;
        //						}
        //					}
        //				}
        //				else
        //				{
        //					if ((j < 2 * hist->Bins + 20) && j > 20)
        //					{
        //						average = ((hist->values[hist->NumberOfLayers *((j - 19) / 2) + k] / ScaleNumber) + (hist->values[hist->NumberOfLayers * ((j - 21) / 2) + k] / ScaleNumber)) / 2;
        //						if (i >(hist->Size_y - 20 - (average)) && (i < hist->Size_y - 20))
        //						{
        //							Img_src->rgbpix[hist->NumberOfLayers * (i * hist->Size_x + j) + k] = 255;
        //						}
        //					}
        //					//Img_src->rgbpix[hist->NumberOfLayers * (i * hist->Size_x + j) + k] = 0;
        //				}
        //			}
        //		}
        //	}
        //	
        //	PrintNumberOnImage(Img_src, MaxValue, hist->Size_x / 2, 15);
        //	PrintNumberOnImage(Img_src, 0, 10, hist->Size_y - 15);
        //	PrintNumberOnImage(Img_src, 255, hist->Size_x - 20, hist->Size_y - 15);
        //}
    }
}
