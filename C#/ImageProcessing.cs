using System;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Drawing.Imaging;
using System.Diagnostics;
using System.Collections.Generic;
using System.Threading;
using System.Windows;

using Emgu.CV;
using Emgu.CV.Structure;
using Emgu.CV.CvEnum;
using Emgu.CV.Util;
using WpfApplication2.Models;
using WpfApplication2.Definitions;

namespace WpfApplication2.Algorithms
{
    public static class ImageProcessing
    {
        /*Get   F R A M E(s) from camera*/

        public static Image<Bgr, Byte> GetFrameFromCameraDefault()
        {
            var listFrames = GetFramesFromCamera(0, 1, 1000);

            return listFrames[0];
        }
        
        /*This function is designed to modify the preview algo image, but it will work on any image with any X,Y equal or bigger than original*/

        public static Image<Bgr, Byte> FixImagetoXY(this Image<Bgr,Byte> imgSrc, int extendedX, int extendedY, out Rectangle rect)
        {
            var imgDst = new Image<Bgr, Byte>(extendedX, extendedY);

            int origWidth = imgSrc.Width;
            int origHeight = imgSrc.Height;

            var xDiff = extendedX - origWidth;
            var yDiff = extendedY - origHeight;
            var imgSrcData = imgSrc.Data;
            var imgDstData = imgDst.Data;

            rect = new Rectangle(0, yDiff / 2, extendedX, origHeight);

            var rowCounter = 0;
            var colCounter = 0;
            for(int i = 0; i < extendedY; i++)
            {
                colCounter = 0;
                for(int j = 0; j < extendedX; j++)
                {
                    if (i < yDiff / 2 || i >= extendedY - (yDiff / 2) - 1 || j < xDiff / 2 || j >= extendedX - (xDiff / 2) - 1)
                    {
                        imgDstData[i, j, 0] = 255;
                        imgDstData[i, j, 1] = 255;
                        imgDstData[i, j, 2] = 255;

                        continue;
                    }

                    imgDstData[i, j, 0] = imgSrcData[rowCounter, colCounter, 0];
                    imgDstData[i, j, 1] = imgSrcData[rowCounter, colCounter, 1];
                    imgDstData[i, j, 2] = imgSrcData[rowCounter, colCounter, 2];

                    colCounter++;
                }
                if (!(i < yDiff / 2 || i >= extendedY - yDiff / 2))
                {
                    rowCounter++;
                }
            }

            return imgDst;
        }

        public static List<Image<Bgr, Byte>> GetFramesFromCamera(int deviceNumber, int numberOfFramesToCatch, int interval)
        {
            var listFrames = new List<Image<Bgr, Byte>>();

            var capture = new Capture(deviceNumber);
            Thread.Sleep(interval);

            for (int i = 0; i < numberOfFramesToCatch; i++)
            {
                var frame = capture.QueryFrame();
                listFrames.Add(frame.Copy());
                if(numberOfFramesToCatch > 1) Thread.Sleep(interval);
            }

            capture.Dispose();

            return listFrames;
        }


        /*E M B O S S*/
        public static Image<Bgr, byte> Emboss(this Image<Bgr, byte> imgSrc, double azimuth, double angle, int depth)
        {
            Image<Bgr, byte> imgDst;// = new Image<Bgr, byte>(imgSrc.Width, imgSrc.Height);

            imgDst = EmbossPrivate.EmbossCalc(imgSrc, azimuth, angle, depth);

            return imgDst;
        }

        
        /*U N S H A R P  -  M A S K*/
        public static Image<TColor, Byte> UnsharpenMask<TColor, TDepth>(this Image<TColor, TDepth> img, double radius, double amount, double threshold)
            where TColor : struct, IColor
            where TDepth : new()
        {
            // sharpen image using "unsharp mask" algorithm

            Image<TColor, Single> imgSingle = null;
            Image<TColor, Single> imgGauss = null;
            Image<TColor, Single> imgGaussSingle = null;
            Image<TColor, Single> imgDiff = null;
            Image<TColor, Single> imgThresh = null;
            Image<TColor, Single> imgSharpen = null;
            Image<TColor, Byte> imgClamp = null;

            try
            {
                imgSingle = img.Convert<TColor, Single>();

                imgGauss = imgSingle.SmoothGaussian(0, 0, radius, radius);

                imgGaussSingle = imgGauss.Convert<TColor, Single>();

                imgGauss.Dispose();
                imgGauss = null;

                imgDiff = imgSingle - imgGaussSingle;

                imgGaussSingle.Dispose();
                imgGaussSingle = null;

                imgThresh = imgDiff.Convert<Single>(delegate(Single d)
                { return Math.Abs(2 * d) < threshold ? 0 : (Single)(d * amount); });

                imgDiff.Dispose();
                imgDiff = null;

                imgSharpen = imgSingle + imgThresh;

                imgThresh.Dispose();
                imgThresh = null;

                imgSingle.Dispose();
                imgSingle = null;

                // do not release the last one as it will be returned to caller
                imgClamp = imgSharpen.Convert<Byte>(delegate(Single d)
                { if (d < 0) d = 0; if (d > 255) d = 255; return (byte)d; });

                imgSharpen.Dispose();
                imgSharpen = null;
            }
            finally
            {
                if (imgSingle != null)
                {
                    imgSingle.Dispose();
                }
                if (imgGauss != null)
                {
                    imgGauss.Dispose();
                }
                if (imgGaussSingle != null)
                {
                    imgGaussSingle.Dispose();
                }
                if (imgDiff != null)
                {
                    imgDiff.Dispose();
                }
                if (imgThresh != null)
                {
                    imgThresh.Dispose();
                }
                if (imgSharpen != null)
                {
                    imgSharpen.Dispose();
                }
            }

            return imgClamp;
        }


        /*G A U S S I A N  -  B L U R */
        public static int _GaussianBlur(this Image<Bgr, Byte> img, int blurPixelRadius, float neighborCoefficient)
        {
            double averSum = 0;

            if (blurPixelRadius < 5)
            {
                blurPixelRadius = 5;
            }

            if (neighborCoefficient > 1)
            {
                neighborCoefficient /= 100;
            }

            if (neighborCoefficient < 0)
            {
                neighborCoefficient *= -1;
            }

            int height = img.Height;
            int width = img.Width;
            int channels = img.NumberOfChannels;
            var data = img.Data;

            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    for (int k = 0; k < channels; k++)
                    {
                        int acc = 0;
                        int blurEdgeCount = 0;

                        blurPixelRadius = blurPixelRadius / 2;

                        for (int z = -blurPixelRadius; z <= blurPixelRadius; z++)
                        {
                            for (int t = -blurPixelRadius; t <= blurPixelRadius; t++)
                            {
                                if ((z == 0) && (t == 0))
                                {
                                    continue;
                                }

                                if ((y - z >= 0) && (x - t >= 0) &&
                                    (y - z < height) && (x - t < width))
                                {
                                    acc += data[y - z, x - t, k];
                                }
                                else
                                {
                                    blurEdgeCount++;
                                }
                            }
                        }

                        float acc2 = ((float)(neighborCoefficient) / (float)((blurPixelRadius * blurPixelRadius - blurEdgeCount + 1)));
                        acc2 *= acc;

                        float acc3 = (1 - neighborCoefficient) * data[y, x, k];
                        acc3 += (int)acc2;

                        if (acc3 > 255)
                        {
                            acc3 = 255;
                        }
                        if (acc3 < 0)
                        {
                            acc3 = 0;
                        }

                        data[y, x, k] = (byte)acc3;

                        if (k == 2)
                        {
                            averSum += (byte)acc3;
                        }
                    }
                }
            }

            return (int)(averSum / (width * height));
        }


        /*G A U S S I A N  -  B L U R (inplace)*/
        public static int _GaussianBlur(this Image<Gray, Byte> img, int blurPixelRadius, float neighborCoefficient)
        {
            double averSum = 0;

            if (blurPixelRadius < 5)
            {
                blurPixelRadius = 5;
            }

            if (neighborCoefficient > 1)
            {
                neighborCoefficient /= 100.0F;
            }

            if (neighborCoefficient < 0)
            {
                neighborCoefficient *= -1;
            }

            int height = img.Height;
            int width = img.Width;
            int channels = img.NumberOfChannels;
            var data = img.Data;

            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    for (int k = 0; k < channels; k++)
                    {
                        int acc = 0;
                        int blurEdgeCount = 0;

                        blurPixelRadius = blurPixelRadius / 2;
                        for (int z = -blurPixelRadius; z <= blurPixelRadius; z++)
                        {
                            for (int t = -blurPixelRadius; t <= blurPixelRadius; t++)
                            {
                                if ((z == 0) && (t == 0))
                                {
                                    continue;
                                }

                                if ((y - z >= 0) && (x - t >= 0) &&
                                    (y - z < height) && (x - t < width))
                                {
                                    acc += data[y - z, x - t, k];
                                }
                                else
                                {
                                    blurEdgeCount++;
                                }
                            }
                        }

                        float acc2 = ((float)(neighborCoefficient) / (float)((blurPixelRadius * blurPixelRadius - blurEdgeCount + 1)));
                        acc2 *= acc;

                        float acc3 = (1 - neighborCoefficient) * data[y, x, k];
                        acc3 += (int)acc2;

                        if (acc3 > 255)
                        {
                            acc3 = 255;
                        }
                        if (acc3 < 0)
                        {
                            acc3 = 0;
                        }

                        data[y, x, k] = (byte)acc3;

                        if (k == 2)
                        {
                            averSum += (byte)acc3;
                        }
                    }
                }
            }

            return (int)(averSum / (width * height));
        }


        /*B L U R     I M A G E  -  A R O U N D    P O I N T*/
        public static Image<Bgr, Byte> BlurImageAroundPoint(this Image<Bgr, Byte> imgSrc, Structures.point_xy CentralPoint, int BlurPixelRadius, int SizeOfBlur, int BlurOrSharp, int BlurAgression)
        {
        	double MaxRatio = 0;
        	double Distance = 0;
        	double DistanceRatio = 0;

            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;

            var imgMatrix = new Image<Gray, float>(width, height);
            var matrixData = imgMatrix.Data;

        	double Chislo = 0, Chislo2 = 0;
        	int Sybiraemo = 0;
        	int i, j, z, t, l;
        
        	/* Only odd nubers are allowed (bigger than 5*/
        	if (BlurPixelRadius % 2 == 0) BlurPixelRadius += 1;
        	if (BlurPixelRadius < 5) BlurPixelRadius = 5;

            var imgDst = new Image<Bgr, Byte>(width, height);
            var imgDstData = imgDst.Data;

            double distRatio = SizeOfBlur*width/100.0;
            MaxRatio = Computation.Max(CentralPoint.X - (distRatio), width - CentralPoint.X + (distRatio) / (distRatio));
        	for (i = 0; i < height; i++)
        	{
        		for (j = 0; j < width; j++)
        		{
        			//luma = ui->imageData[row * width + col];
        			Distance = Math.Sqrt(Math.Pow(Computation.Abs((float)CentralPoint.X - j), 2) + Math.Pow(Computation.Abs((float)CentralPoint.Y - i), 2));
        			if (Distance < ((float)SizeOfBlur * width / 100))
        			{
        				matrixData[i, j, 0] = 1;
        			}
        			else
        			{
        				DistanceRatio = Distance / (distRatio);
        				matrixData[i, j, 0] = (float)(1 - (BlurAgression / 100.0 * (DistanceRatio / MaxRatio)));
        				if (matrixData[i, j, 0] < 0) matrixData[i, j, 0] = 0;
        			}
        		}
        	}
        
        	for (i = 0; i < height; i++)
        	{
        		for (j = 0; j < width; j++)
        		{
        			if (i < BlurPixelRadius / 2 || j < BlurPixelRadius / 2 || j >= width - BlurPixelRadius / 2 || i >= height - BlurPixelRadius / 2)
        			{
        				for (l = 0; l < 3; l++)
        				{
        					imgDstData[i, j, l] = imgSrcData[i, j, l];
        				}
        				continue;
        			}
        			for (l = 0; l < 3; l++)
        			{
        				Sybiraemo = 0;
        				if (BlurOrSharp == 0)
        					Chislo2 = ((matrixData[i, j, 0]) / (Math.Pow(BlurPixelRadius, 2) - 1 - (12 + (2 * (BlurPixelRadius - 5)))));
        				else
        					Chislo2 = ((1 - matrixData[i, j, 0]) / (Math.Pow(BlurPixelRadius, 2) - 1 - (12 + (2 * (BlurPixelRadius - 5)))));
        				for (z = 0; z < BlurPixelRadius / 2; z++)
        				{
        					for (t = 0; t < BlurPixelRadius / 2; t++)
        					{
        						if (z == 0 && t == 0) continue;
        						Sybiraemo += imgSrcData[i - z, j - t, l];
        						Sybiraemo += imgSrcData[i - z, j + t, l];
        						Sybiraemo += imgSrcData[i + z, j - t, l];
        						Sybiraemo += imgSrcData[i + z, j + t, l];
        					}
        				}
        
        				Chislo2 *= Sybiraemo;

        				if (BlurOrSharp == 0)
        					Chislo = (1 - matrixData[i, j, 0]) * imgSrcData[i, j, l] + (int)Chislo2;
        				else
        					Chislo = matrixData[i, j, 0] * imgSrcData[i, j, l] + (int)Chislo2;
        				if (Chislo > 255)
        					Chislo = 255;
        				if (Chislo < 0)
        					Chislo = 0;
        				imgDstData[i, j, l] = (byte)Chislo;
        			}
        		}
        	}
        
        	return imgDst;
        }


        /*S U B S T R A C T  -  from  - C H A N N E L (inplace)*/
        public static void _SubstractFromChannel(this Image<Bgr, Byte> imgSrc, int value, int channel)
        {
            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;

            if (channel < 0 || channel > 2)
            {
                return;
            }

            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    if (imgSrcData[i, j, channel] - value < 0)
                    {
                        imgSrcData[i, j, channel] = 0;
                    }
                    else
                    {
                        imgSrcData[i, j, channel] = (byte)(imgSrcData[i, j, channel] - value);
                    }
                }
            }
        }


        /*S U B S T R A C T  -  from  - C H A N N E L*/
        public static Image<Bgr, Byte> SubstractFromChannel(this Image<Bgr, Byte> imgSrc, int value, int channel)
        {
            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;

            if (channel < 0 || channel > 2)
            {
                return imgSrc;
            }

            var imgDst = new Image<Bgr, Byte>(width, height);
            var imgDstData = imgDst.Data;

            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    if (imgSrcData[i, j, channel] - value < 0)
                    {
                        imgDstData[i, j, channel] = 0;
                    }
                    else
                    {
                        imgDstData[i, j, channel] = (byte)(imgSrcData[i, j, channel] - value);
                    }
                }
            }

            return imgDst;
        }


        /*S U B S T R A C T  -  from  - G R A Y image (inplace)*/
        public static void _SubstractFrom(this Image<Gray, Byte> imgSrc, int subsVal)
        {
            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgData = imgSrc.Data;

            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    int acc = subsVal - imgData[i, j, 0];
                    if (acc < 0)
                    {
                        acc = 0;
                    }
                    else if (acc > 255)
                    {
                        acc = 255;
                    }
                    imgData[i, j, 0] = (byte)acc;
                }
            }
        }


        /*C O M I C S    E F F E C T */
        public static Image<Bgr, Byte> ComicsEffect(this Image<Bgr, Byte> imgSrc, int maxSizeToSearch, int contourExtractAlgo, int thresholdEdge, int showContours)
        {
            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;
            var imgDst = imgSrc.CopyBlank();
            var imgDstData = imgDst.Data;

            var imgContours = imgSrc.EdgeExtraction(contourExtractAlgo);

            var imgContoursData = imgContours.Data;
            
            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    double averageR = 0;
                    double averageG = 0;
                    double averageB = 0;

                    int countR = 1;
                    int countG = 1;
                    int countB = 1;

                    int dir1 = 0; //upper left
                    int dir2 = 0; //up
                    int dir3 = 0; //upper right
                    int dir4 = 0; //left
                    int dir5 = 0; //right
                    int dir6 = 0; //down-left
                    int dir7 = 0; //down
                    int dir8 = 0; //dorn-right

                    for (int k = 0; k < maxSizeToSearch; k++)
                    {
                        int xMin = j - k;
                        if (xMin < 0) xMin = 0;
                        int xMax = j + k;
                        if (xMax > width - 1) xMax = width - 1;

                        int yMin = i - k;
                        if (yMin < 0) yMin = 0;
                        int yMax = i + k;
                        if (yMax > height - 1) yMax = height - 1;

                        if (dir1 == 0 && imgContoursData[yMin, xMin, 0] < thresholdEdge)
                        {
                            countR++;
                            countG++;
                            countB++;
                            averageR += imgSrcData[yMin, xMin, 2];
                            averageG += imgSrcData[yMin, xMin, 1];
                            averageB += imgSrcData[yMin, xMin, 0];
                        }
                        else
                        {
                            if (showContours == 0)
                            {
                                countR++;
                                countG++;
                                countB++;
                                averageR += imgSrcData[yMin, xMin, 2];
                                averageG += imgSrcData[yMin, xMin, 1];
                                averageB += imgSrcData[yMin, xMin, 0];
                            }
                            dir1 = 1;
                        }

                        if (dir2 == 0 && imgContoursData[yMin, j, 0] < thresholdEdge)
                        {
                            countR++;
                            countG++;
                            countB++;
                            averageR += imgSrcData[yMin, j, 2];
                            averageG += imgSrcData[yMin, j, 1];
                            averageB += imgSrcData[yMin, j, 0];
                        }
                        else
                        {
                            if (showContours == 0)
                            {
                                countR++;
                                countG++;
                                countB++;
                                averageR += imgSrcData[yMin, j, 2];
                                averageG += imgSrcData[yMin, j, 1];
                                averageB += imgSrcData[yMin, j, 0];
                                
                            }
                            dir2 = 1;
                        }

                        if (dir3 == 0 && imgContoursData[yMin, xMax, 0] < thresholdEdge)
                        {
                            countR++;
                            countG++;
                            countB++;
                            averageR += imgSrcData[yMin, xMax, 2];
                            averageG += imgSrcData[yMin, xMax, 1];
                            averageB += imgSrcData[yMin, xMax, 0];
                        }
                        else
                        {
                            if (showContours == 0)
                            {
                                countR++;
                                countG++;
                                countB++;
                                averageR += imgSrcData[yMin, xMax, 2];
                                averageG += imgSrcData[yMin, xMax, 1];
                                averageB += imgSrcData[yMin, xMax, 0];
                                
                            }
                            dir3 = 1;
                        }

                        if (dir4 == 0 && imgContoursData[i, xMin, 0] < thresholdEdge)
                        {
                            countR++;
                            countG++;
                            countB++;
                            averageR += imgSrcData[i, xMin, 2];
                            averageG += imgSrcData[i, xMin, 1];
                            averageB += imgSrcData[i, xMin, 0];
                        }
                        else
                        {
                            if (showContours == 0)
                            {
                                countR++;
                                countG++;
                                countB++;
                                averageR += imgSrcData[i, xMin, 2];
                                averageG += imgSrcData[i, xMin, 1];
                                averageB += imgSrcData[i, xMin, 0];
                                
                            }
                            dir4 = 1;
                        }

                        if (dir5 == 0 && imgContoursData[i, xMax, 0] < thresholdEdge)
                        {
                            countR++;
                            countG++;
                            countB++;
                            averageR += imgSrcData[i, xMax, 2];
                            averageG += imgSrcData[i, xMax, 1];
                            averageB += imgSrcData[i, xMax, 0];
                        }
                        else
                        {
                            if (showContours == 0)
                            {
                                countR++;
                                countG++;
                                countB++;
                                averageR += imgSrcData[i, xMax, 2];
                                averageG += imgSrcData[i, xMax, 1];
                                averageB += imgSrcData[i, xMax, 0];
                                
                            }
                            dir5 = 1;
                        }

                        if (dir6 == 0 && imgContoursData[yMax, xMin, 0] < thresholdEdge)
                        {
                            countR++;
                            countG++;
                            countB++;
                            averageR += imgSrcData[yMax, xMin, 2];
                            averageG += imgSrcData[yMax, xMin, 1];
                            averageB += imgSrcData[yMax, xMin, 0];
                        }
                        else
                        {
                            if (showContours == 0)
                            {
                                countR++;
                                countG++;
                                countB++;
                                averageR += imgSrcData[yMax, xMin, 2];
                                averageG += imgSrcData[yMax, xMin, 1];
                                averageB += imgSrcData[yMax, xMin, 0];
                                
                            }
                            dir6 = 1;
                        }

                        if (dir7 == 0 && imgContoursData[yMax, j, 0] < thresholdEdge)
                        {
                            countR++;
                            countG++;
                            countB++;
                            averageR += imgSrcData[yMax, j, 2];
                            averageG += imgSrcData[yMax, j, 1];
                            averageB += imgSrcData[yMax, j, 0];
                        }
                        else
                        {
                            if (showContours == 0)
                            {
                                countR++;
                                countG++;
                                countB++;
                                averageR += imgSrcData[yMax, j, 2];
                                averageG += imgSrcData[yMax, j, 1];
                                averageB += imgSrcData[yMax, j, 0];
                                
                            }
                            dir7 = 1;
                        }

                        if (dir8 == 0 && imgContoursData[yMax, xMax, 0] < thresholdEdge)
                        {
                            countR++;
                            countG++;
                            countB++;
                            averageR += imgSrcData[yMax, xMax, 2];
                            averageG += imgSrcData[yMax, xMax, 1];
                            averageB += imgSrcData[yMax, xMax, 0];
                        }
                        else
                        {
                            if (showContours == 0)
                            {
                                countR++;
                                countG++;
                                countB++;
                                averageR += imgSrcData[yMax, xMax, 2];
                                averageG += imgSrcData[yMax, xMax, 1];
                                averageB += imgSrcData[yMax, xMax, 0];
                                
                            }
                            dir8 = 1;
                        }
                    }

                    double calcR = averageR / (double)countR;
                    double calcG = averageG / (double)countG;
                    double calcB = averageB / (double)countB;

                    int R = (int)Computation.RoundValue_toX_SignificantBits(calcR, 0);
                    int G = (int)Computation.RoundValue_toX_SignificantBits(calcG, 0);
                    int B = (int)Computation.RoundValue_toX_SignificantBits(calcB, 0);

                    imgDstData[i, j, 0] = (byte)B;
                    imgDstData[i, j, 1] = (byte)G;
                    imgDstData[i, j, 2] = (byte)R;
                }
            }
            
            return imgDst;
        }


        /*D I F F  - of - G A U S S*/
        public static Image<Gray, Byte> DiffOfGaussBlur(this Image<Gray, Byte> imgSrc, int innerRaduis, int outerRadius)
        {
            int width = imgSrc.Width;
            int height = imgSrc.Height;
            int channels = imgSrc.NumberOfChannels;

            var imgGauss1 = imgSrc.BlurWithGaussRle(innerRaduis);
            var imgGauss2 = imgSrc.BlurWithGaussRle(outerRadius);

            int maxDiff;
            var imgDiff = imgGauss1.ComputeDifferenceWith(imgGauss2, out maxDiff);

            imgDiff._NormalizeAndInvert(maxDiff);

            return imgDiff;
        }


        /*G A U S S I A N  -  B LU R --by-- C O L U M N S*/
        public static unsafe void GaussBlurByColumns(this Image<Bgr, Byte> imgSrc, Image<Bgr, Byte> imgDst, double radius)
        {
            int start, end, i, val, calc, chan, dstPix, sk;

            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;

            var imgDstData = imgDst.Data;

            int Bytes = imgSrc.NumberOfChannels;
            int MaxFromWidthAndHeight = Math.Max(width, height);

            int[] buf = new int[2 * MaxFromWidthAndHeight];

            /*  allocate buffers for source and destination pixels  */
            int[] src = new int[Bytes * (MaxFromWidthAndHeight + 1)];//g_new (guchar, MAX (width, height) * Bytes);
            int[] dest = new int[Bytes * (MaxFromWidthAndHeight + 1)];

            /*  First the vertical pass  */
            radius = Math.Abs(radius) + 1.0;
            double std_dev = Math.Sqrt(-(radius * radius) / (2 * Math.Log(1.0 / 255.0)));

            int length = 0;
            int[] curve = MakeCurve(std_dev, out length);
            int[] sum = new int[2 * length + 1];

            sum[0] = 0;

            fixed (int* cur = &curve[0])
            {
                for (i = 1; i <= length * 2; i++)
                {
                    sum[i] = cur[i - 1] + sum[i - 1];
                }
            }

            int total = 0;
            fixed (int* suM = &sum[length])
            {
                total = suM[length] - suM[-length];

                for (int col = 0; col < width; col++)
                {
                    //get the column and put it in the src array
                    for (sk = 0; sk < height; sk++)
                    {
                        for (int b = 0; b < Bytes; b++)
                        {
                            src[sk * Bytes + b] = (int)imgSrcData[sk, col, b];
                        }
                    }
                    fixed (int* sp = &src[0])
                    {
                        fixed (int* dp = &dest[0])
                        {
                            for (int b = 0; b < Bytes; b++)
                            {
                                int* initial_p = &sp[b];

                                int* initial_m = &sp[(height - 1) * Bytes + b];

                                fixed (int* arrPts = &buf[0])
                                {
                                    /*  Determine a run-length encoded version of the row  */
                                    RunLengthEncode(sp + b, arrPts, Bytes, height);
                                }
                                for (int row = 0; row < height; row++)
                                {
                                    start = (row < length) ? -row : -length;
                                    end = (height <= (row + length) ?
                                           (height - row - 1) : length);

                                    val = 0;
                                    i = start;
                                    fixed (int* bb = &buf[(row + i) * 2])
                                    {
                                        if (start != -length)
                                            val += *initial_p * (suM[start] - suM[-length]);

                                        int* fi_xedBB = bb;
                                        while (i < end)
                                        {
                                            int pixels = fi_xedBB[0];
                                            i += pixels;

                                            if (i > end)
                                                i = end;

                                            val += fi_xedBB[1] * (suM[i] - suM[start]);
                                            fi_xedBB += (pixels * 2);
                                            start = i;
                                        }

                                        if (end != length)
                                            val += *initial_m * (suM[length] - suM[end]);

                                        if (total == 0)
                                            calc = 1;
                                        else
                                            calc = val / total;

                                        if (calc > 255) calc = 255;
                                        dp[row * Bytes + b] = calc;
                                    }
                                }
                            }

                            //set the calculated column in the destination image
                            for (dstPix = 0; dstPix < height; dstPix++)
                            {
                                for (chan = 0; chan < imgSrc.NumberOfChannels; chan++)
                                {
                                    imgDstData[dstPix, col, chan] = (Byte)dp[dstPix * Bytes + chan];
                                }
                            }
                        }
                    }
                }
            }
        }


        /*G A U S S I A N  -  B LU R --by-- R O W S*/
        public static unsafe void GaussBlurByRows(this Image<Bgr, Byte> imgSrc, Image<Bgr, Byte> imgDst, double radius)
        {
            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;

            var imgDstData = imgDst.Data;

            int Bytes = imgSrc.NumberOfChannels;
            int MaxFromWidthAndHeight = Math.Max(width, height);

            int[] buf = new int[2 * MaxFromWidthAndHeight];

            /*  allocate buffers for source and destination pixels  */
            int[] src = new int[Bytes * (MaxFromWidthAndHeight + 1)];//g_new (guchar, MAX (width, height) * Bytes);
            int[] dest = new int[Bytes * (MaxFromWidthAndHeight + 1)];

            /*  First the vertical pass  */
            radius = Math.Abs(radius) + 1.0;
            double std_dev = Math.Sqrt(-(radius * radius) / (2 * Math.Log(1.0 / 255.0)));

            int length = 0;
            int[] curve = MakeCurve(std_dev, out length);
            int[] sum = new int[2 * length + 1];

            sum[0] = 0;

            fixed (int* cur = &curve[0])
            {
                for (int i = 1; i <= length * 2; i++)
                {
                    sum[i] = cur[i - 1] + sum[i - 1];
                }
            }

            int total = 0;
            fixed (int* suM = &sum[length])
            {
                total = suM[length] - suM[-length];

                /*  Now the horizontal pass  */

                for (int row = 0; row < height; row++)
                {
                    //get the column and put it in the src array
                    for (int sk = 0; sk < width; sk++)
                    {
                        for (int b = 0; b < Bytes; b++)
                        {
                            src[sk * Bytes + b] = (int)imgDstData[row, sk, b];
                        }
                    }

                    
                    fixed (int* sp = &src[0])
                    {
                        fixed (int* dp = &dest[0])
                        {
                            for (int b = 0; b < Bytes; b++)
                            {
                                int* initial_p = &sp[b];
                                int* initial_m = &sp[(width - 1) * Bytes + b];

                                /*  Determine a run-length encoded version of the row  */
                                fixed (int* arrPts = &buf[0])
                                {
                                    RunLengthEncode(sp + b, arrPts, Bytes, width);
                                }
                                for (int col = 0; col < width; col++)
                                {
                                    int start = (col < length) ? -col : -length;
                                    int end = (width <= (col + length)) ? (width - col - 1) : length;

                                    int val = 0;
                                    int i = start;
                                    fixed (int* bb = &buf[(col + i) * 2])
                                    {
                                        if (start != -length)
                                            val += *initial_p * (suM[start] - suM[-length]);

                                        int* fi_xedBB = bb;
                                        while (i < end)
                                        {
                                            int pixels = fi_xedBB[0];
                                            i += pixels;

                                            if (i > end)
                                                i = end;

                                            val += fi_xedBB[1] * (suM[i] - suM[start]);
                                            fi_xedBB += (pixels * 2);
                                            start = i;
                                        }
                                    }

                                    if (end != length)
                                        val += *initial_m * (suM[length] - suM[end]);

                                    dp[col * Bytes + b] = val / total;
                                }
                            }
                            // set row to the output image;
                            for (int dstPix = 0; dstPix < width; dstPix++)
                            {
                                for (int chan = 0; chan < imgSrc.NumberOfChannels; chan++)
                                {
                                    imgDstData[row, dstPix, chan] = (Byte)dp[dstPix * Bytes + chan];
                                }
                            }
                        }
                    }
                }
            }
        }


        /*G A U S S I A N -- D I F F E R E N C E*/
        private static Image<Gray, Byte> ComputeDifferenceWith(this Image<Gray, Byte> imgGauss1, Image<Gray, Byte> imgGauss2, out int maxDiff)
        {
            maxDiff = 0;

            int width = imgGauss1.Width;
            int height = imgGauss1.Height;
            var imgGauss1Data = imgGauss1.Data;
            var imgGauss2Data = imgGauss2.Data;

            Image<Gray, Byte> imgDst = new Image<Gray, Byte>(width, height);
            var imgDstData = imgDst.Data;

            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    imgDstData[i, j, 0] = (byte)Computation.Clamp(imgGauss1Data[i, j, 0] - imgGauss2Data[i, j, 0], 255);
                    maxDiff = Math.Max(maxDiff, imgDstData[i, j, 0]);
                }
            }

            return imgDst;
        }


        /*N O R M A L I Z E - and - I N V E R T*/
        public static void _NormalizeAndInvert(this Image<Gray, Byte> imgSrc, int maxDiff)
        {
            double factor = 255 / (double)maxDiff;

            int channels = imgSrc.NumberOfChannels;
            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgData = imgSrc.Data;

            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    int number = (int)(factor * imgData[y, x, 0]);
                    if (number > 255)
                    {
                        number = 255;
                    }

                    imgData[y, x, 0] = (byte)(255 - number);
                }
            }
        }

        
        /*M A K E  -  G A U S S I A N  -  C U R V E*/
        public static unsafe int[] MakeCurve(double sigma, out int length)
        {
            length = 0;
            double sigma2 = 2 * sigma * sigma;
            double l = Math.Sqrt(-sigma2 * Math.Log(1.0 / 255.0));

            int n = (int)(Math.Ceiling(l) * 2);
            if ((n % 2) == 0)
            {
                n += 1;
            }

            int[] curve = new int[n + 1];

            length = n / 2;
            int s = length;
            //curve += length;
            curve[n / 2] = 255;

            fixed (int* curv = &curve[0])
            {
                int temp;
                for (int i = 1; i <= length; i++)
                {
                    temp = (int)(Math.Exp(-(i * i) / sigma2) * 255);
                    curve[(length) - i] = temp;
                    curve[(length) + i] = temp;
                }
            }

            return curve;
        }


        /*R L E*/
        public static unsafe void RunLengthEncode(int* src, int* dest, int Bytes, int width)
        {
            int start;
            int i;
            int j;
            int last;

            last = *src;

            src += Bytes;
            start = 0;

            for (i = 1; i < width; i++)
            {
                if (*src != last)
                {
                    for (j = start; j < i; j++)
                    {
                        *dest++ = (i - j);
                        *dest++ = last;
                    }
                    start = i;
                    last = *src;
                }
                src += Bytes;
            }

            for (j = start; j < i; j++)
            {
                *dest++ = (i - j);
                *dest++ = last;
            }
        }


        /*B L U R  -  with  - R L E*/
        private static unsafe Image<Gray, Byte> BlurWithGaussRle(this Image<Gray, Byte> imgSrc, double radius)
        {
            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;

            Image<Gray, Byte> imgDst = new Image<Gray, Byte>(width, height);
            Image<Gray, Byte> imgDst2 = new Image<Gray, Byte>(width, height);

            var imgDstData = imgDst.Data;
            if (width < 1 || height < 1)
            {
                return imgSrc;
            }

            int Bytes = imgSrc.NumberOfChannels;
            int MaxFromWidthAndHeight = Math.Max(width, height);

            int[] buf = new int[2 * MaxFromWidthAndHeight];

            /*  allocate buffers for source and destination pixels  */
            int[] src = new int[Bytes * (MaxFromWidthAndHeight + 1)];//g_new (guchar, MAX (width, height) * Bytes);
            int[] dest = new int[Bytes * (MaxFromWidthAndHeight + 1)];

            /*  First the vertical pass  */
            radius = Math.Abs(radius) + 1.0;
            double std_dev = Math.Sqrt(-(radius * radius) / (2 * Math.Log(1.0 / 255.0)));

            int length = 0;
            int[] curve = MakeCurve(std_dev, out length);
            int[] sum = new int[2 * length + 1];

            sum[0] = 0;

            fixed (int* cur = &curve[0])
            {
                for (int i = 1; i <= length * 2; i++)
                {
                    sum[i] = cur[i - 1] + sum[i - 1];
                }
            }

            int total = 0;
            fixed (int* suM = &sum[length])
            {
                total = suM[length] - suM[-length];

                for (int col = 0; col < width; col++)
                {
                    //get the column and put it in the src array
                    for (int sk = 0; sk < height; sk++)
                    {
                        for (int b = 0; b < Bytes; b++)
                        {
                            src[sk * Bytes + b] = (int)imgSrcData[sk, col, b];
                        }
                    }

                    fixed (int* sp = &src[0])
                    {
                        fixed (int* dp = &dest[0])
                        {
                            for (int b = 0; b < Bytes; b++)
                            {
                                int* initial_p = &sp[b];

                                int* initial_m = &sp[(height - 1) * Bytes + b];

                                fixed (int* arrPts = &buf[0])
                                {
                                    /*  Determine a run-length encoded version of the row  */
                                    RunLengthEncode(sp + b, arrPts, Bytes, height);
                                }

                                int calc = 0;
                                for (int row = 0; row < height; row++)
                                {
                                    int start = (row < length) ? -row : -length;
                                    int end = (height <= (row + length) ? (height - row - 1) : length);

                                    int val = 0;
                                    int i = start;
                                    fixed (int* bb = &buf[(row + i) * 2])
                                    {
                                        if (start != -length)
                                            val += *initial_p * (suM[start] - suM[-length]);

                                        int* fi_xedBB = bb;
                                        while (i < end)
                                        {
                                            int pixels = fi_xedBB[0];
                                            i += pixels;

                                            if (i > end)
                                            {
                                                i = end;
                                            }

                                            val += fi_xedBB[1] * (suM[i] - suM[start]);
                                            fi_xedBB += (pixels * 2);
                                            start = i;
                                        }

                                        if (end != length)
                                        {
                                            val += *initial_m * (suM[length] - suM[end]);
                                        }

                                        if (total == 0)
                                        {
                                            calc = 1;
                                        }
                                        else
                                        {
                                            calc = val / total;
                                        }

                                        if (calc > 255)
                                        {
                                            calc = 255;
                                        }

                                        dp[row * Bytes + b] = calc;
                                    }
                                }
                            }

                            //set the calculated column in the destination image
                            for (int dstPix = 0; dstPix < height; dstPix++)
                            {
                                for (int chan = 0; chan < imgSrc.NumberOfChannels; chan++)
                                {
                                    imgDstData[dstPix, col, chan] = (Byte)dp[dstPix * Bytes + chan];
                                }
                            }
                        }
                    }
                }

                var imgDst2Data = imgDst2.Data;
                /*  Now the horizontal pass  */

                for (int row = 0; row < height; row++)
                {
                    //get the column and put it in the src array
                    for (int sk = 0; sk < width; sk++)
                    {
                        for (int b = 0; b < Bytes; b++)
                        {
                            src[sk * Bytes + b] = (int)imgDstData[row, sk, b];
                        }
                    }


                    fixed (int* sp = &src[0])
                    {
                        fixed (int* dp = &dest[0])
                        {
                            for (int b = 0; b < Bytes; b++)
                            {
                                int* initial_p = &sp[b];
                                int* initial_m = &sp[(width - 1) * Bytes + b];

                                /*  Determine a run-length encoded version of the row  */
                                fixed (int* arrPts = &buf[0])
                                {
                                    RunLengthEncode(sp + b, arrPts, Bytes, width);
                                }
                                for (int col = 0; col < width; col++)
                                {
                                    int start = (col < length) ? -col : -length;
                                    int end = (width <= (col + length)) ? (width - col - 1) : length;

                                    int val = 0;
                                    int i = start;
                                    fixed (int* bb = &buf[(col + i) * 2])
                                    {
                                        if (start != -length)
                                            val += *initial_p * (suM[start] - suM[-length]);

                                        int* fi_xedBB = bb;
                                        while (i < end)
                                        {
                                            int pixels = fi_xedBB[0];
                                            i += pixels;

                                            if (i > end)
                                                i = end;

                                            val += fi_xedBB[1] * (suM[i] - suM[start]);
                                            fi_xedBB += (pixels * 2);
                                            start = i;
                                        }
                                    }

                                    if (end != length)
                                        val += *initial_m * (suM[length] - suM[end]);

                                    dp[col * Bytes + b] = val / total;
                                }
                            }
                            // set row to the output image;
                            for (int dstPix = 0; dstPix < width; dstPix++)
                            {
                                for (int chan = 0; chan < imgSrc.NumberOfChannels; chan++)
                                {
                                    imgDst2Data[row, dstPix, chan] = (Byte)dp[dstPix * Bytes + chan];
                                }
                            }
                        }
                    }
                }
            }

            return imgDst2;
        }


        /*F I L L  - E D G E S - with color*/
        public static void FillContourEdges(this Image<Gray, Byte> imgSrc, int color1, int startPos, int size)
        {
            var imgData = imgSrc.Data;
            int height = imgSrc.Height;
            int width = imgSrc.Width;

            int length = startPos + size;
            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    if ((i >= startPos && i <= length) || (i <= height - startPos - 1 && i >= height - length - 1) ||
                        (j >= startPos && j <= length) || (j <= width - startPos - 1 && j >= width - length - 1))
                    {
                        imgData[i, j, 0] = (byte) color1;
                    }
                }
            }
        }


        /*E D G E    E X T R A C T I O N*/
        public static Image<Gray, Byte> EdgeExtraction(this Image<Bgr, Byte> imgSrc, int kernelType)
        {
            /* define kernels */
            float[] Gx =          { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
            float[] Gy =          { 1, 2, 1, 0, 0, 0, -1, -2, -1 };
            float[] Laplace =     { 0, 1, 0, 1, -4, 1, 0, 1, 0 };
            float[] Prewitt_X_1 = { -5, -5, -5, 0, 0, 0, 5, 5, 5 };
            float[] Prewitt_Y_1 = { -5, 0, 5, -5, 0, 5, -5, 0, 5 };
            float[] Prewitt_X_2 = { 5, 5, 5, 0, 0, 0, -5, -5, -5 };
            float[] Prewitt_Y_2 = { 5, 0, -5, 5, 0, -5, 5, 0, -5 };
            float[] Sobel_X_1 =   { -1, -2, -1, 0, 0, 0, 1, 2, 1 };
            float[] Sobel_Y_1 =   { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
            float[] Sobel_X_2 =   { 1, 2, 1, 0, 0, 0, -1, -2, -1 };
            float[] Sobel_Y_2 =   { 1, 0, -1, 2, 0, -2, 1, 0, -1 };

            int srcWidth = imgSrc.Width;
            int srcHeight = imgSrc.Height;

            var imgSrcGrayscale = imgSrc.ConvertToGrayscale();
            var imgSrcData = imgSrcGrayscale.Data;

            if (kernelType == 0) // Prewitt
            {
                var DerrivativeX = Computation.Convolution(imgSrcGrayscale, Prewitt_X_1, 3);
                var DerrivativeY = Computation.Convolution(imgSrcGrayscale, Prewitt_Y_1, 3);

                var imgMagnitude = FindMagnitudeOfGradient(DerrivativeX, DerrivativeY);
                return imgMagnitude;
            }
            else if (kernelType == 1) // Sobel
            {
                var DerrivativeX = Computation.Convolution(imgSrcGrayscale, Sobel_X_1, 3);
                var DerrivativeY = Computation.Convolution(imgSrcGrayscale, Sobel_Y_1, 3);

                var imgMagnitude = FindMagnitudeOfGradient(DerrivativeX, DerrivativeY);
                return imgMagnitude;
            }
            else // gradient
            {
                var DerrivativeX = Computation.Convolution(imgSrcGrayscale, Gx, 3);
                var DerrivativeY = Computation.Convolution(imgSrcGrayscale, Gy, 3);

                var imgMagnitude = FindMagnitudeOfGradient(DerrivativeX, DerrivativeY);
                return imgMagnitude;
            }
        }


        /*Find   M A G N I T U D E  of Gradient*/
        public static Image<Gray, Byte> FindMagnitudeOfGradient(Image<Gray, Byte> DerrivativeX_image, Image<Gray, Byte> DerrivativeY_image)
        {
            int r, c, sq1, sq2;
            int rows = DerrivativeX_image.Height;
            int cols = DerrivativeX_image.Width;

            var derX = DerrivativeX_image.Data;
            var derY = DerrivativeY_image.Data;
            Image<Gray, Byte> Magnitude = DerrivativeY_image.CopyBlank();
            var magXY = Magnitude.Data;

            for (r = 0; r < rows; r++)
            {
                for (c = 0; c < cols; c++)
                {
                    sq1 = derX[r, c, 0] * derX[r, c, 0];
                    sq2 = derY[r, c, 0] * derY[r, c, 0];
                    magXY[r, c, 0] = (Byte)(0.5 + Math.Sqrt((float)sq1 + (float)sq2));
                }
            }
            return Magnitude;
        }


        /*S H A R P  image*/
        public static Image<Bgr, Byte> SharpImage(this Image<Bgr, Byte> imgSrc, float percentageShaprness, int EdgeExtractionType)
        {
            var src2Data = imgSrc.Data;
            var imgDst = imgSrc.CopyBlank();
            var imgDstData = imgDst.Data;

            int width = imgSrc.Width;
            int height = imgSrc.Height;

            int EDGEcolor = 0;
            percentageShaprness /= 100;
            var imgContours = imgSrc.EdgeExtraction(EdgeExtractionType);
            
            var imgContoursData = imgContours.Data;

            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    for (int k = 0; k < 3; k++)
                    {
                        if (imgContoursData[i, j, 0] >= EDGEcolor) //POSSIBLE_EDGE)
                        {
                            int calc = (int)(src2Data[i, j, k] - percentageShaprness*imgContoursData[i, j, 0]);

                            if (calc > 255)
                            {
                                imgDstData[i, j, k] = 255;
                            }
                            else if (calc < 0)
                            {
                                imgDstData[i, j, k] = 0;
                            }
                            else
                            {
                                imgDstData[i, j, k] =
                                    (Byte)(calc);
                            }
                        }
                        else
                        {
                            imgDstData[i, j, k] = src2Data[i, j, k];
                        }
                    }
                }
            }
            return imgDst;
        }


        /*S H A R P  image*/
        private static Image<Bgr, Byte> SharpImageUsingContours(this Image<Bgr, Byte> imgContours, Image<Bgr, Byte> imgSrc, float percentageShaprness, int EDGEcolor)
        {
            var magnitudeData = imgContours.Data;
            var src2Data = imgSrc.Data;
            var imgDst = imgSrc.CopyBlank();
            var imgDstData = imgDst.Data;

            int width = imgSrc.Width;
            int height = imgSrc.Height;

            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    for (int k = 0; k < 3; k++)
                    {
                        if (magnitudeData[i, j, k] >= EDGEcolor) //POSSIBLE_EDGE)
                        {
                            if (percentageShaprness*magnitudeData[i, j, k] + src2Data[i, j, k] > 255)
                            {
                                imgDstData[i, j, k] = 255;
                            }
                            else if (percentageShaprness*magnitudeData[i, j, k] + src2Data[i, j, k] < 0)
                            {
                                imgDstData[i, j, k] = 0;
                            }
                            else
                            {
                                imgDstData[i, j, k] =
                                    (Byte) (percentageShaprness*magnitudeData[i, j, k] + src2Data[i, j, k]);
                            }
                        }
                        else
                        {
                            imgDstData[i, j, k] = src2Data[i, j, k];
                        }
                    }
                }
            }
            return imgDst;
        }


        /*E D G E    E X T R A C T I O N  -   S O B E L*/
        public static Image<Gray, byte> EdgeExtractionSobel(this Image<Gray, byte> imgSrc)
        {
            int rows = imgSrc.Rows;
            int cols = imgSrc.Cols;
            var imgSrcData = imgSrc.Data;
            Image<Gray, byte> imgDst = new Image<Gray, byte>(cols, rows);
            var imgDstData = imgDst.Data;

            int hor_gradient = 0;
            int ver_gradient = 0;
            int gradient = 0;

            /*  allocate row buffers  */
            byte[] prevRow = new byte[cols];
            byte[] curRow = new byte[cols];
            byte[] nextRow = new byte[cols];

            /*  loop through the rows, applying the sobel convolution  */
            for (int row = 1; row < rows - 1; row++)
            {
                for (int rp = 0; rp < cols; rp++)
                {
                    prevRow[rp] = imgSrcData[row - 1, rp, 0];
                    curRow[rp] = imgSrcData[row, rp, 0];
                    nextRow[rp] = imgSrcData[row + 1, rp, 0];
                }

                for (int col = 1; col < cols - 1; col++)
                {
                    hor_gradient = (prevRow[col - 1] + 2 * prevRow[col] + prevRow[col + 1]) -
                             (nextRow[col - 1] + 2 * nextRow[col] + nextRow[col + 1]);
                    ver_gradient = ((prevRow[col - 1] + 2 * curRow[col - 1] + nextRow[col - 1]) -
                             (prevRow[col + 1] + 2 * curRow[col + 1] + nextRow[col + 1]));

                    gradient = (int)(Math.Round(Computation.PitagorTheorem(hor_gradient, ver_gradient)) / 5.66);
                    //gradient = (int)(127 + (Math.Round((hor_gradient + ver_gradient) / 8.0)));
                    if (gradient > 255) gradient = 255;
                    imgDstData[row, col, 0] = (byte)gradient;
                }
            }

            return imgDst;
        }

        /*W H I T E    B A L A N C E   -  algo 1*/
        /* algorithm using RGB and Temperature - low T -blue image, higher - Red image. ~ok around 6000k*/
        public static Image<Bgr, Byte> WhiteBalanceAlgo1(this Image<Bgr, Byte> imgSrc, int temperature)
        {
        	float LuminanceAverage = 0;
        	Structures.ColorPoint_UV UV;
        	Structures.WhitePoint WhitePointXYZ_new;
        	Structures.ColorPoint_RGB RGB;
        	Structures.ColorPoint_XYZ XYZ;
        	Structures.ColorPoint_XYZ XYZ_D;
        
        	float maxv;
        	float P, Number;
        	int i, j, z;
        	double R_Global = 0;
        	double G_Global = 0;
        	double B_Global = 0;
        	float MAX_VALUE = 0;
        	int Razlika_R, Razlika_G, Razlika_B;
        	int tmpKelvin = 0;
        
        	float R, G, B, dult;
        	float X, Y, Z, L, a, b, bbb, intensity;
        
        	R_Global = 0;
        	G_Global = 0;
        	B_Global = 0;

            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;

        	for (i = 0; i < height; i++)
        	{
        		for (j = 0; j < width; j++)
        		{
        			R = (float)imgSrcData[i, j, 2];
        			G = (float)imgSrcData[i, j, 1];
        			B = (float)imgSrcData[i, j, 0];
        
        			R_Global += R;
        			G_Global += G;
        			B_Global += B;
        		}
        	}

            int totalNumberOfPixels = width*height;
        	R_Global /= (float)(totalNumberOfPixels);
        	G_Global /= (float)(totalNumberOfPixels);
        	B_Global /= (float)(totalNumberOfPixels);
        
        	RGB.R = (int)Computation.RoundValue_toX_SignificantBits(R_Global, 0);
        	RGB.G = (int)Computation.RoundValue_toX_SignificantBits(G_Global, 0);
        	RGB.B = (int)Computation.RoundValue_toX_SignificantBits(B_Global, 0);
        
        	XYZ = RGB.ConvertPointRGBtoXYZ();
            UV = XYZ.ConvertPointXYZtoUV();
        
        	WhitePointXYZ_new.u = UV.u;
        	WhitePointXYZ_new.v = UV.v;
            WhitePointXYZ_new.Temperature = temperature;
        	WhitePointXYZ_new.X = XYZ.X;
        	WhitePointXYZ_new.Y = XYZ.Y;
        	WhitePointXYZ_new.Z = XYZ.Z;

            WhitePointXYZ_new.ColorTemperature(0);// EXP_HIGH_T);
        
        	tmpKelvin = WhitePointXYZ_new.Temperature / 100;
        
        	if (tmpKelvin <= 66){
        
        		Razlika_R = 255;
        
        		Razlika_G = tmpKelvin;
        		Razlika_G = (int)(99.4708025861 * Math.Log(Razlika_G) - 161.1195681661);
        
        
        		if (tmpKelvin <= 19)
        		{
        			Razlika_B = 0;
        		}
        		else 
                {
        			Razlika_B = tmpKelvin - 10;
        			Razlika_B = (int)(138.5177312231 * Math.Log(Razlika_B) - 305.0447927307);
        		}
        	}
        	else 
        	{
        		Razlika_R = tmpKelvin - 60;
        		Razlika_R = (int)(329.698727446 * Math.Pow(Razlika_R, -0.1332047592));
        
        		Razlika_G = tmpKelvin - 60;
        		Razlika_G = (int)(288.1221695283 * Math.Pow(Razlika_G, -0.0755148492));
        
        		Razlika_B = 255;
        	}

            var imgDst = imgSrc.CopyBlank();
            var imgDstData = imgDst.Data;

        	for (i = 0; i < height; i++)
        	{
        		for (j = 0; j < width; j++)
        		{
        			maxv = 255;
        
        			R = (float)imgSrcData[i, j , 2];
        			G = (float)imgSrcData[i, j , 1];
        			B = (float)imgSrcData[i, j , 0];
        
        			RGB.R = (int)(Computation.RoundValue_toX_SignificantBits(R * 255 / (float)Razlika_R, 0));
        			RGB.G = (int)(Computation.RoundValue_toX_SignificantBits(G * 255 / (float)Razlika_G, 0));
        			RGB.B = (int)(Computation.RoundValue_toX_SignificantBits(B * 255 / (float)Razlika_B, 0));
        			
        			if (RGB.R > 255) RGB.R = 255;
        			if (RGB.G > 255) RGB.G = 255;
        			if (RGB.B > 255) RGB.B = 255;
        			if (RGB.R < 0) RGB.R = 0;
        			if (RGB.G < 0) RGB.G = 0;
        			if (RGB.B < 0) RGB.B = 0;
        
        			imgDstData[i, j, 2] = (byte)RGB.R;
        			imgDstData[i, j, 1] = (byte)RGB.G;
        			imgDstData[i, j, 0] = (byte)RGB.B;
        		}
        	}
            return imgDst;
        }
        

        /*W H I T E   B A L A N C E  - algo2*/
        public static Image<Bgr, Byte> WhiteBalanceAlgo2(this Image<Bgr, Byte> imgSrc, int temperature)
        {
        	double LuminanceAverage = 0;
        	Structures.ColorPoint_UV UV;
        	Structures.WhitePoint WhitePointXYZ_new;
            Structures.WhitePoint WhitePoint_XYZ;
            WhitePoint_XYZ.X = 0;
            WhitePoint_XYZ.Y = 0;
            WhitePoint_XYZ.Z = 0;
            WhitePoint_XYZ.u = 0;
            WhitePoint_XYZ.v = 0;
            WhitePoint_XYZ.Temperature = 0;
            Structures.SetWhiteBalanceValues(ref WhitePoint_XYZ, temperature);
        	Structures.ColorPoint_RGB RGB;
        	Structures.ColorPoint_XYZ XYZ;
        	Structures.ColorPoint_XYZ XYZ_D;
        	double RatioX, RatioY, RatioZ;
        	double e = 0.008856;
        	double u,v;
        	double k = 903.3;
        	double F_x, F_z, F_y;
        	
        	//XYZ
        	//double [] Matrix_M_a = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
        	//double [] matrix_M_min1 = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
        	//BRADFORD
        	//double [] Matrix_M_a = { 0.8951000, 0.2664000, -0.1614000, -0.7502000, 1.7135000, 0.0367000, 0.0389000, -0.0685000, 1.0296000 };
        	//double [] matrix_M_min1 = { 0.9869929, -0.1470543, 0.1599627, 0.4323053, 0.5183603, 0.0492912, -0.0085287, 0.0400428, 0.9684867 };
        	//CAT97s
        	//double [] Matrix_M_a = { 0.08562, 0.3372, -0.1934, -0.836, 1.8327, 0.0033, 0.0357, -0.0469, 1.0112 };
        	//double [] matrix_M_min1 = { 0.9869929, -0.1470543, 0.1599627, 0.4323053, 0.5183603, 0.0492912, -0.0085287, 0.0400428, 0.9684867 };
        	//CAT02s
        	double [] Matrix_M_a = { 0.7328, 0.4296, -0.1624, -0.7036, 1.6975, 0.0061, 0.0030, 0.0136, 0.9834 };
        	double [] matrix_M_min1 = { 0.9869929, -0.1470543, 0.1599627, 0.4323053, 0.5183603, 0.0492912, -0.0085287, 0.0400428, 0.9684867 };
        	//VON Kries
        	//double [] Matrix_M_a = { 0.40024, 0.7076, -0.08081, -0.2263, 1.16532, 0.0457, 0, 0, 0.91822 };
        	//double [] matrix_M_min1 = { 1.8599364, -1.1293816, 0.2198974, 0.3611914, 0.6388125, -0.0000064, 0, 0, 1.08906 };
        	
        	double []S_params = new double[3];
        	double []D_params = new double[3];
            double[] S_D_ParamsMatrix = new double[9];
        	double []MatrixMultiplication_1 = new double[9];
        	double []matrix_M_final = new double[9];
        	double maxv;
        	double P, Number;
        	int i, j, z;
        	double R_Global = 0;
        	double G_Global = 0;
        	double B_Global = 0;
        	double MAX_VALUE = 0; 
        	int Razlika_R, Razlika_G, Razlika_B;
        	int tmpKelvin = 0;
            
        	double R, G, B, dult;
        	double X, Y, Z, L, a, b, bbb, intensity;
        
        	R_Global = 0;
        	G_Global = 0;
        	B_Global = 0;

            int height = imgSrc.Height;
            int width = imgSrc.Width;
            var imgSrcData = imgSrc.Data;

            var imgDst = imgSrc.CopyBlank();
            var imgDstData = imgDst.Data;

        	for (i = 0; i < height; i++)
        	{
        		for (j = 0; j < width; j++)
        		{
        			R = imgSrcData[i, j, 2];
        			G = imgSrcData[i, j, 1];
        			B = imgSrcData[i, j, 0];
        
        			/* if there is a perfect white pixel in the image */

        			R_Global += R;
        			G_Global += G;
        			B_Global += B;
        		}
        	}

            int totalNumberOfPixels = width*height;
        	R_Global /= (double)(totalNumberOfPixels);
        	G_Global /= (double)(totalNumberOfPixels);
        	B_Global /= (double)(totalNumberOfPixels);
        
        	LuminanceAverage = R_Global + G_Global + B_Global;
        	R = R_Global;
        	G = G_Global;
        	B = B_Global;
        	MAX_VALUE = Math.Max(R, Math.Max(G, B));
        
        	Razlika_B = (int)(255 - MAX_VALUE);

        	RGB.R = (int)(Computation.RoundValue_toX_SignificantBits(R, 0));
        	RGB.G = (int)(Computation.RoundValue_toX_SignificantBits(G, 0));
        	RGB.B = (int)(Computation.RoundValue_toX_SignificantBits(B, 0));
        
        	XYZ = RGB.ConvertPointRGBtoXYZ();
        	UV = XYZ.ConvertPointXYZtoUV();
        
        	WhitePointXYZ_new.u = UV.u;
        	WhitePointXYZ_new.v = UV.v;
        
        	WhitePointXYZ_new.X = XYZ.X;
        	WhitePointXYZ_new.Y = XYZ.Y;
        	WhitePointXYZ_new.Z = XYZ.Z;
            WhitePointXYZ_new.Temperature = temperature;
        	WhitePointXYZ_new.ColorTemperature(0);// EXP_HIGH_T);
       
        	tmpKelvin = temperature / 100;
        
        	if (tmpKelvin <= 66)
            {
                R = 255;
        
        		G = tmpKelvin;
        		G = 99.4708025861 * Math.Log(G) - 161.1195681661;
        
        		if (tmpKelvin <= 19)
        		{
        			B = 0;
        		}
        		else 
                {
        			B = tmpKelvin - 10;
                    B = 138.5177312231 * Math.Log(B) - 305.0447927307;
        		}
        	}
        	else 
            {
        		R = tmpKelvin - 60;
        		R = 329.698727446 * Math.Pow(R, -0.1332047592);
        
        		G = tmpKelvin - 60;
                G = 288.1221695283 * Math.Pow(G, -0.0755148492);
        
        		B = 255;
        	}
        
        	tmpKelvin = WhitePointXYZ_new.Temperature / 100;
        
        	if (tmpKelvin <= 66)
            {
        		Razlika_R = 255;
        
        		Razlika_G = tmpKelvin;
        		Razlika_G = (int)(99.4708025861 * Math.Log(Razlika_G) - 161.1195681661);
        
        		if (tmpKelvin <= 19)
        		{
        			Razlika_B = 0;
        		}
        		else 
                {
        			Razlika_B = tmpKelvin - 10;
        			Razlika_B = (int)(138.5177312231 * Math.Log(Razlika_B) - 305.0447927307);
        		}
        
        	}
        	else 
            {
        		Razlika_R = tmpKelvin - 60;
        		Razlika_R = (int)(329.698727446 * Math.Pow(Razlika_R, -0.1332047592));
        
        		Razlika_G = tmpKelvin - 60;
                Razlika_G = (int)(288.1221695283 * Math.Pow(Razlika_G, -0.0755148492));
        
        		Razlika_B = 255;
        	}
        
        	/*Luminance between 0 and 1*/
         	LuminanceAverage /=(3.0 * 255);
        	/*change Matrix_M_a to match the luminance*/
        	for(i = 0; i < 9; i++)
        	{
        		//if(i == 2)Matrix_M_a[i] *= (0 *LuminanceAverage); // cherveno
        		//if(i == 8)Matrix_M_a[i] *= (0 *LuminanceAverage); // Zeleno
        		//else
        		Matrix_M_a[i] *= (1.4 * LuminanceAverage);
        	    //matrix_M_min1[i] *= (0.4 * LuminanceAverage);
        		//Matrix_M_a[i] += LuminanceAverage - 0.8; // Epic fail
        		//Matrix_M_a[i] += LuminanceAverage - 0.9;
        		//if (i == 1)  Matrix_M_a[i] *= (0.1 * LuminanceAverage);
        		//else Matrix_M_a[i] *= (0.7 * LuminanceAverage);
        	}
        
        	WhitePointXYZ_new.X /= 100.0;
        	WhitePointXYZ_new.Y /= 100.0;
        	WhitePointXYZ_new.Z /= 100.0;
        
        	for (i = 0; i < height; i++)
        	{
        		for (j = 0; j < width; j++)
        		{
        			maxv = 255;
        
        			R = (double)imgSrcData[i, j, 2];
        			G = (double)imgSrcData[i, j, 1];
        			B = (double)imgSrcData[i, j, 0];
        
        			RGB.R = (int)R;
        			RGB.G = (int)G;
        			RGB.B = (int)B;

        			// Convert RGB point to XYZ
        			XYZ = RGB.ConvertPointRGBtoXYZ();
        			XYZ.X /= 100.0;
        			XYZ.Y /= 100.0;
        			XYZ.Z /= 100.0;
        			
        			/*
        			| X_d |           | X_s |
        			| Y_d |   = |M| * | Y_s |
        			| Z_s |           | Z_s |
        			http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
        			*/
        			S_params[0] = WhitePointXYZ_new.X * Matrix_M_a[0] + WhitePointXYZ_new.Y * Matrix_M_a[1] + WhitePointXYZ_new.Z * Matrix_M_a[2];
        			S_params[1] = WhitePointXYZ_new.X * Matrix_M_a[3] + WhitePointXYZ_new.Y * Matrix_M_a[4] + WhitePointXYZ_new.Z * Matrix_M_a[5];
        			S_params[2] = WhitePointXYZ_new.X * Matrix_M_a[6] + WhitePointXYZ_new.Y * Matrix_M_a[7] + WhitePointXYZ_new.Z * Matrix_M_a[8];
        
        			D_params[0] = WhitePoint_XYZ.X * Matrix_M_a[0] + WhitePoint_XYZ.Y * Matrix_M_a[1] + WhitePoint_XYZ.Z * Matrix_M_a[2];
        			D_params[1] = WhitePoint_XYZ.X * Matrix_M_a[3] + WhitePoint_XYZ.Y * Matrix_M_a[4] + WhitePoint_XYZ.Z * Matrix_M_a[5];
        			D_params[2] = WhitePoint_XYZ.X * Matrix_M_a[6] + WhitePoint_XYZ.Y * Matrix_M_a[7] + WhitePoint_XYZ.Z * Matrix_M_a[8];
        
        			/* Compute M_min1 matrix * S/D */
        			S_D_ParamsMatrix[0] = D_params[0] / S_params[0];
        			S_D_ParamsMatrix[1] = 0;
        			S_D_ParamsMatrix[2] = 0;
        			S_D_ParamsMatrix[3] = 0;
        			S_D_ParamsMatrix[4] = D_params[1] / S_params[1];
        			S_D_ParamsMatrix[5] = 0;
        			S_D_ParamsMatrix[6] = 0;
        			S_D_ParamsMatrix[7] = 0;
        			S_D_ParamsMatrix[8] = D_params[2] / S_params[2];
        
        			MatrixMultiplication_1[0] = matrix_M_min1[0] * S_D_ParamsMatrix[0];
        			MatrixMultiplication_1[1] = matrix_M_min1[1] * S_D_ParamsMatrix[4];
        			MatrixMultiplication_1[2] = matrix_M_min1[2] * S_D_ParamsMatrix[8];
        			MatrixMultiplication_1[3] = matrix_M_min1[3] * S_D_ParamsMatrix[0];
        			MatrixMultiplication_1[4] = matrix_M_min1[4] * S_D_ParamsMatrix[4];
        			MatrixMultiplication_1[5] = matrix_M_min1[5] * S_D_ParamsMatrix[8];
        			MatrixMultiplication_1[6] = matrix_M_min1[6] * S_D_ParamsMatrix[0];
        			MatrixMultiplication_1[7] = matrix_M_min1[7] * S_D_ParamsMatrix[4];
        			MatrixMultiplication_1[8] = matrix_M_min1[8] * S_D_ParamsMatrix[8];
        			
        			/* Compute MatrixMultiplication_1 * matrix_M */
        
        			matrix_M_final[0] = MatrixMultiplication_1[0] * Matrix_M_a[0] + MatrixMultiplication_1[1] * Matrix_M_a[3] + MatrixMultiplication_1[2] * Matrix_M_a[6];
        			matrix_M_final[1] = MatrixMultiplication_1[0] * Matrix_M_a[1] + MatrixMultiplication_1[1] * Matrix_M_a[4] + MatrixMultiplication_1[2] * Matrix_M_a[7];
        			matrix_M_final[2] = MatrixMultiplication_1[0] * Matrix_M_a[2] + MatrixMultiplication_1[1] * Matrix_M_a[5] + MatrixMultiplication_1[2] * Matrix_M_a[8];
        			matrix_M_final[3] = MatrixMultiplication_1[3] * Matrix_M_a[0] + MatrixMultiplication_1[4] * Matrix_M_a[3] + MatrixMultiplication_1[5] * Matrix_M_a[6];
        			matrix_M_final[4] = MatrixMultiplication_1[3] * Matrix_M_a[1] + MatrixMultiplication_1[4] * Matrix_M_a[4] + MatrixMultiplication_1[5] * Matrix_M_a[7];
        			matrix_M_final[5] = MatrixMultiplication_1[3] * Matrix_M_a[2] + MatrixMultiplication_1[4] * Matrix_M_a[5] + MatrixMultiplication_1[5] * Matrix_M_a[8];
        			matrix_M_final[6] = MatrixMultiplication_1[6] * Matrix_M_a[0] + MatrixMultiplication_1[7] * Matrix_M_a[3] + MatrixMultiplication_1[8] * Matrix_M_a[6];
        			matrix_M_final[7] = MatrixMultiplication_1[6] * Matrix_M_a[1] + MatrixMultiplication_1[7] * Matrix_M_a[4] + MatrixMultiplication_1[8] * Matrix_M_a[7];
        			matrix_M_final[8] = MatrixMultiplication_1[6] * Matrix_M_a[2] + MatrixMultiplication_1[7] * Matrix_M_a[5] + MatrixMultiplication_1[8] * Matrix_M_a[8];
        
        			XYZ_D.X = matrix_M_final[0] * XYZ.X + matrix_M_final[1] * XYZ.Y + matrix_M_final[2] * XYZ.Z;
        			XYZ_D.Y = matrix_M_final[3] * XYZ.X + matrix_M_final[4] * XYZ.Y + matrix_M_final[5] * XYZ.Z;
        			XYZ_D.Z = matrix_M_final[6] * XYZ.X + matrix_M_final[7] * XYZ.Y + matrix_M_final[8] * XYZ.Z;
        			//RGB: 150,55,7
        			//XYZ: 0.14, 0.09, 0.01
                    RGB = XYZ_D.ConvertPointXYZtoRGB();
        
        			imgDstData[i, j, 2] = (byte)Computation.Clamp(RGB.R, 255);
                    imgDstData[i, j, 1] = (byte)Computation.Clamp(RGB.G, 255);
                    imgDstData[i, j, 0] = (byte)Computation.Clamp(RGB.B, 255);
        		}
        	}

            return imgDst;
        }


        /* Correct     W H I T E  B A L A N C E  - RGB*/
        public static Image<Bgr, Byte> WhiteBalanceAlgo3(this Image<Bgr, Byte> imgSrc, int Algotype)
        {
        	int i, j;
        
        	int MaxR = 0, MaxG = 0, MaxB = 0;
        	float GtoR_Ratio = 1;
        	float GtoB_Ratio = 1;
        	long SumG = 0;
        	long SumR = 0;
        	long SumB = 0;
        	float Gto255_Ratio = 1;
        	int check3Values = 0;

            int height = imgSrc.Height;
            int width = imgSrc.Width;
            var imgSrcData = imgSrc.Data;
            var imgDst = imgSrc.CopyBlank();
            var imgDstData = imgDst.Data;

        	if (Algotype == 1)
        	{
        		//Green world - automatic white detection
        		for (i = 0; i < height; i++)
        		{
        			for (j = 0; j < width; j++)
        			{
        				check3Values = 0;
        			    int B = imgSrcData[i, j, 0];
                        int G = imgSrcData[i, j, 1];
                        int R = imgSrcData[i, j, 2];
        				if (R > MaxR) { MaxR = R; check3Values++; }
        				if (G > MaxG) { MaxG = G; check3Values++;}
        				if (B > MaxB) { MaxB = B; check3Values++; }
        				
        				if (check3Values == 3)
        				{
        					//Calculate ratios
        					GtoR_Ratio = (float)G / (float)R;
        					GtoB_Ratio = (float)G / (float)B;
        				}
        			}
        		}
        		/*Calculate new values based on GtoR amd GtoB ratios*/
        		for (i = 0; i < height; i++)
        		{
        			for (j = 0; j < width; j++)
        			{
        			    int calc1 = (int)GtoB_Ratio*imgSrcData[i, j, 0];
                        if (calc1 <= 255)
                            imgDstData[i, j, 0] = (byte)calc1;
                        else imgDstData[i, j, 0] = 255;

                        int calc2 = (int)GtoR_Ratio * imgSrcData[i, j, 2];
                        if (calc2 <= 255)
                            imgDstData[i, j, 2] = (byte)calc2;
                        else imgDstData[i, j, 2] = 255;
        				
        				imgDstData[i, j, 1] = imgSrcData[i, j, 1];
        			}
        		}
        	}
        	else if (Algotype == 2)
        	{
        		for (i = 0; i < height; i++)
        		{
        			for (j = 0; j < width; j++)
        			{
        			    int G = imgSrcData[i, j, 1];
                        int R = imgSrcData[i, j, 2];
                        int B = imgSrcData[i, j, 0];
        				if (G > MaxG) 
        				{ 
        					MaxG = G;
        					GtoR_Ratio = (float)MaxG / (float)R;
                            GtoB_Ratio = (float)MaxG / (float)B;
        				}
        			}
        		}
        		/*Calculate new values based on GtoR and GtoB ratios*/
        		for (i = 0; i < height; i++)
        		{
        			for (j = 0; j < width; j++)
        			{
        			    int calc1 = (int)GtoB_Ratio*imgSrcData[i, j, 0];

                        if (calc1 <= 255)
        					imgDstData[i, j, 0] = (byte)calc1;
        				else imgDstData[i, j, 0] = 255;

        			    int calc2 = (int) GtoR_Ratio*imgSrcData[i, j, 2];
                        if (calc2 <= 255)
        					imgDstData[i, j, 2] = (byte)calc2;
        				else imgDstData[i, j, 2] = 255;
        
        				imgDstData[i, j, 1] = imgSrcData[i, j, 1];
        			}
        		}
        
        	}
        	else if (Algotype == 3)
        	{
        		//Green world - automatic white detection
        		for (i = 0; i < height; i++)
        		{
        			for (j = 0; j < width; j++)
        			{
        				check3Values = 0;
        				if (imgSrcData[i, j, 2] > MaxR) { MaxR = imgSrcData[i, j, 2]; check3Values++; }
        				if (imgSrcData[i, j, 1] > MaxG) { MaxG = imgSrcData[i, j, 1]; check3Values++; }
        				if (imgSrcData[i, j, 0] > MaxB) { MaxB = imgSrcData[i, j, 0]; check3Values++; }
        
        				if (check3Values == 3)
        				{
        					//Calculate ratios
        					Gto255_Ratio = 255 / MaxG;
        					GtoR_Ratio = (float)(Gto255_Ratio *(MaxG / MaxR));
        					GtoB_Ratio = (float)(Gto255_Ratio *(MaxG / MaxB));
        				}
        			}
        		}
        		/*Calculate new values based on GtoR amd GtoB ratios*/
        		for (i = 0; i < height; i++)
        		{
        			for (j = 0; j < width; j++)
        			{
        			    int calc1 = (int)GtoB_Ratio*imgSrcData[i, j, 0];
                        if (calc1 <= 255)
        					imgDstData[i, j, 0] = (byte)calc1;
        				else imgDstData[i, j, 0] = 255;

        			    int calc2 = (int) GtoR_Ratio*imgSrcData[i, j, 2];
        				if (calc2 <= 255)
        					imgDstData[i, j, 2] = (byte)calc2;
        				else imgDstData[i, j, 2] = 255;

        			    int calc3 = (int) Gto255_Ratio*imgSrcData[i, j, 1];
                        if (calc3 <= 255)
        					imgDstData[i, j, 1] = (byte)calc2;
        				else imgDstData[i, j, 1] = 255;
        			}
        		}
        	}
        	else if (Algotype == 4)
        	{
        	    //Green world - automatic white detection
        	    for (i = 0; i < height; i++)
        	    {
        	        for (j = 0; j < width; j++)
        	        {
        	            SumB += imgSrcData[i, j, 0];
        	            SumG += imgSrcData[i, j, 1];
        	            SumR += imgSrcData[i, j, 2];
        	        }
        	    }
        	    GtoR_Ratio = (float) SumG/SumR;
        	    GtoB_Ratio = (float) SumG/SumB;
        	    Gto255_Ratio = (float) SumG/(255*height*width);
        	    /*	if (GtoB_Ratio < 0.8 || GtoR_Ratio < 0.8)
        		{
        			GtoR_Ratio = GtoR_Ratio * ;
        			GtoB_Ratio = (float)SumG / SumB;
        		}*/
        	    /*Calculate new values based on GtoR amd GtoB ratios*/
        	    for (i = 0; i < height; i++)
        	    {
        	        for (j = 0; j < width; j++)
        	        {
        	            int calc1 = (int) GtoB_Ratio*imgSrcData[i, j, 0];
        	            if (calc1 <= 255)
        	                imgDstData[i, j, 0] = (byte)calc1;
        	            else imgDstData[i, j, 0] = 255;

                        int calc2 = (int)GtoR_Ratio * imgSrcData[i, j, 2];
                        if (calc2 <= 255)
                            imgDstData[i, j, 2] = (byte)calc2;
        	            else imgDstData[i, j, 2] = 255;

        	            imgDstData[i, j, 1] = imgSrcData[i, j, 1];
        	        }
        	    }
        	}
        	else
        	{
        	    return imgSrc;
        	}

            return imgDst;
        }


        /* Correct   B R I G H T N E S S  - RGB and Lab only !*/
        public static Image<Bgr, Byte> BrightnessCorrection(Image<Bgr, Byte> imgSrc, double Algo_paramBrightnessOrEV, int Algotype)
        {
        	int i, j, l;

            var width = imgSrc.Width;
            var height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;
            var imgDst = imgSrc.CopyBlank();
            var imgDstData = imgDst.Data;

            if (Algotype == 1)
            {
            	for (i = 0; i < height; i++)
            	{
            		for (j = 0; j < width; j++)
            		{
            			for (l = 0; l < 3; l++)
            			{
            			    int calc = (int)Algo_paramBrightnessOrEV + imgSrcData[i, j, l];

            				if (calc > 255)
            					calc = 255;
                            else if (calc < 0)
            					calc = 0;
            			    imgDstData[i, j, l] = (byte)calc;
            			}
            		}
            	}
            }
            else if (Algotype == 2)
            {
            	for (i = 0; i < height; i++)
            	{
            		for (j = 0; j < width; j++)
            		{
            			for (l = 0; l < 3; l++)
            			{
            			    int calc = (int)Math.Pow(2, Algo_paramBrightnessOrEV)*imgSrcData[i, j, l];
                            if (calc > 255)
                                calc = 255;
                            else if (calc < 0)
                                calc = 0;
            			    imgDstData[i, j, l] = (byte)calc;
            			}
            		}
            	}
            }
        	
        	return imgDst;
        }

        /*C O N T R A S T  -  correction for Image*/
        public static Image<Bgr, Byte> ContrastCorrection(this Image<Bgr, Byte> imgSrc, double percentage)
        {
            /* The percentage value should be between -100 and 100*/

            /* The percentage value should be between -100 and 100*/
            double pixel = 0;
            double contrast = 0;
            int i, j, l;

            if (percentage < -100) percentage = -100;
            if (percentage > 100) percentage = 100;
            contrast = (100.0 + percentage) / 100.0;

            contrast *= contrast;

            int rows = imgSrc.Rows;
            int cols = imgSrc.Cols;
            var imgSrcData = imgSrc.Data;
            int numChannels = imgSrc.NumberOfChannels;
            Image<Bgr, Byte> imgDst = new Image<Bgr, Byte>(cols, rows);
            var imgDstData = imgDst.Data;

            for (i = 0; i < rows; i++)
            {
                for (j = 0; j < cols; j++)
                {
                    for (l = 0; l < numChannels; l++)
                    {
                        pixel = (float)imgSrcData[i, j, l] / 255;


                        pixel -= 0.5;
                        pixel *= contrast;
                        pixel += 0.5;
                        pixel = pixel * 255;
                        if (pixel > 255) pixel = 255;
                        if (pixel < 0) pixel = 0;

                        imgDstData[i, j, l] = (Byte)(0.5 + pixel);
                    }
                }
            }
            return imgDst;
        }


        /*C O N T R A S T  -  correction for Pixel*/
        public static Byte PixelcontrastCorrection(Byte value, double percentage)
        {
            /* The percentage value should be between -100 and 100*/

            /* The percentage value should be between -100 and 100*/
            double pixel = 0;
            double contrast = 0;

            if (percentage < -100) percentage = -100;
            if (percentage > 100) percentage = 100;
            contrast = (100.0 + percentage) / 100.0;

            contrast *= contrast;

            pixel = value / 255.0;

            pixel -= 0.5;
            pixel *= contrast;
            pixel += 0.5;
            pixel = pixel * 255;
            if (pixel > 255) pixel = 255;
            if (pixel < 0) pixel = 0;

            value = (Byte)(pixel);

            return value;
        }


        /* Correct    N O I S E */
        public static Image<Bgr, Byte> NoiseCorrection(this Image<Bgr, Byte> imgSrc, double threshold, int Algotype)
        {
        	int i, j, z;
        	int CurrentValue;
        	int ProbablityValue = 0;
    
        	/* if the current pixel is X % different from the pixels around -> it is noise*/
        
            var width = imgSrc.Width;
            var height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;
            var imgDst = imgSrc.CopyBlank();
            var imgDstData = imgDst.Data;

        	if (Algotype == 1)
        	{
        		for (i = 1; i < height - 1; i++)
        		{
        			for (j = 1; j < width - 1; j++)
        			{
        				for (z = 0; z < 3; z++)
        				{
        					ProbablityValue = 0;
        					CurrentValue = imgSrcData[i, j, z];
        
        					if (threshold * CurrentValue < imgSrcData[(i - 1), j, z]       || CurrentValue > threshold *  imgSrcData[(i - 1), j , z]) ProbablityValue++;
        					if (threshold * CurrentValue < imgSrcData[(i + 1), j, z]       || CurrentValue > threshold *  imgSrcData[(i + 1), j , z]) ProbablityValue++;
        					if (threshold * CurrentValue < imgSrcData[(i    ), (j - 1), z] || CurrentValue > threshold *  imgSrcData[(i    ), (j - 1) ,z]) ProbablityValue++;
        					if (threshold * CurrentValue < imgSrcData[(i    ), (j + 1), z] || CurrentValue > threshold *  imgSrcData[(i    ), (j + 1) ,z]) ProbablityValue++;
        
        					if (ProbablityValue >= 3)
        					{
        						imgDstData[i, j, z] = (byte)((imgSrcData[i-1, j, z] + imgSrcData[i+1, j, z] + imgSrcData[i, j-1, z] + imgSrcData[i, j+1, z]) / 4);
        					}
        				}
        			}
        		}
        	}
        	else if (Algotype == 2)
        	{
        		//future implementation
        	}
        
        	return imgDst;
        }

        /*M E R G E   Layers - Blend Effect*/
        public static Image<Bgr, Byte> BlendImageLayers(this Image<Bgr, Byte> imgSrc, Image<Bgr, Byte> imgBlendedSrc, double OpacitySrc1, double OpacitySrc2, LayerParams imgParams, bool whiteAsTransperant)
        {
            /*imgSrc will always be equal or bigger than blendedImage*/
            var width = imgSrc.Width;
            var height = imgSrc.Height;

            var blendWidth = imgBlendedSrc.Width;
            var blendHeight = imgBlendedSrc.Height;

            var imgDst = imgSrc.CopyBlank();
            var imgSrcData = imgSrc.Data;
            var imgDstData = imgDst.Data;
            var imgBlendData = imgBlendedSrc.Data;

            if (Computation.Abs(OpacitySrc1) > 1) OpacitySrc1 /= 100;
            if (Computation.Abs(OpacitySrc2) > 1) OpacitySrc2 /= 100;
            
            OpacitySrc2 = (1 - OpacitySrc1) * OpacitySrc2;

            int blendImageStartX = imgParams.offsetX;
            int blendImageStartY = imgParams.offsetY;

            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    var copyOpacity1 = OpacitySrc1;
                    var copyOpacity2 = OpacitySrc2;

                    if(whiteAsTransperant)
                    {
                        if (imgSrcData[i, j, 0] == 255 &&
                                imgSrcData[i, j, 1] == 255 &&
                                imgSrcData[i, j, 2] == 255)
                            {
                                copyOpacity2 = 1;
                                copyOpacity1 = 0;
                            }

                    }
                    for(int k = 0; k < 3; k++)
                    {
                        /*if we are not in region of BlendImage*/
                        if(i < blendImageStartY || 
                            i >= blendImageStartY + blendHeight ||
                            j < blendImageStartX ||
                            j >= blendImageStartX + blendWidth)
                        {
                            imgDstData[i, j, k] = imgSrcData[i, j, k];
                        }

                        else
                        {
                            int calc = (int)((copyOpacity2) * imgBlendData[i - blendImageStartY, j - blendImageStartX, k] + (copyOpacity1) * imgSrcData[i, j, k]);
                            if (calc > 255)
                            {
                                imgDstData[i, j, k] = 255;
                            }
                            else
                            {
                                imgDstData[i, j, k] = (byte)(calc);
                            }
                        }
                    }
                }
            }

            return imgDst;
        }

        /*B L E N D I N G  - similar to image sharpening - but the contours are from another image*/
        /*Example usage: imgSrc.BlendImage(imgSrcBlend, 40, 2, 3, 30);*/
        public static Image<Bgr, Byte> BlendImage(this Image<Bgr, Byte> imgSrc,  Image<Bgr, Byte> imgBlendedSrc, double Percentage, int AlgoParam1, int Algoparam2, int BlacOrWhiteThreshold)
        {
        	int i, j, l;
            enums.AlgoType_Blend blendEnums;
            var width = imgSrc.Width;
            var height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;
            var imgDst = imgSrc.CopyBlank();
            var imgDstData = imgDst.Data;

        	if (Computation.Abs(Percentage) > 1) Percentage /= 100;

            var imgBlendData = imgBlendedSrc.Data;

        	for (i = 0; i < height; i++)
        	{
        		for (j = 0; j < width; j++)
        		{
        			for (l = 0; l < 3; l++)
        			{
        				if (AlgoParam1 == 1)
        				{
                            if ((Algoparam2 == 4 && (imgBlendData[i, j, 0] <= BlacOrWhiteThreshold) && (imgBlendData[i, j, 1] <= BlacOrWhiteThreshold) && (imgBlendData[i, j, 2] <= BlacOrWhiteThreshold))
        						||
        						(Algoparam2 == 3 && (imgBlendData[i, j , 0] >= 255 - BlacOrWhiteThreshold) && (imgBlendData[i, j, 1] >= 255 - BlacOrWhiteThreshold) && (imgBlendData[i, j, 2] >= 255 - BlacOrWhiteThreshold)))
                                imgDstData[i, j, l] = imgSrcData[i, j, l];

        				    int calc = (int)((Percentage)*imgBlendData[i, j, l] + (1 - Percentage)*imgSrcData[i, j, l]);
                            if (calc > 255)
        				    {
        				    	imgDstData[i, j, l] = 255;
        				    }
                            else if (calc < 0)
        				    {
        				    	imgDstData[i, j, l] = 0;
        				    }
        				    else
        				    {
                                imgDstData[i, j, l] = (byte)(calc);
        				    }
        				}
        				else
        				{
        					if (!((Algoparam2 == 4 && (imgBlendData[i, j, 0] <= BlacOrWhiteThreshold) && (imgBlendData[i, j, 1] <= BlacOrWhiteThreshold) && (imgBlendData[i, j, 2] <= BlacOrWhiteThreshold))
        						||
        						(Algoparam2 == 3 && (imgBlendData[i, j, 0] >= 255 - BlacOrWhiteThreshold) && (imgBlendData[i, j, 1] >= 255 - BlacOrWhiteThreshold) && (imgBlendData[i, j, 2] >= 255 - BlacOrWhiteThreshold))))
        					{
        					    int calc = (int)((Percentage)*imgBlendData[i, j, l] + (1 - Percentage)*imgSrcData[i, j, l]);
                                if (calc > 255)
        						{
        							imgDstData[i, j, l] = 255;
        						}
                                else if (calc < 0)
        						{
        							imgDstData[i, j, l] = 0;
        						}
        						else
        						{
                                    imgDstData[i, j, l] = (byte)calc;
        						}
        					}
        					else
        						imgDstData[i, j, l] = imgSrcData[i, j, l];
        				}
        			}
        		}
        	}

        	return imgDst;
        }
    }
}
