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
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WpfApplication2.Algorithms
{
    public static class Transforms
    {
        public static int FIXED_SHIFT = 10;
        public static int FIXED_UNIT = 1 << FIXED_SHIFT;

        public static int DOUBLE2FIXED(double val)
        {
            return (int)val * FIXED_UNIT;
        }

        public static int RINT(double val)
        {
            return (int)Math.Floor(val + 0.5);
        }

        private static int lerp(int v1, int v2, int r)
        {
            return (((int)(v1) * (FIXED_UNIT - (int)(r)) +
                  (int)(v2) * (int)(r)) >> FIXED_SHIFT);
        }


        private class Matrix3x3Float
        {
            public float[,] Elem;

            public Matrix3x3Float(float a0, float a1, float a2, float b0, float b1, float b2, float c0, float c1, float c2)
            {
                Elem = new float[3, 3];

                Elem[0, 0] = a0;
                Elem[0, 1] = a1;
                Elem[0, 2] = a2;
                Elem[1, 0] = b0;
                Elem[1, 1] = b1;
                Elem[1, 2] = b2;
                Elem[2, 0] = c0;
                Elem[2, 1] = c1;
                Elem[2, 2] = c2;
            }

            public Matrix3x3Float(double a0, double a1, double a2, double b0, double b1, double b2, double c0, double c1, double c2)
            {
                Elem = new float[3, 3];

                Elem[0, 0] = (float)a0;
                Elem[0, 1] = (float)a1;
                Elem[0, 2] = (float)a2;
                Elem[1, 0] = (float)b0;
                Elem[1, 1] = (float)b1;
                Elem[1, 2] = (float)b2;
                Elem[2, 0] = (float)c0;
                Elem[2, 1] = (float)c1;
                Elem[2, 2] = (float)c2;
            }
        }

        public static Image<Bgr, Byte> RotateCubic(this Image<Bgr, Byte> imgSrc, double angle)
        {
            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;
            var imgDst = new Image<Bgr, Byte>(width, height);
            var imgDstData = imgDst.Data;

            var matrix = GenerateMatrixFloatForRotation(angle);

            var R = new byte[width * height];
            var G = new byte[width * height];
            var B = new byte[width * height];

            SplitBufferToChannels(imgSrcData, height, width, ref R, ref G, ref B);

            for (int i = 0; i < height; i++)
            {
                // Get4XWidthDataFromBuffer(imgSrcData, i, width, height, ref R, ref G, ref B);

                for (int j = 0; j < width; j++)
                {
                    var currentPixel = CubicRotate(matrix, j, i, width, height, R, G, B);

                    for (int k = 0; k < 3; k++)
                    {
                        imgDstData[i, j, k] = currentPixel[k];
                    }
                }
            }
            return imgDst;
        }

        private static Matrix3x3Float GenerateMatrixFloatForRotation(double angle)
        {
            double angleInRadians = angle * Math.PI / 180.0;
            double cos = Math.Cos(angleInRadians);
            double sin = Math.Sin(angleInRadians);

            var matrix = new Matrix3x3Float(cos, sin, 0, -sin, cos, 0, 0, 0, 1);

            return matrix;
        }

        private static void SplitRowFromBufferToChannels(byte[, ,] imgSrcData, int i, int width, ref byte [] R, ref byte [] G, ref byte [] B)
        {
            for (int j = 0; j < width; j++)
            {
                R[j] = imgSrcData[i, j, 2];
                G[j] = imgSrcData[i, j, 1];
                B[j] = imgSrcData[i, j, 0];
            }
        }

        private static void SplitBufferToChannels(byte[, ,] imgSrcData, int height, int width, ref byte[] R, ref byte[] G, ref byte[] B)
        {
            for(int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    int index = i*width + j;
                    R[index] = imgSrcData[i, j, 2];
                    G[index] = imgSrcData[i, j, 1];
                    B[index] = imgSrcData[i, j, 0];
                }
            }
        }

        private static void Get4XWidthDataFromBuffer(byte[, ,] buffer, int indY, int width, int height, ref byte [] R, ref byte [] G, ref byte [] B)
        {
            int startY = indY - 1;
            if (startY < 0)
            {
                startY = 0;
            }
            int endY = startY + 4;
            if (endY > height - 1)
            {
                endY = height - 1;
            }

            int curRow = 0;

            for (int i = startY; i < endY; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    int index = curRow*width + j;

                    R[index] = buffer[i, j, 2];
                    G[index] = buffer[i, j, 1];
                    B[index] = buffer[i, j, 0];
                }
                curRow++;
            }
        }

        private static byte [] CubicRotate(Matrix3x3Float matrix, int x, int y, int width, int height, byte [] R, byte [] G, byte [] B)
        {
            var Pixel = new byte[3];

            int i, pp;
            double t, u;
            double kx, ky;

            kx = matrix.Elem[0, 0]*x + matrix.Elem[0, 1]*y + matrix.Elem[0, 2]*1;
            ky = matrix.Elem[1, 0]*x + matrix.Elem[1, 1]*y + matrix.Elem[1, 2]*1;
            
            int maxx, maxy, i1, i2, ii, coord, rxx, ryy;
            var val = new int[4, 3];
            var c = new int[4, 3];
            
            u = kx - Math.Floor(kx);
            t = ky - Math.Floor(ky);

            maxx = (int)(width*matrix.Elem[0, 0]);
            maxy = (int)(height*matrix.Elem[1, 1]);

            rxx = (int)Math.Floor(kx);
            ryy = (int)Math.Floor(ky);


            for (i = -1; i < 3; i++)
            {
                for (ii = -1; ii < 3; ii++)
                {
                    i1 = i;
                    i2 = ii;

                    if (ryy == 0 && ii == -1) // in case the first row gets clipped   OK
                    {
                        i2 = 0;
                    }
                    if (ryy == maxy - 1 && (ii == 2 || ii == 1)) // in case the third and fourth rows get clipped
                    {
                        i2 = 0;
                    }
                    if (ryy == maxy - 2 && ii == 2) // in case the third and fourth rows get clipped
                    {
                        i2 = 0;
                    }
                    if (rxx == 0 && i == -1) // in case the first column gets clipped    OK
                    {
                        i1 = 0;
                    }
                    if (rxx == maxx - 1 && (i == 2 || i == 1)) // in case the third and fourth columns get clipped
                    {
                        i1 = 0;
                    }
                    if (rxx == maxx - 2 && i == 2) // in case the third and fourth rows get clipped
                    {
                        i1 = 0;
                    }

                    coord = ryy*(int) (width) + i2*(int) (width) + rxx + i1;
                    
                    if ((coord > (width * height - 1) || coord < 0))
                    { 
                        Pixel[0] = 0;
                        Pixel[1] = 0;
                        Pixel[2] = 0;
                        return Pixel;
                    }

                    c[ii + 1, 0] = B[coord];
                    c[ii + 1, 1] = G[coord];
                    c[ii + 1, 2] = R[coord];
                }

                for (pp = 0; pp < 3; pp++)
                {
                    val[i + 1, pp] = RINT((t*(t*(t*(c[3, pp] - 3*c[2, pp] + 3*c[1, pp] - c[0, pp]) +
                                            (-c[3, pp] + 4*c[2, pp] - 5*c[1, pp] + 2*c[0, pp])) +
                                            (c[2, pp] - c[0, pp])) + 2*c[1, pp])/2.0);
                }
            }
            
            for (pp = 0; pp < 3; pp++)
            {
                var curVal = RINT((u*(u*(u*(val[3, pp] - 3*val[2, pp] + 3*val[1, pp] - val[0, pp]) +
                                 (-val[3, pp] + 4*val[2, pp] - 5*val[1, pp] + 2*val[0, pp])) +
                                 (val[2, pp] - val[0, pp])) + 2*val[1, pp])/2.0);

                curVal = Computation.Clamp(curVal, 255);
                
                Pixel[pp] = (byte)curVal;
            }

            return Pixel;
        }


        /*R O T A T E - 180*/
        public static Image<TColor, TDepth> Rotate180<TColor, TDepth>(this Image<TColor, TDepth> img)
            where TColor : struct, IColor
            where TDepth : new()
        {
            var imgRotated = img.CopyBlank();
            
            using (var imgTransposed = new Image<TColor, TDepth>(img.Height, img.Width))
            {
                CvInvoke.cvTranspose(img.Ptr, imgTransposed.Ptr);
                using (var imgTransposeRotated = imgTransposed.Flip(FLIP.VERTICAL))
                {
                    CvInvoke.cvTranspose(imgTransposeRotated.Ptr, imgRotated.Ptr);
                    imgRotated = imgRotated.Flip(FLIP.VERTICAL);
                }
            }

            return imgRotated;
        }


        /*R O T A T E  -  I M A G E*/
        public static Image<Bgr, Byte> RotateImage(this Image<Bgr, Byte> imgSrc, double correctionAngle)
        {
            if (correctionAngle != 0)
            {
                Matrix<float> rot_mat = new Matrix<float>(2, 3);
                Bgr ui = new Bgr(0, 0, 0);
                Point center = new Point(imgSrc.Width / 2, imgSrc.Height / 2);

                rot_mat.Ptr = CvInvoke.cv2DRotationMatrix(center, correctionAngle, 1, rot_mat.Ptr);
                var imgRot = imgSrc.WarpAffine(rot_mat, INTER.CV_INTER_CUBIC, WARP.CV_WARP_DEFAULT, ui);

                return imgRot;
            }
            else
            {
                return imgSrc;
            }
        }


        /*M I R R O R  image - vertical*/
        public static Image<Bgr, Byte> MirrorImageVertical(Image<Bgr, Byte> imgSrc)
        {
        	int i, j, l;

            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;
            var imgDst = imgSrc.CopyBlank();
            var imgDstData = imgDst.Data;

        	for (i = 0; i < height; i++)
        	{
        		for (j = 0; j < width; j++)
        		{
        			for (l = 0; l < 3; l++)
        			{
        				imgDstData[i, width - 1 - j, l] = imgSrcData[i, j, l];
        			}
        		}
        	}
        
        	return imgDst;
        }


        /*M I R R O R  image - horizontal*/
        public static Image<Bgr, Byte> MirrorImageHorizontal(Image<Bgr, Byte> imgSrc)
        {
            int i, j, l;

            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;
            var imgDst = imgSrc.CopyBlank();
            var imgDstData = imgDst.Data;

            for (i = height - 1; i >= 0; i++)
            {
                for (j = 0; j < width; j++)
                {
                    for (l = 0; l < 3; l++)
                    {
                        imgDstData[height - 1 - i, j, l] = imgSrcData[i, j, l];
                    }
                }
            }

            return imgDst;
        }


        /*M O R P H O L O G Y  -  Dilation*/
        public static Image<Gray, Byte> MorphDilate(this Image<Gray, Byte>imgSrc, int ElementSize, int NumberOfIterations)
        {
        	int i, j, l, k, z;
        	//float StructureElement[9] = { 0, 0, 0, 0, 1, 0, 0, 0, 0 };
        	float [] StructureElement = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };
        
        	if (ElementSize < 3) ElementSize = 3;
        	if (NumberOfIterations < 1) NumberOfIterations = 1;

        	var imgDst = Computation.Convolution(imgSrc, StructureElement, ElementSize);
        
        	if (NumberOfIterations % 2 == 0)
        	{
        		if (NumberOfIterations > 0 && NumberOfIterations % 2 == 0)
        		{
        			NumberOfIterations -= 1;
        			if (NumberOfIterations != 0) imgDst = imgSrc.MorphDilate(ElementSize, NumberOfIterations);
        			return imgDst;
        		}
        	}
        	else
        	{
        		if (NumberOfIterations > 0 && NumberOfIterations % 2 == 1)
        		{
        			NumberOfIterations -= 1;
        			if (NumberOfIterations != 0) imgSrc = imgDst.MorphDilate(ElementSize, NumberOfIterations);
        			return imgSrc;
        		}
        	}
            return imgDst;
        }
        

        /*M O R P H O L O G Y  -  Erosion*/
        public static Image<Gray, Byte> MorphErode(this Image<Gray, Byte>imgSrc, int ElementSize, int NumberOfIterations)
        {
        	int i, j, l, k, z;
        	//float StructureElement[9] = { 0, 0, 0, 0, 1, 0, 0, 0, 0 };
        	float [] StructureElement = { 0, 1, 0, 1, 1, 1, 0, 1, 0 };
        
        	if (ElementSize < 3) ElementSize = 3;
        	if (NumberOfIterations < 1) NumberOfIterations = 1;

        	var imgDst = Computation.Convolution(imgSrc, StructureElement, ElementSize);
 
        	if (NumberOfIterations % 2 == 0)
        	{
        		if (NumberOfIterations > 0 && NumberOfIterations % 2 == 0)
        		{
        			NumberOfIterations -= 1;
        			if(NumberOfIterations != 0) imgDst = imgSrc.MorphErode(ElementSize, NumberOfIterations);
        			return imgDst;
        		}
        	}
        	else
        	{
        		if (NumberOfIterations > 0 && NumberOfIterations % 2 == 1)
        		{
        			NumberOfIterations -= 1;
        			if (NumberOfIterations != 0) imgSrc = imgDst.MorphErode(ElementSize, NumberOfIterations);
        			return imgSrc;
        		}
        	}
            return imgDst;
        }
        
        

        /*S C A L E   to XY*/
        public static Image<Gray, Byte> ScaleImageToXY(this Image<Gray, Byte> imgSrc, int newWidth, int newHeight)
        {
            int i, j;
            int newX = 0;
            int newY = 0;

            float ScalePercentageX;
            float ScalePercentageY;

            var Img_dst = new Image<Gray, Byte>(newWidth, newHeight);
            /* Modify Img_dst */

            ScalePercentageX = 100 - imgSrc.Width * 100 / newWidth;//Img_src->Width / (float) Img_dst->Width;
            ScalePercentageY = 100 - imgSrc.Height * 100 / newHeight;

            int height = Img_dst.Height;
            int width = Img_dst.Width;
            int srcHeight = imgSrc.Height;
            int srcWidth = imgSrc.Width;
            var imgSrcData = imgSrc.Data;
            var imgDstData = Img_dst.Data;
            for (i = 0; i < height; i++)
            {
                for (j = 0; j < width; j++)
                {
                    //if(ScalePercentageX< 0)
                    newX = (int)(0.5 + j * (1 - (ScalePercentageX / 100.0)));
                    //else NewX = j / (1 + (ScalePercentageX / 100.0));

                    //if(ScalePercentageY< 0)
                    newY = (int)(0.5 + i * (1 - (ScalePercentageY / 100.0)));
                    //else NewY = i / (1 + (ScalePercentageY / 100.0));

                    if (newX < 0) newX = 0;
                    if (newY < 0) newY = 0;
                    if (newX >= srcWidth) newX = srcWidth - 1;
                    if (newY >= srcHeight) newY = srcHeight - 1;

                    imgDstData[i, j, 0] = imgSrcData[newY, newX, 0];
                }
            }

            return Img_dst;
        }


        /*A F F I N E  - scale (zoom) image - in/out */
        public static Image<Bgr, Byte> ScaleImageToXY(this Image<Bgr, Byte> imgSrc, int newWidth, int newHeight)
        {
            int i, j, z;
            int newX = 0;
            int newY = 0;

            float ScalePercentageX;
            float ScalePercentageY;

            var Img_dst = new Image<Bgr, Byte>(newWidth, newHeight);
            /* Modify Img_dst */

            ScalePercentageX = 100 - imgSrc.Width * 100 / newWidth;//Img_src->Width / (float) Img_dst->Width;
            ScalePercentageY = 100 - imgSrc.Height * 100 / newHeight;

            int height = Img_dst.Height;
            int width = Img_dst.Width;
            int srcHeight = imgSrc.Height;
            int srcWidth = imgSrc.Width;
            var imgSrcData = imgSrc.Data;
            var imgDstData = Img_dst.Data;
            for (i = 0; i < height; i++)
            {
                for (j = 0; j < width; j++)
                {
                    //if(ScalePercentageX< 0)
                    newX = (int)(0.5 + j * (1 - (ScalePercentageX / 100.0)));
                    //else NewX = j / (1 + (ScalePercentageX / 100.0));

                    //if(ScalePercentageY< 0)
                    newY = (int)(0.5 + i * (1 - (ScalePercentageY / 100.0)));
                    //else NewY = i / (1 + (ScalePercentageY / 100.0));

                    if (newX < 0) newX = 0;
                    if (newY < 0) newY = 0;
                    if (newX >= srcWidth) newX = srcWidth - 1;
                    if (newY >= srcHeight) newY = srcHeight - 1;

                    for (z = 0; z < 3; z++)
                    {
                        imgDstData[i, j, z] = imgSrcData[newY, newX, z];
                    }
                }
            }

            return Img_dst;
        }


        /*S C A L E  -  NN  - GIMP function*/
        public static Image<Bgr, Byte> ScaleNN(this Image<Bgr, Byte> imgSrc, int newWidth, int newHeight)
        {
            Image<Bgr, Byte> imgDst = new Image<Bgr, Byte>(newWidth, newHeight);
            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;
            var imgDstData = imgDst.Data;

            double xRatio = width / (double)newWidth;
            double yRatio = height / (double)newHeight;

            double tu, tv, tw;   /* undivided source coordinates and divisor */

            /* set up inverse transform steps */
            double EPSILON = 0.00001;
            for (int i = 0; i < newHeight; i++)
            {
                tu = xRatio * (0 + .5);
                tv = yRatio * (i + .5);
                tw = 1;

                int j = 0;
                int wid = newWidth;
                while (wid-- > 0)
                {
                    double u, v; /* source coordinates */
                    int iu, iv;

                    /*  normalize homogeneous coords  */
                    normalize_coords(tu, tv, tw, out u, out v);

                    iu = (int)Math.Floor(u + 0.5 + EPSILON);
                    iv = (int)Math.Floor(v + 0.5 + EPSILON);

                    for (int k = 0; k < 3; k++)
                    {
                        imgDstData[i, j, k] = imgSrcData[iv, iu, k];
                    }

                    j++;
                    tu += xRatio;
                }
            }

            return imgDst;
        }


        /*N O R M A L I Z E   -   coords*/
        private static void normalize_coords(double tu, double tv, double tw, out double u, out double v)
        {
            u = tu / tw - 0.5;
            v = tv / tw - 0.5;
        }
    }
}
