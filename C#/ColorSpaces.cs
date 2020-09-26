using System;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Drawing.Imaging;
using System.Diagnostics;
using System.Collections.Generic;
using System.Security.Cryptography.X509Certificates;
using Emgu.CV;
using Emgu.CV.Structure;
using Emgu.CV.CvEnum;
using Emgu.CV.Util;
using WpfApplication2.Models;

namespace WpfApplication2.Algorithms
{
    public static class ColorSpace
    {
        /*
         * 1.Grayscale - Binary
         * 2.BGR - Binary
         * 3.BGR - Grayscale
         * 4.Grayscale - BGR
         * 5.BGR - HSL
         * 6.HSL - BGR
         * 7.BGR - XYZ
         * 8.XYZ - BGR
         * 9.BGR - LAB
         * 10.LAB - BGR
         * 11.XYZ - UV
         * 12.UV - XYZ
         */

        /*G R A Y S C A L E   to  B I N A R Y*/
        public static Image<Gray, Byte> Threshold(this Image<Gray, Byte> imgSrc, int threshVal)
        {
            int width = imgSrc.Width;
            int height = imgSrc.Height;

            var imgSrcData = imgSrc.Data;

            var imgDst = new Image<Gray, Byte>(width, height);
            var imgDstData = imgDst.Data;

            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    if (imgSrcData[i, j, 0] > threshVal)
                    {
                        imgDstData[i, j, 0] = 255;
                    }
                    else
                    {
                        imgDstData[i, j, 0] = 0;
                    }
                }
            }

            return imgDst;
        }


        /*B G R   to  B I N A R Y*/
        public static Image<Gray, Byte> Threshold(this Image<Bgr, Byte> imgSrc, int threshVal)
        {
            int width = imgSrc.Width;
            int height = imgSrc.Height;

            var imgSrcData = imgSrc.Data;

            var imgDst = new Image<Gray, Byte>(width, height);
            var imgDstData = imgDst.Data;

            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    int calc = (imgSrcData[i, j, 0] + imgSrcData[i, j, 1] + imgSrcData[i, j, 2]) / 3;

                    if (calc > threshVal)
                    {
                        imgDstData[i, j, 0] = 255;
                    }
                    else
                    {
                        imgDstData[i, j, 0] = 0;
                    }
                }
            }

            return imgDst;
        }


        /*B G R   to   G R A Y S C A L E*/
        public static Image<Gray, Byte> ConvertToGrayscale(this Image<Bgr, Byte> imgSrc)
        {
            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;

            var imgDst = new Image<Gray, Byte>(width, height);
            var imgDstData = imgDst.Data;

            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    imgDstData[i, j, 0] = (byte)(Math.Min(0.11 * imgSrcData[i, j, 0] + 0.59 * imgSrcData[i, j, 1] + 0.3 * imgSrcData[i, j, 2], 255));
                }
            }
            return imgDst;
        }


        /*G R A Y S C A L E   to   B G R*/
        public static Image<Bgr, Byte> CopyGrayToAllChannels(this Image<Gray, Byte> imgSrc)
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
                    byte pix = imgSrcData[i, j, 0];
                    imgDstData[i, j, 0] = pix;
                    imgDstData[i, j, 1] = pix;
                    imgDstData[i, j, 2] = pix;
                }
            }

            return imgDst;
        }


        /*Chech Color values - Used in HSL - RGB conversion
        */
        private static double CheckColorValue(double TempColor, double Temporary_1, double Temporary_2)
        {
        	double NewColor;
        
        	if (6 * TempColor < 1)
        	{
        		NewColor = Temporary_2 + ((Temporary_1 - Temporary_2) * 6 * TempColor);
        	}
        	else
        	{
        		if (2 * TempColor < 1)
        		{
        			NewColor = Temporary_1;
        		}
        		else
        		{
        			if (3 * TempColor < 2)
        			{
        				NewColor = Temporary_2 + ((Temporary_1 - Temporary_2) * 6 * (0.6666 - TempColor));
        			}
        			else
        				NewColor = Temporary_2;
        		}
        	}
        
        	return NewColor;
        }
        
        /*C O N V E R T -  RGB to HSV*/
        public static Image<Bgr, Byte> ConvertImage_RGB_to_HSL(this Image<Bgr, Byte> imgSrc)
        {
        	double R_scaled;
        	double G_scaled;
        	double B_scaled;
        	double C_max;
        	double C_min;
        	double Delta;
        	double Hue = 0;
        
        	double del_R, del_B, del_G;
        	double Saturation, Luma = 0;
        
        	int i, j;

            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;
            var imgDst = imgSrc.CopyBlank();
            var imgDstData = imgDst.Data;

        	for (i = 0; i < height; i++)
        	{
        		for (j = 0; j < width; j++)
        		{
        			R_scaled = imgSrcData[i, j, 2] / (float)255;
        			G_scaled = imgSrcData[i, j, 1] / (float)255;
        			B_scaled = imgSrcData[i, j, 0] / (float)255;
        			C_max = Math.Max(R_scaled, Math.Max(G_scaled, B_scaled));
        			C_min = Math.Min(R_scaled, Math.Min(G_scaled, B_scaled));
        			Delta = C_max - C_min;
        
        			// HUE
        			if (C_max == R_scaled)
        			{
        				Hue = Computation.RoundValue_toX_SignificantBits((60 * ((((G_scaled - B_scaled) / Delta) % 6))), 2);
        				if (Hue < 0) Hue = 360 + Hue;
        				if (Hue > 360) Hue = Hue % 360;
        			}
        			else if (C_max == G_scaled)
        			{
        				Hue = Computation.RoundValue_toX_SignificantBits((60 * (((B_scaled - R_scaled) / Delta) + 2)),2);
        				if (Hue < 0) Hue = 360 + Hue;
        				if (Hue > 360) Hue = Hue % 360;
        			}
        			else if (C_max == B_scaled)
        			{
        				Hue = Computation.RoundValue_toX_SignificantBits((60 * (((R_scaled - G_scaled) / Delta) + 4)),2);
        				if (Hue < 0) Hue = 360 + Hue;
        				if (Hue > 360) Hue = Hue % 360;
        			}
        
        			// LUMA
        			Luma = (C_max + C_min) / (float)2;
        
        			// SATURATION
        			if (Delta == 0) 
        			{
        				Saturation = 0; 
        				Hue = 0;
        			}
        			else
        			{ 
        				Saturation = Luma > 0.5 ? Delta / (float)(2 - C_max - C_min) : Delta / (float)(C_max + C_min);
        			}
        
        			imgDstData[i, j, 0] = (byte)Computation.RoundValue_toX_SignificantBits((Hue / (float)360) * 100, 2);
        			imgDstData[i, j, 1] = (byte)Computation.RoundValue_toX_SignificantBits(Saturation * 100, 2);
        			imgDstData[i, j, 2] = (byte)(Luma * 100);//round(Luma * 100);
        
        		}
        	}
            return imgDst;
        }
        

        /*C O N V E R T -  HSL to RGB*/
        public static Image<Bgr, Byte> ConvertImage_HSL_to_RGB(this Image<Bgr, Byte> imgSrc)
        {
        	double R_temp;
        	double G_temp;
        	double B_temp;
        	double C, X, m;
        	double Hue;
        	double Temporary_1, Temporary_2;
        	double Saturation = 0, Luma = 0;
        
        	int i, j;
            
            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;
            var imgDst = imgSrc.CopyBlank();
            var imgDstData = imgDst.Data;
        
        	for (i = 0; i < height; i++)
        	{
        		for (j = 0; j < width; j++)
        		{
        			
        			Hue = imgSrcData[i, j, 0] / 100.0;

                    Saturation = imgSrcData[i, j, 1] / (float)100;

                    Luma = imgSrcData[i, j, 2] / (float)100;
        			
        			if (Saturation == 0)
        			{
        				R_temp = Luma * 255;
        				if (R_temp > 255) 
        				{
        					R_temp = 255; B_temp = 255; G_temp = 255;
        				}
        				else
        					B_temp = G_temp = R_temp;
        			}
        			else
        			{
        				if (Luma >= 0.5)
        				{
        					Temporary_1 = ((Luma + Saturation)  - (Luma * Saturation));
        				}
        				else
        				{
        					Temporary_1 = Luma * (1 + Saturation);
        				}
        				Temporary_2 = 2 * Luma - Temporary_1;
        				
        				R_temp = Hue + 0.33333;
        				if (R_temp < 0) R_temp += 1;
        				if (R_temp > 1) R_temp -= 1;
        				
        				G_temp = Hue;
        				if (G_temp < 0) G_temp += 1;
        				if (G_temp > 1) G_temp -= 1;
        
        				B_temp = Hue - 0.33333;
        				if (B_temp < 0) B_temp += 1;
        				if (B_temp > 1) B_temp -= 1;
        
        				// Check R
        				R_temp = CheckColorValue(R_temp, Temporary_1, Temporary_2);
        
        				// Check G
        				G_temp = CheckColorValue(G_temp, Temporary_1, Temporary_2);
        
        				// Check B
        				B_temp = CheckColorValue(B_temp, Temporary_1, Temporary_2);
        
        				
        				R_temp *= 255;
        				if (R_temp > 255) R_temp = 255;
        				G_temp *= 255;
        				if (G_temp > 255) G_temp = 255;
        				B_temp *= 255;
        				if (B_temp > 255) B_temp = 255;
        			}
        			
        			imgDstData[i, j, 2] = (byte)Computation.RoundValue_toX_SignificantBits(R_temp,2);
        			imgDstData[i, j, 1] = (byte)Computation.RoundValue_toX_SignificantBits(G_temp,2);
        			imgDstData[i, j, 0] = (byte)Computation.RoundValue_toX_SignificantBits(B_temp,2);
        
        		}
        	}
            return imgDst;
        }


        /* E X A M P L E   usage of CMY and CMYK
            var CMYK = ConvertRGBtoCMYKnormed(imgSrc);
            var CMY = ConvertRGBtoCMYnormed(imgSrc);

            var KchannelGray = ConvertArrayToImageChannel(CMYK, 3, width, height);
            var CchannelGray = ConvertArrayToImageChannel(CMY, 0, width, height);
            var MchannelGray = ConvertArrayToImageChannel(CMY, 1, width, height);
            var YchannelGray = ConvertArrayToImageChannel(CMY, 2, width, height);

            var KchannelBgr = CopyGrayToAllChannels(KchannelGray);
            var CchannelBgr = CopyGrayToAllChannels(CchannelGray);
            var MchannelBgr = CopyGrayToAllChannels(MchannelGray);
            var YchannelBgr = CopyGrayToAllChannels(YchannelGray);
         */

        /*P I X E L  convert  R G B  to  C M Y K*/
        public static double[] PixelConvertRGBtoCMYKnormed(Rgb pixelRGB)
        {
            double[] pixelCMYK = new double[4];

            double Rnorm = pixelRGB.Red / 255;
            double Gnorm = pixelRGB.Green / 255;
            double Bnorm = pixelRGB.Blue / 255;

            pixelCMYK[3] = 255 * (1 - (Math.Max(Math.Max(Rnorm, Gnorm), Bnorm))); // K
            pixelCMYK[0] = (1 - Rnorm - pixelCMYK[3]) / (1 - pixelCMYK[3]);        // C
            pixelCMYK[1] = (1 - Gnorm - pixelCMYK[3]) / (1 - pixelCMYK[3]);        // M
            pixelCMYK[2] = (1 - Bnorm - pixelCMYK[3]) / (1 - pixelCMYK[3]);        // Y

            return pixelCMYK;
        }


        /*R G B  to  C M Y K - normed*/
        public static double[,] ConvertRGBtoCMYKnormed(this Image<Bgr, Byte> imgSrc)
        {
            int width = imgSrc.Width;
            int height = imgSrc.Height;

            var CMYK = new double[4, width * height];
            var imgSrcData = imgSrc.Data;

            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    int position = i * width + j;
                    var pixelCMYK = PixelConvertRGBtoCMYKnormed(new Rgb(imgSrcData[i, j, 2], imgSrcData[i, j, 1], imgSrcData[i, j, 0]));
                    CMYK[0, position] = pixelCMYK[0];
                    CMYK[1, position] = pixelCMYK[1];
                    CMYK[2, position] = pixelCMYK[2];
                    CMYK[3, position] = pixelCMYK[3];
                }
            }

            return CMYK;
        }


        /*R G B  to  C M Y  -  normed*/
        public static double[,] ConvertRGBtoCMYnormed(this Image<Bgr, Byte> imgSrc)
        {
            int width = imgSrc.Width;
            int height = imgSrc.Height;

            var CMY = new double[3, width * height];
            var imgSrcData = imgSrc.Data;

            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    int position = i * width + j;
                    int R = imgSrcData[i, j, 2];
                    int G = imgSrcData[i, j, 1];
                    int B = imgSrcData[i, j, 0];

                    CMY[0, position] = (1 - (R / 255.0));
                    CMY[1, position] = (1 - (G / 255.0));
                    CMY[2, position] = (1 - (B / 255.0));
                }
            }

            return CMY;
        }
        

        /*C M Y   to   R G B*/
        public static Image<Bgr, Byte> ConvertCMYtoRGB(double[,] CMY, int width, int height)
        {
            var imgDst = new Image<Bgr, Byte>(width, height);

            var imgDstData = imgDst.Data;

            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    int position = i * width + j;
                    int R = (int)(0.5 + (255 * CMY[0, position]));
                    int G = (int)(0.5 + (255 * CMY[1, position]));
                    int B = (int)(0.5 + (255 * CMY[2, position]));
                    if (R > 255)
                    {
                        R = 255;
                    }
                    if (G > 255)
                    {
                        G = 255;
                    }
                    if (B > 255)
                    {
                        B = 255;
                    }
                    imgDstData[i, j, 0] = (byte)B;
                    imgDstData[i, j, 1] = (byte)G;
                    imgDstData[i, j, 2] = (byte)R;
                }
            }

            return imgDst;
        }


        /*C M Y   to   C M Y K - normed*/
        public static double[,] ConvertCMYtoCMYKnormed(double[,] CMY, int width, int height)
        {

            var CMYK = new double[4, width * height];

            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    double varK = 1;
                    int position = i * width + j;

                    double C = CMY[0, position];
                    double M = CMY[1, position];
                    double Y = CMY[2, position];
                    double K = 0;

                    if (C < varK) varK = C;
                    if (M < varK) varK = M;
                    if (Y < varK) varK = Y;
                    if (varK == 1)
                    {
                        C = 0;
                        M = 0;
                        Y = 0;
                    }
                    else
                    {
                        C = (C - varK) / (1 - varK);
                        M = (M - varK) / (1 - varK);
                        Y = (Y - varK) / (1 - varK);
                    }
                    K = varK;

                    CMYK[0, position] = C;
                    CMYK[1, position] = M;
                    CMYK[2, position] = Y;
                    CMYK[3, position] = K;
                }
            }

            return CMYK;
        }


        /*A R R A Y   to  Image C H A N N E L*/
        public static Image<Gray, Byte> ConvertArrayToImageChannel(double[] arrCMYK, int width, int height)
        {
            var imgDst = new Image<Gray, byte>(width, height);
            var imgDstData = imgDst.Data;

            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    int position = i * width + j;
                    int calc = (int)(0.5 + (255 * arrCMYK[position]));
                    if (calc > 255)
                    {
                        calc = 255;
                    }
                    imgDstData[i, j, 0] = (byte)calc;
                }
            }

            return imgDst;
        }


        /*X Y Z   to   C O L O R  temperature*/
        public static void ColorTemperature(this Structures.WhitePoint WhitePoint_lab, int AlgoType)
        {
        	double X_e = 0.3366;
        	double Y_e = 0.1735;
        	double A_0 = -949.86315;
        	double A_1 = 6253.803;
        	double t_1 = 0.92159;
        	double A_2 = 28.706;
        	double t_2 = 0.20039;
        	double A_3 = 0.00004;
        	double t_3 = 0.07125;
        	//
        	double n;
        	double CCT;
        	Structures.point_xy XY;
        	Structures.ColorPoint_UV UV;

        	if (AlgoType == 0)
        	{
        		// Calculate color temperature and XYZ coordinates from UV coordinates
        		if (WhitePoint_lab.u != 0)
        		{
        			UV.u = WhitePoint_lab.u;
        			UV.v = WhitePoint_lab.v;
        
        			//Caclulate XY from UV
                    XY = UV.ConvertPointUVtoXY();
        		}
        		else
        		{
        			//Calculate XY from XYZ;
        			XY.X = WhitePoint_lab.X / (WhitePoint_lab.X + WhitePoint_lab.Y + WhitePoint_lab.Z);
        			XY.Y = WhitePoint_lab.Y / (WhitePoint_lab.X + WhitePoint_lab.Y + WhitePoint_lab.Z);
        		}
        
        		n = (XY.X - 0.332) / (0.1858 - XY.Y);
                CCT = 449 * Math.Pow(n, 3) + 3525 * Math.Pow(n, 2) + 6823.3 * n + 5520.33;
        	}
        	else
        	{
        		//EXP_HIGH_T
        		XY.X = WhitePoint_lab.X / (WhitePoint_lab.X + WhitePoint_lab.Y + WhitePoint_lab.Z);
        		XY.Y = WhitePoint_lab.Y / (WhitePoint_lab.X + WhitePoint_lab.Y + WhitePoint_lab.Z);
        		n = (XY.X - X_e) / (XY.Y - Y_e);
        		CCT = A_0 + A_1 * Math.Exp(-n / t_1) + A_2 * Math.Exp((-n / t_2) + A_3*Math.Exp(-n/t_3));
        	}
        
        	//Differences in values for both algorithms
        	// 6347 - pic1     4287 - pic 2; //Algo_type = 0
        	// 6344 - pic1     4293 - pic 2; // Algo_type = 1;
        	WhitePoint_lab.Temperature = (int)CCT;
        }


        /*P O I N T  - convert  XYZ to UV  coordinates*/
        public static Structures.ColorPoint_UV ConvertPointXYZtoUV(this Structures.ColorPoint_XYZ XYZ)
        {
        	Structures.ColorPoint_UV UV;
        	UV.u = 4 * XYZ.X / (float)(XYZ.X + 15 * XYZ.Y + 3 * XYZ.Z);
        	UV.v = 6 * XYZ.Y / (float)(XYZ.X + 15 * XYZ.Y + 3 * XYZ.Z);
        
        	return UV;
        }


        /*P O I N T  - convert  UV to XYZ  coordinates  */
        public static Structures.point_xy ConvertPointUVtoXY(this Structures.ColorPoint_UV UV)
        {
        	Structures.point_xy XY;
        	
        	XY.X = 3 * UV.u / (2 * UV.u - 8 * UV.v + 4);
        	XY.Y = 2 * UV.v / (2 * UV.u - 8 * UV.v + 4);
        
        	return XY;
        }


        /*P O I N T  - convert RGB to XYZ*/
        public static Structures.ColorPoint_XYZ ConvertPointRGBtoXYZ(this Structures.ColorPoint_RGB RGB_Point)
        {
        	double R, G, B;
        	Structures.ColorPoint_XYZ XYZ;
        	R = RGB_Point.R;
        	G = RGB_Point.G;
        	B = RGB_Point.B;
        
        	R /= 255;
        	G /= 255;
        	B /= 255;
        
        	if (R > 0.04045)
        		R = Math.Pow(((R + 0.055) / 1.055), 2.4);
        	else
        		R = R / 12.92;
        	if (G > 0.04045)
        		G = Math.Pow(((G + 0.055) / 1.055), 2.4);
        	else
        		G = G / 12.92;
        	if (B > 0.04045)
        		B = Math.Pow(((B + 0.055) / 1.055), 2.4);
        	else
        		B = B / 12.92;
        	
        	R = R * 100;
        	G = G * 100;
        	B = B * 100;
        	
        	XYZ.X = R * 0.4124 + G * 0.3576 + B * 0.1805;
        	XYZ.Y = R * 0.2126 + G * 0.7152 + B * 0.0722;
        	XYZ.Z = R * 0.0193 + G * 0.1192 + B * 0.9505;
        
        	return XYZ;
        }
        

        /*P O I N T  - convert XYZ to RGB*/
        public static Structures.ColorPoint_RGB ConvertPointXYZtoRGB(this Structures.ColorPoint_XYZ XYZ)
        {
        	double R, G, B;
        	Structures.ColorPoint_RGB RGB;
        	double X, Y, Z;
        
        	X = XYZ.X;
        	Y = XYZ.Y;
        	Z = XYZ.Z;
        	
        	R = X *  3.2406 + Y * -1.5372 + Z * -0.4986;
        	G = X * -0.9689 + Y *  1.8758 + Z *  0.0415;
        	B = X *  0.0557 + Y * -0.2040 + Z *  1.0570;
        	
        	if (R < 0) 
        		R = 0;
        	if (G < 0) 
        		G = 0;
        	if (B < 0) 
        		B = 0;
        	
        	if (R > 0.0031308)
        		R = 1.055 * Math.Pow(R, (1 / 2.4)) - 0.055;
        	else
        		R = 12.92 * R;
        	if (G > 0.0031308)
        		G = 1.055 * Math.Pow(G, (1 / 2.4)) - 0.055;
        	else
        		G = 12.92 * G;
        	if (B > 0.0031308) 
        		B = 1.055 * Math.Pow(B , (1 / 2.4)) - 0.055;
        	else
        		B = 12.92 * B;
        	
        	R = R * 255;
        	if (R > 255) R = 255;
        	if (R < 0) R = 0;
        	G = G * 255;
        	if (G > 255) G = 255;
        	if (G < 0) G = 0;
        	B = B * 255;
        	if (B > 255) B = 255;
        	if (B < 0) B = 0;
        	
        	RGB.R = (int)(Computation.RoundValue_toX_SignificantBits(R, 0));
        	RGB.G = (int)(Computation.RoundValue_toX_SignificantBits(G, 0));
        	RGB.B = (int)(Computation.RoundValue_toX_SignificantBits(B, 0));
        
        	return RGB;
        }


        /*convert  X Y Z   to   L A B*/
        public static double convertXYZtoLAB(double c)
        {
            return c > 216.0 / 24389.0 ? Math.Pow(c, 1.0 / 3.0) : c * (841.0 / 108.0) + (4.0 / 29.0);
        }


        /*convert  R G B   to   L A B*/
        public static Image<Bgr, Byte> ConvertRGBtoLAB(this Image<Bgr, Byte> imgSrc, Structures.WhitePoint WhitePoint_XYZ)
        {
            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;

        	int i, j;
        	double L, a, b;
        	double X, Y, Z;
        	double F_x, F_y, F_z;
        	double R_t, G_t, B_t;
        	double RatioY, RatioX, RatioZ;
        	double e = 0.008856;
        	double k = 903.3;
        	double dult;

            var imgDst = imgSrc.CopyBlank();
            var imgDstData = imgDst.Data;

        	// 1st step: Convert to XYZ color space
        	var imgXYZ = imgSrc.ConvertRGBtoXYZ();
            var imgXYZdata = imgXYZ.Data;

        	for (i = 0; i < height; i++)
        	{
        		for (j = 0; j < width; j++)
        		{
        			X = imgXYZdata[i, j, 0];// +Img_XYZ.rgbpix[6 * (i * Img_dst->Width + j) + 3] / 100.0;
        			Y = imgXYZdata[i, j, 1];// +Img_XYZ.rgbpix[6 * (i * Img_dst->Width + j) + 4] / 100.0;
        			Z = imgXYZdata[i, j, 2];// +Img_XYZ.rgbpix[6 * (i * Img_dst->Width + j) + 5] / 100.0;
        
        			X /= 100.0;
        			Y /= 100.0;
        			Z /= 100.0;
        
        			RatioX = X / 1;// (100 * WhitePoint_XYZ.X);
        			RatioY = Y / 1;// (100 * WhitePoint_XYZ.Y);
        			RatioZ = Z / 1;// (100 * WhitePoint_XYZ.Z);
        
        			RatioX = Computation.RoundValue_toX_SignificantBits(RatioX, 4);
        			RatioY = Computation.RoundValue_toX_SignificantBits(RatioY, 4);
        			RatioZ = Computation.RoundValue_toX_SignificantBits(RatioZ, 4);
        
        			if (RatioX > e)
        			{
        				F_x = Math.Pow(RatioX, Math.Pow(3, -1));
        			}
        			else
        			{
        				F_x = (k * RatioX + 16) / 116;
        			}
        
        			if (RatioY > e)
        			{
        				F_y = Math.Pow(RatioY, Math.Pow(3, -1));
        			}
        			else
        			{
        				F_y = (k * RatioY + 16) / 116;
        
        			}
        
        			if (RatioZ > e)
        			{
        				F_z = Math.Pow(RatioZ, Math.Pow(3, -1));
        			}
        			else
        			{
        				F_z = (k * RatioZ + 16) / 116;
        			}
        
        			
        			//L = 116 * F_y - 16;
        			R_t = imgSrcData[i, j, 2] / 255.0;
        			G_t = imgSrcData[i, j, 1] / 255.0;
        			B_t = imgSrcData[i, j, 0] / 255.0;
        			
        			F_x = Computation.RoundValue_toX_SignificantBits(F_x, 3);
        			F_y = Computation.RoundValue_toX_SignificantBits(F_y, 3);
        			F_z = Computation.RoundValue_toX_SignificantBits(F_z, 3);
        
        			dult = Math.Min(R_t, Math.Min(G_t, B_t)) + Math.Max(R_t, Math.Max(G_t, B_t));
        			L = ((float)dult / 2) * 100;
        
        			a = 500 * (F_x - F_y);
        			b = 200 * (F_y - F_z);
        
        			L = Computation.RoundValue_toX_SignificantBits(L, 0);
        			a = Computation.RoundValue_toX_SignificantBits(a, 0);
        			b = Computation.RoundValue_toX_SignificantBits(b, 0);
        			a += 128;
        			b += 128;
        			if (a > 255) 
        				a = 255;
        			if (a < 0) 
        				a = 0;
        
        			if (b > 255) 
        				b = 255;
        			if (b < 0) 
        				b = 0;
        
        			if (L < 0) L = 0;
        			if (L > 255) L = 255;
        
        			imgDstData[i, j, 0] = (byte)L;
        			if (L - imgDstData[i, j, 0] > 0.5) imgDstData[i, j, 0] += 1;
        			imgDstData[i, j, 1] = (byte)a;
        			if (a - imgDstData[i, j, 1] > 0.5) imgDstData[i, j, 1] += 1;
        			imgDstData[i, j, 2] = (byte)b;
        			if (b - imgDstData[i, j, 2] > 0.5) imgDstData[i, j, 2] += 1;
        		}
        	}
            return imgDst;
        }
        

        /*convert  L A B   to   R G B*/
        public static Image<Bgr, Byte> ConvertLABtoRGB(this Image<Bgr, Byte> imgSrc, Structures.WhitePoint WhitePoint_XYZ)
        {
        	int i, j;
        	double L, a, b;
        	double X, Y, Z;
        	double P;
        	double Number;
        	double RatioY, RatioX, RatioZ;

            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;

            var imgXYZ = imgSrc.CopyBlank();
            var imgXYZData = imgXYZ.Data;

        	// 1st step: Convert to XYZ color space
        
        	for (i = 0; i < height; i++)
        	{
        		for (j = 0; j < width; j++)
        		{
        			L = imgSrcData[i, j, 0];
        			a = imgSrcData[i, j, 1];
        			b = imgSrcData[i, j, 2];
        
        			a -= 128;
        			b -= 128;
        
        			Y = L * (1.0 / 116.0) + 16.0 / 116.0;
        			X = a * (1.0 / 500.0) + Y;
        			Z = b * (-1.0 / 200.0) + Y;
        
        			X = X > 6.0 / 29.0 ? X * X * X : X * (108.0 / 841.0) - 432.0 / 24389.0;
        			Y = L > 8.0 ? Y * Y * Y : L * (27.0 / 24389.0);
        			Z = Z > 6.0 / 29.0 ? Z * Z * Z : Z * (108.0 / 841.0) - 432.0 / 24389.0;
        			
        			X *= 100;
        			Y *= 100;
        			Z *= 100;
        
        			X = Computation.RoundValue_toX_SignificantBits(X, 0);
        			Y = Computation.RoundValue_toX_SignificantBits(Y, 0);
        			Z = Computation.RoundValue_toX_SignificantBits(Z, 0);
                    imgXYZData[i, j, 0] = (byte)X;
                    imgXYZData[i, j, 1] = (byte)Y;
        			imgXYZData[i, j, 2] = (byte)Z;
        		}
        	}
            var imgDst = imgXYZ.ConvertXYZtoRGB();

            return imgDst;
        }


        /*convert  R G B   to  X Y Z*/
        public static Image<Bgr, Byte> ConvertRGBtoXYZ(this Image<Bgr, Byte> imgSrc)
        {
        	int i, j;
        	double X, Y, Z;
        	double var_R, var_B, var_G;
        
            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;

            var imgDst = imgSrc.CopyBlank();
            var imgDstData = imgDst.Data;

        	for (i = 0; i < height; i++)
        	{
        		for (j = 0; j < width; j++)
        		{
        			var_R = imgSrcData[i, j, 0];
        			var_G = imgSrcData[i, j, 1];
        			var_B = imgSrcData[i, j, 2];
        
        			var_R /= 255;
        			var_G /= 255;
        			var_B /= 255;
        
        			if (var_R > 0.04045)
        				var_R = Math.Pow(((var_R + 0.055) / 1.055), 2.4);
        			else
        				var_R = var_R / 12.92;
        			if (var_G > 0.04045)
        				var_G = Math.Pow(((var_G + 0.055) / 1.055), 2.4);
        			else
        				var_G = var_G / 12.92;
        			if (var_B > 0.04045)
        				var_B = Math.Pow(((var_B + 0.055) / 1.055), 2.4);
        			else
        				var_B = var_B / 12.92;
        
        			var_R = var_R * 100;
        			var_G = var_G * 100;
        			var_B = var_B * 100;
        
        			X = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805;
        			Y = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722;
        			Z = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505;
        			
        			//X = var_R * 0.4887 + var_G * 0.3107 + var_B * 0.2006;
        			//Y = var_R * 0.1762 + var_G * 0.8130 + var_B * 0.0108;
        			//Z = var_R * 0 + var_G * 0.102 + var_B * 0.9898;
        
        			//X = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805;
        			//Y = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722;
        			//Z = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505;
        			/*
        				Calculate color temperature;
        			*/
        
        			if (X < 0) X = 0;
        			if (Y < 0) Y = 0;
        			if (Z < 0) Z = 0;
        
        			X = Computation.RoundValue_toX_SignificantBits(X, 0);
        			Y = Computation.RoundValue_toX_SignificantBits(Y, 0);
        			Z = Computation.RoundValue_toX_SignificantBits(Z, 0);
        			
        			imgDstData[i, j, 0] = (byte)X;
        			imgDstData[i, j, 1] = (byte)Y;
        			imgDstData[i, j, 2] = (byte)Z;
        
        		}
        	}
            return imgDst;
        }
        

        /*convert  X Y Z   to  R G B*/
        public static Image<Bgr, Byte> ConvertXYZtoRGB(this Image<Bgr, Byte> imgSrc)
        {
        	int i, j;
        	double var_X, var_Y, var_Z;
        	double R, B, G;

            int width = imgSrc.Width;
            int height = imgSrc.Height;
            var imgSrcData = imgSrc.Data;

            var imgDst = imgSrc.CopyBlank();
            var imgDstData = imgDst.Data;

        	for (i = 0; i < height; i++)
        	{
        		for (j = 0; j < width; j++)
        		{
        			var_X = imgSrcData[i, j, 0];// +Img_src->rgbpix[6 * (i * Img_dst->Width + j) + 3] / 100.0;
        			var_Y = imgSrcData[i, j, 1];// +Img_src->rgbpix[6 * (i * Img_dst->Width + j) + 4] / 100.0;
        			var_Z = imgSrcData[i, j, 2];// +Img_src->rgbpix[6 * (i * Img_dst->Width + j) + 5] / 100.0;
        			
        			var_X /= 100.0;
        			var_Y /= 100.0;
        			var_Z /= 100.0;
        
        			R = var_X *  3.2406 + var_Y * -1.5372 + var_Z * -0.4986;
        			G = var_X * -0.9689 + var_Y *  1.8758 + var_Z *  0.0415;
        			B = var_X *  0.0557 + var_Y * -0.2040 + var_Z *  1.0570;
        
        			//R = var_X *  2.3707 + var_Y * -0.9001 + var_Z * -0.4706;
        			//G = var_X * -0.5139 + var_Y *  1.4253 + var_Z *  0.0886;
        			//B = var_X *  0.0053 + var_Y * -0.0147 + var_Z *  1.0094;
        
        			if (R < 0) 
        				R = 0;
        			if (G < 0) 
        				G = 0;
        			if (B < 0) 
        				B = 0;
        
        			if (R > 0.0031308)
        				R = 1.055 * Math.Pow(R, (1 / 2.4)) - 0.055;
        			else
        				R = 12.92 * R;
        			if (G > 0.0031308)
                        G = 1.055 * Math.Pow(G, (1 / 2.4)) - 0.055;
        			else
        				G = 12.92 * G;
        			if (B > 0.0031308)
                        B = 1.055 * Math.Pow(B, (1 / 2.4)) - 0.055;
        			else
        				B = 12.92 * B;
        			
        			R = R * 255;
        			if (R > 255) R = 255;
        			if (R < 0) R = 0;
        			G = G * 255;
        			if (G > 255) G = 255;
        			if (G < 0) G = 0;
        			B = B * 255;
        			if (B > 255) B = 255;
        			if (B < 0) B = 0;
        
        			R = Computation.RoundValue_toX_SignificantBits(R, 0);
        			G = Computation.RoundValue_toX_SignificantBits(G, 0);
        			B = Computation.RoundValue_toX_SignificantBits(B, 0);
        
        			imgDstData[i, j, 2] = (byte)R;
        			imgDstData[i, j, 1] = (byte)G;
        			imgDstData[i, j, 0] = (byte)B;
        		}
        	}
            return imgDst;
        }

    }
}
