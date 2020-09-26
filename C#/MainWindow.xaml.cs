using Emgu.CV;
using Emgu.CV.Structure;
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Threading;
using WpfApplication2.Algorithms;
using WpfApplication2.Definitions;
using WpfApplication2.Models;
using System.Drawing;
using PPC_Setup.Utils.DragAndDrop;
using System.Collections.ObjectModel;
using WpfApplication2.Windows;
using PPC_Setup.Windows;

namespace WpfApplication2
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        const int NUMBER_OF_UNDO_ACTIONS = 3;
        const int PREVIEW_IMAGE_SIZE = 256; //Change also in XAML

        public enum StatusType
        {
            Notification,
            Process,
            Warning,
            Error,
        }

        public static MainWindow _MainWindow;
        Image<Bgr, Byte> imgSrc;
        Image<Bgr, Byte> imgPreview;
        Image<Bgr, Byte> imgSelected;

        ObservableCollection<Layer> listLayers;
        List<Image<Bgr, Byte>> layersImgList;

        List<Image<Bgr, Byte>> previousImages;

        UInt16 curZoomFactor;
        string filename;
        Rectangle selectedArea;
        Rectangle previewArea;
        bool isAlgoSelected = false;
        algoTypes lastSelectedAlgoType;
        bool isCtrlKeyPressed = false;
        bool isReset = false;
        bool openImgAsLayer = false;

        public MainWindow()
        {
            InitializeComponent();
            _MainWindow = this;
            
            /*Initialize class parameters*/
            previewArea = new Rectangle();

            this.curZoomFactor = 100;
            this.tbZoomFactor.Text = curZoomFactor.ToString();

            this.previousImages = new List<Image<Bgr, byte>>(NUMBER_OF_UNDO_ACTIONS);
            this.layersImgList = new List<Image<Bgr, byte>>();

            for(int i = 0; i < NUMBER_OF_UNDO_ACTIONS; i++)
            {
                this.previousImages.Add(new Image<Bgr, byte>(0,0));
            }

            var layerModesStr = new List<string>();
            layerModesStr.Add("Divide");
            layerModesStr.Add("Substract");
            layerModesStr.Add("Burn");
            layerModesStr.Add("Grain Extract");

            cbLayerModes.ItemsSource = layerModesStr;

            /*Modify buttons*/
            #region modifyButtons

            this.buttonOpenBlank.ToolTip = "Create Empty image";
            this.buttonOpenBlank.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(new Image<Bgr, byte>(@"..\..\Images\blankDoc.png"));
            
            this.buttonOpen.ToolTip = "Open Image file";
            this.buttonOpen.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(new Image<Bgr, byte>(@"..\..\Images\openFile.gif"));
            
            this.buttonSave.ToolTip = "Save";
            this.buttonSave.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(new Image<Bgr, byte>(@"..\..\Images\saveFile.png"));

            this.buttonSaveAsNew.ToolTip = "Save as new Image";
            this.buttonSaveAsNew.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(new Image<Bgr, byte>(@"..\..\Images\saveFile.png"));

            this.buttonZoomIn.ToolTip = "Zoom In";
            this.buttonZoomIn.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(new Image<Bgr, byte>(@"..\..\Images\zoomIn.png"));

            this.buttonZoomOut.ToolTip = "Zoom Out";
            this.buttonZoomOut.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(new Image<Bgr, byte>(@"..\..\Images\zoomOut.png"));

            this.buttonRevert.ToolTip = "Reload Image";
            this.buttonRevert.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(new Image<Bgr, byte>(@"..\..\Images\revert.gif"));

            this.buttonBrightLess.ToolTip = "Brighness -";
            this.buttonBrightLess.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(new Image<Bgr, byte>(@"..\..\Images\brightLess.png"));

            this.buttonBrightMore.ToolTip = "Brighness +";
            this.buttonBrightMore.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(new Image<Bgr, byte>(@"..\..\Images\brightMore.png"));

            this.buttonUndo.ToolTip = "Undo";
            this.buttonUndo.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(new Image<Bgr, byte>(@"..\..\Images\undo.jpg"));

            this.buttonRectSelect.ToolTip = "Select area from the current image";
            this.buttonRectSelect.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(new Image<Bgr, byte>(@"..\..\Images\undo.jpg"));

            this.buttonMoveSelect.ToolTip = "Move currently selected area";
            this.buttonMoveSelect.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(new Image<Bgr, byte>(@"..\..\Images\undo.jpg"));

            this.buttonAddNewLayer.ToolTip = "Add Layer";
            this.buttonAddNewLayer.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(new Image<Bgr, byte>(@"..\..\Images\addLayer.jpg"));

            this.buttonRemoveLayer.ToolTip = "Remove Layer";
            this.buttonRemoveLayer.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(new Image<Bgr, byte>(@"..\..\Images\removeLayer.png"));

            this.buttonCopyLayer.ToolTip = "Copy Layer";
            this.buttonCopyLayer.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(new Image<Bgr, byte>(@"..\..\Images\copyLayer.png"));

            this.buttonOpenImageAsLayer.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(new Image<Bgr, byte>(@"..\..\Images\openFile.gif"));
            this.buttonOpenImageAsLayer.ToolTip = "Open Image as Layer";

            #endregion

            listLayers = new ObservableCollection<Layer>();
            
            this.layersList.ItemsSource = listLayers;
            RefreshButtons();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            new ListViewDragDropManager<Layer>(this.layersList).ProcessDrop += OnProcessDrop;
        }

        #region DragDropEvents

        void OnProcessDrop(object sender, ProcessDropEventArgs<Layer> e)
        {
            e.ItemsSource.Move(e.OldIndex, e.NewIndex);
            
            e.Effects = DragDropEffects.Move;
            
            if(e.OldIndex > e.NewIndex)
            {
                var oldLayer = layersImgList[e.OldIndex];

                for (int i = e.OldIndex; i > e.NewIndex; i--)
                {
                    layersImgList[i] = layersImgList[i - 1];
                }
                layersImgList[e.NewIndex] = oldLayer;
            }

            else
            {
                var oldLayer = layersImgList[e.OldIndex];
                
                for (int i = e.OldIndex; i < e.NewIndex; i++)
                {
                    layersImgList[i] = layersImgList[i + 1];
                }
                layersImgList[e.NewIndex] = oldLayer;
            }

            CalculateNewCenterImage();
        }

        #endregion

        private void viewAlternativesGrid_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            switch (this.viewAlternativesGrid.SelectedIndex)
            {
                case 2:
                    if (!(this.imgSrc != null && this.isAlgoSelected))
                    {
                        this.algoParamsVariant1.Visibility = System.Windows.Visibility.Collapsed;
                        this.algoParamsVariant2.Visibility = System.Windows.Visibility.Collapsed;
                    }

                    break;
            }
        }

        private void ButtonSelector_Click(object sender, RoutedEventArgs e)
        {
            (sender as Button).ContextMenu.IsEnabled = true;
            (sender as Button).ContextMenu.PlacementTarget = (sender as Button);
            (sender as Button).ContextMenu.Placement = System.Windows.Controls.Primitives.PlacementMode.Bottom;
            (sender as Button).ContextMenu.IsOpen = true;
        }

        private void RefreshButtons()
        {
            if(this.imgSrc == null)
            {
                this.buttonRevert.IsEnabled = false;
                this.buttonSave.IsEnabled = false;
                this.buttonSaveAsNew.IsEnabled = false;
                this.buttonZoomIn.IsEnabled = false;
                this.buttonZoomOut.IsEnabled = false;
                this.tbZoomFactor.Text = string.Empty;
                this.tbZoomFactor.IsEnabled = false;
                this.buttonBrightLess.IsEnabled = false;
                this.buttonBrightMore.IsEnabled = false;
                this.buttonUndo.IsEnabled = false;
                this.menuProcessing.IsEnabled = false;
                this.buttonAWB.IsEnabled = false;
            }
            else
            {
                this.buttonRevert.IsEnabled = true;
                this.buttonSave.IsEnabled = true;
                this.buttonSaveAsNew.IsEnabled = true;
                this.buttonZoomIn.IsEnabled = true;
                this.buttonZoomOut.IsEnabled = true;
                this.tbZoomFactor.Text = this.curZoomFactor.ToString();
                this.tbZoomFactor.IsEnabled = true;
                this.buttonBrightLess.IsEnabled = true;
                this.buttonBrightMore.IsEnabled = true;
                this.buttonAWB.IsEnabled = true;

                if(previousImages.Count > 0)
                {
                    this.buttonUndo.IsEnabled = true;
                }
                this.menuProcessing.IsEnabled = true;
            }

            if(this.isAlgoSelected == false)
            {
                this.viewAlternativesGrid.SelectedIndex = 0;
            }

            if(this.listLayers.Count == 0 || this.layersList.SelectedIndex == -1)
            {
                this.buttonRemoveLayer.IsEnabled = false;
                this.buttonCopyLayer.IsEnabled = false;
            }
            else
            {
                if (this.layersList.SelectedIndex != -1)
                {
                    this.buttonRemoveLayer.IsEnabled = true;
                    this.buttonCopyLayer.IsEnabled = true;
                }
            }
        }

        private void FixImageGrid()
        {
            this.imgGridSrc.Width = imgSrc.Width;
            this.imgGridSrc.Height = imgSrc.Height;

            this.selectedArea = new Rectangle(0, 0, imgSrc.Width, imgSrc.Height);
            this.imgSelected = Core.CropImage(this.imgSrc, this.selectedArea);
        }

        private Image<Bgr, Byte> UpdateAlgo(Image<Bgr, Byte> imgSr)
        {
            if (imgSr != null)
            {
                var imgDst = new Image<Bgr, Byte>(imgSr.Width, imgSr.Height);
                //ColectInfo
                switch (this.lastSelectedAlgoType)
                {
                    case algoTypes.ConvoMatrix:
                        try
                        {
                            ChangeStatusText("Convolution in process ...", StatusType.Process);

                            var matrix = new float[25];

                            matrix[0] = Int16.Parse(this.tbConvoParam1.Text);
                            matrix[1] = Int16.Parse(this.tbConvoParam2.Text);
                            matrix[2] = Int16.Parse(this.tbConvoParam3.Text);
                            matrix[3] = Int16.Parse(this.tbConvoParam4.Text);
                            matrix[4] = Int16.Parse(this.tbConvoParam5.Text);
                            matrix[5] = Int16.Parse(this.tbConvoParam6.Text);
                            matrix[6] = Int16.Parse(this.tbConvoParam7.Text);
                            matrix[7] = Int16.Parse(this.tbConvoParam8.Text);
                            matrix[8] = Int16.Parse(this.tbConvoParam9.Text);
                            matrix[9] = Int16.Parse(this.tbConvoParam10.Text);
                            matrix[10] = Int16.Parse(this.tbConvoParam11.Text);
                            matrix[11] = Int16.Parse(this.tbConvoParam12.Text);
                            matrix[12] = Int16.Parse(this.tbConvoParam13.Text);
                            matrix[13] = Int16.Parse(this.tbConvoParam14.Text);
                            matrix[14] = Int16.Parse(this.tbConvoParam15.Text);
                            matrix[15] = Int16.Parse(this.tbConvoParam16.Text);
                            matrix[16] = Int16.Parse(this.tbConvoParam17.Text);
                            matrix[17] = Int16.Parse(this.tbConvoParam18.Text);
                            matrix[18] = Int16.Parse(this.tbConvoParam19.Text);
                            matrix[19] = Int16.Parse(this.tbConvoParam20.Text);
                            matrix[20] = Int16.Parse(this.tbConvoParam21.Text);
                            matrix[21] = Int16.Parse(this.tbConvoParam22.Text);
                            matrix[22] = Int16.Parse(this.tbConvoParam23.Text);
                            matrix[23] = Int16.Parse(this.tbConvoParam24.Text);
                            matrix[24] = Int16.Parse(this.tbConvoParam25.Text);

                            imgDst = imgSr.ConvolutionGimp(matrix, 5);
                        }
                        catch (Exception)
                        {
                            ChangeStatusText("Unable to Parse algo parameters", StatusType.Error);
                        }

                        break;

                    case algoTypes.Brightness:
                        try
                        {
                            ChangeStatusText("Brightness correction in process ...", StatusType.Process);

                            var brightnessParam = Int16.Parse(this.tbAlgoParam1.Text);
                            imgDst = ImageProcessing.BrightnessCorrection(imgSr, brightnessParam, 1);
                        }
                        catch (Exception)
                        {
                            ChangeStatusText("Unable to Parse algo parameters", StatusType.Error);
                        }

                        break;

                    case algoTypes.Contrast:
                        try
                        {
                            ChangeStatusText("Contrast correction in process ...", StatusType.Process);

                            var contrastParam = Int16.Parse(this.tbAlgoParam1.Text);
                            imgDst = ImageProcessing.ContrastCorrection(imgSr, contrastParam);
                        }
                        catch (Exception)
                        {
                            ChangeStatusText("Unable to Parse algo parameters", StatusType.Error);
                        }

                        break;

                    case algoTypes.Unsharp:
                        try
                        {
                            ChangeStatusText("Unsharp mask in process ...", StatusType.Process);

                            var radiusParam = double.Parse(this.tbAlgoParam1.Text);
                            var amountParam = double.Parse(this.tbAlgoParam2.Text);
                            var threshParam = double.Parse(this.tbAlgoParam3.Text);

                            imgDst = imgSr.UnsharpenMask(radiusParam, amountParam, threshParam);
                        }
                        catch (Exception)
                        {
                            ChangeStatusText("Unable to Parse algo parameters", StatusType.Error);
                        }

                        break;

                    case algoTypes.Emboss:
                        try
                        {
                            ChangeStatusText("Emboss in process ...", StatusType.Process);

                            var azimuthParam = double.Parse(this.tbAlgoParam1.Text);
                            var angleParam = double.Parse(this.tbAlgoParam2.Text);
                            var depthParam = Int32.Parse(this.tbAlgoParam3.Text);

                            imgDst = imgSr.Emboss(azimuthParam, angleParam, depthParam);
                        }
                        catch (Exception)
                        {
                            ChangeStatusText("Unable to Parse algo parameters", StatusType.Error);
                        }

                        break;

                    case algoTypes.Comics:
                        try
                        {
                            ChangeStatusText("Comics effect in process ...", StatusType.Process);

                            var searchSize = Int32.Parse(this.tbAlgoParam1.Text);
                            var algoSelected = this.cbChooseParam6.SelectedIndex;
                            var thresholdEdge = Int32.Parse(this.tbAlgoParam2.Text);

                            int showContours = 0;
                            if (this.chbChooseParam5.IsChecked == true)
                            {
                                showContours = 1;
                            }
                            imgDst = imgSr.ComicsEffect(searchSize, algoSelected, thresholdEdge, showContours);
                        }
                        catch (Exception)
                        {
                            ChangeStatusText("Unable to Parse algo parameters", StatusType.Error);
                        }

                        break;

                    case algoTypes.Sharp:
                        try
                        {
                            ChangeStatusText("Diff of Gauss in process ...", StatusType.Process);

                            var percentage = Int32.Parse(this.tbAlgoParam1.Text);
                            var algoSelected = this.cbChooseParam6.SelectedIndex;

                            imgDst = imgSr.SharpImage(percentage, algoSelected);
                        }
                        catch (Exception)
                        {
                            ChangeStatusText("Unable to Parse algo parameters", StatusType.Error);
                        }

                        break;

                    case algoTypes.AWB:
                        try
                        {
                            ChangeStatusText("Auto White Balance in process ...", StatusType.Process);

                            var temperature = UInt32.Parse(this.tbAlgoParam1.Text);
                            var algoSelected = this.cbChooseParam6.SelectedIndex;

                            if (algoSelected == 0)
                            {
                                imgDst = imgSr.WhiteBalanceAlgo1((int)temperature);
                            }
                            else if (algoSelected == 1)
                            {
                                imgDst = imgSr.WhiteBalanceAlgo2((int)temperature);
                            }
                            else
                            {
                                imgDst = imgSr.WhiteBalanceAlgo3(1);
                            }
                        }
                        catch (Exception)
                        {
                            ChangeStatusText("Unable to Parse algo parameters", StatusType.Error);
                        }

                        break;

                }
                NullifyStatusLine();

                return imgDst;
            }
            else
            {
                return null;
            }
        }

        private Image<Bgr, Byte> MakeSmallImageFromLayer(Image<Bgr, Byte> imgSr, int newSize)
        {
            //Keep proportion - scale the bigger side to PREVIEW_IMAGE_SIZE px
            int origWidth = imgSr.Width;
            int origHeight = imgSr.Height;
            int newWidth = 0;
            int newHeight = 0;

            if (origWidth > origHeight)
            {
                var proportion = origWidth / (double)newSize;
                newWidth = newSize;
                newHeight = (int)(origHeight / proportion);
            }
            else
            {
                var proportion = origHeight / (double)newSize;
                newHeight = newSize;
                newWidth = (int)(origWidth / proportion);
            }

            var smallImage = imgSr.ScaleImageToXY(newWidth, newHeight);
            var fixedPreviewImage = smallImage.FixImagetoXY(newSize, newSize, out this.previewArea);

            return fixedPreviewImage;
        }

        private void UpdatePreviewImage()
        {
            var fixedPreviewImage = MakeSmallImageFromLayer(this.imgSelected, PREVIEW_IMAGE_SIZE);
            this.imgPreview = fixedPreviewImage;
            this.previewAlgoGrid.Background = WpfApplication2.Models.ImageConverter.FromEmguToBrush(fixedPreviewImage);
        }

        private void UndoBufferMove()
        {
            for (int i = 0; i < previousImages.Count - 1; i++)
            {
                previousImages[i].CopyTo(previousImages[i + 1]);
            }

            this.imgSrc.CopyTo(previousImages[0]);
        }

        #region Buttons

        private void buttonOpenBlank_Click(object sender, RoutedEventArgs e)
        {
            bool modify = false;

            if (this.listLayers.Count > 0)
            {
                var result = MessageBox.Show("Do you want to Reset workplace?", "Warning", MessageBoxButton.OKCancel);

                if(result == MessageBoxResult.OK)
                {
                    modify = true;
                }
            }
            else
            {
                modify = true;
            }

            if(modify == true)
            {
                try
                {
                    this.listLayers = new ObservableCollection<Layer>();
                    this.layersImgList = null;
                    this.layersImgList = new List<Image<Bgr, byte>>();

                    this.tbZoomFactor.Text = "100";
                    this.listLayers = new ObservableCollection<Layer>();

                    this.layersList.ItemsSource = this.listLayers;

                    this.layersList.ItemsSource = null;
                    this.layersList.ItemsSource = listLayers;

                    AddNewLayer_Click(null, null);
                    this.imgSrc = this.layersImgList[0];
                    this.imgGridSrc.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(imgSrc);

                    FixImageGrid();
                    RefreshButtons();
                }
                catch(Exception)
                {

                }
            }
        }

        private void buttonOpen_Click(object sender, RoutedEventArgs e)
        {
            ChangeStatusText("Select Image file ...", StatusType.Notification);

            try
            {
                OpenFileDialog dlg = new OpenFileDialog();
                
                bool? result = dlg.ShowDialog();

                if (result == true)
                {
                    ChangeStatusText("Opening Image file ...", StatusType.Process);

                    var imgOpened = new Image<Bgr, Byte>(dlg.FileName);

                    if (this.openImgAsLayer == false)
                    {
                        this.imgSrc = imgOpened;
                        this.imgGridSrc.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(imgSrc);

                        this.listLayers = new ObservableCollection<Layer>();
                        this.layersImgList = null;
                        this.layersImgList = new List<Image<Bgr, byte>>();

                        this.tbZoomFactor.Text = "100";
                        this.listLayers = new ObservableCollection<Layer>();
                        this.filename = dlg.FileName;
                    }

                    FixImageGrid();
                    RefreshButtons();

                    this.layersList.ItemsSource = this.listLayers;

                    var smallImage = MakeSmallImageFromLayer(imgOpened, 40);
                    var layerParam = new LayerParams();
                    layerParam.offsetY = 0;
                    layerParam.offsetX = 0;
                    layerParam.height = imgOpened.Height;
                    layerParam.width = imgOpened.Width;

                    this.listLayers.Add(new Layer("layer" + (this.listLayers.Count + 1).ToString(), smallImage, layerParam));
                    this.layersImgList.Add(imgOpened);

                    this.layersList.ItemsSource = null;
                    this.layersList.ItemsSource = listLayers;
                    NullifyStatusLine();
                }
            }
            catch (Exception)
            {
                ChangeStatusText("Unable to load file", StatusType.Error);
            }
        }

        private void buttonSave_Click(object sender, RoutedEventArgs e)
        {
            var emguImg = WpfApplication2.Models.ImageConverter.FromImageSourceToEmgu(this.imgGridSrc.Source);
            emguImg.Save(filename);
        }

        private void buttonSaveAsNew_Click(object sender, RoutedEventArgs e)
        {
            ChangeStatusText("Save Image as new File ...", StatusType.Notification);

            try
            {
                SaveFileDialog dlg = new SaveFileDialog();
                //dlg.Filter = "ppjs bin files (.ppjs.bin)|*.ppjs.bin";
                bool? result = dlg.ShowDialog();

                if (result == true)
                {
                    ChangeStatusText("Saving Image file ...", StatusType.Process);

                    var emguImg = WpfApplication2.Models.ImageConverter.FromImageSourceToEmgu(this.imgGridSrc.Source);
                    emguImg.Save(dlg.FileName);

                    this.filename = dlg.FileName;
                    NullifyStatusLine();
                }
            }
            catch (Exception)
            {
                ChangeStatusText("Unable to save file", StatusType.Error);
            }
        }

        private void buttonZoomIn_Click(object sender, RoutedEventArgs e)
        {
            this.curZoomFactor += 10;
            this.tbZoomFactor.Text = this.curZoomFactor.ToString();
        }

        private void buttonZoomOut_Click(object sender, RoutedEventArgs e)
        {
            this.curZoomFactor -= 10;

            if (this.curZoomFactor < 10) this.curZoomFactor = 10;
            this.tbZoomFactor.Text = this.curZoomFactor.ToString();
        }

        private void buttonRevert_Click(object sender, RoutedEventArgs e)
        {
            imgSrc = new Image<Bgr, Byte>(this.filename);
            this.imgGridSrc.Source = null;
            this.imgGridSrc.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(imgSrc);
            this.imgGridSrc.UpdateLayout();

            FixImageGrid();

            this.listLayers = new ObservableCollection<Layer>();
            this.layersList.ItemsSource = this.listLayers;

            var zoomFact = this.curZoomFactor;
            this.tbZoomFactor.Text = "100";
            this.tbZoomFactor.Text = zoomFact.ToString();
            this.curZoomFactor = zoomFact;

            foreach(var image in previousImages)
            {
                image.Dispose();
            }
            this.previousImages = new List<Image<Bgr, byte>>(NUMBER_OF_UNDO_ACTIONS);
            for (int i = 0; i < NUMBER_OF_UNDO_ACTIONS; i++)
            {
                this.previousImages.Add(new Image<Bgr, byte>(0, 0));
            }

            RefreshButtons();
        }

        private void buttonUndo_Click(object sender, RoutedEventArgs e)
        {
            this.previousImages[0].CopyTo(this.imgSrc);

            for(int i = 0; i < NUMBER_OF_UNDO_ACTIONS - 1; i++)
            {
                this.previousImages[i] = this.previousImages[i + 1];
            }

            FixImageGrid();
            var curZoomFact = this.curZoomFactor;
            this.tbZoomFactor.Text = "100";
            this.tbZoomFactor.Text = curZoomFact.ToString();
        }

        private void buttonBrightLess_Click(object sender, RoutedEventArgs e)
        {
            //UndoBufferMove();

            this.imgSrc = ImageProcessing.BrightnessCorrection(this.imgSrc, -5, 1);
            this.imgGridSrc.Source = null;
            this.imgGridSrc.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(this.imgSrc);
            this.imgGridSrc.UpdateLayout();
        }

        private void buttonBrightMore_Click(object sender, RoutedEventArgs e)
        {
            //UndoBufferMove();

            this.imgSrc = ImageProcessing.BrightnessCorrection(this.imgSrc, 5, 1);
            this.imgGridSrc.Source = null;
            this.imgGridSrc.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(this.imgSrc);
            this.imgGridSrc.UpdateLayout();
        }

        private void buttonAWB_Click(object sender, RoutedEventArgs e)
        {
            menuAWB_Click(null, null);
        }

        private void applyAlgoButton_Click(object sender, RoutedEventArgs e)
        {
            //UndoBufferMove();

            this.imgSrc = this.UpdateAlgo(this.imgSrc);

            this.imgGridSrc.Source = null;
            this.imgGridSrc.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(this.imgSrc);
            this.imgGridSrc.UpdateLayout();
        }

        #endregion

        #region Menu
        private void menuAbout_Click(object sender, RoutedEventArgs e)
        {
            new AboutWindow().ShowDialog();
        }

        private void menuOpenFile_Click(object sender, RoutedEventArgs e)
        {
            this.buttonOpen_Click(null, null);
        }

        private void menuConvoMatrix_Click(object sender, RoutedEventArgs e)
        {
            this.isAlgoSelected = true;
            this.lastSelectedAlgoType = algoTypes.ConvoMatrix;
            RefreshButtons();

            this.textAlgoName.Text = "Convolution Matrix";

            this.algoParamsVariant1.Visibility = System.Windows.Visibility.Collapsed;
            this.algoParamsVariant2.Visibility = System.Windows.Visibility.Visible;
            this.viewAlternativesGrid.SelectedIndex = 0;
            this.viewAlternativesGrid.SelectedIndex = 2;
            UpdatePreviewImage();
        }

        private void menuBrightness_Click(object sender, RoutedEventArgs e)
        {
            this.isAlgoSelected = true;
            this.lastSelectedAlgoType = algoTypes.Brightness;
            RefreshButtons();

            this.algoParamsVariant1.Visibility = System.Windows.Visibility.Visible;
            this.algoParamsVariant2.Visibility = System.Windows.Visibility.Collapsed;

            this.textAlgoName.Text = "Brightness correction";

            this.textAlgoParam1.Visibility = System.Windows.Visibility.Visible;
            this.tbAlgoParam1.Visibility = System.Windows.Visibility.Visible;
            this.textAlgoParam2.Visibility = System.Windows.Visibility.Collapsed;
            this.tbAlgoParam2.Visibility = System.Windows.Visibility.Collapsed;
            this.textAlgoParam3.Visibility = System.Windows.Visibility.Collapsed;
            this.tbAlgoParam3.Visibility = System.Windows.Visibility.Collapsed;
            this.textAlgoParam4.Visibility = System.Windows.Visibility.Collapsed;
            this.tbAlgoParam4.Visibility = System.Windows.Visibility.Collapsed;
            this.chbChooseParam5.Visibility = System.Windows.Visibility.Collapsed;
            this.textAlgoParam5.Visibility = System.Windows.Visibility.Collapsed;
            this.cbChooseParam6.Visibility = System.Windows.Visibility.Collapsed;
            this.textAlgoParam6.Visibility = System.Windows.Visibility.Collapsed;

            this.textAlgoParam1.Text = "Luma diff";

            this.viewAlternativesGrid.SelectedIndex = 0;
            this.viewAlternativesGrid.SelectedIndex = 2;
            UpdatePreviewImage();
        }

        private void menuContrast_Click(object sender, RoutedEventArgs e)
        {
            this.isAlgoSelected = true;
            this.lastSelectedAlgoType = algoTypes.Contrast;
            RefreshButtons();

            this.algoParamsVariant1.Visibility = System.Windows.Visibility.Visible;
            this.algoParamsVariant2.Visibility = System.Windows.Visibility.Collapsed;

            this.textAlgoName.Text = "Contrast Correction";

            this.textAlgoParam1.Visibility = System.Windows.Visibility.Visible;
            this.tbAlgoParam1.Visibility = System.Windows.Visibility.Visible;
            this.textAlgoParam2.Visibility = System.Windows.Visibility.Collapsed;
            this.tbAlgoParam2.Visibility = System.Windows.Visibility.Collapsed;
            this.textAlgoParam3.Visibility = System.Windows.Visibility.Collapsed;
            this.tbAlgoParam3.Visibility = System.Windows.Visibility.Collapsed;
            this.textAlgoParam4.Visibility = System.Windows.Visibility.Collapsed;
            this.tbAlgoParam4.Visibility = System.Windows.Visibility.Collapsed;
            this.chbChooseParam5.Visibility = System.Windows.Visibility.Collapsed;
            this.textAlgoParam5.Visibility = System.Windows.Visibility.Collapsed;
            this.cbChooseParam6.Visibility = System.Windows.Visibility.Collapsed;
            this.textAlgoParam6.Visibility = System.Windows.Visibility.Collapsed;

            this.textAlgoParam1.Text = "Contrast diff";

            this.viewAlternativesGrid.SelectedIndex = 0;
            this.viewAlternativesGrid.SelectedIndex = 2;
            UpdatePreviewImage();
        }

        private void menuUnsharp_Click(object sender, RoutedEventArgs e)
        {
            this.isAlgoSelected = true;
            this.lastSelectedAlgoType = algoTypes.Unsharp;
            RefreshButtons();

            this.algoParamsVariant1.Visibility = System.Windows.Visibility.Visible;
            this.algoParamsVariant2.Visibility = System.Windows.Visibility.Collapsed;

            this.textAlgoName.Text = "Unsharp Mask";

            this.textAlgoParam1.Visibility = System.Windows.Visibility.Visible;
            this.tbAlgoParam1.Visibility = System.Windows.Visibility.Visible;
            this.textAlgoParam2.Visibility = System.Windows.Visibility.Visible;
            this.tbAlgoParam2.Visibility = System.Windows.Visibility.Visible;
            this.textAlgoParam3.Visibility = System.Windows.Visibility.Visible;
            this.tbAlgoParam3.Visibility = System.Windows.Visibility.Visible;
            this.textAlgoParam4.Visibility = System.Windows.Visibility.Collapsed;
            this.tbAlgoParam4.Visibility = System.Windows.Visibility.Collapsed;
            this.chbChooseParam5.Visibility = System.Windows.Visibility.Collapsed;
            this.textAlgoParam5.Visibility = System.Windows.Visibility.Collapsed;
            this.cbChooseParam6.Visibility = System.Windows.Visibility.Collapsed;
            this.textAlgoParam6.Visibility = System.Windows.Visibility.Collapsed;

            this.textAlgoParam1.Text = "Radius";
            this.textAlgoParam2.Text = "Amount";
            this.textAlgoParam3.Text = "Threshold";

            this.viewAlternativesGrid.SelectedIndex = 0;
            this.viewAlternativesGrid.SelectedIndex = 2;
            UpdatePreviewImage();
        }

        private void menuEmboss_Click(object sender, RoutedEventArgs e)
        {
            this.isAlgoSelected = true;
            this.lastSelectedAlgoType = algoTypes.Emboss;
            RefreshButtons();

            this.algoParamsVariant1.Visibility = System.Windows.Visibility.Visible;
            this.algoParamsVariant2.Visibility = System.Windows.Visibility.Collapsed;

            this.textAlgoName.Text = "Emboss effect";

            this.textAlgoParam1.Visibility = System.Windows.Visibility.Visible;
            this.tbAlgoParam1.Visibility = System.Windows.Visibility.Visible;
            this.textAlgoParam2.Visibility = System.Windows.Visibility.Visible;
            this.tbAlgoParam2.Visibility = System.Windows.Visibility.Visible;
            this.textAlgoParam3.Visibility = System.Windows.Visibility.Visible;
            this.tbAlgoParam3.Visibility = System.Windows.Visibility.Visible;
            this.textAlgoParam4.Visibility = System.Windows.Visibility.Collapsed;
            this.tbAlgoParam4.Visibility = System.Windows.Visibility.Collapsed;
            this.chbChooseParam5.Visibility = System.Windows.Visibility.Collapsed;
            this.textAlgoParam5.Visibility = System.Windows.Visibility.Collapsed;
            this.cbChooseParam6.Visibility = System.Windows.Visibility.Collapsed;
            this.textAlgoParam6.Visibility = System.Windows.Visibility.Collapsed;

            this.textAlgoParam1.Text = "Azimuth";
            this.textAlgoParam2.Text = "Angle";
            this.textAlgoParam3.Text = "Depth";

            this.viewAlternativesGrid.SelectedIndex = 0;
            this.viewAlternativesGrid.SelectedIndex = 2;
            UpdatePreviewImage();
        }

        private void menuComics_Click(object sender, RoutedEventArgs e)
        {
            this.isAlgoSelected = true;
            this.lastSelectedAlgoType = algoTypes.Comics;
            RefreshButtons();

            this.algoParamsVariant1.Visibility = System.Windows.Visibility.Visible;
            this.algoParamsVariant2.Visibility = System.Windows.Visibility.Collapsed;

            this.textAlgoName.Text = "Comics Effect";

            this.textAlgoParam1.Visibility = System.Windows.Visibility.Visible;
            this.tbAlgoParam1.Visibility = System.Windows.Visibility.Visible;
            this.textAlgoParam2.Visibility = System.Windows.Visibility.Visible;
            this.tbAlgoParam2.Visibility = System.Windows.Visibility.Visible;
            this.textAlgoParam3.Visibility = System.Windows.Visibility.Collapsed;
            this.tbAlgoParam3.Visibility = System.Windows.Visibility.Collapsed;
            this.textAlgoParam4.Visibility = System.Windows.Visibility.Collapsed;
            this.tbAlgoParam4.Visibility = System.Windows.Visibility.Collapsed;

            this.chbChooseParam5.Visibility = System.Windows.Visibility.Visible;
            this.textAlgoParam5.Visibility = System.Windows.Visibility.Visible;
            this.cbChooseParam6.Visibility = System.Windows.Visibility.Visible;
            this.textAlgoParam6.Visibility = System.Windows.Visibility.Visible;

            this.textAlgoParam1.Text = "Search size";
            this.textAlgoParam2.Text = "Threshold";
            this.textAlgoParam5.Text = "Show contours";
            this.textAlgoParam6.Text = "Edge extract algo";

            List<string> listCb = new List<string>();

            listCb.Add("Prewitt");
            listCb.Add("Sobel");
            listCb.Add("Gradient");

            this.cbChooseParam6.ItemsSource = listCb;
            
            this.viewAlternativesGrid.SelectedIndex = 0;
            this.viewAlternativesGrid.SelectedIndex = 2;
            UpdatePreviewImage();

            this.cbChooseParam6.SelectedIndex = 0;
        }

        private void menuSharp_Click(object sender, RoutedEventArgs e)
        {
            this.isAlgoSelected = true;
            this.lastSelectedAlgoType = algoTypes.Sharp;
            RefreshButtons();

            this.algoParamsVariant1.Visibility = System.Windows.Visibility.Visible;
            this.algoParamsVariant2.Visibility = System.Windows.Visibility.Collapsed;

            this.textAlgoName.Text = "Sharpening";

            this.textAlgoParam1.Visibility = System.Windows.Visibility.Visible;
            this.tbAlgoParam1.Visibility = System.Windows.Visibility.Visible;
            this.textAlgoParam2.Visibility = System.Windows.Visibility.Collapsed;
            this.tbAlgoParam2.Visibility = System.Windows.Visibility.Collapsed;
            this.textAlgoParam3.Visibility = System.Windows.Visibility.Collapsed;
            this.tbAlgoParam3.Visibility = System.Windows.Visibility.Collapsed;
            this.textAlgoParam4.Visibility = System.Windows.Visibility.Collapsed;
            this.tbAlgoParam4.Visibility = System.Windows.Visibility.Collapsed;
            this.chbChooseParam5.Visibility = System.Windows.Visibility.Collapsed;
            this.textAlgoParam5.Visibility = System.Windows.Visibility.Collapsed;
            this.cbChooseParam6.Visibility = System.Windows.Visibility.Visible;
            this.textAlgoParam6.Visibility = System.Windows.Visibility.Visible;

            this.textAlgoParam1.Text = "Diff (%)";
            this.cbChooseParam6.Text = "Outer Radius";

            List<string> listCb = new List<string>();

            listCb.Add("Prewitt");
            listCb.Add("Sobel");
            listCb.Add("Gradient");

            this.cbChooseParam6.ItemsSource = listCb;
            
            this.viewAlternativesGrid.SelectedIndex = 0;
            this.viewAlternativesGrid.SelectedIndex = 2;
            UpdatePreviewImage();

            this.cbChooseParam6.SelectedIndex = 0;
        }
        
        private void menuAWB_Click(object sender, RoutedEventArgs e)
        {
            this.isAlgoSelected = true;
            this.lastSelectedAlgoType = algoTypes.AWB;
            RefreshButtons();

            this.algoParamsVariant1.Visibility = System.Windows.Visibility.Visible;
            this.algoParamsVariant2.Visibility = System.Windows.Visibility.Collapsed;

            this.textAlgoName.Text = "Auto White Balance";

            this.textAlgoParam1.Visibility = System.Windows.Visibility.Visible;
            this.tbAlgoParam1.Visibility = System.Windows.Visibility.Visible;
            this.textAlgoParam2.Visibility = System.Windows.Visibility.Collapsed;
            this.tbAlgoParam2.Visibility = System.Windows.Visibility.Collapsed;
            this.textAlgoParam3.Visibility = System.Windows.Visibility.Collapsed;
            this.tbAlgoParam3.Visibility = System.Windows.Visibility.Collapsed;
            this.textAlgoParam4.Visibility = System.Windows.Visibility.Collapsed;
            this.tbAlgoParam4.Visibility = System.Windows.Visibility.Collapsed;
            this.chbChooseParam5.Visibility = System.Windows.Visibility.Collapsed;
            this.textAlgoParam5.Visibility = System.Windows.Visibility.Collapsed;
            this.cbChooseParam6.Visibility = System.Windows.Visibility.Visible;
            this.textAlgoParam6.Visibility = System.Windows.Visibility.Visible;

            List<string> listCb = new List<string>();

            listCb.Add("XYZ-RGB");
            listCb.Add("XYZ-UV");
            listCb.Add("Basic RGB");

            this.cbChooseParam6.ItemsSource = listCb;

            this.viewAlternativesGrid.SelectedIndex = 0;
            this.viewAlternativesGrid.SelectedIndex = 2;
            UpdatePreviewImage();

            this.cbChooseParam6.SelectedIndex = 0;
        }

        private void menuSelector_Click(object sender, RoutedEventArgs e)
        {
            SelectButtonWindow selectorWindow = new SelectButtonWindow();

            if (selectorWindow.ShowDialog() == true)
            {

            }
        }

        #endregion

        #region Key_Events
        private void Window_KeyDown(object sender, KeyEventArgs e)
        {
            if(e.Key == Key.LeftCtrl)
            {
                this.isCtrlKeyPressed = true;
            }

            if(e.Key == Key.O && this.isCtrlKeyPressed == true)
            {
                this.buttonOpen_Click(null, null);
            }
        }

        private void Window_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.LeftCtrl)
            {
                this.isCtrlKeyPressed = false;
            }
        }

        #endregion

        #region TextChanged_Events
        private void tbZoomFactor_TextChanged(object sender, RoutedEventArgs e)
        {
            if (imgSrc != null)
            {
                UInt16 newPercentage;
                var result = UInt16.TryParse(this.tbZoomFactor.Text, out newPercentage);

                if (result == true)
                {
                    var factor = newPercentage / 100.0;

                    this.imgGridSrc.Width = factor * imgSrc.Width;
                    this.imgGridSrc.Height = factor * imgSrc.Height;

                    this.curZoomFactor = newPercentage;
                }
                else
                {
                    this.tbZoomFactor.Text = "100";
                    this.curZoomFactor = 100;
                }
            }
        }

        private void OnTextChangeEvent()
        {
            if (this.isReset == false)
            {
                if (previewAlgoGrid != null)
                {
                    var imgModified = UpdateAlgo(this.imgPreview);
                    this.previewAlgoGrid.Background = null;
                    this.previewAlgoGrid.Background = WpfApplication2.Models.ImageConverter.FromEmguToBrush(imgModified);

                    this.previewHistogramGrid.Background = null;
                    this.previewHistogramGrid.Background =
                        WpfApplication2.Models.ImageConverter.FromEmguToBrush(Core.CropImage(imgModified, this.previewArea).GetHistImageBgr());
                }
            }
        }

        private void tbAlgoParam1_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbAlgoParam2_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbAlgoParam3_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbAlgoParam4_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void cbChooseParam6_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if(this.lastSelectedAlgoType == algoTypes.AWB)
            {
                if(cbChooseParam6.SelectedIndex == 2)
                {
                    this.textAlgoParam1.Visibility = System.Windows.Visibility.Collapsed;
                    this.tbAlgoParam1.Visibility = System.Windows.Visibility.Collapsed;
                }
                else
                {
                    this.textAlgoParam1.Visibility = System.Windows.Visibility.Visible;
                    this.tbAlgoParam1.Visibility = System.Windows.Visibility.Visible;
                }
            }
            OnTextChangeEvent();
        }

        private void chbChooseParam5_Checked(object sender, RoutedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam1_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam2_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam3_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam4_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam5_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam6_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam7_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam8_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam9_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam10_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam11_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam12_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam13_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam14_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam15_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam16_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam17_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam18_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam19_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam20_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam21_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam22_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam23_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam24_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        private void tbConvoParam25_TextChanged(object sender, TextChangedEventArgs e)
        {
            OnTextChangeEvent();
        }

        #endregion

        #region StatusLine

        public void ChangeStatusText(string text, StatusType type)
        {
            MainWindow._MainWindow.statusLineText.Text = text;

            switch (type)
            {
                case StatusType.Notification:
                    MainWindow._MainWindow.statusLineHeader.Background = System.Windows.Media.Brushes.DarkGray;
                    MainWindow._MainWindow.statusLineText.Background = System.Windows.Media.Brushes.DarkGray;
                    break;

                case StatusType.Process:
                    MainWindow._MainWindow.statusLineHeader.Background = System.Windows.Media.Brushes.LightGreen;
                    MainWindow._MainWindow.statusLineText.Background = System.Windows.Media.Brushes.LightGreen;
                    break;

                case StatusType.Warning:
                    MainWindow._MainWindow.statusLineHeader.Background = System.Windows.Media.Brushes.LightYellow;
                    MainWindow._MainWindow.statusLineText.Background = System.Windows.Media.Brushes.LightYellow;
                    break;

                case StatusType.Error:
                    MainWindow._MainWindow.statusLineHeader.Background = System.Windows.Media.Brushes.LightPink;
                    MainWindow._MainWindow.statusLineText.Background = System.Windows.Media.Brushes.LightPink;
                    break;
            }
        }

        public void NullifyStatusLine()
        {
            MainWindow._MainWindow.statusLineText.Text = string.Empty;

            MainWindow._MainWindow.statusLineHeader.Background = System.Windows.Media.Brushes.DarkGray;
            MainWindow._MainWindow.statusLineText.Background = System.Windows.Media.Brushes.DarkGray;
        }
        #endregion

        private void layersList_MouseRightButtonDown(object sender, MouseButtonEventArgs e)
        {

        }

        private void ResetConvolution_Click(object sender, RoutedEventArgs e)
        {
            this.isReset = true;

            this.tbConvoParam1.Text = "0";
            this.tbConvoParam2.Text = "0";
            this.tbConvoParam3.Text = "0";
            this.tbConvoParam4.Text = "0";
            this.tbConvoParam5.Text = "0";
            this.tbConvoParam6.Text = "0";
            this.tbConvoParam7.Text = "0";
            this.tbConvoParam8.Text = "0";
            this.tbConvoParam9.Text = "0";
            this.tbConvoParam10.Text = "0";
            this.tbConvoParam11.Text = "0";
            this.tbConvoParam12.Text = "0";

            this.tbConvoParam14.Text = "0";
            this.tbConvoParam15.Text = "0";
            this.tbConvoParam16.Text = "0";
            this.tbConvoParam17.Text = "0";
            this.tbConvoParam18.Text = "0";
            this.tbConvoParam19.Text = "0";
            this.tbConvoParam20.Text = "0";
            this.tbConvoParam21.Text = "0";
            this.tbConvoParam22.Text = "0";
            this.tbConvoParam23.Text = "0";
            this.tbConvoParam24.Text = "0";
            this.tbConvoParam25.Text = "0";

            /*Update on the last parameter*/
            this.isReset = false;
            this.tbConvoParam13.Text = "0";
            this.tbConvoParam13.Text = "1";
        }

        #region layer_buttons
        private void OpenImageAsLayer_Click(object sender, RoutedEventArgs e)
        {
            this.openImgAsLayer = true;

            this.buttonOpen_Click(null, null);

            this.openImgAsLayer = false;
        }

        private void AddNewLayer_Click(object sender, RoutedEventArgs e)
        {
            var addLayerWindow = new AddLayerWindow();

            if (addLayerWindow.ShowDialog() == true)
            {
                if (addLayerWindow.DialogResult == true)
                {
                    var newImgLayer = new Image<Bgr, Byte>(addLayerWindow.layerParams.width, addLayerWindow.layerParams.height, addLayerWindow.layerParams.initColor);
                    var smallImage = MakeSmallImageFromLayer(newImgLayer, 40);
                    this.listLayers.Add(new Layer("layer" + (this.listLayers.Count + 1).ToString(), smallImage, addLayerWindow.layerParams));

                    this.layersImgList.Add(newImgLayer);
                    CalculateNewCenterImage();
                }
            }
        }

        private void CopyLayer_Click(object sender, RoutedEventArgs e)
        {
            var selectedIndex = layersList.SelectedIndex;
            var newImgLayer = layersImgList[selectedIndex];

            var smallImage = MakeSmallImageFromLayer(newImgLayer, 40);
            this.listLayers.Add(new Layer("layer" + (this.listLayers.Count + 1).ToString(), smallImage, listLayers[selectedIndex].layerParams));

            this.layersImgList.Add(newImgLayer);
            CalculateNewCenterImage();
        }

        private void RemoveLayer_Click(object sender, RoutedEventArgs e)
        {
            this.listLayers.RemoveAt(this.layersList.SelectedIndex);
            this.layersImgList.RemoveAt(this.layersList.SelectedIndex);
            CalculateNewCenterImage();
        }

        private void layersList_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            this.RefreshButtons();
        }
        #endregion

        #region ModifyLayer_events
        private void CalculateNewCenterImage()
        {
            //Use the blend effect starting from the bottom layers

            int maxWidth = 0;
            int maxHeight = 0;

            //calculate imgDimensions
            foreach(var layer in listLayers)
            {
                var curWidth = layer.layerParams.width + layer.layerParams.offsetX;
                var curHeight = layer.layerParams.height + layer.layerParams.offsetY;

                if (maxWidth < curWidth) maxWidth = curWidth;
                if (maxHeight < curHeight) maxHeight = curHeight;
            }

            try
            {
                int previousVisibleIndex = -1;
                var imgBlend = new Image<Bgr, Byte>(maxWidth, maxHeight, new Bgr(255,255,255));

                var transparentColor = this.chbWhiteColorTransperant.IsChecked == true ? true: false;

                if (this.listLayers[0].IsVisible == true)
                {
                    imgBlend = imgBlend.BlendImageLayers(this.layersImgList[0], 0, listLayers[0].Opacity, listLayers[0].layerParams, transparentColor);
                    previousVisibleIndex = 0;
                }

                for (int i = 0; i < listLayers.Count; i++)
                {
                    if (this.listLayers[i].IsVisible == true)
                    {
                        int lastVisibleOpacity = previousVisibleIndex == -1 ? 0 : listLayers[previousVisibleIndex].Opacity;
                        imgBlend = imgBlend.BlendImageLayers(this.layersImgList[i], lastVisibleOpacity, listLayers[i].Opacity, listLayers[i].layerParams, transparentColor);
                        
                        previousVisibleIndex = i;
                    }
                }

                this.imgSrc = imgBlend;
            }
            catch(Exception)
            {
                MessageBox.Show("There was an error with Layers!");
            }

            this.imgGridSrc.Source = WpfApplication2.Models.ImageConverter.FromEmguToImageSource(imgSrc);
            this.imgGridSrc.UpdateLayout();

            FixImageGrid();
        }

        private void chbLayerVisibility_Checked(object sender, RoutedEventArgs e)
        {
            CalculateNewCenterImage();
        }

        private void chbLayerVisibility_Unchecked(object sender, RoutedEventArgs e)
        {
            CalculateNewCenterImage();
        }

        private void TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            try
            {
                /*Update all Layer opacity fields*/
                //for (int i = 0; i < listLayers.Count; i++)
                //{
                listLayers[layersList.SelectedIndex].Opacity = Int32.Parse((sender as TextBox).Text);
                //}
                
                CalculateNewCenterImage();
            }
            
            catch(Exception)
            {

            }
        }

        private void layersList_SourceUpdated(object sender, DataTransferEventArgs e)
        {
            CalculateNewCenterImage();
        }

        private void chbWhiteColorTransperant_Checked(object sender, RoutedEventArgs e)
        {
            CalculateNewCenterImage();
        }

        private void chbWhiteColorTransperant_Unchecked(object sender, RoutedEventArgs e)
        {
            CalculateNewCenterImage();
        }
        #endregion

        #region Tools_DropButtons
        private void buttonMoveSelect_Click(object sender, RoutedEventArgs e)
        {
            ImageBrush imgMoveTool = new ImageBrush();
            imgMoveTool.ImageSource = new BitmapImage(new Uri(@"..\..\Images\copyLayer.png", UriKind.Relative));
            this.dropDownSelectionMenu.Background = imgMoveTool;
            this.dropDownSelectionMenu.ToolTip = "Move selected Area";

            this.buttonMoveSelect.Visibility = System.Windows.Visibility.Collapsed;
            this.dropDownSelectionMenu.IsOpen = false;
        }

        private void buttonRectSelect_Click(object sender, RoutedEventArgs e)
        {
            ImageBrush imgRectTool = new ImageBrush();
            imgRectTool.ImageSource = new BitmapImage(new Uri(@"..\..\Images\undo.jpg", UriKind.Relative));
            this.dropDownSelectionMenu.Background = imgRectTool;
            this.dropDownSelectionMenu.ToolTip = "Select Rect Area";

            this.buttonRectSelect.Visibility = System.Windows.Visibility.Collapsed;
            this.dropDownSelectionMenu.IsOpen = false;
        }

        private void buttonRectDraw_Click(object sender, RoutedEventArgs e)
        {

        }

        private void buttonCircleDraw_Click(object sender, RoutedEventArgs e)
        {

        }

        private void dropDownSelectionMenu_Click(object sender, RoutedEventArgs e)
        {

        }

        private void dropDownDrawingMenu_Click(object sender, RoutedEventArgs e)
        {

        }
        #endregion

    }
}
