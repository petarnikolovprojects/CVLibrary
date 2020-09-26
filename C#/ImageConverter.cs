using Emgu.CV;
using Emgu.CV.Structure;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace WpfApplication2.Models
{
    public static class ImageConverter
    {
        public static ImageBrush FromEmguToBrush(Image<Bgr, Byte> imgSrc)
        {
            var imgBrush = new ImageBrush();

            imgBrush.ImageSource = (ImageSource)Imaging.CreateBitmapSourceFromHBitmap(imgSrc.Bitmap.GetHbitmap(), IntPtr.Zero, Int32Rect.Empty, BitmapSizeOptions.FromEmptyOptions());

            return imgBrush;
        }

        public static ImageSource FromEmguToImageSource(Image<Bgr, Byte> imgSrc)
        {
            var imgBrush = new ImageBrush();

            return (ImageSource)Imaging.CreateBitmapSourceFromHBitmap(imgSrc.Bitmap.GetHbitmap(), IntPtr.Zero, Int32Rect.Empty, BitmapSizeOptions.FromEmptyOptions());
        }

        public static Image<Bgr, Byte> FromImageSourceToEmgu(ImageSource imgSource)
        {
            var bmpSource = imgSource as BitmapSource;
            int width = bmpSource.PixelWidth;
            int height = bmpSource.PixelHeight;

            int bytesPerPixel = bmpSource.Format.BitsPerPixel / 8;
            int stride = 4 * ((width * bytesPerPixel + 3) / 4);
            
            IntPtr ptr = IntPtr.Zero;
            try
            {
                ptr = Marshal.AllocHGlobal(height * stride);
                bmpSource.CopyPixels(new Int32Rect(0, 0, width, height), ptr, height * stride, stride);
                using (var btm = new System.Drawing.Bitmap(width, height, stride, System.Drawing.Imaging.PixelFormat.Format32bppArgb, ptr))
                {
                    // Clone the bitmap so that we can dispose it and
                    // release the unmanaged memory at ptr
                    return new Image<Bgr,Byte>(new System.Drawing.Bitmap(btm));
                }
            }
            finally
            {
                if (ptr != IntPtr.Zero)
                    Marshal.FreeHGlobal(ptr);
            }
            
        }

        public static Bitmap FromEmguToBitmapImage(Image<Bgr, Byte> imgSrc)
        {
            return imgSrc.Bitmap;
        }
    }
}
