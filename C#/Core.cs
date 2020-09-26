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
    public static class Core
    {
        /*C O P Y  - 3 channels*/
        public static Image<Bgr, Byte> CopytTo3Channels(this Image<Gray, byte> imgSrc)
        {
            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;
            var imgDst = new Image<Bgr, Byte>(width, height);
            var imgDstData = imgDst.Data;

            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    imgDstData[i, j, 0] = imgSrcData[i, j, 0];
                    imgDstData[i, j, 1] = imgSrcData[i, j, 0];
                    imgDstData[i, j, 2] = imgSrcData[i, j, 0];
                }
            }
            return imgDst;
        }


        /*S A V E  -  to  - P N G*/
        public static void SavePng<TColor, TDepth>(this Image<TColor, TDepth> img, string filename, double quality)
            where TColor : struct, IColor
            where TDepth : new()
        {
            var encoderParams = new EncoderParameters(1);
            encoderParams.Param[0] = new EncoderParameter(
                System.Drawing.Imaging.Encoder.Quality,
                (long)quality
                );

            var pngCodec = (from codec in ImageCodecInfo.GetImageEncoders()
                            where codec.MimeType == "image/png"
                            select codec).Single();

            img.Bitmap.Save(filename, pngCodec, encoderParams);
        }

        /* C O P Y  -  R O I */
        private static Image<TColor, TDepth> CopyRoi<TColor, TDepth>(this Image<TColor, TDepth> img, Rectangle roi)
            where TColor : struct, IColor
            where TDepth : new()
        {
            var imgSubRect = img.GetSubRect(roi);
            Image<TColor, TDepth> result = imgSubRect.Copy();

            return result;
        }

        /* S A F E  _  C O P Y  -  R O I */
        public static Image<TColor, TDepth> SafeCopyRoi<TColor, TDepth>(this Image<TColor, TDepth> img, Rectangle roi)
            where TColor : struct, IColor
            where TDepth : new()
        {
            Image<TColor, TDepth> result = null;

            if ((roi.X >= 0) &&
                (roi.Y >= 0) &&
                (roi.Width > 0) &&
                (roi.Height > 0) &&
                (img.Width >= roi.X + roi.Width) &&
                (img.Height >= roi.Y + roi.Height))
            {
                result = img.CopyRoi(roi);
            }

            return result;
        }


        /*D R A W  -  C O N T O U R*/
        public static void DrawContour<TColor, TDepth>(this Image<TColor, TDepth> img, Contour<Point> contour, TColor color, int thickness)
            where TColor : struct, IColor
            where TDepth : new()
        {
            var poly = contour.Select(p => new Point((int)p.X, (int)p.Y)).ToArray();
            img.DrawPolyline(poly, true, color, thickness);
        }


        /*C R O P  image*/
        public static Image<Bgr, Byte> CropImage(Image<Bgr, Byte> imgSrc, Rectangle Rect)
        {
            int i, j, l, k, z;
            int NewWidth;
            int NewHeight;

            NewWidth = Rect.Width;
            NewHeight = Rect.Height;

            if(imgSrc.Width < NewWidth)
            {
                NewWidth = imgSrc.Width;
            }
            if(imgSrc.Height < NewHeight)
            {
                NewHeight = imgSrc.Height;
            }

            if (imgSrc.Data == null) return imgSrc;

            Image<Bgr, Byte> imgDst = new Image<Bgr, Byte>(NewWidth, NewHeight);

            var imgSrcData = imgSrc.Data;
            var imgDstData = imgDst.Data;

            int channels = imgSrc.NumberOfChannels;
            k = Rect.Y;

            for (i = 0; i < NewHeight; i++)
            {
                z = Rect.X;

                for (j = 0; j < NewWidth; j++)
                {
                    for (l = 0; l < channels; l++)
                    {
                        imgDstData[i, j, l] = imgSrcData[i + k, j + z, l];
                    }
                }
            }

            return imgDst;
        }


        /*C R O P  image*/
        public static Image<Gray, Byte> CropImage(Image<Gray, Byte> imgSrc, Rectangle Rect)
        {
            int i, j, k, z;
            int NewWidth;
            int NewHeight;
            NewWidth = Rect.Width;
            NewHeight = Rect.Height;
            if (imgSrc.Data == null) return imgSrc;

            Image<Gray, Byte> imgDst = new Image<Gray, Byte>(NewWidth, NewHeight);

            int channels = imgSrc.NumberOfChannels;
            k = Rect.Y;

            for (i = 0; i < NewHeight; i++)
            {
                z = Rect.X;

                for (j = 0; j < NewWidth; j++)
                {
                    imgDst.Data[i, j, 0] = imgSrc.Data[i + k, j + z, 0];
                }
            }

            return imgDst;
        }


        /*C R O P  -  R E G I O N S  - and  M E R G E*/
        public static Image<Bgr, Byte> CropRegionsFromImageAndMerge(this Image<Bgr, Byte> imgSrc, List<Rectangle> rectAreas)
        {
            /* The regions should have the same width and to start from the same X pos*/
            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;

            int dstWidth = 0;
            int dstHeight = 0;
            int numberOfRegions = 0;
            foreach (var rectArea in rectAreas)
            {
                dstWidth = rectArea.Width;
                dstHeight += rectArea.Height;
                numberOfRegions++;
            }

            var imgDst = new Image<Bgr, Byte>(dstWidth, dstHeight + numberOfRegions);
            var imgDstData = imgDst.Data;

            int curY = 0;
            foreach (var rectArea in rectAreas)
            {
                int startX = rectArea.X;
                int startY = rectArea.Y;
                int areaWidth = rectArea.Width;
                int areaHeight = rectArea.Height;
                int endY = startY + areaHeight;
                int endX = startX + areaWidth;

                for (int i = startY; i < endY; i++)
                {
                    int curX = 0;
                    for (int j = startX; j < endX; j++)
                    {
                        imgDstData[curY, curX, 0] = imgSrcData[i, j, 0];
                        imgDstData[curY, curX, 1] = imgSrcData[i, j, 1];
                        imgDstData[curY, curX, 2] = imgSrcData[i, j, 2];
                        curX++;
                    }
                    curY++;
                }
                int curX2 = 0;
                for (int j = startX; j < endX; j++)
                {
                    imgDstData[curY, curX2, 0] = 0;
                    imgDstData[curY, curX2, 1] = 0;
                    imgDstData[curY, curX2, 2] = 0;
                    curX2++;
                }
                curY++;
            }

            return imgDst;
        }


        /*N O R M A L I Z E  - pass maxMin difference which could be calculated using GetRange() method - (inplace)*/
        public static void _NormalizeImageUsingMaxMinDiff(this Image<Gray, Byte> imgSrc, int maxDiff)
        {
            double factor = 255 / (double)maxDiff;

            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgData = imgSrc.Data;

            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    int number = (int)(0.5 + factor * imgData[y, x, 0]);
                    if (number > 255)
                    {
                        number = 255;
                    }
                    imgData[y, x, 0] = (byte)(number);
                }
            }
        }


        /*N O R M A L I Z E  - pass maxMin difference which could be calculated using GetRange() method*/
        public static Image<Gray,Byte> NormalizeImageUsingMaxMinDiff(this Image<Gray, Byte> imgSrc, int maxDiff)
        {
            double factor = 255 / (double)maxDiff;

            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgData = imgSrc.Data;

            var imgDst = imgSrc.Clone();
            var imgDstData = imgDst.Data;

            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    int number = (int)(0.5 + factor * imgData[y, x, 0]);
                    if (number > 255)
                    {
                        number = 255;
                    }
                    imgDstData[y, x, 0] = (byte)(number);
                }
            }

            return imgDst;
        }


        /*E X T R A C T  single  C H A N N E L*/
        public static Image<Gray, Byte> ExtractSingleChannel(this Image<Bgr, Byte> imgSrc, int channelNumber)
        {
            int width = imgSrc.Width;
            int height = imgSrc.Height;

            var imgSrcData = imgSrc.Data;

            var imgDst = new Image<Gray, byte>(width, height);
            var imgDstData = imgDst.Data;

            if (channelNumber < 0 || channelNumber > 2)
            {
                channelNumber = 0;
            }

            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    imgDstData[i, j, 0] = imgSrcData[i, j, channelNumber];
                }
            }

            return imgDst;
        }

    }
}
