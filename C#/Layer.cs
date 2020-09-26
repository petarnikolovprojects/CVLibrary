using Emgu.CV;
using Emgu.CV.Structure;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using WpfApplication2.Definitions;

namespace WpfApplication2.Models
{
    public class Layer
    {
        Color color;

        public string Name { get; set; }
        public int OrderNumber { get; private set; }

        private int _Opacity;

        public int Opacity
        {
            get { return _Opacity; }

            set { _Opacity = Int32.Parse(value.ToString()); }
        }
        public bool IsVisible { get; set; }

        public bool isSingleChannel = false;
        public LayerParams layerParams;

        public ImageSource ImgSmallPreview { get; private set; }

        public Layer(string name, Image<Bgr, Byte> imgPreviewSmall, LayerParams layerParameters)
        {
            this.Name = name;
            this.Opacity = 100;

            this.layerParams = layerParameters;

            ImgSmallPreview = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(imgPreviewSmall);
            this.IsVisible = true;
        }
    }
}
