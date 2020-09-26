/************************************************************************************
*																					*
*  This header file contain all function prototypes used in the CV library			*
*  																					*
*	The file is limited for the purpose of 	https://github.com/petarnikolovprojects	*	
*																					*
*	Author: Petar Nikolov															*
*																					*
*																					*
*************************************************************************************/

/* Function prototypes - Filtered list*/


/* Creates new image, based on a prototype ( for width, height, Num of chanels and color space ) */
struct Image	 CreateNewImage_BasedOnPrototype(struct Image *Prototype, struct Image *Img_dst);
/* Creates new image. Specify width, height, number of channels and color space */
struct Image	 CreateNewImage(struct Image *Img_dst, int Width, int Height, int NumChannels, int ColorSpace, int Depth);
/* Set destination Image parameters to match the ones in source image */
struct Image	 SetDestination(struct Image *Prototype, struct Image *Img_dst);
/* Release the memory allocated by calling CreateNewImage */
void			 DestroyImage(struct Image *Img);
/* Read Jpeg file and load it into memory */
struct Image	 ReadImage(char *filename);
struct Image	 read_Image_file(FILE *file);
/* Write Jpeg file. Parameter: quality (0, 100) */
void			 WriteImage(char *filename, struct Image, int quality);
/* Blur image - circle around point. Parameter: Point, Radius, BlurOrSharp: 0= Blur */
struct Image	 BlurImageAroundPoint(struct Image *Img_src, struct Image *Img_dst, struct point_xy CentralPoint, int BlurPixelRadius, int SizeOfBlur, int BlurOrSharp, int BlurAgression);

struct Image	 BlurImageGussian(struct Image *Img_src, struct Image *Img_dst, int BlurPixelRadius, float NeighborCoefficient);
struct Image	 BrightnessCorrection(struct Image *Img_src, struct Image *Img_dst, float Algo_paramBrightnessOrEV, int Algotype);
struct Image	 ContrastCorrection(struct Image *Img_src, struct Image *Img_dst, float percentage);
struct Image	 WhiteBalanceCorrectionRGB(struct Image *Img_src, struct Image *Img_dst, int Algotype);
//struct Image	 WhiteBalanceCorrectionLAB(struct Image *Img_src, struct Image *Img_dst, struct WhitePoint WhitePointXYZ);
struct Image	 NoiseCorrection(struct Image *Img_src, struct Image *Img_dst, float threshold, int Algotype);
struct Image	 GammaCorrection(struct Image *Img_src, struct Image *Img_dst, float RedGamma, float GreenGamma, float BlueGamma);
void			 getPositionFromIndex(struct Image *Img_src, int pixIdx, int *red, int *col);
int				 getPixelIndex(struct Image *Img_src, int *pixIdx, int red, int col);
struct Image	 ConvertToGrayscale_3Channels(struct Image *Img_src, struct Image *Img_dst);
struct Image	 ConvertToGrayscale_1Channel(struct Image *Img_src, struct Image *Img_dst);
struct Image	 ScaleImage(struct Image *Img_src, struct Image *Img_dst, float ScalePercentage);
struct Image     ScaleImageToXY(struct Image *Img_src, struct Image *Img_dst, int NewWidth, int NewHeight);
struct Image	 TranslateImage(struct Image *Img_src, struct Image *Img_dst, struct point_xy ToPoint);
struct Image	 RotateImage(struct Image *Img_src, struct Image *Img_dst, float RotationAngle, struct point_xy CentralPoint);
struct ArrPoints EdgeExtraction(struct Image *Img_src, struct Image *Img_dst, int Algotype, float Algo_param1, float Algo_param2);
void			 FindDerrivative_XY(struct Image *Img_src, struct Image *DerrivativeX_image, struct Image *DerrivativeY_image);
void			 FindMagnitudeOfGradient(struct Image *DerrivativeX_image, struct Image *DerrivativeY_image, struct Image *Magnitude);
void			 FindNonMaximumSupp(struct Image *Magnitude, struct Image *DerrivativeX, struct Image *DerrivativeY, struct Image *NMS);
void			 FindHysteresis(struct Image *Magnitude, struct Image *NMS, struct Image *Img_dst, float Algo_param1, float Algo_param2);
void			 Follow_edges(unsigned char *edgemapptr, unsigned char *edgemagptr, unsigned char lowval, int cols);
void			 Convolution(unsigned char *InputArray, unsigned char *OutputArray, int rows, int cols, float *Kernel, int KernelSize);
void			 ConvolutionBinary(unsigned char *InputArray, unsigned char *OutputArray, int rows, int cols, float *Kernel, int KernelSize, int DilateOrErode);
struct Image	 MirrorImageHorizontal(struct Image *Img_src, struct Image *Img_dst);
struct Image	 MirrorImageVertical(struct Image *Img_src, struct Image *Img_dst);
struct Image	 CropImage(struct Image *Img_src, struct Image *Img_dst, struct point_xy CentralPoint, int NewWidth, int NewHeight);
struct Image	 MorphDilate(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberofIterations);
struct Image	 MorphErode(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberofIterations);
struct Image	 MorphOpen(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberofIterations);
struct Image	 MorphClose(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberofIterations);
struct Image     SharpImageContours(struct Image *Img_src, struct Image *Img_dst , float Percentage);
struct Image     SharpImageBinary(struct Image *Img_src, struct Image *Img_dst, struct Image *Img_Binary , float Percentage);
struct Image     ColorFromGray(struct Image *Img_src, struct Image *Img_dst, struct ColorPoint_RGB ColorPoint);
struct Image	 ConvertToBinary(struct Image *Img_src, struct Image *Img_dst, /* 0 for automatic */ int Threshold);
/* Change image color space - RGB to HSL. Both Src and Dst have to be 3 channeled images.*/
void			 ConvertImage_RGB_to_HSL(struct Image *Img_src, struct Image *Img_dst);
/* Change image color space - HSL to RGB. Both Src and Dst have to be 3 channeled images.*/
void			 ConvertImage_HSL_to_RGB(struct Image *Img_src, struct Image *Img_dst);
/* Change image saturation by percentage. Params: 1: Input Image - HSL or RGB, 2: Output Image - HSL or RGB, Percentage to increase/decrease saturation (-100, 100) */
struct Image	 Saturation(struct Image *Img_src, struct Image *Img_dst, float percentage);
/* Change image color space - RGB to L*ab. Both Src and Dst have to be 3 channeled images.*/
void			 ConvertImage_RGB_to_LAB(struct Image *Img_src, struct Image *Img_dst, struct WhitePoint WhitePoint_XYZ);
/* Change image color space - L*ab to RGB. Both Src and Dst have to be 3 channeled images.*/
void			 ConvertImage_LAB_to_RGB(struct Image *Img_src, struct Image *Img_dst, struct WhitePoint WhitePoint_XYZ);
void			 SetWhiteBalanceValues(struct WhitePoint *WhitePoint_lab, int TYPE);
void			 WhiteBalanceGREENY(struct Image *src, struct Image *dst, struct WhitePoint WhitePoint_lab);
void			 WhitebalanceCorrectionBLUEorRED(struct Image *src, struct Image *dst, struct WhitePoint WhitePoint_lab);
float			 RoundValue_toX_SignificantBits(float Value, int X);
void			 Convert_RGB_to_XYZ(struct Image *Img_src, struct Image *Img_dst);
void             Convert_XYZ_to_RGB(struct Image *Img_src, struct Image *Img_dst);
void			 ColorTemperature(struct WhitePoint *WhitePoint_lab, int AlgoType);
struct			 ColorPoint_UV POINT_Convert_XYZ_to_UV(struct ColorPoint_XYZ *XYZ);
struct			 ColorPoint_XYZ POINT_Convert_RGB_to_XYZ(struct ColorPoint_RGB *RGB_Point);
struct			 ColorPoint_RGB POINT_Convert_XYZ_to_RGB(struct  ColorPoint_XYZ *XYZ);
struct			 point_xy POINT_Convert_UV_to_XY(struct ColorPoint_UV *UV);
float			 pow_func(float Number, float Stepen, int precision);
struct Image *	 CreateImageLayersBasedOnPrototype(struct Image *Img_src, int NumberofLayers);
struct Image	 CombineLayers(struct Image *Layers, struct Image *Img_dst, struct Image Mask);
struct Image 	 CreateMaskForLayers(struct Image *LayerPrototype, int MaskType, int NumberOfLayers);
struct Image	 BlendImage(struct Image *Img_src, struct Image *Img_BlendedSrc, struct Image *Img_dst, float Percentage, int AlgoParam1, int AlgoParam2, int BlacOrWhiteThreshold);
struct Image	 InverseImage0to255(struct Image *Img_src, struct Image *Img_dst);
void			 SpatialToFrequencyDomain(struct Image *img_src, struct Image *img_dst);
void             Convert3ChannelsTo4Channels(struct Image *channels3, struct Image * channels4);
void             Convert4ChannelsTo3Channels(struct Image *channels4, struct Image * channels3);
void			 HistogramForImage(struct Histogram *hist, struct Image *Img_src, short NumberOfLayers);
void			 ConvertHistToImage(struct Histogram *hist, struct Image *Img_src);
int              inverse_dft(long int length, int length2, long double real_sample[], long double imag_sample[], long double temp_real[], long double temp_imag[]);
int              dft(long int length, int length2, long double real_sample[], long double imag_sample[], long double temp_real[], long double temp_imag[]);
