using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WpfApplication2.Models
{
    public static class Structures
    {
        public struct point_xy
        {
        	public double X;				  // x-axis
        	public double Y;
        }
        
        public struct ArrPoints
        {
        	point_xy [] ArrayOfPoints;
        }
        
        /* White Balance -> L*ab algorithm */
        public struct WhitePoint
        {
        	public int Temperature;
        
        	public double X;
        	public double Y;
        	public double Z;
        
        	public double u;
        	public double v;
        }
        
        public struct ColorPoint_RGB
        {
        	public int R;
        	public int G;
        	public int B;
        }
        
        public struct ColorPoint_Lab
        {
        	public double L;
        	public double a;
        	public double b;
        
        }
        
        public struct ColorPoint_HSV
        {
        	public int H;
        	public int S;
        	public int V;
        }
        
        public struct ColorPoint_YCbCr
        {
        	public int Y;
        	public int Cb;
        	public int Cr;
        }
        
        public struct ColorPoint_XYZ
        {
        	public double X;
        	public double Y;
        	public double Z;
        }
        
        public struct ColorPoint_UV
        {
        	public double u;
        	public double v;
        }
        
        public struct Histogram
        {
        	public int Bins;
        	public int Size_x;
        	public int Size_y;
        	public int []values;  // the hist image is here
        	public int MaxValue;
        	public short NumberOfLayers;
        }


        /*W H I T E   P O I N T - fill structure*/
        /* White Balance function and structure*/
        public static void SetWhiteBalanceValues(ref WhitePoint WhitePoint_lab, int KelvinTemperature)
        {
            if (KelvinTemperature <= 3000)			// A         // 2856K  // Halogen
        	{
        		WhitePoint_lab.Temperature = 2856;
        		WhitePoint_lab.X = 0.44757;
        		WhitePoint_lab.Y = 0.40744;
        		WhitePoint_lab.Z = 0.14499;
        		WhitePoint_lab.u = 0;
        		WhitePoint_lab.v = 0;
        	}
            else if (KelvinTemperature < 4100)		// F11       // 4000K  // Narrow-band Fluorescent
        	{
        		WhitePoint_lab.Temperature = 4000;
        		WhitePoint_lab.X = 0.38054;
        		WhitePoint_lab.Y = 0.37691;
        		WhitePoint_lab.Z = 0.24254;
        		WhitePoint_lab.u = 0;
        		WhitePoint_lab.v = 0;
        	}
            else if (KelvinTemperature < 4600)		// F2        // 4200K   // Cool white Fluorescent
        	{
        		WhitePoint_lab.Temperature = 4200;
        		WhitePoint_lab.X = 0.372;
        		WhitePoint_lab.Y = 0.3751;
        		WhitePoint_lab.Z = 0.2528;
        		WhitePoint_lab.u = 0;
        		WhitePoint_lab.v = 0;
        	}
            else if (KelvinTemperature < 4900)		// B         // 4874K  // Direct sunlight at noon - obsolete
        	{
        		WhitePoint_lab.Temperature = 4874;
        		WhitePoint_lab.X = 0.44757;
        		WhitePoint_lab.Y = 0.40744;
        		WhitePoint_lab.Z = 0.14499;
        		WhitePoint_lab.u = 0;
        		WhitePoint_lab.v = 0;
        	}
            else if (KelvinTemperature < 5301)		// D50       // 5000K  // Daylight - for color rendering
        	{
        		WhitePoint_lab.Temperature = 5000;
        		WhitePoint_lab.X = 0.34567;
        		WhitePoint_lab.Y = 0.35850;
        		WhitePoint_lab.Z = 0.29583;
        		WhitePoint_lab.u = 0;
        		WhitePoint_lab.v = 0;
        	}
            else if (KelvinTemperature < 5451)		// E		 // 5400K  // Uniform energy
        	{
        		WhitePoint_lab.Temperature = 5400;
        		WhitePoint_lab.X = 0.333;
        		WhitePoint_lab.Y = 0.333;
        		WhitePoint_lab.Z = 0.333;
        		WhitePoint_lab.u = 0;
        		WhitePoint_lab.v = 0;
        	}
            else if (KelvinTemperature < 6001)		// D55       // 5500K  // Daylight - for photography
        	{
        		WhitePoint_lab.Temperature = 5500;
        		WhitePoint_lab.X = 0.9642;//0.33242;
        		WhitePoint_lab.Y = 1;// 0.34743;
        		WhitePoint_lab.Z = 0.8252;// 0.32015;
        		WhitePoint_lab.u = 0;
        		WhitePoint_lab.v = 0;
        	}
            else if (KelvinTemperature < 6506)		// D65       //  6504K  // North Sky - Daylight(NewVersion)
        	{						
        		WhitePoint_lab.Temperature = 6504;
        		WhitePoint_lab.X = 0.3127;		 
        		WhitePoint_lab.Y = 0.329;
        		WhitePoint_lab.Z = 0.3583;
        		WhitePoint_lab.u = 0;
        		WhitePoint_lab.v = 0;
        	}
            else if (KelvinTemperature < 6950)		// C         //  6774K  // North Sky - Daylight
        	{
        		WhitePoint_lab.Temperature = 6774;
        		WhitePoint_lab.X = 0.31006;
        		WhitePoint_lab.Y = 0.31615;
        		WhitePoint_lab.Z = 0.37379;
        		WhitePoint_lab.u = 0;
        		WhitePoint_lab.v = 0;
        	}
            else if (KelvinTemperature < 7800)		// D75	     //  7500K  // Daylight
        	{					
        		WhitePoint_lab.Temperature = 7500;
        		WhitePoint_lab.X = 0.29902;     
        		WhitePoint_lab.Y = 0.31485;     
        		WhitePoint_lab.Z = 0.38613;  
        		WhitePoint_lab.u = 0;
        		WhitePoint_lab.v = 0;
        	}
            else if (KelvinTemperature < 9500)	// D93	     //  9300K  // High eff.Blue Phosphor monitors
        	{
        		WhitePoint_lab.Temperature = 9300;
        		WhitePoint_lab.X = 0.2848;
        		WhitePoint_lab.Y = 0.2932;
        		WhitePoint_lab.Z = 0.422;
        		WhitePoint_lab.u = 0;
        		WhitePoint_lab.v = 0;
        	}
        	else 	// NonExisting - Like Uniform
        	{
        		WhitePoint_lab.Temperature = 5400;
        		WhitePoint_lab.X = 0.333;
        		WhitePoint_lab.Y = 0.333;
        		WhitePoint_lab.Z = 0.333;
        		WhitePoint_lab.u = 0;
        		WhitePoint_lab.v = 0;
        
        		//WhitePoint_lab->X = 1;
        		//WhitePoint_lab->Y = 1;
        		//WhitePoint_lab->Z = 1;
        	}
        }
    }
}
