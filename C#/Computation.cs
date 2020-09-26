using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Diagnostics;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Emgu.CV;
using Emgu.CV.Structure;
using Emgu.CV.CvEnum;
using Emgu.CV.Util;
namespace WpfApplication2.Algorithms
{
    public static class Computation
    {
        /*C L A M P*/
        public static int Clamp(int value, int max)
        {
            int min = 0;
            return (value < min) ? min : (value > max) ? max : value;
        }


        /*P I T A G O R    T H E O R E M*/
        public static double PitagorTheorem(double a, double b)
        {
            return (Math.Sqrt((a * a) + (b * b)));
        }

        /*A B S*/
        public static double Abs(double a)
        {
            if (a < 0)
            {
                return -a;
            }
            else
            {
                return a;
            }
        }

        /*A B S*/
        public static int Abs(int a)
        {
            if (a < 0)
            {
                return -a;
            }
            else
            {
                return a;
            }
        }


        /*M I N*/
        public static int Min(int a, int b)
        {
            return a > b ? b : a;
        }


        /*M I N*/
        public static double Min(double a, double b)
        {
            return a > b ? b : a;
        }


        /*M I N*/
        public static int Max(int a, int b)
        {
            return a > b ? a : b;
        }


        /*M I N*/
        public static double Max(double a, double b)
        {
            return a > b ? a : b;
        }


        /*R O U N D   -  RoundValues to X significant bit*/
        public static double RoundValue_toX_SignificantBits(double value, int X)
        {
            int ValueTimesX = 0;
            double Number = value;

            ValueTimesX = (int)(value * Math.Pow(10, X));
            Number *= Math.Pow(10, X);

            if (Number - ValueTimesX > 0.5)
                ValueTimesX += 1;

            Number = ValueTimesX / Math.Pow(10, X);

            if (Number * Math.Pow(10, X) < ValueTimesX)
                Number += (1 / Math.Pow(10, X));
            return Number;
        }


        /*C O N V O L U T I O N   -  G R A Y image*/
        public static Image<Gray, Byte> Convolution(Image<Gray, Byte> imgSrc, float [] Kernel, int KernelSize)
        {
        	int i, j, n, m;
        	int FinalNum;
        	int devideNumber = (int)(Math.Pow((float)KernelSize, 2));

            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;
            var imgDst = imgSrc.CopyBlank();
            var imgDstData = imgDst.Data;

            int endYpos = height - KernelSize/2;
            int endXpos = width - KernelSize / 2;

            for (i = KernelSize / 2; i < endXpos; i++)
        	{
                for (j = KernelSize / 2; j < endYpos; j++)
        		{
        			int c = 0;
        			FinalNum = 0;
        			for (n = -KernelSize / 2; n <= KernelSize / 2; n++)
        			{
        				devideNumber = 9;
        				for (m = -KernelSize / 2; m <= KernelSize / 2; m++)
        				{
        					FinalNum += (int)(imgSrcData[(j - n), i - m,  0] * Kernel[c]);
        					c++;
        				}
        			}
        		    if (devideNumber <= 0)
        		    {
        		        FinalNum = 0;
        		    }
        		    else
        		    {
        		        FinalNum = (int) FinalNum/devideNumber;
        		    }

        		    if (FinalNum < 0) FinalNum = 0;
        			else if (FinalNum > 255) FinalNum = 255;
        			//if (FinalNum > 128) FinalNum = 255;
        			//else FinalNum = 0;
        
        			imgDstData[j, i, 0] = (byte)FinalNum;
        		}
        	}

            return imgDst;
        }


        /*C O N V O L U T I O N*/
        public static Image<Bgr, Byte> Convolution(Image<Bgr, Byte> imgSrc, float[] Kernel, int KernelSize)
        {
            int i, j, n, m;
            int FinalNum;
            int devideNumber = (int)(Math.Pow((float)KernelSize, 2));

            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;
            var imgDst = imgSrc.CopyBlank();
            var imgDstData = imgDst.Data;

            int endYpos = height - KernelSize / 2;
            int endXpos = width - KernelSize / 2;

            for (i = KernelSize / 2; i < endXpos; i++)
            {
                for (j = KernelSize / 2; j < endYpos; j++)
                {
                    for (int k = 0; k < 3; k++)
                    {
                        int c = 0;
                        FinalNum = 0;
                        for (n = -KernelSize/2; n <= KernelSize/2; n++)
                        {
                            devideNumber = 9;
                            for (m = -KernelSize/2; m <= KernelSize/2; m++)
                            {
                                FinalNum += (int) (imgSrcData[(j - n), i - m, k]*Kernel[c]);
                                c++;
                            }
                        }
                        if (devideNumber <= 0)
                        {
                            FinalNum = 0;
                        }
                        else
                        {
                            FinalNum = (int) FinalNum/devideNumber;
                        }

                        if (FinalNum < 0) FinalNum = 0;
                        else if (FinalNum > 255) FinalNum = 255;

                        imgDstData[j, i, k] = (byte) FinalNum;
                    }
                }
            }

            return imgDst;
        }


        /*C O N V O L U T I O N  -  G I M P*/
        public static Image<Bgr, Byte> ConvolutionGimp(this Image<Bgr, Byte> imgSrc, float[] convoMatrix, int matrixSize)
        {
            int src_x1, src_y1, src_x2, src_y2;

            int DEST_ROWS = matrixSize / 2 + 1;

            int width = imgSrc.Width;
            int height = imgSrc.Height;
            int bpp = 3;
            var imgSrcData = imgSrc.Data;
            int shiftInDirection = matrixSize / 2;

            var imgDst = new Image<Bgr, Byte>(width - (2 * shiftInDirection), height - (2 * shiftInDirection));
            var imgDstData = imgDst.Data;

            byte[, ,] dest_row = new byte[matrixSize, (width - (2 * shiftInDirection)), 3];

            src_x1 = 0;
            src_x2 = imgDst.Width;
            src_y1 = 0;
            src_y2 = imgDst.Height;

            var src_row = GetSetOfRowsFromImage(imgSrc, 0, matrixSize);

            for (int row = src_y1; row < src_y2; row++)
            {
                for (int col = src_x1; col < src_x2; col++)
                {
                    for (int channel = 0; channel < bpp; channel++)
                    {
                        int result;

                        result = (int)Math.Round(convolve_pixel(src_row, col, channel, convoMatrix, matrixSize));
                        result = Clamp(result, 255);

                        dest_row[matrixSize / 2, col, channel] = (byte)result;
                    }
                }
                if (row >= src_y1 + shiftInDirection)
                {
                    SetBufferToArray(ref imgDstData, dest_row, src_x1, row - shiftInDirection, imgDst.Width);
                }

                if (row < src_y2 - 1)
                {
                    var tmp_row = GetRow(dest_row, 0, bpp, imgDst.Width);

                    for (int i = 0; i < DEST_ROWS - 1; i++)
                    {
                        var curRow = GetRow(dest_row, i + 1, bpp, imgDst.Width);
                        SetBufferToArray(ref dest_row, curRow, 0, i, imgDst.Width);
                    }
                    SetBufferToArray(ref dest_row, tmp_row, 0, DEST_ROWS - 1, imgDst.Width);

                    tmp_row = GetRow(src_row, 0, bpp, imgSrc.Width);

                    for (int i = 0; i < matrixSize - 1; i++)
                    {
                        var curRow = GetRow(src_row, i + 1, bpp, width);
                        SetBufferToArray(ref src_row, curRow, 0, i, width);
                    }

                    //SetBufferToArray(ref src_row, tmp_row, 0, matrixSize - 1, imgDst.Width);

                    var srcCurRow = GetSetOfRowsFromImage(imgSrc, row + shiftInDirection + 1, 1);
                    SetBufferToArray(ref src_row, srcCurRow, 0, matrixSize - 1, width);
                }
            }

            for (int i = 1; i < DEST_ROWS; i++)
            {
                var curRow = GetRow(dest_row, i, bpp, imgDst.Width);
                SetBufferToArray(ref imgDstData, curRow, 0, src_y2 + i - 1 - shiftInDirection, imgDst.Width);
                //gimp_pixel_rgn_set_row(&destPR, dest_row[i], src_x1, src_y2 + i - 1 - HALF_WINDOW, src_w);
            }

            return imgDst;
        }

        #region private members
        private static void SetBufferToArray(ref byte[, ,] imgDstData, byte[, ,] dest_row, int startIndex, int outputStartRow, int Width)
        {
            for (int i = startIndex; i < Width; i++)
            {
                for (int k = 0; k < 3; k++)
                {
                    imgDstData[outputStartRow, i, k] = dest_row[0, i, k];
                }
            }
        }

        private static byte[, ,] GetRow(byte[, ,] srcRow, int index, int numOfChannels, int width)
        {
            var dstRow = new byte[1, width, numOfChannels];
            for (int i = 0; i < width; i++)
            {
                for (int k = 0; k < numOfChannels; k++)
                {
                    dstRow[0, i, k] = srcRow[index, i, k];
                }
            }
            return dstRow;
        }

        private static byte[, ,] GetSetOfRowsFromImage(Image<Bgr, Byte> imgSrc, int startRow, int numberOfRows)
        {
            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgData = imgSrc.Data;

            byte[, ,] src_row = new byte[numberOfRows, width, 3];

            int endRow = startRow + numberOfRows;

            int curIndex = 0;
            for (int i = startRow; i < endRow; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    for (int k = 0; k < 3; k++)
                    {
                        src_row[curIndex, j, k] = imgData[i, j, k];
                    }
                }
                curIndex++;
            }

            return src_row;
        }

        private static float convolve_pixel(byte[, ,] src_row, int xElement, int channel, float[] matrix, int matrixSize)
        {
            float sum = 0;

            int x, y;

            for (y = 0; y < matrixSize; y++)
            {
                for (x = 0; x < matrixSize; x++)
                {
                    float temp = matrix[y * matrixSize + x];

                    temp *= src_row[y, xElement + x, channel];
                    sum += temp;
                }
            }
            return sum;
        }

        #endregion


        /*C A L C  -  M E A N  of array - full array or till specific index*/
        public static double MeanOfDataSet(double[] allPoints, int numberOfPts)
        {
            //step 1: calculate mean of X and Y and correlation;
            double meanX, sumX = 0;

            for (int i = 0; i < numberOfPts; i++)
            {
                sumX += allPoints[i];
            }
            meanX = sumX / (double)numberOfPts;

            return meanX;
        }


        /*C A L C  -  C O R R E L A T I O N  of array - full array or till specific index*/
        public static double CorrelationOf2DataSets(double[] dataSet1, double[] dataSet2, int numberOfPts)
        {
            double sumCor_1 = 0, sumCor_2 = 0, sumCor_3 = 0;
            double correlation;

            for (int i = 0; i < numberOfPts; i++)
            {
                sumCor_1 += (dataSet1[i] * dataSet2[i]);
                sumCor_2 += (dataSet1[i] * dataSet1[i]);
                sumCor_3 += (dataSet2[i] * dataSet2[i]);
            }

            correlation = sumCor_1 / (Math.Sqrt(sumCor_2 * sumCor_3));

            return correlation;
        }


        /*C A L C  -  S T. D E V   of array - full array or till specific index*/
        public static double StandardDevOfDataSet(double[] dataSet, int numberOfPts)
        {
            double standardDev;

            var variance = VarianceOfDataSet(dataSet, numberOfPts);
            standardDev = Math.Sqrt(variance);

            return standardDev;
        }


        /*C A L C  -  V A R I A N C E   of array - full array or till specific index*/
        public static double VarianceOfDataSet(double[] dataSet, int numberOfPts)
        {
            double variance;

            var mean = MeanOfDataSet(dataSet, numberOfPts);

            double VarianceX;
            
            double sum = 0;

            for (int j = 0; j < numberOfPts; j++)
            {
                sum += ((dataSet[j] - mean) * (dataSet[j] - mean));
            }

            variance = sum / (double)(numberOfPts - 1);
                
            return variance;
        }


        /*C A L C  -  C O - V A R I A N C E   of array - full array or till specific index*/
        public static double CovarianceOf2DataSets(double[] dataSet1, double[] dataSet2, int numberOfPts)
        {
            double variance1, variance2, covariance;

            var mean1 = MeanOfDataSet(dataSet1, numberOfPts);
            var mean2 = MeanOfDataSet(dataSet2, numberOfPts);
                
            double sum = 0;

            for (int j = 0; j < numberOfPts; j++)
            {
                sum += ((dataSet1[j] - mean1) * (dataSet2[j] - mean2));
            }

            covariance = sum / (double)(numberOfPts - 1);

            return covariance;
        }


        /*C O V A R I A N C E    M A T R I X*/
        public static double[] CovarianceMatrix2DataSets(double[] dataSet1, double[] dataSet2, int numberOfPts)
        {
            double [] CovarianceMatrix = new double[4];
            CovarianceMatrix[0] = CovarianceOf2DataSets(dataSet1, dataSet1, numberOfPts);
            CovarianceMatrix[1] = CovarianceOf2DataSets(dataSet1, dataSet2, numberOfPts);
            CovarianceMatrix[2] = CovarianceMatrix[1];
            CovarianceMatrix[3] = CovarianceOf2DataSets(dataSet2, dataSet2, numberOfPts);

            return CovarianceMatrix;
        }


        public static double [] CovarianceMatrix(List<double[]> dataSets, int numberOfPts)
        {
            int numberOfDataSest = dataSets.Count();

            double [] CovarianceMatrix = new double[numberOfDataSest * numberOfDataSest];

            int initPos = 0;
            for(int i = 0; i < numberOfDataSest; i++)
            {
                for (int j = 0; j < numberOfDataSest; j++)
                {
                    CovarianceMatrix[i * numberOfDataSest + j] = CovarianceOf2DataSets(dataSets[i], dataSets[j], numberOfPts);
                }
            }

            return CovarianceMatrix;
        }


        /*Co- F A C T O R  expansion - sign calculation*/
        public static int CalcSign(int matrixSize, int index)
        {
            int sign = 1;

            int row = index / matrixSize;
            int col = index - (matrixSize * row);

            if(row % 2 == 1)
            {
                if(col % 2 == 1)
                {
                    sign = 1;
                }
                else
                {
                    sign = -1;
                }
            }
            else
            {
                if (col % 2 == 1)
                {
                    sign = -1;
                }
                else
                {
                    sign = 1;
                }
            }

            return sign;
        }


        /*Co - F A C T O R  expansion*/
        public static int CofactorExpansion(int matrixSize, int index)
        {
            return 0;
        }


        /*D E T E R M I N A N T   of   M A T R I X   ---  2x2*/
        public static double DeterminantOfMatrix2x2(double[] matrix)
        {
            double determinant = (matrix[0] * matrix[3]) - (matrix[1] * matrix[2]);
            return determinant;
        }


        ///*D E T E R M I N A N T   of   M A T R I X*/
        //public static double DeterminantOfMatrix(double[] matrix)
        //{
        //    int sizeOfMatrix = Math.Sqrt(matrix.Count());
        //    return 0;
        //}
        //
        //
        ///*L O W E R  matrix  D I M E N S I O N S*/
        //public static double [] LowerMatrixDimensions(double[] matrix)
        //{
        //    
        //}


        /*I D E N T I T Y   M A T R I X*/
        public static double [] IdentityfMatrix(int matrixSize)
        {
            double[] identityMatrix = new double[matrixSize * matrixSize];

            int initPos = 0;
            for(int i = 0; i < matrixSize; i++)
            {
                identityMatrix[initPos] = 1;
                initPos += (matrixSize + 1);
            }

            return identityMatrix;
        }


        ///*E I G E N  V E C T O R S*/
        //public static double[] EigenVectors(double[] squareMatrix)
        //{
        //    int sizeOfMatrix = Math.Sqrt(squareMatrix.Count());
        //
        //}


        /*S T A N D A R D  -  D E V and C O R R E L A T I O N*/
        public static void CalculateStDevAndCorrelation(int[,] allPointsXY, out double stDevX, out double stDevY, out double correlation, int numberOfPts)
        {
            //formulas:
            //http://onlinestatbook.com/2/describing_bivariate_data/calculation.html
            //http://onlinestatbook.com/2/summarizing_distributions/variability.html

            stDevX = 0;
            stDevY = 0;
            correlation = 0;
            //step 1: calculate mean of X and Y and correlation;
            double meanX, meanY, sumX = 0, sumY = 0;
            double sumCor_1 = 0, sumCor_2 = 0, sumCor_3 = 0;
            for (int i = 0; i < numberOfPts; i++)
            {
                sumX += allPointsXY[i, 0];
                sumY += allPointsXY[i, 1];

                sumCor_1 += (allPointsXY[i, 0] * allPointsXY[i, 1]);
                sumCor_2 += (allPointsXY[i, 0] * allPointsXY[i, 0]);
                sumCor_3 += (allPointsXY[i, 1] * allPointsXY[i, 1]);
            }
            meanX = sumX / (double)numberOfPts;
            meanY = sumY / (double)numberOfPts;
            correlation = sumCor_1 / (Math.Sqrt(sumCor_2 * sumCor_3));

            // step 2: calculate variance of X and Y;
            double VarianceX, VarianceY;
            sumX = 0;
            sumY = 0;
            for (int j = 0; j < numberOfPts; j++)
            {
                sumX += ((allPointsXY[j, 0] - meanX) * (allPointsXY[j, 0] - meanX));
                sumY += ((allPointsXY[j, 1] - meanY) * (allPointsXY[j, 1] - meanY));
            }
            VarianceX = sumX / (double)(numberOfPts - 1);
            VarianceY = sumY / (double)(numberOfPts - 1);

            //step 3: calc stDev
            stDevX = meanX;//VarianceX * VarianceX;
            stDevY = meanY;//VarianceY * VarianceY;
        }
    }
}
