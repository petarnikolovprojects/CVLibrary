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
    public static class EmbossPrivate
    {
        private struct EmbossFilter
        {
            public double Lx;
            public double Ly;
            public double Lz;
            public double Nz;
            public double Nz2;
            public double NzLz;
            public double bg;
        }


        /*E M B O S S - process single row*/
        private static void EmbossRow(byte[] src, byte[] texture, byte[] dst, int width, EmbossFilter Filter)
        {
            byte[,] s = new byte[3, 3 * width];
            double[,] M = new double[3, 3];
            int x, bytes;

            /* mung pixels, avoiding edge pixels */
            for (int i = 0; i < 3 * width; i++)
            {
                s[0, i] = src[i];
                s[1, i] = src[(3 * width) + i];
                s[2, i] = src[2 * (3 * width) + i];
                //texture[i] = src[2 * (3 * width) + i];
            }

            bytes = 3;
            int k = 0;

            for (x = 1; x < width - 2; x++)
            {
                double a;
                long Nx, Ny, NdotL;
                int shade, b;
                int i, j;

                for (i = 0; i < 3; i++)
                    for (j = 0; j < 3; j++)
                        M[i, j] = 0.0;

                for (b = 0; b < bytes; b++)
                {
                    for (i = 0; i < 3; i++)
                    {
                        for (j = 0; j < 3; j++)
                        {
                            a = 1.0;

                            M[i, j] += a * s[i, (x + j) * 3 + b];
                        }
                    }
                }

                Nx = (long)(M[0, 0] + M[1, 0] + M[2, 0] - M[0, 2] - M[1, 2] - M[2, 2]);
                Ny = (long)(M[2, 0] + M[2, 1] + M[2, 2] - M[0, 0] - M[0, 1] - M[0, 2]);

                /* shade with distant light source */
                if (Nx == 0 && Ny == 0)
                {
                    shade = (int)Filter.bg;
                }
                else if ((NdotL = (long)(Nx * Filter.Lx + Ny * Filter.Ly + Filter.NzLz)) < 0)
                {
                    shade = 0;
                }
                else
                {
                    shade = (int)(NdotL / Math.Sqrt(Nx * Nx + Ny * Ny + Filter.Nz2));
                }

                /* do something with the shading result */

                //int text;
                for (b = 0; b < bytes; b++)
                {
                    //text = ((texture[k] * shade) >> 8);  // uncomment this line and you will get BUMPMAP effect
                    if (shade > 255) shade = 255;
                    if (shade < 0) shade = 0;
                    dst[k] = (byte)shade; // = text
                    k++;
                }
            }
        }


        /*E M B O S S  - core function*/
        private static void ProcessEmboss(Image<Bgr, byte> imgSrc, Image<Bgr, byte> imgDst, double azimuth, double elevation, double depth)
        {
            EmbossFilter filter;
            double pixelScale = 255.9; // constant in GIMP code
            elevation = elevation * 3.14 / 180.0;
            azimuth = azimuth * 3.14 / 180.0;
            /*
            * compute the light vector from the input parameters.
            * normalize the length to pixelScale for fast shading calculation.
            */
            filter.Lx = Math.Cos(azimuth) * Math.Cos(elevation) * pixelScale;
            filter.Ly = Math.Sin(azimuth) * Math.Cos(elevation) * pixelScale;
            filter.Lz = Math.Sin(elevation) * pixelScale;

            /*
             * constant z component of image surface normal - this depends on the
             * image slope we wish to associate with an angle of 45 degrees, which
             * depends on the width of the filter used to produce the source image.
             */
            filter.Nz = (6 * 255) / depth;
            filter.Nz2 = filter.Nz * filter.Nz;
            filter.NzLz = filter.Nz * filter.Lz;

            /* optimization for vertical normals: L.[0 0 1] */
            filter.bg = filter.Lz;

            var imgSrcData = imgSrc.Data;
            var imgDstData = imgDst.Data;
            int height = imgSrc.Rows;
            int width = imgSrc.Cols;
            byte[] srcbuf = new byte[3 * 3 * width]; // 3 rows * columns * RGB
            byte[] texture = new byte[3 * width];
            byte[] dstbuf = new byte[3 * width];

            for (int y = 1; y < height - 2; y++)
            {
                int k = 0;
                //copy row data to srcbuf
                for (int s = 0; s < width; s++)
                {
                    texture[k] = imgSrcData[y, s, 0];
                    srcbuf[k] = imgSrcData[y - 1, s, 0];
                    srcbuf[k + 3 * width] = imgSrcData[y, s, 0];
                    srcbuf[k + 2 * 3 * width] = imgSrcData[y + 1, s, 0];
                    k++;

                    texture[k] = imgSrcData[y, s, 1];
                    srcbuf[k] = imgSrcData[y - 1, s, 1];
                    srcbuf[k + 3 * width] = imgSrcData[y, s, 1];
                    srcbuf[k + 2 * 3 * width] = imgSrcData[y + 1, s, 1];
                    k++;

                    texture[k] = imgSrcData[y, s, 2];
                    srcbuf[k] = imgSrcData[y - 1, s, 2];
                    srcbuf[k + 3 * width] = imgSrcData[y, s, 2];
                    srcbuf[k + 2 * 3 * width] = imgSrcData[y + 1, s, 2];
                    k++;
                }

                EmbossRow(srcbuf, texture, dstbuf, width, filter);

                k = 0;
                for (int s = 0; s < width; s++)
                {
                    imgDstData[y, s, 0] = dstbuf[3 * s + 0];
                    imgDstData[y, s, 1] = dstbuf[3 * s + 1];
                    imgDstData[y, s, 2] = dstbuf[3 * s + 2];
                }
            }
        }


        /*E M B O S S  - define initial emboss parameters*/
        public static Image<Bgr, byte> EmbossCalc(Image<Bgr, byte> imgSrc, double azimuth, double angle, int depth)
        {
            Image<Bgr, byte> imgDst = new Image<Bgr, byte>(imgSrc.Width, imgSrc.Height);

            //check variables
            if (azimuth < 0) azimuth = 0;
            if (azimuth > 360) azimuth = 360;
            if (angle < 0) angle = 0;
            if (angle > 180) angle = 180;
            if (depth < 1) depth = 1;
            if (depth > 10) depth = 10;

            // process_emboss
            ProcessEmboss(imgSrc, imgDst, azimuth, angle, depth);

            return imgDst;
        }
    }
}
