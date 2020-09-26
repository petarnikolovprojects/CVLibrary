/************************************************************************************
*																					*
*  This header file contains GUI window button/sliders/menu implementations			*
*  including reading/writing														*
*  																					*
*																					*
*	The file is limited for the purpose of 	https://github.com/petarnikolovprojects	*	
*																					*
*	Author: Petar Nikolov															*										
*************************************************************************************

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>
#include <qimage.h>
#include <stdio.h>
#include <QApplication>
#include <QtGui>
#include <QLabel>
#include <math.h>
#include <string.h>
#include <QMessageBox>

#include "CV_Library.h"

struct Image ScaledImage;
struct Image InputImage;
struct Image ScaledImage2;
struct Image AppliedImage;
struct Image HistogramImage_3Channels;
struct Image HistogramImage_3Channels2;
struct Image HistogramImage_4Channels;

int ListOperations [700][6];
int MousePos_X, MousePos_y;
int CtrlKeyPressed;
int NeighborCoefficients  = 0;
int Radius = 0;
int Aggression = 1;
float RedGamma = 1, GreenGamma = 1, BlueGamma = 1;
int CurrentOperationNumber;

QString Filename;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setMouseTracking(true);

    ui->BrightnessSlider->setDisabled(TRUE);
    ui->checkBox_1->setDisabled(TRUE);
    ui->MoveTogetherCheckBox->setDisabled(TRUE);
    ui->BlurImageSlider->setDisabled(TRUE);
    ui->ContrastSlider->setDisabled(TRUE);
    ui->verticalSlider_4->setDisabled(TRUE);
    ui->horizontalSlider->setDisabled(TRUE);
    ui->horizontalSlider_2->setDisabled(TRUE);
    ui->horizontalSlider_3->setDisabled(TRUE);
    ui->horizontalSlider_4->setDisabled(TRUE);
    ui->horizontalSlider_5->setDisabled(TRUE);
    ui->horizontalSlider_6->setDisabled(TRUE);
    ui->horizontalSlider_7->setDisabled(TRUE);
    ui->pushButton->setDisabled(TRUE);
    ui->pushButton_2->setDisabled(TRUE);
    ui->pushButton_3->setDisabled(TRUE);
    ListOperations[0][1] = 0;
    ListOperations[0][0] = 0;
    ListOperations[0][2] = 0;
    ListOperations[0][3] = 0;
    ListOperations[0][4] = 0;
    CtrlKeyPressed = 0;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control)
    {
        CtrlKeyPressed = 1;
    }
}
void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control)
    {
        CtrlKeyPressed = 0;
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    MousePos_X = event->x();
    MousePos_y = event->y();

    //Check if the CTRL key is pressed
    if(CtrlKeyPressed == 1)
    {
        struct point_xy poi;
        //Check if the MousePos is in the image area
        if(MousePos_X >=10 && MousePos_X <= (ui->label_9->width()+10) && MousePos_y >=10 && MousePos_y <=  (ui->label_9->height()+10))
        {
            poi.X = MousePos_X - 10;
            poi.Y = MousePos_y - 10;

            SetDestination(&ScaledImage, &ScaledImage2);

            struct Image Scaled_3Channels = CreateNewImage(&Scaled_3Channels, ScaledImage.Width, ScaledImage.Height,3,3, 8);
            //convert from 4-channel ScaledImage to 3Channels - Scaled_3Channels
            Convert4ChannelsTo3Channels(&ScaledImage, &Scaled_3Channels);

            Scaled_3Channels.ColorSpace = 2;
            struct Image Destination = CreateNewImage_BasedOnPrototype(&Scaled_3Channels, &Destination);

            BlurImageAroundPoint(&Scaled_3Channels,&Destination, poi, NeighborCoefficients, Radius, 1, Aggression );

            Convert3ChannelsTo4Channels(&Destination, &ScaledImage2);

            QImage Dest = QImage(ScaledImage2.rgbpix, ScaledImage2.Width, ScaledImage2.Height,  QImage::Format_RGB32);
            QPixmap pixmapObject;
            pixmapObject.convertFromImage(Dest);
            ui->label_9->setPixmap(pixmapObject);

            if(ListOperations[CurrentOperationNumber - 1][0] != 6)
            {
                ListOperations[CurrentOperationNumber][0] = 6;
                ListOperations[CurrentOperationNumber][1] = poi.X;
                ListOperations[CurrentOperationNumber][2] = poi.Y;
                ListOperations[CurrentOperationNumber][3] = NeighborCoefficients;
                ListOperations[CurrentOperationNumber][4] = Radius;
                ListOperations[CurrentOperationNumber++][5] = Aggression;
            }
            else
            {
                ListOperations[CurrentOperationNumber-1][1] = poi.X;
                ListOperations[CurrentOperationNumber-1][2] = poi.Y;
                ListOperations[CurrentOperationNumber-1][3] = NeighborCoefficients;
                ListOperations[CurrentOperationNumber-1][4] = Radius;
                ListOperations[CurrentOperationNumber-1][5] = Aggression;
            }
            if(ui->ApplyCheckBox->isChecked())
            {
                on_pushButton_clicked();
            }
        }
    }
}

void MainWindow::on_graphicsView_rubberBandChanged(const QRect &viewportRect, const QPointF &fromScenePoint, const QPointF &toScenePoint)
{

}

/*DETECT MOUSE action */
void label_9::mouseMoveEvent(QMouseEvent *ev)
{
    this->x = ev->x();
    this->y = ev->y();
    emit Mouse_Pos();
}

void label_9::mousePressEvent(QMouseEvent *ev)
{
    emit Mouse_Pressed();
    MousePos_X = ev->x();
    MousePos_y = ev->y();
    //printf("%d %d\n",MousePos_X, MousePos_y);


}

void label_9::leaveEvent(QEvent *)
{
    emit Mouse_Left();
}


void MainWindow:: on_spinBox_valueChanged(int arg1)
{
    NeighborCoefficients = arg1;
}

void MainWindow::on_spinBox_2_valueChanged(int arg1)
{
    Radius = arg1;
}


/* RADIO BUTTONS - choose image scale */
void MainWindow::on_radioButton_clicked()  // 3/2 format
{
    /* if there is a loaded image - ask user if he want to continue and to reset the image */
    if(ui->ContrastSlider->isEnabled())
    {
        QMessageBox msgBox;
        msgBox.setText("To change the preview, you have to reset the image.");
        //msgBox.setInformativeText("Do you want to continue?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();
        switch (ret)
        {
            case QMessageBox::Yes:
                // Yes was clicked
                ui->radioButton_2->setChecked(FALSE);
                ui->radioButton_3->setChecked(FALSE);
                ui->radioButton_4->setChecked(FALSE);
                ui->label_9->setFixedWidth(400);
                ui->label_9->setFixedHeight(266);
                ui->ImageWindow->setFixedWidth(420);
                ui->ImageWindow->setFixedHeight(286);

                on_pushButton_4_clicked(); // Load Image in the new preview format

            break;
            case QMessageBox::No:
            // No was clicked - Do nothing
            break;
        }
  }
    else
    {
       ui->radioButton_2->setChecked(FALSE);
       ui->radioButton_3->setChecked(FALSE);
       ui->radioButton_4->setChecked(FALSE);
       ui->label_9->setFixedWidth(400);
       ui->label_9->setFixedHeight(266);
       ui->ImageWindow->setFixedWidth(420);
       ui->ImageWindow->setFixedHeight(286);
   }
}

void MainWindow::on_radioButton_2_clicked() // 4/3
{
    /* if there is a loaded image - ask user if he want to continue and to reset the image */
    if(ui->ContrastSlider->isEnabled())
    {
        QMessageBox msgBox;
        msgBox.setText("To change the preview, you have to reset the image.");
        //msgBox.setInformativeText("Do you want to continue?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();
        switch (ret)
        {
            case QMessageBox::Yes:
                // Yes was clicked
                ui->radioButton->setChecked(FALSE);
                ui->radioButton_3->setChecked(FALSE);
                ui->radioButton_4->setChecked(FALSE);
                ui->label_9->setFixedWidth(400);
                ui->label_9->setFixedHeight(300);
                ui->ImageWindow->setFixedWidth(420);
                ui->ImageWindow->setFixedHeight(320);

                on_pushButton_4_clicked(); // Load Image in the new preview format

            break;
            case QMessageBox::No:
            // No was clicked - Do nothing
            break;
        }
  }
    else
    {
        ui->radioButton->setChecked(FALSE);
        ui->radioButton_3->setChecked(FALSE);
        ui->radioButton_4->setChecked(FALSE);
        ui->label_9->setFixedWidth(400);
        ui->label_9->setFixedHeight(300);
        ui->ImageWindow->setFixedWidth(420);
        ui->ImageWindow->setFixedHeight(320);
   }
}

void MainWindow::on_radioButton_3_clicked() // 16/9
{

    /* if there is a loaded image - ask user if he want to continue and to reset the image */
    if(ui->ContrastSlider->isEnabled())
    {
        QMessageBox msgBox;
        msgBox.setText("To change the preview, you have to reset the image.");
        //msgBox.setInformativeText("Do you want to continue?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();
        switch (ret)
        {
            case QMessageBox::Yes:
                // Yes was clicked
                ui->radioButton_2->setChecked(FALSE);
                ui->radioButton->setChecked(FALSE);
                ui->radioButton_4->setChecked(FALSE);
                ui->label_9->setFixedWidth(450);
                ui->label_9->setFixedHeight(300);
                ui->ImageWindow->setFixedWidth(470);
                ui->ImageWindow->setFixedHeight(320);

                ui->radioButton->setGeometry(20,350,41, 17);
                ui->radioButton_2->setGeometry(80,350,41, 17);
                ui->radioButton_3->setGeometry(130,350,41, 17);
                ui->radioButton_4->setGeometry(190,350,81, 17);

                on_pushButton_4_clicked(); // Load Image in the new preview format

            break;
            case QMessageBox::No:
            // No was clicked - Do nothing
            break;
        }
  }
    else
    {
        ui->radioButton_2->setChecked(FALSE);
        ui->radioButton->setChecked(FALSE);
        ui->radioButton_4->setChecked(FALSE);
        ui->label_9->setFixedWidth(450);
        ui->label_9->setFixedHeight(300);
        ui->ImageWindow->setFixedWidth(470);
        ui->ImageWindow->setFixedHeight(320);

        ui->radioButton->setGeometry(20,350,41, 17);
        ui->radioButton_2->setGeometry(80,350,41, 17);
        ui->radioButton_3->setGeometry(130,350,41, 17);
        ui->radioButton_4->setGeometry(190,350,81, 17);
   }
}

void MainWindow::on_radioButton_4_clicked()  // 3/4
{
    /* if there is a loaded image - ask user if he want to continue and to reset the image */
    if(ui->ContrastSlider->isEnabled())
    {
        QMessageBox msgBox;
        msgBox.setText("To change the preview, you have to reset the image.");
        //msgBox.setInformativeText("Do you want to continue?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();
        switch (ret)
        {
            case QMessageBox::Yes:
                // Yes was clicked
                ui->radioButton_2->setChecked(FALSE);
                ui->radioButton_3->setChecked(FALSE);
                ui->radioButton->setChecked(FALSE);
                ui->label_9->setFixedWidth(300);
                ui->label_9->setFixedHeight(400);
                ui->ImageWindow->setFixedWidth(320);
                ui->ImageWindow->setFixedHeight(420);

                ui->radioButton->setGeometry(335,39,41, 17);
                ui->radioButton_2->setGeometry(335,59,41, 17);
                ui->radioButton_3->setGeometry(335,79,41, 17);
                ui->radioButton_4->setGeometry(335,99,81, 17);

                on_pushButton_4_clicked(); // Load Image in the new preview format

            break;
            case QMessageBox::No:
            // No was clicked - Do nothing
            break;
        }
  }
    else
    {
       ui->radioButton_2->setChecked(FALSE);
       ui->radioButton_3->setChecked(FALSE);
       ui->radioButton->setChecked(FALSE);
       ui->label_9->setFixedWidth(300);
       ui->label_9->setFixedHeight(400);
       ui->ImageWindow->setFixedWidth(320);
       ui->ImageWindow->setFixedHeight(420);

       ui->radioButton->setGeometry(435,39,41, 17);
       ui->radioButton_2->setGeometry(435,59,41, 17);
       ui->radioButton_3->setGeometry(435,79,41, 17);
       ui->radioButton_4->setGeometry(435,99,81, 17);
   }
}



// ACTION 1 - BRIGHTNESS
void MainWindow::on_BrightnessSlider_sliderMoved(int position)
{
    SetDestination(&ScaledImage, &ScaledImage2);

    struct Image Scaled_3Channels = CreateNewImage(&Scaled_3Channels, ScaledImage.Width, ScaledImage.Height,3,3, 8);
    //convert from 4-channel ScaledImage to 3Channels - Scaled_3Channels
    Convert4ChannelsTo3Channels(&ScaledImage, &Scaled_3Channels);

    Scaled_3Channels.ColorSpace = 2;
    struct Image Destination = CreateNewImage_BasedOnPrototype(&Scaled_3Channels, &Destination);

    BrightnessCorrection(&Scaled_3Channels,&Destination,position,1);
    /*
     * HISTOGRAM
     */
    struct Histogram hist;
    ScaledImage.imageDepth = 8;
    HistogramImage_3Channels = CreateNewImage_BasedOnPrototype(&Destination, &HistogramImage_3Channels);
    HistogramImage_3Channels2 = CreateNewImage_BasedOnPrototype(&Destination, &HistogramImage_3Channels2);
    HistogramImage_4Channels = CreateNewImage_BasedOnPrototype(&Destination, &HistogramImage_4Channels);

    HistogramForImage(&hist, &Destination, 3);
    ConvertHistToImage(&hist, &HistogramImage_3Channels);

    ScaleImageToXY(&HistogramImage_3Channels, &HistogramImage_3Channels2, ui->label_16->width(), ui->label_16->height());
    HistogramImage_4Channels.rgbpix = (unsigned char *)realloc(HistogramImage_4Channels.rgbpix, 4 * HistogramImage_3Channels2.Width * HistogramImage_3Channels2.Height * sizeof(unsigned char));
    HistogramImage_4Channels.Height = ui->label_16->height();
    HistogramImage_4Channels.Width = ui->label_16->width();
    HistogramImage_4Channels.Num_channels = 4;
    Convert3ChannelsTo4Channels(&HistogramImage_3Channels2, &HistogramImage_4Channels);

    QImage Hysto = QImage(HistogramImage_4Channels.rgbpix, HistogramImage_4Channels.Width, HistogramImage_4Channels.Height,  QImage::Format_RGB32);
    QPixmap pixmapObject2;
    pixmapObject2.convertFromImage(Hysto);
    ui->label_16->setPixmap(pixmapObject2);
    //HistogramImage
    /*
     * End of histogram
     */
    Convert3ChannelsTo4Channels(&Destination, &ScaledImage2);

    QImage Dest = QImage(ScaledImage2.rgbpix, ScaledImage2.Width, ScaledImage2.Height,  QImage::Format_RGB32);
    QPixmap pixmapObject;
    pixmapObject.convertFromImage(Dest);
    ui->label_9->setPixmap(pixmapObject);

    if(ListOperations[CurrentOperationNumber - 1][0] != 1)
    {
        ListOperations[CurrentOperationNumber][0] = 1;
        ListOperations[CurrentOperationNumber++][1] = position;
    }
    else
        ListOperations[CurrentOperationNumber-1][1] = position;
    if(ui->ApplyCheckBox->isChecked())
    {
        on_pushButton_clicked();
    }
}

// ACTION 2 - CONTRAST
void MainWindow::on_ContrastSlider_valueChanged(int value)
{
    SetDestination(&ScaledImage, &ScaledImage2);

    struct Image Scaled_3Channels = CreateNewImage(&Scaled_3Channels, ScaledImage.Width, ScaledImage.Height,3,3, 8);
    //convert from 4-channel ScaledImage to 3Channels - Scaled_3Channels
    Convert4ChannelsTo3Channels(&ScaledImage, &Scaled_3Channels);

    Scaled_3Channels.ColorSpace = 2;
    struct Image Destination = CreateNewImage_BasedOnPrototype(&Scaled_3Channels, &Destination);

    ContrastCorrection(&Scaled_3Channels,&Destination,value);
    /*
     * HISTOGRAM
     */
    struct Histogram hist;
    ScaledImage.imageDepth = 8;
    HistogramImage_3Channels = CreateNewImage_BasedOnPrototype(&Destination, &HistogramImage_3Channels);
    HistogramImage_3Channels2 = CreateNewImage_BasedOnPrototype(&Destination, &HistogramImage_3Channels2);
    HistogramImage_4Channels = CreateNewImage_BasedOnPrototype(&Destination, &HistogramImage_4Channels);

    HistogramForImage(&hist, &Destination, 3);
    ConvertHistToImage(&hist, &HistogramImage_3Channels);

    ScaleImageToXY(&HistogramImage_3Channels, &HistogramImage_3Channels2, ui->label_16->width(), ui->label_16->height());
    HistogramImage_4Channels.rgbpix = (unsigned char *)realloc(HistogramImage_4Channels.rgbpix, 4 * HistogramImage_3Channels2.Width * HistogramImage_3Channels2.Height * sizeof(unsigned char));
    HistogramImage_4Channels.Height = ui->label_16->height();
    HistogramImage_4Channels.Width = ui->label_16->width();
    HistogramImage_4Channels.Num_channels = 4;
    Convert3ChannelsTo4Channels(&HistogramImage_3Channels2, &HistogramImage_4Channels);

    QImage Hysto = QImage(HistogramImage_4Channels.rgbpix, HistogramImage_4Channels.Width, HistogramImage_4Channels.Height,  QImage::Format_RGB32);
    QPixmap pixmapObject2;
    pixmapObject2.convertFromImage(Hysto);
    ui->label_16->setPixmap(pixmapObject2);
    //HistogramImage
    /*
     * End of histogram
     */

    Convert3ChannelsTo4Channels(&Destination, &ScaledImage2);

    QImage Dest = QImage(ScaledImage2.rgbpix, ScaledImage2.Width, ScaledImage2.Height,  QImage::Format_RGB32);
    QPixmap pixmapObject;
    pixmapObject.convertFromImage(Dest);
    ui->label_9->setPixmap(pixmapObject);

    if(ListOperations[CurrentOperationNumber - 1][0] != 2)
    {
        ListOperations[CurrentOperationNumber][0] = 2;
        ListOperations[CurrentOperationNumber++][1] = value;
    }
    else
        ListOperations[CurrentOperationNumber-1][1] = value;
    if(ui->ApplyCheckBox->isChecked())
    {
        on_pushButton_clicked();
    }
}

// ACTION 3 - White balance algorithm choose
void MainWindow::on_horizontalSlider_sliderMoved(int position)
{
    //FILE *fp;
    QImage Dest;
    SetDestination(&ScaledImage, &ScaledImage2);
    struct Image channel3_Image = CreateNewImage(&channel3_Image, ScaledImage.Width, ScaledImage.Height,3,2, 8);
    struct Image channel3_Image2 = CreateNewImage(&channel3_Image2, ScaledImage.Width, ScaledImage.Height,3,2, 8);
    struct WhitePoint WhitePoint_lab1;


    switch(position)
    {
        case 0:
            memcpy(ScaledImage2.rgbpix, ScaledImage.rgbpix, 4 * ScaledImage2.Width * ScaledImage2.Height * sizeof(unsigned char));

            break;

        case 1:

            Convert4ChannelsTo3Channels(&ScaledImage, &channel3_Image);
            WhiteBalanceCorrectionRGB(&channel3_Image, &channel3_Image2, 1);
            Convert3ChannelsTo4Channels(&channel3_Image2, &ScaledImage2 );

            break;

        case 2:

            Convert4ChannelsTo3Channels(&ScaledImage, &channel3_Image);
            WhiteBalanceCorrectionRGB(&channel3_Image, &channel3_Image2, 2);
            Convert3ChannelsTo4Channels(&channel3_Image2, &ScaledImage2 );

            break;

        case 3:

            Convert4ChannelsTo3Channels(&ScaledImage, &channel3_Image);
            WhiteBalanceCorrectionRGB(&channel3_Image, &channel3_Image2, 3);
            Convert3ChannelsTo4Channels(&channel3_Image2, &ScaledImage2 );

            break;

        case 4:

            Convert4ChannelsTo3Channels(&ScaledImage, &channel3_Image);
            WhiteBalanceCorrectionRGB(&channel3_Image, &channel3_Image2, 4);
            Convert3ChannelsTo4Channels(&channel3_Image2, &ScaledImage2 );

        case 5:

            Convert4ChannelsTo3Channels(&ScaledImage, &channel3_Image);
            SetWhiteBalanceValues(&WhitePoint_lab1, 7);
            //WhiteBalanceCorrectionRGB(&channel3_Image, &channel3_Image2, 4);
            WhitebalanceCorrectionBLUEorRED(&channel3_Image, &channel3_Image2, WhitePoint_lab1 );
            Convert3ChannelsTo4Channels(&channel3_Image2, &ScaledImage2 );

            break;

        case 6:

            Convert4ChannelsTo3Channels(&ScaledImage, &channel3_Image);
            //WhiteBalanceCorrectionRGB(&channel3_Image, &channel3_Image2, 4);
            SetWhiteBalanceValues(&WhitePoint_lab1, 7);
            WhiteBalanceGREENY(&channel3_Image, &channel3_Image2, WhitePoint_lab1 );
            Convert3ChannelsTo4Channels(&channel3_Image2, &ScaledImage2 );

            break;
    }

    Dest = QImage(ScaledImage2.rgbpix, ScaledImage2.Width, ScaledImage2.Height,  QImage::Format_RGB32);
    QPixmap pixmapObject;
    pixmapObject.convertFromImage(Dest);
    ui->label_9->setPixmap(pixmapObject);

    if(ListOperations[CurrentOperationNumber - 1][0] != 3)
    {
        ListOperations[CurrentOperationNumber][0] = 3;
        ListOperations[CurrentOperationNumber++][1] = position;
    }
    else
        ListOperations[CurrentOperationNumber-1][1] = position;
    if(ui->ApplyCheckBox->isChecked())
    {
        on_pushButton_clicked();
    }
}


//ACTION 4 - SATURATION
void MainWindow::on_verticalSlider_4_valueChanged(int value)
{
    SetDestination(&ScaledImage, &ScaledImage2);

    struct Image Scaled_3Channels = CreateNewImage(&Scaled_3Channels, ScaledImage.Width, ScaledImage.Height,3,3, 8);
    //convert from 4-channel ScaledImage to 3Channels - Scaled_3Channels
    Convert4ChannelsTo3Channels(&ScaledImage, &Scaled_3Channels);

    Scaled_3Channels.ColorSpace = 2;
    struct Image Destination = CreateNewImage_BasedOnPrototype(&Scaled_3Channels, &Destination);

    Saturation(&Scaled_3Channels,&Destination,value);
    /*
     * HISTOGRAM
     */
    struct Histogram hist;
    ScaledImage.imageDepth = 8;
    HistogramImage_3Channels = CreateNewImage_BasedOnPrototype(&Destination, &HistogramImage_3Channels);
    HistogramImage_3Channels2 = CreateNewImage_BasedOnPrototype(&Destination, &HistogramImage_3Channels2);
    HistogramImage_4Channels = CreateNewImage_BasedOnPrototype(&Destination, &HistogramImage_4Channels);

    HistogramForImage(&hist, &Destination, 3);
    ConvertHistToImage(&hist, &HistogramImage_3Channels);

    ScaleImageToXY(&HistogramImage_3Channels, &HistogramImage_3Channels2, ui->label_16->width(), ui->label_16->height());
    HistogramImage_4Channels.rgbpix = (unsigned char *)realloc(HistogramImage_4Channels.rgbpix, 4 * HistogramImage_3Channels2.Width * HistogramImage_3Channels2.Height * sizeof(unsigned char));
    HistogramImage_4Channels.Height = ui->label_16->height();
    HistogramImage_4Channels.Width = ui->label_16->width();
    HistogramImage_4Channels.Num_channels = 4;
    Convert3ChannelsTo4Channels(&HistogramImage_3Channels2, &HistogramImage_4Channels);

    QImage Hysto = QImage(HistogramImage_4Channels.rgbpix, HistogramImage_4Channels.Width, HistogramImage_4Channels.Height,  QImage::Format_RGB32);
    QPixmap pixmapObject2;
    pixmapObject2.convertFromImage(Hysto);
    ui->label_16->setPixmap(pixmapObject2);
    //HistogramImage
    /*
     * End of histogram
     */
    Convert3ChannelsTo4Channels(&Destination, &ScaledImage2);

    QImage Dest = QImage(ScaledImage2.rgbpix, ScaledImage2.Width, ScaledImage2.Height,  QImage::Format_RGB32);
    QPixmap pixmapObject;
    pixmapObject.convertFromImage(Dest);
    ui->label_9->setPixmap(pixmapObject);

    if(ListOperations[CurrentOperationNumber - 1][0] != 4)
    {
        ListOperations[CurrentOperationNumber][0] = 4;
        ListOperations[CurrentOperationNumber++][1] = value;
    }
    else
        ListOperations[CurrentOperationNumber-1][1] = value;
    if(ui->ApplyCheckBox->isChecked())
    {
        on_pushButton_clicked();
    }
}


// ACTION 5 - BLUR
void MainWindow::on_BlurImageSlider_valueChanged(int value)
{
    Aggression = value;
    SetDestination(&ScaledImage, &ScaledImage2);

    struct Image Scaled_3Channels = CreateNewImage(&Scaled_3Channels, ScaledImage.Width, ScaledImage.Height,3,3, 8);
    //convert from 4-channel ScaledImage to 3Channels - Scaled_3Channels
    Convert4ChannelsTo3Channels(&ScaledImage, &Scaled_3Channels);

    Scaled_3Channels.ColorSpace = 2;
    struct Image Destination = CreateNewImage_BasedOnPrototype(&Scaled_3Channels, &Destination);

    BlurImageGussian(&Scaled_3Channels,&Destination, value, 1);

    Convert3ChannelsTo4Channels(&Destination, &ScaledImage2);

    QImage Dest = QImage(ScaledImage2.rgbpix, ScaledImage2.Width, ScaledImage2.Height,  QImage::Format_RGB32);
    QPixmap pixmapObject;
    pixmapObject.convertFromImage(Dest);
    ui->label_9->setPixmap(pixmapObject);

    if(ListOperations[CurrentOperationNumber - 1][0] != 5)
    {
        ListOperations[CurrentOperationNumber][0] = 5;
        ListOperations[CurrentOperationNumber++][1] = value;
    }
    else
        ListOperations[CurrentOperationNumber-1][1] = value;
    if(ui->ApplyCheckBox->isChecked())
    {
        on_pushButton_clicked();
    }
}

// ACTION 6 - Blur image around point - is defined around line 100;

// ACTION 7 - GammaCorrection
void MainWindow::on_horizontalSlider_3_valueChanged(int value)
{
    RedGamma = value / 10.0;
    if(ui->MoveTogetherCheckBox->isChecked())
    {
        BlueGamma = RedGamma;
        GreenGamma = RedGamma;
        ui->horizontalSlider_4->setValue(value);
        ui->horizontalSlider_7->setValue(value);
    }
    SetDestination(&ScaledImage, &ScaledImage2);

    struct Image Scaled_3Channels = CreateNewImage(&Scaled_3Channels, ScaledImage.Width, ScaledImage.Height,3,3, 8);
    //convert from 4-channel ScaledImage to 3Channels - Scaled_3Channels
    Convert4ChannelsTo3Channels(&ScaledImage, &Scaled_3Channels);

    Scaled_3Channels.ColorSpace = 2;
    struct Image Destination = CreateNewImage_BasedOnPrototype(&Scaled_3Channels, &Destination);

    GammaCorrection(&Scaled_3Channels,&Destination, RedGamma, GreenGamma, BlueGamma);
    /*
     * HISTOGRAM
     */
    struct Histogram hist;
    ScaledImage.imageDepth = 8;
    HistogramImage_3Channels = CreateNewImage_BasedOnPrototype(&Destination, &HistogramImage_3Channels);
    HistogramImage_3Channels2 = CreateNewImage_BasedOnPrototype(&Destination, &HistogramImage_3Channels2);
    HistogramImage_4Channels = CreateNewImage_BasedOnPrototype(&Destination, &HistogramImage_4Channels);

    HistogramForImage(&hist, &Destination, 3);
    ConvertHistToImage(&hist, &HistogramImage_3Channels);

    ScaleImageToXY(&HistogramImage_3Channels, &HistogramImage_3Channels2, ui->label_16->width(), ui->label_16->height());
    HistogramImage_4Channels.rgbpix = (unsigned char *)realloc(HistogramImage_4Channels.rgbpix, 4 * HistogramImage_3Channels2.Width * HistogramImage_3Channels2.Height * sizeof(unsigned char));
    HistogramImage_4Channels.Height = ui->label_16->height();
    HistogramImage_4Channels.Width = ui->label_16->width();
    HistogramImage_4Channels.Num_channels = 4;
    Convert3ChannelsTo4Channels(&HistogramImage_3Channels2, &HistogramImage_4Channels);

    QImage Hysto = QImage(HistogramImage_4Channels.rgbpix, HistogramImage_4Channels.Width, HistogramImage_4Channels.Height,  QImage::Format_RGB32);
    QPixmap pixmapObject2;
    pixmapObject2.convertFromImage(Hysto);
    ui->label_16->setPixmap(pixmapObject2);
    //HistogramImage
    /*
     * End of histogram
     */
    Convert3ChannelsTo4Channels(&Destination, &ScaledImage2);

    QImage Dest = QImage(ScaledImage2.rgbpix, ScaledImage2.Width, ScaledImage2.Height,  QImage::Format_RGB32);
    QPixmap pixmapObject;
    pixmapObject.convertFromImage(Dest);
    ui->label_9->setPixmap(pixmapObject);

    if(ListOperations[CurrentOperationNumber - 1][0] != 7)
    {
        ListOperations[CurrentOperationNumber][0] = 7;
        ListOperations[CurrentOperationNumber][1] = RedGamma *10;
        ListOperations[CurrentOperationNumber][2] = GreenGamma *10;
        ListOperations[CurrentOperationNumber++][3] = BlueGamma *10;
    }
    else
    {
        ListOperations[CurrentOperationNumber-1][1] = RedGamma * 10;
        ListOperations[CurrentOperationNumber-1][2] = GreenGamma * 10;
        ListOperations[CurrentOperationNumber-1][3] = BlueGamma * 10;
    }
    if(ui->ApplyCheckBox->isChecked())
    {
        on_pushButton_clicked(); // APPLY
    }
}

void MainWindow::on_horizontalSlider_4_valueChanged(int value)
{
    GreenGamma = value / 10.0;
    if(ui->MoveTogetherCheckBox->isChecked())
    {
        BlueGamma = GreenGamma;
        RedGamma = GreenGamma;
        ui->horizontalSlider_3->setValue(value);
        ui->horizontalSlider_7->setValue(value);
    }
    SetDestination(&ScaledImage, &ScaledImage2);

    struct Image Scaled_3Channels = CreateNewImage(&Scaled_3Channels, ScaledImage.Width, ScaledImage.Height,3,3, 8);
    //convert from 4-channel ScaledImage to 3Channels - Scaled_3Channels
    Convert4ChannelsTo3Channels(&ScaledImage, &Scaled_3Channels);

    Scaled_3Channels.ColorSpace = 2;
    struct Image Destination = CreateNewImage_BasedOnPrototype(&Scaled_3Channels, &Destination);

    GammaCorrection(&Scaled_3Channels,&Destination, RedGamma, GreenGamma, BlueGamma);
    /*
     * HISTOGRAM
     */
    struct Histogram hist;
    ScaledImage.imageDepth = 8;
    HistogramImage_3Channels = CreateNewImage_BasedOnPrototype(&Destination, &HistogramImage_3Channels);
    HistogramImage_3Channels2 = CreateNewImage_BasedOnPrototype(&Destination, &HistogramImage_3Channels2);
    HistogramImage_4Channels = CreateNewImage_BasedOnPrototype(&Destination, &HistogramImage_4Channels);

    HistogramForImage(&hist, &Destination, 3);
    ConvertHistToImage(&hist, &HistogramImage_3Channels);

    ScaleImageToXY(&HistogramImage_3Channels, &HistogramImage_3Channels2, ui->label_16->width(), ui->label_16->height());
    HistogramImage_4Channels.rgbpix = (unsigned char *)realloc(HistogramImage_4Channels.rgbpix, 4 * HistogramImage_3Channels2.Width * HistogramImage_3Channels2.Height * sizeof(unsigned char));
    HistogramImage_4Channels.Height = ui->label_16->height();
    HistogramImage_4Channels.Width = ui->label_16->width();
    HistogramImage_4Channels.Num_channels = 4;
    Convert3ChannelsTo4Channels(&HistogramImage_3Channels2, &HistogramImage_4Channels);

    QImage Hysto = QImage(HistogramImage_4Channels.rgbpix, HistogramImage_4Channels.Width, HistogramImage_4Channels.Height,  QImage::Format_RGB32);
    QPixmap pixmapObject2;
    pixmapObject2.convertFromImage(Hysto);
    ui->label_16->setPixmap(pixmapObject2);
    //HistogramImage
    /*
     * End of histogram
     */
    Convert3ChannelsTo4Channels(&Destination, &ScaledImage2);

    QImage Dest = QImage(ScaledImage2.rgbpix, ScaledImage2.Width, ScaledImage2.Height,  QImage::Format_RGB32);
    QPixmap pixmapObject;
    pixmapObject.convertFromImage(Dest);
    ui->label_9->setPixmap(pixmapObject);

    if(ListOperations[CurrentOperationNumber - 1][0] != 7)
    {
        ListOperations[CurrentOperationNumber][0] = 7;
        ListOperations[CurrentOperationNumber][1] = RedGamma *10;
        ListOperations[CurrentOperationNumber][2] = GreenGamma *10;
        ListOperations[CurrentOperationNumber++][3] = BlueGamma *10;
    }
    else
    {
        ListOperations[CurrentOperationNumber-1][1] = RedGamma * 10;
        ListOperations[CurrentOperationNumber-1][2] = GreenGamma * 10;
        ListOperations[CurrentOperationNumber-1][3] = BlueGamma * 10;
    }
    if(ui->ApplyCheckBox->isChecked())
    {
        on_pushButton_clicked(); // APPLY
    }
}

void MainWindow::on_horizontalSlider_7_valueChanged(int value)
{
    BlueGamma = value / 10.0;
    if(ui->MoveTogetherCheckBox->isChecked())
    {
        GreenGamma = BlueGamma;
        RedGamma = BlueGamma;
        ui->horizontalSlider_3->setValue(value);
        ui->horizontalSlider_4->setValue(value);

    }
    SetDestination(&ScaledImage, &ScaledImage2);

    struct Image Scaled_3Channels = CreateNewImage(&Scaled_3Channels, ScaledImage.Width, ScaledImage.Height,3,3, 8);
    //convert from 4-channel ScaledImage to 3Channels - Scaled_3Channels
    Convert4ChannelsTo3Channels(&ScaledImage, &Scaled_3Channels);

    Scaled_3Channels.ColorSpace = 2;
    struct Image Destination = CreateNewImage_BasedOnPrototype(&Scaled_3Channels, &Destination);

    GammaCorrection(&Scaled_3Channels,&Destination, RedGamma, GreenGamma, BlueGamma);
    /*
     * HISTOGRAM
     */
    struct Histogram hist;
    ScaledImage.imageDepth = 8;
    HistogramImage_3Channels = CreateNewImage_BasedOnPrototype(&Destination, &HistogramImage_3Channels);
    HistogramImage_3Channels2 = CreateNewImage_BasedOnPrototype(&Destination, &HistogramImage_3Channels2);
    HistogramImage_4Channels = CreateNewImage_BasedOnPrototype(&Destination, &HistogramImage_4Channels);

    HistogramForImage(&hist, &Destination, 3);
    ConvertHistToImage(&hist, &HistogramImage_3Channels);

    ScaleImageToXY(&HistogramImage_3Channels, &HistogramImage_3Channels2, ui->label_16->width(), ui->label_16->height());
    HistogramImage_4Channels.rgbpix = (unsigned char *)realloc(HistogramImage_4Channels.rgbpix, 4 * HistogramImage_3Channels2.Width * HistogramImage_3Channels2.Height * sizeof(unsigned char));
    HistogramImage_4Channels.Height = ui->label_16->height();
    HistogramImage_4Channels.Width = ui->label_16->width();
    HistogramImage_4Channels.Num_channels = 4;
    Convert3ChannelsTo4Channels(&HistogramImage_3Channels2, &HistogramImage_4Channels);

    QImage Hysto = QImage(HistogramImage_4Channels.rgbpix, HistogramImage_4Channels.Width, HistogramImage_4Channels.Height,  QImage::Format_RGB32);
    QPixmap pixmapObject2;
    pixmapObject2.convertFromImage(Hysto);
    ui->label_16->setPixmap(pixmapObject2);
    //HistogramImage
    /*
     * End of histogram
     */
    Convert3ChannelsTo4Channels(&Destination, &ScaledImage2);

    QImage Dest = QImage(ScaledImage2.rgbpix, ScaledImage2.Width, ScaledImage2.Height,  QImage::Format_RGB32);
    QPixmap pixmapObject;
    pixmapObject.convertFromImage(Dest);
    ui->label_9->setPixmap(pixmapObject);

    if(ListOperations[CurrentOperationNumber - 1][0] != 7)
    {
        ListOperations[CurrentOperationNumber][0] = 7;
        ListOperations[CurrentOperationNumber][1] = RedGamma *10;
        ListOperations[CurrentOperationNumber][2] = GreenGamma *10;
        ListOperations[CurrentOperationNumber++][3] = BlueGamma *10;
    }
    else
    {
        ListOperations[CurrentOperationNumber-1][1] = RedGamma * 10;
        ListOperations[CurrentOperationNumber-1][2] = GreenGamma * 10;
        ListOperations[CurrentOperationNumber-1][3] = BlueGamma * 10;
    }
    if(ui->ApplyCheckBox->isChecked())
    {
        on_pushButton_clicked(); // APPLY
    }
}

/////////////////////////////////////////////////////////////
void MainWindow::on_pushButton_clicked() // APPLY
{
    memcpy(ScaledImage.rgbpix, ScaledImage2.rgbpix, ScaledImage2.Num_channels * ScaledImage2.Width * ScaledImage2.Height * sizeof(unsigned char));
}

//void MainWindow::on_pushButton_1_clicked() // APPLY
//{
    // convert ScaledImage2 to ScaledImage(4 to 3 channels);
    //Input Image = Apply image = ScaledImage2
    //memcpy(AppliedImage.rgbpix, ScaledImage2.rgbpix, ScaledImage2.Num_channels * ScaledImage2.Width * ScaledImage2.Height * sizeof(unsigned char));
    //memcpy(InputImage.rgbpix, ScaledImage2.rgbpix, ScaledImage2.Num_channels * ScaledImage2.Width * ScaledImage2.Height * sizeof(unsigned char));
//    memcpy(ScaledImage.rgbpix, ScaledImage2.rgbpix, ScaledImage2.Num_channels * ScaledImage2.Width * ScaledImage2.Height * sizeof(unsigned char));
//}
void MainWindow::on_pushButton_2_clicked() // RESET - //Reset to the previous APPLY
{
    int i,j;
    ui->horizontalSlider_3->setValue(10);
    ui->horizontalSlider_4->setValue(10);
    ui->horizontalSlider_7->setValue(10);
    ScaledImage.rgbpix = (unsigned char *)malloc(4 * InputImage.Width * InputImage.Height * sizeof(unsigned char));
    ScaledImage.Num_channels = 3;
    ScaledImage.Height = InputImage.Height;
    ScaledImage.Width = InputImage.Width;

    j = 0;
    for( i = 0; i < 4  * InputImage.Width * InputImage.Height; i++)
    {
        if((i+1) %4 ==0 && i != 0) continue;
         ScaledImage.rgbpix[j++] = InputImage.rgbpix[i];
    }
    ScaledImage2 = CreateNewImage_BasedOnPrototype(&ScaledImage, &ScaledImage2);
    ScaleImageToXY(&ScaledImage, &ScaledImage2, ui->label_9->width(), ui->label_9->height());
    j = 0;

    /*
     * HISTOGRAM
     */
    struct Histogram hist;
    ScaledImage.imageDepth = 8;
    HistogramImage_3Channels = CreateNewImage_BasedOnPrototype(&ScaledImage2, &HistogramImage_3Channels);
    HistogramImage_3Channels2 = CreateNewImage_BasedOnPrototype(&ScaledImage2, &HistogramImage_3Channels2);
    HistogramImage_4Channels = CreateNewImage_BasedOnPrototype(&ScaledImage2, &HistogramImage_4Channels);

    HistogramForImage(&hist, &ScaledImage, 3);
    ConvertHistToImage(&hist, &HistogramImage_3Channels);

    ScaleImageToXY(&HistogramImage_3Channels, &HistogramImage_3Channels2, ui->label_16->width(), ui->label_16->height());
    HistogramImage_4Channels.rgbpix = (unsigned char *)realloc(HistogramImage_4Channels.rgbpix, 4 * HistogramImage_3Channels2.Width * HistogramImage_3Channels2.Height * sizeof(unsigned char));
    HistogramImage_4Channels.Height = ui->label_16->height();
    HistogramImage_4Channels.Width = ui->label_16->width();
    HistogramImage_4Channels.Num_channels = 4;
    Convert3ChannelsTo4Channels(&HistogramImage_3Channels2, &HistogramImage_4Channels);

    QImage Hysto = QImage(HistogramImage_4Channels.rgbpix, HistogramImage_4Channels.Width, HistogramImage_4Channels.Height,  QImage::Format_RGB32);
    QPixmap pixmapObject2;
    pixmapObject2.convertFromImage(Hysto);
    ui->label_16->setPixmap(pixmapObject2);
    //HistogramImage
    /*
     * End of histogram
     */
    ScaledImage.rgbpix = (unsigned char *)realloc(ScaledImage.rgbpix, 4 * ScaledImage2.Width * ScaledImage2.Height * sizeof(unsigned char));
    for(i = 0; i < 3  * ScaledImage2.Width * ScaledImage2.Height; i++)
    {
        ScaledImage.rgbpix[j++] = ScaledImage2.rgbpix[i];
        if((j+1) %4 == 0 && j != 0)  ScaledImage.rgbpix[j++] = 255;
    }

    ScaledImage.Num_channels = 4;
    ScaledImage.Width = ScaledImage2.Width;
    ScaledImage.Height = ScaledImage2.Height;
    QImage ScaledIm = QImage(ScaledImage.rgbpix, ScaledImage2.Width, ScaledImage2.Height,  QImage::Format_RGB32);
    QPixmap pixmapObject;
    pixmapObject.convertFromImage(ScaledIm);
    ui->label_9->setPixmap(pixmapObject);

    ui->BrightnessSlider->setSliderPosition(0);
    ui->ContrastSlider->setSliderPosition(0);
    ui->horizontalSlider->setSliderPosition(0);
    ui->verticalSlider_4->setSliderPosition(0);
    ui->BlurImageSlider->setSliderPosition(0);


    AppliedImage =  CreateNewImage_BasedOnPrototype(&InputImage, &AppliedImage);
    AppliedImage.rgbpix = (unsigned char *)malloc(4 * InputImage.Width * InputImage.Height * sizeof(unsigned char));
    memcpy(AppliedImage.rgbpix, InputImage.rgbpix, InputImage.Num_channels * InputImage.Width * InputImage.Height * sizeof(unsigned char));

    CurrentOperationNumber = 1;
}

void MainWindow::on_pushButton_4_clicked() // LOAD
{
    int i,j;
    struct Histogram hist;
    //FILE *fp;
    ui->horizontalSlider_3->setValue(10);
    ui->horizontalSlider_4->setValue(10);
    ui->horizontalSlider_7->setValue(10);
    Filename = QFileDialog::getOpenFileName(this,tr("Open Image file"),QDir::currentPath(),tr("Image files (*.jpg)"));
    ui->textEdit->setText(Filename);
    InputImage = ReadImage((char *)Filename.toUtf8().constData());

   // fp = fopen("D:\\Projects\\ImageGUI\\baba.txt","wt");
    //for(int i = 3 * InputImage.Width * InputImage.Height; i < 4 * InputImage.Width * InputImage.Height; i++ )
     //   fprintf(fp,"%d ",InputImage.rgbpix[i]);
    //fclose(fp);
    AppliedImage =  CreateNewImage_BasedOnPrototype(&InputImage, &AppliedImage);
    AppliedImage.rgbpix = (unsigned char *)malloc(4 * InputImage.Width * InputImage.Height * sizeof(unsigned char));
    memcpy(AppliedImage.rgbpix, InputImage.rgbpix, InputImage.Num_channels * InputImage.Width * InputImage.Height * sizeof(unsigned char));

    ScaledImage.rgbpix = (unsigned char *)malloc(4 * InputImage.Width * InputImage.Height * sizeof(unsigned char));
    ScaledImage.Num_channels = 3;
    ScaledImage.Height = InputImage.Height;
    ScaledImage.Width = InputImage.Width;

    j = 0;
    for( i = 0; i < 4  * InputImage.Width * InputImage.Height; i++)
    {
        if((i+1) %4 ==0 && i != 0) continue;
         ScaledImage.rgbpix[j++] = InputImage.rgbpix[i];
    }
    ScaledImage2 = CreateNewImage_BasedOnPrototype(&ScaledImage, &ScaledImage2);
    ScaleImageToXY(&ScaledImage, &ScaledImage2, ui->label_9->width(), ui->label_9->height());
    j = 0;

    /*
     * HISTOGRAM
     */
    ScaledImage.imageDepth = 8;
    HistogramImage_3Channels = CreateNewImage_BasedOnPrototype(&ScaledImage2, &HistogramImage_3Channels);
    HistogramImage_3Channels2 = CreateNewImage_BasedOnPrototype(&ScaledImage2, &HistogramImage_3Channels2);
    HistogramImage_4Channels = CreateNewImage_BasedOnPrototype(&ScaledImage2, &HistogramImage_4Channels);

    HistogramForImage(&hist, &ScaledImage, 3);
    ConvertHistToImage(&hist, &HistogramImage_3Channels);

    ScaleImageToXY(&HistogramImage_3Channels, &HistogramImage_3Channels2, ui->label_16->width(), ui->label_16->height());
    HistogramImage_4Channels.rgbpix = (unsigned char *)realloc(HistogramImage_4Channels.rgbpix, 4 * HistogramImage_3Channels2.Width * HistogramImage_3Channels2.Height * sizeof(unsigned char));
    HistogramImage_4Channels.Height = ui->label_16->height();
    HistogramImage_4Channels.Width = ui->label_16->width();
    HistogramImage_4Channels.Num_channels = 4;
    Convert3ChannelsTo4Channels(&HistogramImage_3Channels2, &HistogramImage_4Channels);

    QImage Hysto = QImage(HistogramImage_4Channels.rgbpix, HistogramImage_4Channels.Width, HistogramImage_4Channels.Height,  QImage::Format_RGB32);
    QPixmap pixmapObject2;
    pixmapObject2.convertFromImage(Hysto);
    ui->label_16->setPixmap(pixmapObject2);
    //HistogramImage
    /*
     * End of histogram
     */

    ScaledImage.rgbpix = (unsigned char *)realloc(ScaledImage.rgbpix, 4 * ScaledImage2.Width * ScaledImage2.Height * sizeof(unsigned char));

    for(i = 0; i < 3  * ScaledImage2.Width * ScaledImage2.Height; i++)
    {
        ScaledImage.rgbpix[j++] = ScaledImage2.rgbpix[i];
        if((j+1) %4 == 0 && j != 0)  ScaledImage.rgbpix[j++] = 255;
    }

    ScaledImage.Num_channels = 4;
    ScaledImage.Width = ScaledImage2.Width;
    ScaledImage.Height = ScaledImage2.Height;

    QImage ScaledIm = QImage(ScaledImage.rgbpix, ScaledImage2.Width, ScaledImage2.Height,  QImage::Format_RGB32);
    QPixmap pixmapObject;
    pixmapObject.convertFromImage(ScaledIm);
    ui->label_9->setPixmap(pixmapObject);

    CurrentOperationNumber = 1;
    ui->BrightnessSlider->setSliderPosition(0); //action 1 - Brightness
    ui->ContrastSlider->setSliderPosition(0);   //action 2 - conrast
    ui->horizontalSlider->setSliderPosition(0); //action 3 - white balance
    ui->verticalSlider_4->setSliderPosition(0); //action 4 - Saturation
    ui->BlurImageSlider->setSliderPosition(0);


    /* Enable all butons and sliders */
    ui->BrightnessSlider->setEnabled(TRUE);
    ui->checkBox_1->setEnabled(TRUE);
    ui->MoveTogetherCheckBox->setEnabled(TRUE);
    ui->BlurImageSlider->setEnabled(TRUE);
    ui->ContrastSlider->setEnabled(TRUE);
    ui->verticalSlider_4->setEnabled(TRUE);
    ui->horizontalSlider->setEnabled(TRUE);
    ui->horizontalSlider_2->setEnabled(TRUE);
    ui->horizontalSlider_3->setEnabled(TRUE);
    ui->horizontalSlider_4->setEnabled(TRUE);
    ui->horizontalSlider_5->setEnabled(TRUE);
    ui->horizontalSlider_6->setEnabled(TRUE);
    ui->horizontalSlider_7->setEnabled(TRUE);
    ui->pushButton->setEnabled(TRUE);
    ui->pushButton_2->setEnabled(TRUE);
    ui->pushButton_3->setEnabled(TRUE);
}

void MainWindow::on_pushButton_3_clicked()  // SAVE
{
   struct point_xy poi;
   struct WhitePoint WhitePoint_lab1;

   struct Image channel3_Image = CreateNewImage(&channel3_Image, InputImage.Width, InputImage.Height,3,2, 8);
   struct Image channel3_Image2 = CreateNewImage(&channel3_Image2, InputImage.Width, InputImage.Height,3,2, 8);

   SetWhiteBalanceValues(&WhitePoint_lab1, 7);
   Convert4ChannelsTo3Channels(&InputImage, &channel3_Image2);


   for(int i = 1; i < CurrentOperationNumber ; i++)
   {
       printf("funk %d, 1: %d, 2: %d, 3: %d, 4: %d, 5: %d \n", ListOperations[i][0], ListOperations[i][1], ListOperations[i][2], ListOperations[i][3], ListOperations[i][4], ListOperations[i][5]);
       if(ListOperations[i][0] == 1) // BRIGHTNESS
       {
           if(i%2 == 0)
               BrightnessCorrection(&channel3_Image, &channel3_Image2, ListOperations[i][1],1);
           else
               BrightnessCorrection(&channel3_Image2, &channel3_Image, ListOperations[i][1],1);
       }
       else if(ListOperations[i][0] == 2) // CONTRAST
       {
           if(i%2 == 0)
               ContrastCorrection(&channel3_Image, &channel3_Image2, ListOperations[i][1]);
           else
               ContrastCorrection(&channel3_Image2, &channel3_Image, ListOperations[i][1]);
       }
       else if(ListOperations[i][0] == 3) // WHITE BALANCE
       {
           if(i%2 == 0)
           {
               if(ListOperations[i][1] == 0)
                   memcpy(channel3_Image2.rgbpix, channel3_Image.rgbpix, 3 * sizeof(unsigned char) * channel3_Image.Width * channel3_Image.Height );
               else if(ListOperations[i][1] >= 1 && ListOperations[i][1] <= 4)
                   WhiteBalanceCorrectionRGB(&channel3_Image, &channel3_Image2, ListOperations[i][1]);
               else if(ListOperations[i][1] == 5)
                   WhitebalanceCorrectionBLUEorRED(&channel3_Image, &channel3_Image2, WhitePoint_lab1);
               else if(ListOperations[i][1] == 6)
                   WhiteBalanceGREENY(&channel3_Image, &channel3_Image2, WhitePoint_lab1);
           }
           else
           {
               if(ListOperations[i][1] == 0)
                   memcpy(channel3_Image.rgbpix, channel3_Image2.rgbpix, 3 * sizeof(unsigned char) * channel3_Image.Width * channel3_Image.Height );
               else if(ListOperations[i][1] >= 1 && ListOperations[i][1] <= 4)
                   WhiteBalanceCorrectionRGB(&channel3_Image2, &channel3_Image, ListOperations[i][1]);
               else if(ListOperations[i][1] == 5)
                   WhitebalanceCorrectionBLUEorRED(&channel3_Image2, &channel3_Image, WhitePoint_lab1);
               else if(ListOperations[i][1] == 6)
                   WhiteBalanceGREENY(&channel3_Image2, &channel3_Image, WhitePoint_lab1);
           }
       }
       else if(ListOperations[i][0] == 4) // SATURATION
       {
           if(i%2 == 0)
               Saturation(&channel3_Image, &channel3_Image2, ListOperations[i][1]);
           else
               Saturation(&channel3_Image2, &channel3_Image, ListOperations[i][1]);
       }
       else if(ListOperations[i][0] == 5) // BLUR - gaussian
       {
           if(i%2 == 0)
               BlurImageGussian(&channel3_Image, &channel3_Image2, ListOperations[i][1], ListOperations[i][1]);
           else
               BlurImageGussian(&channel3_Image2, &channel3_Image, ListOperations[i][1], ListOperations[i][1]);
       }
       else if(ListOperations[i][0] == 6) // BLUR - around point
       {
           poi.X = ListOperations[i][1];
           poi.Y = ListOperations[i][2];

           if(i%2 == 0)
               BlurImageAroundPoint(&channel3_Image, &channel3_Image2, poi, ListOperations[i][3], ListOperations[i][4], 1, ListOperations[i][5] );
           else
               BlurImageAroundPoint(&channel3_Image2, &channel3_Image, poi, ListOperations[i][3], ListOperations[i][4], 1, ListOperations[i][5] );
       }
   }
   if(CurrentOperationNumber % 2 == 0) Convert3ChannelsTo4Channels(&channel3_Image, &AppliedImage);
   else Convert3ChannelsTo4Channels(&channel3_Image2, &AppliedImage);

   if(ui->checkBox_1->isChecked())  WriteImage((char *)ui->textEdit->toPlainText().toUtf8().constData(),AppliedImage,100);
   else
   {
       QString QName;
       QName = QFileDialog::getSaveFileName( this, "Save file", "", "jpg" );
       QImage Image_Output2= QImage(AppliedImage.rgbpix, AppliedImage.Width, AppliedImage.Height,  QImage::Format_RGB32);

       Image_Output2.save(QName.toStdString().c_str(),"",100);
   }
}


using namespace std;
int main(int argc, char *argv[])
{
    int i,j, l;
    //struct Image ui;
    QApplication a(argc, argv);
    MainWindow w;
    //QPixmap pixmapObject("D:\\Projects\\ImageGUI\\build-ImageGUI-Desktop-Release\\pana32.jpg");
    //ui->label_9->setPixmap(pixmapObject);
    w.show();

    return a.exec();
}


/********************************************************************************
*																				*
*	CV Library - ImageIO.c											*
*																				*
*	Author:  Petar Nikolov														*
*																				*
*																				*
*	The algorithms included in this file are:									*
*																				*
*	- Open / read image															*
*	- Write image																*
*	- Create new Image / with prototype											*
*	- Destroy Image																*
*	- White Balance - fill structure											*
*																				*
*																				*
*********************************************************************************/






static void put_scanline(unsigned char buffer[], int line, int width,
                         int height, unsigned char *rgbpix);
static int debug = 0;                /* = 1; prints every pixel */



/*
    O P E N   I M A G E
*/
struct Image ReadImage(char *FileName)
{
    FILE *f_ptr = NULL;
    struct Image Img_src;

#ifdef VS_LIBRARIES
    int isJpeg = 0;
    int Counter = 0;
    Img_src.isLoaded = 0;
    /* Open the file only with extension JPEG or JPG*/
#pragma warning (disable : 4996)
    f_ptr = fopen(FileName, "rb");
    if (f_ptr == NULL)
    {
        printf("Error opening file. Program will now close\n");
        _getch();
        return Img_src;
    }
    /* Check filename extension */
    for (Counter = 0; Counter < 255; Counter++)
    {
        if (FileName[Counter] == '.' && Counter < 253)
        {
            if (FileName[Counter + 1] == 'J' || FileName[Counter + 1] == 'j')
            {
                if (FileName[Counter + 2] == 'P' || FileName[Counter + 2] == 'p')
                {
                    isJpeg = 1;
                }
            }
        }
    }
    if (isJpeg == 0)
    {
        printf("Only Jpeg file extensions are allowed\n");
        _getch();
        return Img_src;
    }

    Img_src = read_Image_file(f_ptr);
    Img_src.Image_FileName = FileName;
#endif
#ifdef QT_LIBRARIES

    QImage InputImage(FileName);
    InputImage.convertToFormat(QImage::Format_RGB32);
    //QImage::Format blqk = InputImage.format();

    //printf("\n\n%d\n\n",blqk);
    //Img_src.rgbpix = InputImage.scanLine(0);
    Img_src.rgbpix = (unsigned char *)malloc(4 * InputImage.width() *InputImage.height() * sizeof(unsigned char));
    memcpy( Img_src.rgbpix, InputImage.scanLine(0),4 * InputImage.width() *InputImage.height() * sizeof(unsigned char));
    Img_src.Width = InputImage.width();
    Img_src.Height = InputImage.height();
    Img_src.Num_channels = 3;//InputImage.depth();
    Img_src.ColorSpace = 2;
    Img_src.imageDepth = 8;

#endif // QT_LIBRARIES

    Img_src.isLoaded = 1;
    return Img_src;
}


/*
    W R I T E   I M A G E
*/
void WriteImage(char * filename, struct Image Img_src, int quality)//uint16 *image_buffer, int image_width, int image_height)
{
#ifdef VS_LIBRARIES
  struct jpeg_compress_struct cinfo;

  struct jpeg_error_mgr jerr;
  /* More stuff */
  FILE * outfile;		/* target file */
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
  int row_stride;		/* physical row width in image buffer */

  cinfo.err = jpeg_std_error(&jerr);

  jpeg_create_compress(&cinfo);
  //filename = "C:\\Users\\Petar\\Downloads\\pic.jpg";
  if ((outfile = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    return;
  }
  jpeg_stdio_dest(&cinfo, outfile);

  cinfo.image_width = Img_src.Width; 	/* image width and height, in pixels */
  cinfo.image_height = Img_src.Height;
  if (Img_src.Num_channels == 3)
  {
      cinfo.input_components = 3;		/* # of color components per pixel */
      /*Currently HSL and Lab are not supported so we will save it as RGB.*/
      if (Img_src.ColorSpace == 2 || Img_src.ColorSpace == 5 || Img_src.ColorSpace == 4)
          cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
      else if (Img_src.ColorSpace == 3)
          cinfo.in_color_space = JCS_YCbCr;

      row_stride = Img_src.Width * 3;
  }
  else if (Img_src.Num_channels == 1)
  {
      cinfo.input_components = 1;		/* # of color components per pixel */
      cinfo.in_color_space = JCS_GRAYSCALE; 	/* colorspace of input image */
      row_stride = Img_src.Width;
  }
  jpeg_set_defaults(&cinfo);

  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  jpeg_start_compress(&cinfo, TRUE);

    /* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height)
  {
    row_pointer[0] = & Img_src.rgbpix[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  jpeg_finish_compress(&cinfo);

  fclose(outfile);

  jpeg_destroy_compress(&cinfo);
#endif
#ifdef QT_LIBRARIES

  QImage Image_Output2= QImage(Img_src.rgbpix, Img_src.Width, Img_src.Height,  QImage::Format_RGB32);

  Image_Output2.save(filename,"",100);

#endif
}

#ifdef VS_LIBRARY
struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};


typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  (*cinfo->err->output_message) (cinfo);

  longjmp(myerr->setjmp_buffer, 1);
}


/*
    R E A D   I M A G E
*/
struct Image read_Image_file(FILE * infile)
{
  struct jpeg_decompress_struct cinfo;

  struct Image Img_src;
  struct my_error_mgr jerr;

  JSAMPARRAY buffer;		/* Output row buffer */
  int row_stride;		/* physical row width in output buffer */

  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return Img_src;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_stdio_src(&cinfo, infile);

  /* Step 3: read file parameters with jpeg_read_header() */

  (void) jpeg_read_header(&cinfo, TRUE);


  (void) jpeg_start_decompress(&cinfo);

  row_stride = cinfo.output_width * cinfo.output_components;

  /* Fill the structure */
  Img_src.Width = cinfo.image_width;
  Img_src.Height = cinfo.image_height;
  Img_src.Num_channels = cinfo.num_components;
  Img_src.ColorSpace = cinfo.jpeg_color_space;


  /* TO  DELETE */
  Img_src.ColorSpace = 2;
   Img_src.imageDepth = 8;
  /***************/


  Img_src.rgbpix = (unsigned char *)calloc(Img_src.Num_channels * Img_src.Width*Img_src.Height, sizeof(unsigned char));
  buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);


  while (cinfo.output_scanline < cinfo.output_height)
  {

    (void) jpeg_read_scanlines(&cinfo, buffer, 1);

    if(debug)printf("cinfo.output_scanline=%d\n", cinfo.output_scanline);
    if(debug)printf("cinfo.output_height=%d\n", cinfo.output_height);
    if(debug)printf("row_stride=%d\n", row_stride);
    put_scanline(buffer[0], cinfo.output_scanline, cinfo.output_width,
                 cinfo.output_height, Img_src.rgbpix);
  }

  (void) jpeg_finish_decompress(&cinfo);

  jpeg_destroy_decompress(&cinfo);

  fclose(infile);

  return Img_src;
}

static void put_scanline(unsigned char buffer[], int line, int width,
                         int height, unsigned char *rgbpix)
{
  int i, k;

    //k = (height-line)*3*width;
    k = (line-1) * 3 * width;
    for(i=0; i<3*width; i+=3)
    {
      rgbpix[k+i]   = buffer[i];
      rgbpix[k+i+1] = buffer[i+1];
      rgbpix[k+i+2] = buffer[i+2];
    }
} /* end put_scanline */

#endif


/*
    Convert 4 channels to 3 channels
*/
void Convert4ChannelsTo3Channels(struct Image *channels4, struct Image * channels3)
{
    int i,j;

    j = 0;
    for( i = 0; i < 4  * channels4->Width * channels4->Height; i++)
    {
        if((i+1) %4 ==0 && i != 0) continue;
         channels3->rgbpix[j++] = channels4->rgbpix[i];
    }
}

/*
    Convert 3 channels to 4 channels
*/
void Convert3ChannelsTo4Channels(struct Image *channels3, struct Image * channels4)
{
    int i,j;

    j = 0;

    for(i = 0; i < 3  * channels3->Width * channels3->Height; i++)
    {
        channels4->rgbpix[j++] = channels3->rgbpix[i];
        if((j+1) %4 == 0 && j != 0)  channels4->rgbpix[j++] = 255;
    }
}

/*
    C R E A T E -  New Image - Based On Prototype
*/
struct Image CreateNewImage_BasedOnPrototype(struct Image *Prototype, struct Image *Img_dst)
{
    Img_dst->ColorSpace = Prototype->ColorSpace;
    Img_dst->Height = Prototype->Height;
    Img_dst->Width = Prototype->Width;
    Img_dst->Num_channels = Prototype->Num_channels;
    Img_dst->ColorSpace = Prototype->ColorSpace;
    Img_dst->imageDepth = Prototype->imageDepth;

    Img_dst->rgbpix = (unsigned char *)calloc(Img_dst->Height * Img_dst->Width * Img_dst->Num_channels, sizeof(unsigned char));
    if (Img_dst->rgbpix == NULL)
    {
        printf("cannot allocate memory for the new image\n");
        Img_dst->isLoaded = 0;
    }
    else
        Img_dst->isLoaded = 1;

    memcpy(Img_dst->rgbpix, Prototype->rgbpix, Prototype->Num_channels * Prototype->Width * Prototype->Height * sizeof(unsigned char));

    return *Img_dst;
}

/*
    C R E A T E -  New Image
*/
struct Image CreateNewImage(struct Image *Img_dst, int Width, int Height, int NumChannels, int ColorSpace, int Depth)
{
    FILE *fdebug = NULL;
    Img_dst->ColorSpace = ColorSpace;
    Img_dst->Height = Height;
    Img_dst->Width = Width;
    Img_dst->Num_channels = NumChannels;
    Img_dst->imageDepth = Depth;

    /* If we have Binary or Grayscale image, NumChannels should be == 1*/
    if (ColorSpace < 2 && NumChannels != 1)
    {
        Img_dst->isLoaded = 0;
        return *Img_dst;
    }
    Img_dst->ColorSpace = ColorSpace;

    Img_dst->rgbpix = (unsigned char *)calloc(Img_dst->Height * Img_dst->Width * Img_dst->Num_channels, sizeof(unsigned char));
    if (Img_dst->rgbpix == NULL)
    {

        #ifdef DEBUG_FILE
            fdebug = fopen(DEBUG_FILE, "wt");
            fprintf(fdebug,"cannot allocate memory for the new image\n");
            fclose(fdebug);
        #endif // DEBUG_FILE

        Img_dst->isLoaded = 0;
    }
    else
        Img_dst->isLoaded = 1;

    return *Img_dst;
}

/*
    S E T   D E S T I N A T I O N  - to match the size of the prototype
*/
struct Image SetDestination(struct Image *Prototype, struct Image *Img_dst)
{
    FILE *fdebug = NULL;

    Img_dst->ColorSpace = Prototype->ColorSpace;
    Img_dst->Height = Prototype->Height;
    Img_dst->Width = Prototype->Width;
    Img_dst->Num_channels = Prototype->Num_channels;
    Img_dst->imageDepth = Prototype->imageDepth;

    Img_dst->rgbpix = (unsigned char *)realloc(Img_dst->rgbpix, Img_dst->Height * Img_dst->Width * Img_dst->Num_channels* sizeof(unsigned char));
    if (Img_dst->rgbpix == NULL)
    {
        #ifdef DEBUG_FILE
                fdebug = fopen(DEBUG_FILE, "wt");
                fprintf(fdebug, "cannot allocate memory for the new image\n");
                fclose(fdebug);
        #endif // DEBUG_FILE
        Img_dst->isLoaded = 0;
    }
    else
        Img_dst->isLoaded = 1;
    return *Img_dst;
}

/*
    D E S T R O Y  image
*/
void DestroyImage(struct Image *Img)
{
    free(Img->rgbpix);
}

/*
    L A Y E R S - Create number of layers for prototype image
*/
struct Image * CreateImageLayersBasedOnPrototype(struct Image *Prototype, int NumberofLayers)
{
    int i = 0;
    struct Image *ArrOfLayers = NULL;
    struct Image Layer = CreateNewImage_BasedOnPrototype(Prototype, &Layer);

    ArrOfLayers = (struct Image *)calloc(NumberofLayers, sizeof(Layer));

    for (i = 0; i < NumberofLayers; i++)
    {
        ArrOfLayers[i] = CreateNewImage_BasedOnPrototype(Prototype, &ArrOfLayers[i]);
    }
    return ArrOfLayers;
}

/*
    L A Y E R S - Merge
*/
struct Image CombineLayers(struct Image *Layers, struct Image *Img_dst, struct Image Mask)
{

    int i, j, k;
    if (Layers[0].Width != Img_dst->Width)
    {
        SetDestination(&Layers[0], Img_dst);
    }

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            for (k = 0; k < Img_dst->Num_channels; k++)
            {
                /* we do not verify binary mask */
                Img_dst->rgbpix[Img_dst->Num_channels *(i * Img_dst->Width + j) + k] = Layers[Mask.rgbpix[i * Img_dst->Width + j]].rgbpix[Img_dst->Num_channels *(i * Img_dst->Width + j) + k];
            }
        }
    }

    return *Img_dst;
}

/*
    M A S K -  for Merging Layers
*/
struct Image  CreateMaskForLayers(struct Image *LayerPrototype, int MaskType, int NumberOfLayers, int algoParam1, int algoparam2)
{
    int i, j, k;
    struct Image Mask;
    int CurWidth, CurHeight = 0;
    if (MaskType < 1) MaskType = 1;

    Mask = CreateNewImage(&Mask, LayerPrototype->Width, LayerPrototype->Height, 1, 1, 8);

    if (MaskType == 1)
    {
        for (k = 0; k < NumberOfLayers; k++)
        {
            for (i = k * Mask.Height / NumberOfLayers; i < (k + 1) * Mask.Height / NumberOfLayers; i++)
            {
                for (j = 0; j < Mask.Width; j++)
                {
                    Mask.rgbpix[i * Mask.Width + j] = k;
                }
            }
        }
    }
    if (MaskType == 2)
    {
        for (k = 0; k < NumberOfLayers; k++)
        {
            for (j = k * Mask.Width / NumberOfLayers; j < (k + 1) * Mask.Width / NumberOfLayers; j++)
            {
                for (i = 0; i < Mask.Height; i++)
                {
                    Mask.rgbpix[i * Mask.Width + j] = k;
                }
            }
        }
    }

    return Mask;
}

/*
     W H I T E   P O I N T - fill structure
*/
/* White Balance function and structure*/
void SetWhiteBalanceValues(struct WhitePoint *WhitePoint_lab, int TYPE)
{
    if (TYPE == 0)			// A         // 2856K  // Halogen
    {
        WhitePoint_lab->Temperature = 2856;
        WhitePoint_lab->X = 0.44757;
        WhitePoint_lab->Y = 0.40744;
        WhitePoint_lab->Z = 0.14499;
        WhitePoint_lab->u = 0;
        WhitePoint_lab->v = 0;
    }
    else if (TYPE == 1)		// F11       // 4000K  // Narrow-band Fluorescent
    {
        WhitePoint_lab->Temperature = 4000;
        WhitePoint_lab->X = 0.38054;
        WhitePoint_lab->Y = 0.37691;
        WhitePoint_lab->Z = 0.24254;
        WhitePoint_lab->u = 0;
        WhitePoint_lab->v = 0;
    }
    else if (TYPE == 2)		// F2        // 4200K   // Cool white Fluorescent
    {
        WhitePoint_lab->Temperature = 4200;
        WhitePoint_lab->X = 0.372;
        WhitePoint_lab->Y = 0.3751;
        WhitePoint_lab->Z = 0.2528;
        WhitePoint_lab->u = 0;
        WhitePoint_lab->v = 0;
    }
    else if (TYPE == 3)		// B         // 4874K  // Direct sunlight at noon - obsolete
    {
        WhitePoint_lab->Temperature = 4874;
        WhitePoint_lab->X = 0.44757;
        WhitePoint_lab->Y = 0.40744;
        WhitePoint_lab->Z = 0.14499;
        WhitePoint_lab->u = 0;
        WhitePoint_lab->v = 0;
    }
    else if (TYPE == 4)		// D50       // 5000K  // Daylight - for color rendering
    {
        WhitePoint_lab->Temperature = 5000;
        WhitePoint_lab->X = 0.34567;
        WhitePoint_lab->Y = 0.35850;
        WhitePoint_lab->Z = 0.29583;
        WhitePoint_lab->u = 0;
        WhitePoint_lab->v = 0;
    }
    else if (TYPE == 5)		// E		 // 5400K  // Uniform energy
    {
        WhitePoint_lab->Temperature = 5400;
        WhitePoint_lab->X = 0.333;
        WhitePoint_lab->Y = 0.333;
        WhitePoint_lab->Z = 0.333;
        WhitePoint_lab->u = 0;
        WhitePoint_lab->v = 0;
    }
    else if (TYPE == 6)		// D55       // 5500K  // Daylight - for photography
    {
        WhitePoint_lab->Temperature = 5500;
        WhitePoint_lab->X = 0.9642;//0.33242;
        WhitePoint_lab->Y = 1;// 0.34743;
        WhitePoint_lab->Z = 0.8252;// 0.32015;
        WhitePoint_lab->u = 0;
        WhitePoint_lab->v = 0;
    }
    else if (TYPE == 7)		// D65       //  6504K  // North Sky - Daylight(NewVersion)
    {
        WhitePoint_lab->Temperature = 6504;
        WhitePoint_lab->X = 0.3127;
        WhitePoint_lab->Y = 0.329;
        WhitePoint_lab->Z = 0.3583;
        WhitePoint_lab->u = 0;
        WhitePoint_lab->v = 0;
    }
    else if (TYPE == 8)		// C         //  6774K  // North Sky - Daylight
    {
        WhitePoint_lab->Temperature = 6774;
        WhitePoint_lab->X = 0.31006;
        WhitePoint_lab->Y = 0.31615;
        WhitePoint_lab->Z = 0.37379;
        WhitePoint_lab->u = 0;
        WhitePoint_lab->v = 0;
    }
    else if (TYPE == 9)		// D75	     //  7500K  // Daylight
    {
        WhitePoint_lab->Temperature = 7500;
        WhitePoint_lab->X = 0.29902;
        WhitePoint_lab->Y = 0.31485;
        WhitePoint_lab->Z = 0.38613;
        WhitePoint_lab->u = 0;
        WhitePoint_lab->v = 0;
    }
    else if (TYPE == 10)	// D93	     //  9300K  // High eff.Blue Phosphor monitors
    {
        WhitePoint_lab->Temperature = 9300;
        WhitePoint_lab->X = 0.2848;
        WhitePoint_lab->Y = 0.2932;
        WhitePoint_lab->Z = 0.422;
        WhitePoint_lab->u = 0;
        WhitePoint_lab->v = 0;
    }
    else 	// NonExisting - Like Uniform
    {
        WhitePoint_lab->Temperature = 5400;
        WhitePoint_lab->X = 0.333;
        WhitePoint_lab->Y = 0.333;
        WhitePoint_lab->Z = 0.333;
        WhitePoint_lab->u = 0;
        WhitePoint_lab->v = 0;

        //WhitePoint_lab->X = 1;
        //WhitePoint_lab->Y = 1;
        //WhitePoint_lab->Z = 1;
    }
}


/********************************************************************************
*																				*
*	CV Library - Compute.c														*
*																				*
*	Author:  Petar Nikolov														*
*																				*
*																				*
*	The algorithms included in this file are:									*
*																				*
*	- Index to Position															*
*	- Position to Index															*
*	- Round																	    *
*	- Convolution / Binary														*
*																				*
*																				*
*********************************************************************************/



/*
    convert  I N D E X t o P O S
*/
void getPositionFromIndex(struct Image *Img_src, int pixIdx, int *red, int *col)
{
    *red = pixIdx / (Img_src->Num_channels * Img_src->Width);
    *col = pixIdx - ((*red)*Img_src->Width * Img_src->Num_channels);
}

/*
    convert   P O S t o I N D E X
*/
int getPixelIndex(struct Image *Img_src, int *pixIdx, int red, int col)
{
    int pixelIndex = 0;

    *pixIdx = (red * Img_src->Width + col)*Img_src->Num_channels;
    pixelIndex = *pixIdx;

    return pixelIndex;
}

/*
    R O U N D   -  RoundValues to X significant bit
*/
float RoundValue_toX_SignificantBits(float Value, int X)
{
    int ValueTimesX = 0;
    float Number = Value;

    ValueTimesX = Value * pow(10.0, X);
    Number *= pow(10.0, X);

    if (Number - ValueTimesX > 0.5)
        ValueTimesX += 1;

    Number = ValueTimesX / pow(10.0, X);

    if (Number * pow(10.0, X) < ValueTimesX)
        Number += (1 / pow(10.0, X));
    return Number;
}


/*
    C O N V O L U T I O N
*/
void Convolution(unsigned char *InputArray, unsigned char *OutputArray, int rows, int cols, float *Kernel, int KernelSize)
{
    int i, j, n, m;
    int FinalNum;
    int DevideNumber = pow((float)KernelSize, 2);

    for (i = KernelSize / 2; i < cols - KernelSize / 2; i++)
    {
        for (j = KernelSize / 2; j < rows - KernelSize / 2; j++)
        {
            //OutputArray[i * cols + j] = OutputArray[i * cols + j];
            size_t c = 0;
            FinalNum = 0;
            for (n = -KernelSize / 2; n <= KernelSize / 2; n++)
            {
                DevideNumber = 9;
                for (m = -KernelSize / 2; m <= KernelSize / 2; m++)
                {
                    FinalNum += InputArray[(j - n) * cols + i - m] * Kernel[c];
                    //if (FinalNum != 0)printf("%d\n", FinalNum);
                    //if (Kernel[c] == 0) DevideNumber--;
                    c++;
                }
            }
            if (DevideNumber <= 0)
                FinalNum = 0;
            else
                FinalNum = (float)FinalNum / DevideNumber;

            if (FinalNum < 0) FinalNum = 0;
            else if (FinalNum > 255) FinalNum = 255;
            //if (FinalNum > 128) FinalNum = 255;
            //else FinalNum = 0;

            OutputArray[j * cols + i] = FinalNum;
        }
    }
}

/*
    C O N V O L U T I O N - Binary Image
*/
void ConvolutionBinary(unsigned char *InputArray, unsigned char *OutputArray, int rows, int cols, float *Kernel, int KernelSize, int DilateOrErode)
{
    int i, j, n, m;
    int FinalNum;
    int DevideNumber = pow((float)KernelSize, 2);

    for (i = KernelSize / 2; i < cols - KernelSize / 2; i++)
    {
        for (j = KernelSize / 2; j < rows - KernelSize / 2; j++)
        {
            size_t c = 0;
            FinalNum = 0;

            if (InputArray[(j) * cols + i] > 0)
            {
                for (n = -KernelSize / 2; n <= KernelSize / 2; n++)
                {
                    for (m = -KernelSize / 2; m <= KernelSize / 2; m++)
                    {
                        if (Kernel[c] == 1)
                        {
                            if (DilateOrErode == 0)
                                OutputArray[(j - n) * cols + i - m] = 255;
                            else
                                OutputArray[(j - n) * cols + i - m] = 0;
                        }
                        else
                        {
                            OutputArray[(j - n) * cols + i - m] = InputArray[(j - n) * cols + i - m];
                        }
                        c++;
                    }
                }
            }
            else
            {
                OutputArray[(j) * cols + i ] = InputArray[(j) * cols + i];
            }
        }
    }
}

/********************************************************************************
*																				*
*	CV Library - ImageProcessingAlgos.c											*
*																				*
*	Author:  Petar Nikolov														*
*																				*
*																				*
*	The algorithms included in this file are:									*
*																				*
*	- Mirror image																*
*	- Crop																		*
*	- Morphology - opening/closing/dilatation/erotion							*
*	- Sharpening																*
*	- Atificial color															*
*	- BLur - over point or using mask											*
*	- Brightness																*
*	- Contrast																	*
*	- WhiteBalance																*
*	- Noise																		*
*	- Gamma correction															*
*	- Affine transforms - scale / rotation / transaltion						*
*	- Edge extraction - Magnitude / Hysteresis / non-Max supp / follow edges	*
*	- Find Derivative															*
*	- Saturation																*
*																				*
*																				*
*********************************************************************************/

/*
    M I R R O R  image - horizontal
*/
struct Image MirrorImageHorizontal(struct Image *Img_src, struct Image *Img_dst)
{
    int i, j, l, k;

    for (i = Img_dst->Height-1; i >= 0; i--)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            for (l = 0; l < Img_dst->Num_channels; l++)
            {
                Img_dst->rgbpix[((Img_dst->Height - 1 - i) * Img_src->Num_channels * Img_src->Width) + (Img_src->Num_channels * j) + l] = Img_src->rgbpix[(i * Img_src->Num_channels * Img_src->Width) + (Img_src->Num_channels * j) + l];
            }
        }
    }

    return *Img_dst;
}

/*
    M I R R O R  image - vertical
*/
struct Image MirrorImageVertical(struct Image *Img_src, struct Image *Img_dst)
{
    int i, j, l;

    if (Img_src->Num_channels != Img_dst->Num_channels)
        return *Img_dst;

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            for (l = 0; l < Img_dst->Num_channels; l++)
            {
                Img_dst->rgbpix[(i * Img_src->Num_channels * Img_src->Width) + (Img_src->Num_channels * (Img_dst->Width - 1 - j)) + l] = Img_src->rgbpix[(i * Img_src->Num_channels * Img_src->Width) + (Img_src->Num_channels * j) + l];
            }
        }
    }

    return *Img_dst;
}

/*
    C R O P  image - around given point and given new dimensions
*/
struct Image CropImage(struct Image *Img_src, struct Image *Img_dst, struct point_xy CentralPoint, int NewWidth, int NewHeight)
{
    int i, j, l, k, z;

    if (Img_src->Num_channels != Img_dst->Num_channels)
        return *Img_dst;

    if (NewWidth >= Img_src->Width || NewHeight >= Img_src->Height)
        return *Img_dst;

    /* Modify Img_dst */
    Img_dst->Width = NewWidth;
    Img_dst->Height = NewHeight;
    Img_dst->rgbpix = (unsigned char *)realloc(Img_dst->rgbpix, Img_dst->Width * Img_dst->Height * Img_dst->Num_channels * sizeof(unsigned char));

    /* Modify Central point - shift x and y*/
    if (CentralPoint.X > Img_src->Width - 1) CentralPoint.X = Img_src->Width - 1;
    if (CentralPoint.Y > Img_src->Height - 1) CentralPoint.X = Img_src->Height - 1;
    if (CentralPoint.X < 0) CentralPoint.X = 0;
    if (CentralPoint.Y < 0) CentralPoint.X = 0;

    /* for x - too right*/
    if (Img_src->Width < (NewWidth / 2) + CentralPoint.X) CentralPoint.X = Img_src->Width - (NewWidth / 2) - 1;
    /* for y - too down */
    if (Img_src->Height < (NewHeight / 2) + CentralPoint.Y) CentralPoint.Y = Img_src->Height - (NewHeight / 2) - 1;
    /* for x - too left*/
    if (CentralPoint.X - (NewWidth / 2 ) < 0) CentralPoint.X = (NewWidth / 2) + 1;
    /* for y - too up*/
    if (CentralPoint.Y - (NewHeight / 2) < 0) CentralPoint.Y = (NewHeight / 2) + 1;

    k = CentralPoint.Y - (NewHeight / 2);
    for (i = 0; i < Img_dst->Height; i++)
    {
        k++;
        z = CentralPoint.X - (NewWidth / 2);
        for (j = 0; j < Img_dst->Width; j++)
        {
            z++;
            for (l = 0; l < Img_dst->Num_channels; l++)
            {
                Img_dst->rgbpix[(i * Img_dst->Num_channels * Img_dst->Width) + (Img_dst->Num_channels * j) + l] = Img_src->rgbpix[(k * Img_src->Num_channels * Img_src->Width) + (Img_src->Num_channels * z) + l];
            }
        }
    }

    return *Img_dst;
}

/*
    M O R P H O L O G Y  -  Dilation
*/
struct Image MorphDilate(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberOfIterations)
{
    int i, j, l, k, z;
    //float StructureElement[9] = { 0, 1, 0, 1, 1, 1, 0, 1, 0 };
    float StructureElement[9] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };

    /* Only Grayscale is currently supported */
    if ((Img_src->Num_channels != Img_dst->Num_channels) && Img_dst->Num_channels != 1)
        return *Img_dst;
    if (ElementSize < 3) ElementSize = 3;
    if (NumberOfIterations < 0) NumberOfIterations = 0;
    if ((Img_src->Width != Img_dst->Width) || (Img_src->Height != Img_dst->Height)) SetDestination(Img_src, Img_dst);

    ConvolutionBinary(Img_src->rgbpix, Img_dst->rgbpix, Img_src->Height, Img_src->Width, StructureElement, ElementSize, 0);

    if (NumberOfIterations % 2 == 0)
    {
        if (NumberOfIterations > 0 && NumberOfIterations % 2 == 0)
        {
            NumberOfIterations -= 1;
            if (NumberOfIterations != 0) MorphDilate(Img_src, Img_dst, ElementSize, NumberOfIterations);
            return *Img_dst;
        }
    }
    else
    {
        if (NumberOfIterations > 0 && NumberOfIterations % 2 == 1)
        {
            NumberOfIterations -= 1;
            if (NumberOfIterations != 0) MorphDilate(Img_dst, Img_src, ElementSize, NumberOfIterations);
            return *Img_src;
        }
    }

}

/*
    M O R P H O L O G Y  -  Erosion
*/
struct Image MorphErode(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberOfIterations)
{
    int i, j, l, k, z;
    float StructureElement[9] = { 0, 1, 0, 1, 1, 1, 0, 1, 0 };

    if (Img_src->Num_channels != Img_dst->Num_channels)
        return *Img_dst;
    if (ElementSize < 3) ElementSize = 3;
    if (NumberOfIterations < 0) NumberOfIterations = 0;

    if ((Img_src->Width != Img_dst->Width) || (Img_src->Height != Img_dst->Height)) SetDestination(Img_src, Img_dst);


    ConvolutionBinary(Img_src->rgbpix, Img_dst->rgbpix, Img_src->Height, Img_src->Width, StructureElement, ElementSize, 1);

    if (NumberOfIterations % 2 == 0)
    {
        if (NumberOfIterations > 0 && NumberOfIterations % 2 == 0)
        {
            NumberOfIterations -= 1;
            if(NumberOfIterations != 0) MorphErode(Img_src, Img_dst, ElementSize, NumberOfIterations);
            return *Img_dst;
        }
    }
    else
    {
        if (NumberOfIterations > 0 && NumberOfIterations % 2 == 1)
        {
            NumberOfIterations -= 1;
            if (NumberOfIterations != 0) MorphErode(Img_dst, Img_src, ElementSize, NumberOfIterations);
            return *Img_src;
        }
    }
}


/*
    M O R P H O L O G Y  -  Opening
*/
struct Image MorphOpen(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberOfIterations)
{
    struct Image BackupImage = CreateNewImage(&BackupImage, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE, 8);

    MorphErode(Img_src, Img_dst, ElementSize, NumberOfIterations);
    MorphDilate(Img_dst, &BackupImage, ElementSize, NumberOfIterations);

    memcpy(Img_dst->rgbpix, BackupImage.rgbpix, Img_dst->Width * Img_dst->Height * sizeof(unsigned char));
    //memcpy(Img_dst, &BackupImage, sizeof(BackupImage));

    DestroyImage(&BackupImage);
    return *Img_dst;

}

/*
    M O R P H O L O G Y  -  Closing
*/
struct Image MorphClose(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberOfIterations)
{
    struct Image BackupImage = CreateNewImage(&BackupImage, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE, 8);

    MorphDilate(Img_src, Img_dst, ElementSize, NumberOfIterations);
    MorphErode(Img_dst, &BackupImage, ElementSize, NumberOfIterations);

    memcpy(Img_dst->rgbpix, BackupImage.rgbpix, Img_dst->Width * Img_dst->Height * sizeof(unsigned char));
    //memcpy(Img_dst, &BackupImage, sizeof(Img_dst));

    DestroyImage(&BackupImage);
    return *Img_dst;
}

/*
    S H A R P   image - using EdgeExtraction function
*/
struct Image SharpImageContours(struct Image *Img_src, struct Image *Img_dst, float Percentage)
{
    int i, j, l;

    Image Img_dst_Grayscale = CreateNewImage(&Img_dst_Grayscale, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE, 8);
    Image Img_src_Grayscale = CreateNewImage(&Img_src_Grayscale, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE, 8);

    if ((float)fabs((float)Percentage) > 1) Percentage /= 100;
    Percentage *= -1;
    if ((Img_src->Width != Img_dst->Width) || (Img_src->Height != Img_dst->Height)) SetDestination(Img_src, Img_dst);

    ConvertToGrayscale_1Channel(Img_src, &Img_src_Grayscale);

    EdgeExtraction(&Img_src_Grayscale, &Img_dst_Grayscale, EDGES_PREWITT, 1, 0.9);

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            for (l = 0; l < Img_dst->Num_channels; l++)
            {
                if (Img_dst_Grayscale.rgbpix[(i*Img_src->Width + j)] >= EDGE)//POSSIBLE_EDGE)
                {
                    if (Percentage * Img_dst_Grayscale.rgbpix[(i*Img_src->Width + j)] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] > 255)
                    {
                        Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 255;
                    }
                    else if (Percentage * Img_dst_Grayscale.rgbpix[(i*Img_src->Width + j)] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] < 0)
                    {
                        Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 0;
                    }
                    else
                    {
                        Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = Percentage * Img_dst_Grayscale.rgbpix[(i*Img_src->Width + j)] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
                    }
                }
                else
                {
                    Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
                    break;
                }
            }
        }
    }

    DestroyImage(&Img_dst_Grayscale);
    DestroyImage(&Img_src_Grayscale);

    return *Img_dst;
}

/*
    S H A R P   image - using Binary mask
*/
struct Image SharpImageBinary(struct Image *Img_src, struct Image *Img_dst, struct Image *Img_Binary, float Percentage)
{
    int i, j, l;

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            for (l = 0; l < Img_dst->Num_channels; l++)
            {
                if (Img_Binary->rgbpix[(i*Img_src->Width + j)] == 1)
                {
                    if (Percentage * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] > 255)
                        Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 255;
                    else if (Percentage * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] < 0)
                        Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 0;
                    else Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = Percentage * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
                }
                else
                    Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
            }
        }
    }

    return *Img_dst;
}

/*
    C O L O R  -  artificial
*/
struct Image ColorFromGray(struct Image *Img_src, struct Image *Img_dst, struct ColorPoint_RGB ColorPoint)
{
    int i, j, l;
    float R_to_G_Ratio = 0;
    float B_to_G_Ratio = 0;

    // The input should be 1 channel image. The output is 3 channel
    if (Img_src->Num_channels != 1 || Img_dst->Num_channels != 3)
    {
        return *Img_dst;
    }

    R_to_G_Ratio = ColorPoint.R / (float)ColorPoint.G;
    B_to_G_Ratio = ColorPoint.B / (float)ColorPoint.G;

    //step 1: copy the gray information to RGB channels
    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            if (R_to_G_Ratio*Img_src->rgbpix[(i*Img_src->Width + j)] > 255)
                Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = 255;
            else
                Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = R_to_G_Ratio*Img_src->rgbpix[(i*Img_src->Width + j)];
            Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 1] = Img_src->rgbpix[(i*Img_src->Width + j)];
            if (B_to_G_Ratio*Img_src->rgbpix[(i*Img_src->Width + j)] > 255)
                Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = 255;
            else
                Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = B_to_G_Ratio*Img_src->rgbpix[(i*Img_src->Width + j)];
        }
    }
    //step 2: Gamma correction for B and R channels
    //GammaCorrection(Img_src, Img_dst, 0.6, 0.95, 0.7);

    return *Img_dst;
}


/*
    B L U R  image function - around point
*/
struct Image BlurImageAroundPoint(struct Image *Img_src, struct Image *Img_dst, struct point_xy CentralPoint, int BlurPixelRadius, int SizeOfBlur, int BlurOrSharp, int BlurAgression)
{
    float MaxRatio = 0;
    float Distance = 0;
    float DistanceRatio = 0;
    float *matrix = (float *)calloc(Img_src->Width * Img_src->Height, sizeof(float));
    float Chislo = 0, Chislo2 = 0;
    int Sybiraemo = 0;
    int i, j, z, t, l;

    /* Only odd nubers are allowed (bigger than 5*/
    if (BlurPixelRadius % 2 == 0) BlurPixelRadius += 1;
    if (BlurPixelRadius < 5) BlurPixelRadius = 5;

    MaxRatio = (float)MAX(CentralPoint.X - ((float)SizeOfBlur * Img_dst->Width / 100), Img_dst->Width - CentralPoint.X + ((float)SizeOfBlur * Img_dst->Width / 100)) / ((float)SizeOfBlur * Img_dst->Width / 100);
    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            //luma = ui->imageData[row * width + col];
            Distance = sqrt(pow(fabs((float)CentralPoint.X - j), 2) + pow(fabs((float)CentralPoint.Y - i), 2));
            if (Distance < ((float)SizeOfBlur * Img_dst->Width / 100))
            {
                matrix[i * Img_dst->Width + j] = 1;
            }
            else
            {
                DistanceRatio = Distance / ((float)SizeOfBlur * Img_dst->Width / 100);
                matrix[i * Img_dst->Width + j] = 1 - ((float)BlurAgression / 100 * (DistanceRatio / MaxRatio));
                if (matrix[i * Img_dst->Width + j] < 0) matrix[i * Img_dst->Width + j] = 0;
            }
        }
    }

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {

            if (i < BlurPixelRadius / 2 || j < BlurPixelRadius / 2 || j >= Img_dst->Width - BlurPixelRadius / 2 || i >= Img_dst->Height - BlurPixelRadius / 2)
            {
                for (l = 0; l < Img_dst->Num_channels; l++)
                {
                    Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l];
                }
                continue;
            }
            for (l = 0; l < 3; l++)
            {
                if (Img_src->rgbpix[3 * (i*Img_dst->Width + j) + l] > 255)
                    Chislo = 0;

                Sybiraemo = 0;
                if (BlurOrSharp == 0)
                    Chislo2 = ((float)(matrix[i * Img_dst->Width + j]) / (pow((float)BlurPixelRadius, 2) - 1 - (12 + (2 * (BlurPixelRadius - 5)))));
                else
                    Chislo2 = ((float)(1 - matrix[i * Img_dst->Width + j]) / (pow((float)BlurPixelRadius, 2) - 1 - (12 + (2 * (BlurPixelRadius - 5)))));
                for (z = 0; z < BlurPixelRadius / 2; z++)
                {
                    for (t = 0; t < BlurPixelRadius / 2; t++)
                    {
                        if (z == 0 && t == 0) continue;
                        Sybiraemo += Img_src->rgbpix[3*((i - z)*Img_dst->Width + j - t) + l];
                        Sybiraemo += Img_src->rgbpix[3*((i - z)*Img_dst->Width + j + t) + l];
                        Sybiraemo += Img_src->rgbpix[3*((i + z)*Img_dst->Width + j - t) + l];
                        Sybiraemo += Img_src->rgbpix[3*((i + z)*Img_dst->Width + j + t) + l];
                    }
                }

                Chislo2 *= Sybiraemo;
                Chislo = 0;
                if (BlurOrSharp == 0)
                    Chislo = (1 - matrix[i * Img_dst->Width + j])*Img_src->rgbpix[3*(i*Img_dst->Width + j) + l] + (int)Chislo2;
                else
                    Chislo = (matrix[i * Img_dst->Width + j])*Img_src->rgbpix[3*(i*Img_dst->Width + j) + l] + (int)Chislo2;
                if (Chislo > 255)
                    Chislo = 255;
                if (Chislo < 0)
                    Chislo = 0;
                Img_dst->rgbpix[3 * (i*Img_dst->Width + j) + l] = Chislo;
            }
        }
    }

    return *Img_dst;
}

/*
    B L U R  image function - Gaussian
*/
struct Image BlurImageGussian(struct Image *Img_src, struct Image *Img_dst, int BlurPixelRadius, float NeighborCoefficient)
{
    int i, j, l, z, t;
    int Sybiraemo = 0;
    float Chislo = 0;
    float Chislo2 = 0;
    if (BlurPixelRadius < 5) BlurPixelRadius = 5;
    if (NeighborCoefficient > 100) NeighborCoefficient /= 100;
    if (NeighborCoefficient < 0) NeighborCoefficient *= -1;

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            if (i < BlurPixelRadius / 2 || j < BlurPixelRadius / 2 || j >= Img_dst->Width - BlurPixelRadius / 2 || i >= Img_dst->Height - BlurPixelRadius / 2)
            {
                for (l = 0; l < Img_dst->Num_channels; l++)
                {
                    Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l];
                }
                continue;
            }
            for (l = 0; l < Img_dst->Num_channels; l++)
            {
                Sybiraemo = 0;
                Chislo2 = ((float)(NeighborCoefficient) / (pow((float)BlurPixelRadius, 2) - 1 - (12 + (2 * (BlurPixelRadius - 5)))));

                for (z = 0; z < BlurPixelRadius / 2; z++)
                {
                    for (t = 0; t < BlurPixelRadius / 2; t++)
                    {
                        if (z == 0 && t == 0) continue;
                        Sybiraemo += Img_src->rgbpix[Img_dst->Num_channels * ((i - z)*Img_dst->Width + j - t) + l];
                        Sybiraemo += Img_src->rgbpix[Img_dst->Num_channels * ((i - z)*Img_dst->Width + j + t) + l];
                        Sybiraemo += Img_src->rgbpix[Img_dst->Num_channels * ((i + z)*Img_dst->Width + j - t) + l];
                        Sybiraemo += Img_src->rgbpix[Img_dst->Num_channels * ((i + z)*Img_dst->Width + j + t) + l];
                    }
                }

                Chislo2 *= Sybiraemo;
                Chislo = 0;
                Chislo = (1 - NeighborCoefficient)*Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] + (int)Chislo2;

                if (Chislo > 255)
                    Chislo = 255;
                if (Chislo < 0)
                    Chislo = 0;
                Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = Chislo;
            }
        }
    }
    return *Img_dst;
}

/*
    Correct   B R I G H T N E S S  - RGB and Lab only !
*/
struct Image BrightnessCorrection(struct Image *Img_src, struct Image *Img_dst, float Algo_paramBrightnessOrEV, int Algotype)
{
    int i, j, l;

    if (Img_src->Num_channels != Img_dst->Num_channels)
        return *Img_dst;
    if (Img_src->ColorSpace != Img_dst->ColorSpace || Img_src->Width != Img_dst->Width)
        SetDestination(Img_src, Img_dst);

    if (Img_src->ColorSpace == 2) //RGB
    {
        if (Algotype == 1)
        {
            Algo_paramBrightnessOrEV /= 100.0;

            for (i = 0; i < Img_dst->Height; i++)
            {
                for (j = 0; j < Img_dst->Width; j++)
                {
                    for (l = 0; l < Img_dst->Num_channels; l++)
                    {
                        if (Algo_paramBrightnessOrEV * Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] + Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l]> 255)
                            Img_dst->rgbpix[3 * (i*Img_dst->Width + j) + l] = 255;
                        else
                        {
                            if (Algo_paramBrightnessOrEV * Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] + Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] < 0)
                                Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = 0;
                            else
                                Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = Algo_paramBrightnessOrEV * Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] + Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l];
                        }
                    }
                }
            }
        }
        else if (Algotype == 2)
        {
            for (i = 0; i < Img_dst->Height; i++)
            {
                for (j = 0; j < Img_dst->Width; j++)
                {
                    for (l = 0; l < Img_dst->Num_channels; l++)
                    {
                        if (pow(2, Algo_paramBrightnessOrEV) * Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] > 255)
                            Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = 255;
                        else if (pow(2, Algo_paramBrightnessOrEV) * Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] < 0)
                            Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = 0;
                        else
                            Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = pow(2, Algo_paramBrightnessOrEV) * Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l];
                    }
                }
            }
        }
    }
    else if (Img_src->ColorSpace == 4) // Lab
    {
        if (fabs(Algo_paramBrightnessOrEV) > 1) Algo_paramBrightnessOrEV /= 100;

        for (i = 0; i < Img_dst->Height; i++)
        {
            for (j = 0; j < Img_dst->Width; j++)
            {
                float L, a, b;
                L = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 0];
                L = Algo_paramBrightnessOrEV * L  + L;

                Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 0] = L;
                Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 1] = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 1];
                Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 2] = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 2];
            }
        }
    }

    return *Img_dst;
}

/*
    Correct    C O N T R A S T
*/
struct Image ContrastCorrection(struct Image *Img_src, struct Image *Img_dst, float percentage)
{
    /* The percentage value should be between -100 and 100*/
    float pixel = 0;
    float contrast = 0;
    int i, j, l;

    if (Img_src->Num_channels != Img_dst->Num_channels)
        return *Img_dst;

    if (percentage < -100) percentage = -100;
    if (percentage > 100) percentage = 100;
    contrast = (100.0 + percentage) / 100.0;

    contrast *= contrast;

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            for (l = 0; l < Img_dst->Num_channels; l++)
            {
                pixel = (float)Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] / 255;
                pixel -= 0.5;
                pixel *= contrast;
                pixel += 0.5;
                pixel *= 255;
                if (pixel < 0) pixel = 0;
                if (pixel > 255) pixel = 255;
                Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = pixel;
            }
        }
    }

    return *Img_dst;
}

/*
    Correct     W H I T E  B A L A N C E  - RGB
*/
struct Image WhiteBalanceCorrectionRGB(struct Image *Img_src, struct Image *Img_dst, int Algotype)
{
    int i, j;

    int MaxR = 0, MaxG = 0, MaxB = 0;
    float GtoR_Ratio = 1;
    float GtoB_Ratio = 1;
    long SumG = 0;
    long SumR = 0;
    long SumB = 0;
    float Gto255_Ratio = 1;
    int check3Values = 0;

    if ((Img_src->Num_channels != Img_dst->Num_channels) || (Img_src->Num_channels != 3))
        return *Img_dst;

    /* Only RGB is supported for WhiteBalance algos*/
    if (Img_src->ColorSpace != 2)
        return *Img_dst;

    if (Algotype == 1)
    {
        //Green world - automatic white detection
        for (i = 0; i < Img_src->Height; i++)
        {
            for (j = 0; j < Img_src->Width; j++)
            {
                check3Values = 0;
                if (Img_src->rgbpix[3 * (i*Img_src->Width + j) ] > MaxR) { MaxR = Img_src->rgbpix[3 * (i*Img_src->Width + j)]; check3Values++; }
                if (Img_src->rgbpix[3 * (i*Img_src->Width + j)  + 1] > MaxG) { MaxG = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1]; check3Values++;}
                if (Img_src->rgbpix[3 * (i*Img_src->Width + j)  + 2] > MaxB) { MaxB = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2]; check3Values++; }

                if (check3Values == 3)
                {
                    //Calculate ratios
                    GtoR_Ratio = (float)Img_src->rgbpix[3 * (i*Img_src->Width + j) +1] / Img_src->rgbpix[3 * (i*Img_src->Width + j) ];
                    GtoB_Ratio = (float)Img_src->rgbpix[3 * (i*Img_src->Width + j) +1] / Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
                }
            }
        }
        /*Calculate new values based on GtoR amd GtoB ratios*/
        for (i = 0; i < Img_src->Height; i++)
        {
            for (j = 0; j < Img_src->Width; j++)
            {
                if (GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j)] <= 255)
                    Img_dst->rgbpix[3 * (i*Img_src->Width + j) ] = GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j)];
                else Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = 255;
                if (GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2] <= 255)
                    Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
                else Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = 255;

                Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 1] = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1];
            }
        }
    }
    else if (Algotype == 2)
    {
        for (i = 0; i < Img_src->Height; i++)
        {
            for (j = 0; j < Img_src->Width; j++)
            {
                if (Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1] > MaxG)
                {
                    MaxG = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1];
                    GtoR_Ratio = (float)MaxG / Img_src->rgbpix[3 * (i*Img_src->Width + j)];
                    GtoB_Ratio = (float)MaxG / Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
                }
            }
        }
        /*Calculate new values based on GtoR and GtoB ratios*/
        for (i = 0; i < Img_src->Height; i++)
        {
            for (j = 0; j < Img_src->Width; j++)
            {
                if (GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j)] <= 255)
                    Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j)];
                else Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = 255;
                if (GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2] <= 255)
                    Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
                else Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = 255;

                Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 1] = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1];
            }
        }

    }
    else if (Algotype == 3)
    {
        //Green world - automatic white detection
        for (i = 0; i < Img_src->Height; i++)
        {
            for (j = 0; j < Img_src->Width; j++)
            {
                check3Values = 0;
                if (Img_src->rgbpix[3 * (i*Img_src->Width + j)] > MaxR) { MaxR = Img_src->rgbpix[3 * (i*Img_src->Width + j)]; check3Values++; }
                if (Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1] > MaxG) { MaxG = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1]; check3Values++; }
                if (Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2] > MaxB) { MaxB = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2]; check3Values++; }

                if (check3Values == 3)
                {
                    //Calculate ratios
                    Gto255_Ratio = 255 / MaxG;
                    GtoR_Ratio = (float)(Gto255_Ratio *(MaxG / MaxR));
                    GtoB_Ratio = (float)(Gto255_Ratio *(MaxG / MaxB));
                }
            }
        }
        /*Calculate new values based on GtoR amd GtoB ratios*/
        for (i = 0; i < Img_src->Height; i++)
        {
            for (j = 0; j < Img_src->Width; j++)
            {
                if (GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j)] <= 255)
                    Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j)];
                else Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = 255;
                if (GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2] <= 255)
                    Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
                else Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = 255;
                if (Gto255_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1] <= 255)
                    Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 1] = Gto255_Ratio* Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1];
                else Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 1] = 255;
            }
        }
    }
    else if (Algotype == 4)
    {
        //Green world - automatic white detection
        for (i = 0; i < Img_src->Height; i++)
        {
            for (j = 0; j < Img_src->Width; j++)
            {
                SumR += Img_src->rgbpix[3 * (i*Img_src->Width + j)    ];
                SumG += Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1];
                SumB += Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
            }
        }
        GtoR_Ratio =   (float)SumG / SumR;
        GtoB_Ratio =   (float)SumG / SumB;
        Gto255_Ratio = (float)SumG / (255 * Img_src->Height * Img_src->Width);
    /*	if (GtoB_Ratio < 0.8 || GtoR_Ratio < 0.8)
        {
            GtoR_Ratio = GtoR_Ratio * ;
            GtoB_Ratio = (float)SumG / SumB;
        }*/
        /*Calculate new values based on GtoR amd GtoB ratios*/
        for (i = 0; i < Img_src->Height; i++)
        {
            for (j = 0; j < Img_src->Width; j++)
            {
                if (GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) ] <= 255)
                    Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j)];
                else Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = 255;
                if (GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2] <= 255)
                    Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
                else Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = 255;

                Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 1] = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1];
            }
        }
    }
    else return *Img_src;

    return *Img_dst;
}

/*
    W H I T E  B A L A N C E  - for Lab color space
*/
struct Image WhiteBalanceCorrectionLAB(struct Image *Img_src, struct Image *Img_dst, struct WhitePoint WhitePointXYZ)
{
    int i, j, z;
    if (Img_src->Num_channels != 3)
        return *Img_dst;
    SetDestination(Img_src, Img_dst);
}

/*
    Correct    N O I S E
*/
struct Image NoiseCorrection(struct Image *Img_src, struct Image *Img_dst, float threshold, int Algotype)
{
    int i, j, z;
    int CurrentValue;
    int ProbablityValue = 0;

    if (Img_src->Num_channels != Img_dst->Num_channels)
        return *Img_dst;

    memcpy(&Img_dst, &Img_src,sizeof(Image));

    /* if the current pixel is X % different from the pixels around -> it is noise*/

    if (Algotype == 1)
    {
        for (i = 1; i < Img_dst->Height - 1; i++)
        {
            for (j = 1; j < Img_dst->Width - 1; j++)
            {
                for (z = 0; z < Img_dst->Num_channels; z++)
                {
                    ProbablityValue = 0;
                    CurrentValue = Img_src->rgbpix[i * Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z];

                    if (threshold * CurrentValue < Img_src->rgbpix[(i - 1) * Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z] || CurrentValue > threshold *  Img_src->rgbpix[(i - 1) * Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z]) ProbablityValue++;
                    if (threshold * CurrentValue < Img_src->rgbpix[(i + 1) * Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z] || CurrentValue > threshold *  Img_src->rgbpix[(i + 1) * Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z]) ProbablityValue++;
                    if (threshold * CurrentValue < Img_src->rgbpix[(i)* Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * (j - 1) + z] || CurrentValue > threshold *  Img_src->rgbpix[(i)* Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * (j - 1) + z]) ProbablityValue++;
                    if (threshold * CurrentValue < Img_src->rgbpix[(i)* Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * (j + 1) + z] || CurrentValue > threshold *  Img_src->rgbpix[(i)* Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * (j + 1) + z]) ProbablityValue++;

                    if (ProbablityValue >= 3)
                    {
                        Img_dst->rgbpix[(i)* Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z] = (Img_src->rgbpix[(i - 1) * Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z] + Img_src->rgbpix[(i + 1) * Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z] + Img_src->rgbpix[(i)* Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * (j - 1) + z] + Img_src->rgbpix[(i)* Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * (j + 1) + z]) / 4;
                    }
                }
            }
        }
    }
    else if (Algotype == 2)
    {
        //future implementation
    }

    return *Img_dst;
}

/*
    Correction   G A M M A  - RGB only
*/
struct Image GammaCorrection(struct Image *Img_src, struct Image *Img_dst, float RedGamma, float GreenGamma, float BlueGamma)
{
    FILE *fdebug = NULL;
    int i, j;

    int redGamma[256];
    int greenGamma[256];
    int blueGamma[256];

    if ((Img_src->Num_channels != Img_dst->Num_channels) || (Img_src->Num_channels != 3))
        return *Img_dst;
    if (Img_src->ColorSpace != 2)
    {
        #ifdef DEBUG_FILE
                fdebug = fopen(DEBUG_FILE, "wt");
                fprintf(fdebug, "GammaCorrection: The input image is not in RGB color space\n");
                fclose(fdebug);
        #endif // DEBUG_FILE
        return *Img_src;
    }

    for (i = 0; i < 256; ++i)
    {
        redGamma[i] = MIN(255, (int)((255.0 * pow((float)i / 255.0, 1.0 / RedGamma)) + 0.5));
        greenGamma[i] = MIN(255, (int)((255.0 * pow((float)i / 255.0, 1.0 / GreenGamma)) + 0.5));
        blueGamma[i] = MIN(255, (int)((255.0 * pow((float)i / 255.0, 1.0 / BlueGamma)) + 0.5));
    }

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            Img_dst->rgbpix[(i)* 3 * Img_src->Width + 3 * j] = redGamma[Img_src->rgbpix[(i)* 3 * Img_src->Width + 3 * j]];// *Img_src->rgbpix[(i)* 3 * Img_src->Width + 3 * j];
            Img_dst->rgbpix[(i)* 3 * Img_src->Width + 3 * j + 1] = greenGamma[Img_src->rgbpix[(i)* 3 * Img_src->Width + 3 * j + 1]];
            Img_dst->rgbpix[(i)* 3 * Img_src->Width + 3 * j + 2] = blueGamma[Img_src->rgbpix[(i)* 3 * Img_src->Width + 3 * j + 2]];
        }
    }
    return *Img_dst;
}

/*
    A F F I N E  -  Transformation: Rotation
*/
struct Image RotateImage(struct Image *Img_src, struct Image *Img_dst, float RotationAngle, struct point_xy CentralPoint)
{
    float Sx, Matrix[2][2], Cos, Sin;
    int i, j, z ;
    int x_new = 0, y_new = 0;
    int currentPixel_smallImage = 0;

    Sx = (-RotationAngle * 3.14) / 180;
    Cos = cos(Sx);
    Sin = sin(Sx);
    Matrix[0][0] = Cos;
    Matrix[0][1] = -Sin;
    Matrix[1][0] = Sin;
    Matrix[1][1] = Cos;

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            // za vsqko i,j (ot staoroto izobrajenie se namirat novi x_new  i  y_new na koito s epipisvat stoinostite za RGB
            x_new = Matrix[0][0] * (j - CentralPoint.X) + Matrix[0][1] * (i - CentralPoint.Y) + CentralPoint.X;
            y_new = Matrix[1][0] * (j - CentralPoint.X) + Matrix[1][1] * (i - CentralPoint.Y) + CentralPoint.Y;
            if (x_new > Img_dst->Width) x_new = Img_dst->Width;
            if (x_new < 0) x_new = 0;
            if (y_new > Img_dst->Height) y_new = Img_dst->Height;
            if (y_new < 0) y_new = 0;

            for(z = 0; z < Img_dst->Num_channels; z++)
            {
                Img_dst->rgbpix[y_new * Img_dst->Num_channels * Img_dst->Width + Img_dst->Num_channels * x_new + z] = Img_src->rgbpix[i * Img_dst->Num_channels * Img_dst->Width + Img_dst->Num_channels * j + z];
            }
        }
    }

    return *Img_dst;
}

/*
    A F F I N E  - scale (zoom) image - in/out
*/
struct Image ScaleImage(struct Image *Img_src, struct Image *Img_dst, float ScalePercentage)
{
    int i, j, z;
    int NewX = 0;
    int NewY = 0;

    /* Modify Img_dst */
    Img_dst->Width = Img_src->Width * (1 + (ScalePercentage / 100.0));
    Img_dst->Height = Img_src->Height * (1 + (ScalePercentage / 100.0));
    Img_dst->rgbpix = (unsigned char *)realloc(Img_dst->rgbpix, Img_dst->Height * Img_dst->Width * Img_dst->Num_channels* sizeof(unsigned char));

    if (ScalePercentage < 0)
    {
        for (i = 0; i < Img_dst->Height; i++)
        {
            for (j = 0; j < Img_dst->Width; j++)
            {
                NewX = j / (1 + (ScalePercentage / 100.0)); // Plus Sign because of the negative Scale Value
                NewY = i / (1 + (ScalePercentage / 100.0));

                if (NewX < 0) NewX = 0;
                if (NewY < 0) NewY = 0;
                if (NewX >= Img_src->Width) NewX = Img_src->Width - 1;
                if (NewY >= Img_src->Height) NewY = Img_src->Height - 1;

                for(z = 0; z < Img_dst->Num_channels; z++)
                {
                    Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + z ] = Img_src->rgbpix[Img_dst->Num_channels * ((NewY)*Img_src->Width + (NewX)) + z ];
                }
            }
        }
    }
    else if (ScalePercentage > 0)
    {
        for (i = 0; i < Img_dst->Height; i++)
        {
            for (j = 0; j < Img_dst->Width; j++)
            {
                NewX = j / (1 + (ScalePercentage / 100));
                NewY = i / (1 + (ScalePercentage / 100));

                if (NewX < 0) NewX = 0;
                if (NewY < 0) NewY = 0;
                if (NewX >= Img_src->Width) NewX = Img_src->Width - 1;
                if (NewY >= Img_src->Height) NewY = Img_src->Height - 1;

                for(z = 0; z < Img_dst->Num_channels; z++)
                {
                    Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + z ] = Img_src->rgbpix[Img_dst->Num_channels * ((NewY)*Img_src->Width + (NewX)) + z ];
                }
            }
        }
    }
    else return *Img_src;

    return *Img_dst;
}

/*
    A F F I N E  - scale (zoom) image - in/out
*/
struct Image ScaleImageToXY(struct Image *Img_src, struct Image *Img_dst, int NewWidth, int NewHeight)
{
    int i, j, z;
    int NewX = 0;
    int NewY = 0;

    float ScalePercentageX;
    float ScalePercentageY;
    /* Modify Img_dst */
    Img_dst->Width = NewWidth;//Img_src->Width * (1 + (ScalePercentage / 100.0));
    Img_dst->Height = NewHeight;//Img_src->Height * (1 + (ScalePercentage / 100.0));
    Img_dst->rgbpix = (unsigned char *)realloc(Img_dst->rgbpix, Img_dst->Height * Img_dst->Width * Img_dst->Num_channels* sizeof(unsigned char));

    ScalePercentageX = 100 - Img_src->Width * 100 / NewWidth;//Img_src->Width / (float) Img_dst->Width;
    ScalePercentageY = 100 - Img_src->Height * 100 / NewHeight;

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            //if(ScalePercentageX< 0)
                NewX = j * (1 - (ScalePercentageX / 100.0));
            //else NewX = j / (1 + (ScalePercentageX / 100.0));

            //if(ScalePercentageY< 0)
                NewY = i *(1 - (ScalePercentageY / 100.0));
            //else NewY = i / (1 + (ScalePercentageY / 100.0));

            if (NewX < 0) NewX = 0;
            if (NewY < 0) NewY = 0;
            if (NewX >= Img_src->Width) NewX = Img_src->Width - 1;
            if (NewY >= Img_src->Height) NewY = Img_src->Height - 1;

            for(z = 0; z < Img_dst->Num_channels; z++)
            {
                Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + z ] = Img_src->rgbpix[Img_dst->Num_channels * ((NewY)*Img_src->Width + (NewX)) + z ];
            }
        }
    }


    return *Img_dst;
}

/*
    A F F I N E  - Translation
*/
struct Image TranslateImage(struct Image *Img_src, struct Image *Img_dst, struct point_xy ToPoint)
{
    int i, j, z;
    int NewX = 0, NewY = 0;

    int ShiftX = ToPoint.X - Img_dst->Width / 2;
    int ShiftY = ToPoint.Y - Img_dst->Height / 2;


    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            NewX = j + ShiftX;
            NewY = i + ShiftY;
            if (ShiftX < 0 ) if(NewX < 0) continue;
            if (ShiftX > 0)if (NewX >= Img_dst->Width) continue;
            if (ShiftY < 0) if(NewY < 0) continue;
            if (ShiftY > 0)if (NewY >= Img_dst->Height) continue;

            for(z = 0; z < Img_dst->Num_channels; z++)
            {
                Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + z] = Img_src->rgbpix[Img_dst->Num_channels * ((NewY)*Img_src->Width + (NewX)) + z];
            }
        }
    }
    return *Img_dst;
}

/*
    E D G E   Extraction
*/
struct ArrPoints EdgeExtraction(struct Image *Img_src, struct Image *Img_dst, int Algotype, float Algo_param1, float Algo_param2)
{
    int i, j, z, l;
    int NewX = 0, NewY = 0;
    struct ArrPoints ArrPts;
    struct Image DerrivativeX = CreateNewImage(&DerrivativeX, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE, 8);
    struct Image DerrivativeY = CreateNewImage(&DerrivativeY, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE, 8);
    struct Image Magnitude = CreateNewImage(&Magnitude, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE, 8);
    struct Image Magnitude2 = CreateNewImage(&Magnitude2, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE, 8);
    struct Image Magnitude3 = CreateNewImage(&Magnitude3, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE, 8);
    struct Image NMS = CreateNewImage(&NMS, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE, 8);
    struct Image Hysteresis = CreateNewImage(&Hysteresis, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE, 8);

    float Gx[] =
      { -1, 0, 1,
        -2, 0, 2,
        -1, 0, 1 };
    float Gy[] =
      { 1, 2, 1,
        0, 0, 0,
       -1, -2, -1 };

    float HighPass[] =
    { 1 / 9 * (
        -1, -1, -1,
        -1,  8, -1,
        -1, -1, -1) };

    float Laplace[] =
     {  0,  1,  0,
        1, -4,  1,
        0,  1,  0 };

    float Prewitt_X_1[] =
     { -5, -5, -5,
        0,  0,  0,
        5,  5,  5 };
    float Prewitt_Y_1[] =
     { -5,  0,  5,
       -5,  0,  5,
       -5,  0,  5 };

    float Prewitt_X_2[] =
     { 5,  5,  5,
       0,  0,  0,
      -5, -5, -5  };
    float Prewitt_Y_2[] =
     { 5,  0, -5,
       5,  0, -5,
       5,  0, -5 };

    float Sobel_X_1[] =
    { -1, -2, -1,
       0,  0,  0,
       1,  2,  1 };
    float Sobel_Y_1[] =
    { -1,  0,  1,
      -2,  0,  2,
      -1,  0,  1 };

    float Sobel_X_2[] =
    {  1,  2,  1,
       0,  0,  0,
      -1, -2, -1 };
    float Sobel_Y_2[] =
    {  1,  0, -1,
       2,  0, -2,
       1,  0, -1 };

    Img_dst->Width = Img_src->Width;
    Img_dst->Height = Img_src->Height;
    Img_dst->rgbpix = (unsigned char *)realloc(Img_dst->rgbpix, Img_dst->Num_channels * Img_dst->Width * Img_dst->Height * sizeof(unsigned char));
    ArrPts.ArrayOfPoints = (struct point_xy *)calloc(50,sizeof(struct point_xy));



    if (Algotype < 1 || Algotype > 3)
    {
        printf("Non existing Algo for Edge extraction\n");
        return ArrPts;
    }
    /* Canny */
    if (Algotype == 1)
    {
        // Step 1: Perfrom Gaussian Blur
        BlurImageGussian(Img_src, Img_dst, (0.5 * Img_src->Width) / 100, 0.6);
        FindDerrivative_XY(Img_dst, &DerrivativeX, &DerrivativeY);
        FindMagnitudeOfGradient(&DerrivativeX, &DerrivativeY, &Magnitude);
        FindNonMaximumSupp(&Magnitude, &DerrivativeX, &DerrivativeY, &NMS);

        FindHysteresis(&Magnitude, &NMS, &Hysteresis, Algo_param1, Algo_param2);
        memcpy(Img_dst->rgbpix, Hysteresis.rgbpix, Hysteresis.Width* Hysteresis.Height * sizeof(unsigned char));
    }
    /* Sobel */
    else if (Algotype == 2)
    {
        BlurImageGussian(Img_src, Img_dst, (0.5 * Img_src->Width) / 100, 0.6);
        Convolution(Img_dst->rgbpix, DerrivativeX.rgbpix, DerrivativeX.Height, DerrivativeX.Width, Sobel_X_1, 3);
        Convolution(Img_dst->rgbpix, DerrivativeY.rgbpix, DerrivativeY.Height, DerrivativeY.Width, Sobel_Y_1, 3);
        FindMagnitudeOfGradient(&DerrivativeX, &DerrivativeY, &Magnitude);

        Convolution(Img_dst->rgbpix, DerrivativeX.rgbpix, DerrivativeX.Height, DerrivativeX.Width, Sobel_X_2, 3);
        Convolution(Img_dst->rgbpix, DerrivativeY.rgbpix, DerrivativeY.Height, DerrivativeY.Width, Sobel_Y_2, 3);
        if (Algo_param1 == 0)
        {
            FindMagnitudeOfGradient(&DerrivativeX, &DerrivativeY, &Magnitude2);
            memcpy(Img_dst->rgbpix, Magnitude2.rgbpix, Magnitude2.Width* Magnitude2.Height * sizeof(unsigned char));
        }
        else
        {
            memcpy(Img_dst->rgbpix, DerrivativeX.rgbpix, DerrivativeX.Width* DerrivativeX.Height * sizeof(unsigned char));
        }
    }
    /* Prewitt */
    else if (Algotype == 3)
    {
        BlurImageGussian(Img_src, Img_dst, (0.5 * Img_src->Width) / 100, 0.6);
        Convolution(Img_dst->rgbpix, DerrivativeX.rgbpix, DerrivativeX.Height, DerrivativeX.Width, Prewitt_X_1, 3);
        Convolution(Img_dst->rgbpix, DerrivativeY.rgbpix, DerrivativeY.Height, DerrivativeY.Width, Prewitt_Y_1, 3);
        FindMagnitudeOfGradient(&DerrivativeX, &DerrivativeY, &Magnitude);

        Convolution(Img_dst->rgbpix, DerrivativeX.rgbpix, DerrivativeX.Height, DerrivativeX.Width, Prewitt_X_2, 3);
        Convolution(Img_dst->rgbpix, DerrivativeY.rgbpix, DerrivativeY.Height, DerrivativeY.Width, Prewitt_Y_2, 3);
        FindMagnitudeOfGradient(&DerrivativeX, &DerrivativeY, &Magnitude2);
        FindMagnitudeOfGradient(&Magnitude, &Magnitude2, &Magnitude3);
        if (Algo_param1 == 0)
        {
            FindNonMaximumSupp(&Magnitude3, &Magnitude, &Magnitude2, &NMS);
            FindHysteresis(&Magnitude3, &NMS, &Hysteresis, Algo_param1, Algo_param2);

            memcpy(Img_dst->rgbpix, Hysteresis.rgbpix, Hysteresis.Width* Hysteresis.Height * sizeof(unsigned char));
        }
        else
            memcpy(Img_dst->rgbpix, Magnitude3.rgbpix, Magnitude3.Width* Magnitude3.Height * sizeof(unsigned char));
    }

    //DestroyImage(&Hysteresis);
    DestroyImage(&DerrivativeX);
    DestroyImage(&DerrivativeY);
    DestroyImage(&Magnitude);
    DestroyImage(&Magnitude2);
    DestroyImage(&Magnitude3);
    DestroyImage(&NMS);

    return ArrPts;
}

/*
    D E R R I V A T I V E    calculation
*/
void FindDerrivative_XY(struct Image *Img_src, struct Image *DerrivativeX_image, struct Image *DerrivativeY_image)
{
    int r, c, pos;
    int rows = Img_src->Height;
    int cols = Img_src->Width;

    /* Calculate X - derrivative image */
    for (r = 0; r < rows-1; r++)
    {
        pos = r * cols;
        //DerrivativeX_image->rgbpix[pos] = Img_src->rgbpix[pos + 1] - Img_src->rgbpix[pos];
        //pos++;
        for (c = 0; c < (cols - 1); c++, pos++)
        {
            DerrivativeX_image->rgbpix[pos] = abs((Img_src->rgbpix[pos] - Img_src->rgbpix[pos + cols + 1]));
        }
        DerrivativeX_image->rgbpix[pos] = abs(Img_src->rgbpix[pos] - Img_src->rgbpix[pos - 1]);
    }


    /* Calculate Y - derrivative image */
    for (c = 0; c < cols-1; c++)
    {
        pos = c;
        //DerrivativeY_image->rgbpix[pos] = Img_src->rgbpix[pos + cols] - Img_src->rgbpix[pos];
        //pos += cols;
        for (r = 0; r < (rows - 1); r++, pos += cols)
        {
            DerrivativeY_image->rgbpix[pos] = abs(Img_src->rgbpix[pos + 1] - Img_src->rgbpix[pos + cols]);
        }
        DerrivativeY_image->rgbpix[pos] = abs(Img_src->rgbpix[pos] - Img_src->rgbpix[pos - cols]);
    }
}

/*
    Find   M A G N I T U D E  of Gradient
*/
void FindMagnitudeOfGradient(struct Image *DerrivativeX_image, struct Image *DerrivativeY_image, struct Image *Magnitude)
{
    int r, c, pos, sq1, sq2;
    int rows = DerrivativeX_image->Height;
    int cols = DerrivativeX_image->Width;

    for (r = 0, pos = 0; r < rows; r++)
    {
        for (c = 0; c < cols; c++, pos++)
        {
            sq1 = DerrivativeX_image->rgbpix[pos] * DerrivativeX_image->rgbpix[pos];
            sq2 = DerrivativeY_image->rgbpix[pos] * DerrivativeY_image->rgbpix[pos];
            Magnitude->rgbpix[pos] = (int)(0.5 + sqrt((float)sq1 + (float)sq2));
        }
    }
}

/*
    Find  N O N - M A X - S U P P
*/
void FindNonMaximumSupp(struct Image *Magnitude, struct Image *DerrivativeX, struct Image *DerrivativeY, struct Image *NMS)
{
    int rowcount, colcount, count;
    unsigned char *magrowptr, *magptr;
    unsigned char *gxrowptr, *gxptr;
    unsigned char *gyrowptr, *gyptr, z1, z2;
    unsigned char m00, gx = 0, gy = 0;
    float mag1 = 0, mag2 = 0, xperp = 0, yperp = 0;
    unsigned char *resultrowptr, *resultptr;
    int nrows = DerrivativeX->Height;
    int ncols = DerrivativeX->Width;

    unsigned char *result = NMS->rgbpix;
    unsigned char *mag = Magnitude->rgbpix;
    unsigned char *gradx = DerrivativeX->rgbpix;
    unsigned char *grady = DerrivativeY->rgbpix;


    /****************************************************************************
    * Zero the edges of the result image.
    ****************************************************************************/
    for (count = 0, resultrowptr = NMS->rgbpix, resultptr = NMS->rgbpix + ncols*(nrows - 1);
        count<ncols; resultptr++, resultrowptr++, count++){
        *resultrowptr = *resultptr = (unsigned char)0;
    }

    for (count = 0, resultptr = NMS->rgbpix, resultrowptr = NMS->rgbpix + ncols - 1;
        count<nrows; count++, resultptr += ncols, resultrowptr += ncols){
        *resultptr = *resultrowptr = (unsigned char)0;
    }

    /****************************************************************************
    * Suppress non-maximum points.
    ****************************************************************************/
    for (rowcount = 1, magrowptr = mag + ncols + 1, gxrowptr = gradx + ncols + 1,
        gyrowptr = grady + ncols + 1, resultrowptr = result + ncols + 1;
        rowcount<nrows - 2; rowcount++, magrowptr += ncols, gyrowptr += ncols, gxrowptr += ncols,
        resultrowptr += ncols)
    {
        for (colcount = 1, magptr = magrowptr, gxptr = gxrowptr, gyptr = gyrowptr,
            resultptr = resultrowptr; colcount<ncols - 2;
            colcount++, magptr++, gxptr++, gyptr++, resultptr++)
        {
            m00 = *magptr;
            if (m00 == 0){
                *resultptr = (unsigned char)NOEDGE;
            }
            else{
                xperp = -(gx = *gxptr) / ((float)m00);
                yperp = (gy = *gyptr) / ((float)m00);
            }

            if (gx >= 0){
                if (gy >= 0){
                    if (gx >= gy)
                    {
                        /* 111 */
                        /* Left point */
                        z1 = *(magptr - 1);
                        z2 = *(magptr - ncols - 1);

                        mag1 = (m00 - z1)*xperp + (z2 - z1)*yperp;

                        /* Right point */
                        z1 = *(magptr + 1);
                        z2 = *(magptr + ncols + 1);

                        mag2 = (m00 - z1)*xperp + (z2 - z1)*yperp;
                    }
                    else
                    {
                        /* 110 */
                        /* Left point */
                        z1 = *(magptr - ncols);
                        z2 = *(magptr - ncols - 1);

                        mag1 = (z1 - z2)*xperp + (z1 - m00)*yperp;

                        /* Right point */
                        z1 = *(magptr + ncols);
                        z2 = *(magptr + ncols + 1);

                        mag2 = (z1 - z2)*xperp + (z1 - m00)*yperp;
                    }
                }
                else
                {
                    if (gx >= -gy)
                    {
                        /* 101 */
                        /* Left point */
                        z1 = *(magptr - 1);
                        z2 = *(magptr + ncols - 1);

                        mag1 = (m00 - z1)*xperp + (z1 - z2)*yperp;

                        /* Right point */
                        z1 = *(magptr + 1);
                        z2 = *(magptr - ncols + 1);

                        mag2 = (m00 - z1)*xperp + (z1 - z2)*yperp;
                    }
                    else
                    {
                        /* 100 */
                        /* Left point */
                        z1 = *(magptr + ncols);
                        z2 = *(magptr + ncols - 1);

                        mag1 = (z1 - z2)*xperp + (m00 - z1)*yperp;

                        /* Right point */
                        z1 = *(magptr - ncols);
                        z2 = *(magptr - ncols + 1);

                        mag2 = (z1 - z2)*xperp + (m00 - z1)*yperp;
                    }
                }
            }
            else
            {
                if ((gy = *gyptr) >= 0)
                {
                    if (-gx >= gy)
                    {
                        /* 011 */
                        /* Left point */
                        z1 = *(magptr + 1);
                        z2 = *(magptr - ncols + 1);

                        mag1 = (z1 - m00)*xperp + (z2 - z1)*yperp;

                        /* Right point */
                        z1 = *(magptr - 1);
                        z2 = *(magptr + ncols - 1);

                        mag2 = (z1 - m00)*xperp + (z2 - z1)*yperp;
                    }
                    else
                    {
                        /* 010 */
                        /* Left point */
                        z1 = *(magptr - ncols);
                        z2 = *(magptr - ncols + 1);

                        mag1 = (z2 - z1)*xperp + (z1 - m00)*yperp;

                        /* Right point */
                        z1 = *(magptr + ncols);
                        z2 = *(magptr + ncols - 1);

                        mag2 = (z2 - z1)*xperp + (z1 - m00)*yperp;
                    }
                }
                else
                {
                    if (-gx > -gy)
                    {
                        /* 001 */
                        /* Left point */
                        z1 = *(magptr + 1);
                        z2 = *(magptr + ncols + 1);

                        mag1 = (z1 - m00)*xperp + (z1 - z2)*yperp;

                        /* Right point */
                        z1 = *(magptr - 1);
                        z2 = *(magptr - ncols - 1);

                        mag2 = (z1 - m00)*xperp + (z1 - z2)*yperp;
                    }
                    else
                    {
                        /* 000 */
                        /* Left point */
                        z1 = *(magptr + ncols);
                        z2 = *(magptr + ncols + 1);

                        mag1 = (z2 - z1)*xperp + (m00 - z1)*yperp;

                        /* Right point */
                        z1 = *(magptr - ncols);
                        z2 = *(magptr - ncols - 1);

                        mag2 = (z2 - z1)*xperp + (m00 - z1)*yperp;
                    }
                }
            }

            /* Now determine if the current point is a maximum point */

            if ((mag1 > 0.0) || (mag2 > 0.0))
            {
                *resultptr = (unsigned char)NOEDGE;
            }
            else
            {
                if (mag2 == 0.0)
                    *resultptr = (unsigned char)NOEDGE;
                else
                    *resultptr = (unsigned char)POSSIBLE_EDGE;
            }
        }
    }
}

/*
    Find     H Y S T E R E S I S
*/
void FindHysteresis(struct Image *Magnitude, struct Image *NMS, struct Image *Img_dst, float Algo_param1, float Algo_param2)
{
    int r, c, pos, edges, highcount, lowthreshold, highthreshold,
        i, hist[32768], rr, cc;
    unsigned char maximum_mag, sumpix;

    int rows = Img_dst->Height;
    int cols = Img_dst->Width;

    /****************************************************************************
    * Initialize the Img_dst->rgbpix map to possible Img_dst->rgbpixs everywhere the non-maximal
    * suppression suggested there could be an Img_dst->rgbpix except for the border. At
    * the border we say there can not be an Img_dst->rgbpix because it makes the
    * follow_Img_dst->rgbpixs algorithm more efficient to not worry about tracking an
    * Img_dst->rgbpix off the side of the image.
    ****************************************************************************/
    for (r = 0, pos = 0; r<rows; r++){
        for (c = 0; c<cols; c++, pos++){
            if (NMS->rgbpix[pos] == POSSIBLE_EDGE) Img_dst->rgbpix[pos] = POSSIBLE_EDGE;
            else Img_dst->rgbpix[pos] = NOEDGE;
        }
    }

    for (r = 0, pos = 0; r<rows; r++, pos += cols){
        Img_dst->rgbpix[pos] = NOEDGE;
        Img_dst->rgbpix[pos + cols - 1] = NOEDGE;
    }
    pos = (rows - 1) * cols;
    for (c = 0; c<cols; c++, pos++){
        Img_dst->rgbpix[c] = NOEDGE;
        Img_dst->rgbpix[pos] = NOEDGE;
    }

    /****************************************************************************
    * Compute the histogram of the magnitude image. Then use the histogram to
    * compute hysteresis thresholds.
    ****************************************************************************/
    for (r = 0; r<32768; r++) hist[r] = 0;
    for (r = 0, pos = 0; r<rows; r++)
    {
        for (c = 0; c<cols; c++, pos++)
        {
            if (Img_dst->rgbpix[pos] == POSSIBLE_EDGE) hist[Magnitude->rgbpix[pos]]++;
        }
    }

    /****************************************************************************
    * Compute the number of pixels that passed the nonmaximal suppression.
    ****************************************************************************/
    for (r = 1, edges = 0; r<32768; r++)
    {
        if (hist[r] != 0) maximum_mag = r;
        edges += hist[r];
    }

    highcount = (int)(edges * Algo_param2 + 0.5);

    /****************************************************************************
    * Compute the high threshold value as the (100 * Algo_param2) percentage point
    * in the magnitude of the gradient histogram of all the pixels that passes
    * non-maximal suppression. Then calculate the low threshold as a fraction
    * of the computed high threshold value. John Canny said in his paper
    * "A Computational Approach to Img_dst->rgbpix Detection" that "The ratio of the
    * high to low threshold in the implementation is in the range two or three
    * to one." That means that in terms of this implementation, we should
    * choose Algo_param1 ~= 0.5 or 0.33333.
    ****************************************************************************/
    r = 1;
    edges = hist[1];
    while ((r<(maximum_mag - 1)) && (edges < highcount))
    {
        r++;
        edges += hist[r];
    }
    highthreshold = r;
    lowthreshold = (int)(highthreshold * Algo_param1 + 0.5);


    /****************************************************************************
    * This loop looks for pixels above the highthreshold to locate Img_dst->rgbpixs and
    * then calls follow_Img_dst->rgbpixs to continue the Img_dst->rgbpix.
    ****************************************************************************/
    for (r = 0, pos = 0; r<rows; r++)
    {
        for (c = 0; c<cols; c++, pos++)
        {
            if ((Img_dst->rgbpix[pos] == POSSIBLE_EDGE) && (Magnitude->rgbpix[pos] >= highthreshold)){
                Img_dst->rgbpix[pos] = EDGE;
                Follow_edges((Img_dst->rgbpix + pos), (Magnitude->rgbpix + pos), lowthreshold, cols);
            }
        }
    }

    /****************************************************************************
    * Set all the remaining possible Img_dst->rgbpixs to non-Img_dst->rgbpixs.
    ****************************************************************************/
    for (r = 0, pos = 0; r<rows; r++)
    {
        for (c = 0; c<cols; c++, pos++) if (Img_dst->rgbpix[pos] != EDGE) Img_dst->rgbpix[pos] = NOEDGE;
    }
}

/*
    Follow  E D G E S
*/
void Follow_edges(unsigned char *edgemapptr, unsigned char *edgemagptr, unsigned char lowval, int cols)
{
    unsigned char *tempmagptr;
    unsigned char *tempmapptr;
    int i;
    float thethresh;
    int x[8] = { 1, 1, 0, -1, -1, -1, 0, 1 },
        y[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };

    for (i = 0; i<8; i++){
        tempmapptr = edgemapptr - y[i] * cols + x[i];
        tempmagptr = edgemagptr - y[i] * cols + x[i];

        if ((*tempmapptr == POSSIBLE_EDGE) && (*tempmagptr > lowval)){
            *tempmapptr = (unsigned char)EDGE;
            Follow_edges(tempmapptr, tempmagptr, lowval, cols);
        }
    }
}

/*
    S A T U R A T I O N
*/
struct Image Saturation(struct Image *Img_src, struct Image *Img_dst, float percentage)
{
    FILE * fdebug = NULL;
    int i, j;
    struct Image WorkCopy = CreateNewImage(&WorkCopy, Img_src->Width, Img_src->Height, 3, Img_src->ColorSpace, 8);

    /* If the input is RGB -> the output is also RGB. if the input is HSL -> the output is also HSL */
    if (Img_src->ColorSpace != 2 && Img_src->ColorSpace != 5)
    {
        #ifdef DEBUG_FILE
                fdebug = fopen(DEBUG_FILE, "wt");
                fprintf(fdebug, "The input image is not in HSL format\n");
                fclose(fdebug);
        #endif // DEBUG_FILE

        return *Img_dst;
    }

    if ((Img_src->Width * Img_src->Height != Img_dst->Width * Img_dst->Height) || (Img_src->ColorSpace != Img_dst->ColorSpace))
    {
        SetDestination(Img_src, Img_dst);
    }

    /* We have to work in HSL color space */
    if (Img_src->ColorSpace == 5)  // if the input image is HSL
    {
        memcpy(WorkCopy.rgbpix, Img_src->rgbpix, 3 * Img_src->Width * Img_src->Height * sizeof(unsigned char));
    }
    else // if the input image is RGB
    {
        ConvertImage_RGB_to_HSL(Img_src, &WorkCopy);
    }

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            // We work over WorkCopy
            if (WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] * percentage / (float)100 + WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] > 100)
                WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] = 100;
            else if (WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] * percentage / (float)100 + WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] < 0)
                WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] = 0;
            else
                WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] = RoundValue_toX_SignificantBits((WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] * percentage / (float)100) + WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1], 2);
        }
    }

    /* We have to return Image with the same color space as the input */
    if (Img_src->ColorSpace == 5)  // if the input image is HSL
    {
        memcpy(Img_dst->rgbpix, WorkCopy.rgbpix, 3 * WorkCopy.Width * WorkCopy.Height * sizeof(unsigned char));
        Img_dst->ColorSpace = WorkCopy.ColorSpace;
        //DestroyImage(&WorkCopy);
        return WorkCopy;
    }
    else // if the input image is RGB
    {
        ConvertImage_HSL_to_RGB(&WorkCopy, Img_dst);
        DestroyImage(&WorkCopy);
        return *Img_dst;
    }
}

/*
    B L E N D I N G  - similar to image sharpening - but the contours are from another image
*/
struct Image BlendImage(struct Image *Img_src, struct Image *Img_BlendedSrc, struct Image *Img_dst, float Percentage, int AlgoParam1, int Algoparam2, int BlacOrWhiteThreshold)
{
    int i, j, l;

    Image Img_dst_Grayscale = CreateNewImage(&Img_dst_Grayscale, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE, 8);
    Image Img_src_Grayscale = CreateNewImage(&Img_src_Grayscale, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE, 8);

    if (fabs(Percentage) > 1) Percentage /= 100;
    //Percentage *= -1;
    if ((Img_src->Width != Img_dst->Width) || (Img_src->Height != Img_dst->Height)) SetDestination(Img_src, Img_dst);

    if (AlgoParam1 == BLEND_EXTRACT_EDGES)
    {
        ConvertToGrayscale_1Channel(Img_BlendedSrc, &Img_src_Grayscale);
        EdgeExtraction(&Img_src_Grayscale, &Img_dst_Grayscale, EDGES_PREWITT, 1, 0.9);
    }
    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            for (l = 0; l < Img_dst->Num_channels; l++)
            {
                if (AlgoParam1 == BLEND_EXTRACT_EDGES)
                {
                    if ((Algoparam2 == BLEND_REMOVE_BLACK && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 0] <= BlacOrWhiteThreshold) && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 1] <= BlacOrWhiteThreshold) && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 2] <= BlacOrWhiteThreshold))
                        ||
                        (Algoparam2 == BLEND_REMOVE_WHITE && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 0] >= 255 - BlacOrWhiteThreshold) && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 1] >= 255 - BlacOrWhiteThreshold) && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 2] >= 255 - BlacOrWhiteThreshold)))
                        goto Same;

                    if (Img_dst_Grayscale.rgbpix[(i*Img_src->Width + j)] >= 15)
                    {
                        if ((Percentage)* Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + l] + (1 - Percentage) * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] > 255)
                        {
                            Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 255;
                        }
                        else if ((Percentage)* Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + l] + (1 - Percentage)* Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] < 0)
                        {
                            Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 0;
                        }
                        else
                        {
                            Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = (Percentage)* Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + l] + (1 - Percentage) * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
                        }
                    }
                    else
                    {
Same:					Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
                    }
                }
                else
                {
                    if (!((Algoparam2 == BLEND_REMOVE_BLACK && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 0] <= BlacOrWhiteThreshold) && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 1] <= BlacOrWhiteThreshold) && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 2] <= BlacOrWhiteThreshold))
                        ||
                        (Algoparam2 == BLEND_REMOVE_WHITE && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 0] >= 255 - BlacOrWhiteThreshold) && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 1] >= 255 - BlacOrWhiteThreshold) && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 2] >= 255 - BlacOrWhiteThreshold))))
                    {
                        if ((Percentage)* Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + l] + (1 - Percentage) * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] > 255)
                        {
                            Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 255;
                        }
                        else if ((Percentage)* Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + l] + (1 - Percentage)* Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] < 0)
                        {
                            Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 0;
                        }
                        else
                        {
                            Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = (Percentage)* Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + l] + (1 - Percentage) * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
                        }
                    }
                    else
                        Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
                }
            }
        }
    }

    DestroyImage(&Img_dst_Grayscale);
    DestroyImage(&Img_src_Grayscale);

    return Img_dst_Grayscale;
}

/*
    I N V E R S E  images
*/
struct Image InverseImage0to255(struct Image *Img_src, struct Image *Img_dst)
{
    int i, j, z;

    for (i = 0; i < Img_src->Height; i++)
    {
        for (j = 0; j < Img_src->Width; j++)
        {
            for (z = 0; z < Img_src->Num_channels; z++)
            {
                Img_dst->rgbpix[Img_src->Num_channels * (i * Img_src->Width + j) + z] = 255 - Img_src->rgbpix[Img_src->Num_channels * (i * Img_src->Width + j) + z];
            }
        }
    }
return *Img_dst;
}

/********************************************************************************
*																				*
*	CV Library - SpaceConversions.c												*
*																				*
*	Author:  Petar Nikolov														*
*																				*
*																				*
*	The algorithms included in this file are:									*
*																				*
*	- Convert to Grayscale	1 / 3 channels										*
*	- Convert to Binary															*
*	- RGB_to_HSL																*
*	- HSL_to_RGB																*
*	- RGB_to_XYZ																*
*	- XYZ_to_RGB																*
*	- RGB_to_LAB																*
*	- LAB_to_RGB																*
*	- Color Temperature															*
*																				*
*********************************************************************************/


/*
    convert to    G R A Y S C A L E  - 3 channels RGB
*/
struct Image ConvertToGrayscale_3Channels(struct Image *Img_src, struct Image *Img_dst)
{
    int i, j;

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = 0.3 * Img_src->rgbpix[3 * (i*Img_src->Width + j)] + 0.59 * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1] + 0.11 * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
            Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 1] = Img_dst->rgbpix[3 * (i*Img_src->Width + j)];
            Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = Img_dst->rgbpix[3 * (i*Img_src->Width + j)];
        }
    }

    return *Img_dst;
}

/*
    convert to    G R A Y S C A L E  - 1 channel
*/
struct Image ConvertToGrayscale_1Channel(struct Image *Img_src, struct Image *Img_dst)
{
    int i, j;

    if (Img_dst->isLoaded != 1) return *Img_dst;

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            Img_dst->rgbpix[(i*Img_src->Width + j)] = 0.3 * Img_src->rgbpix[3 * (i*Img_src->Width + j)] + 0.59 * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1] + 0.11 * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
        }
    }
}


/*
    B I N A R Y  image
*/
struct Image ConvertToBinary(struct Image *Img_src, struct Image *Img_dst, int Threshold)
{
    int i, j, l;
    int Sum = 0;
    int AverageGray = 0;

    struct Image GrayImage = CreateNewImage(&GrayImage, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE, 8);

    if (Img_src->Width * Img_src->Height != Img_dst->Width * Img_dst->Height)
    {
        SetDestination(Img_src, Img_dst);
    }
    if (Img_src->Num_channels != 1)
    {
        ConvertToGrayscale_1Channel(Img_src, &GrayImage);
    }
    if (Threshold == 0)
    {
        for (i = 0; i < Img_dst->Height; i++)
        {
            for (j = 0; j < Img_dst->Width; j++)
            {
                if (Img_src->Num_channels != 1)
                    Sum += GrayImage.rgbpix[i * Img_dst->Width + j];
                else
                    Sum += Img_src->rgbpix[i * Img_dst->Width + j];
            }
        }
        AverageGray = Sum / (float)(Img_dst->Width * Img_dst->Height);
    }
    else
        AverageGray = Threshold;
    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            if (Img_src->Num_channels == 1)
            {
                if (Img_src->rgbpix[i * Img_src->Width + j] > AverageGray)
                    Img_dst->rgbpix[i * Img_src->Width + j] = 255;
                else
                    Img_dst->rgbpix[i * Img_src->Width + j] = 0;
            }
            else
            {
                if (GrayImage.rgbpix[i * Img_src->Width + j] > AverageGray)
                    Img_dst->rgbpix[i * Img_src->Width + j] = 255;
                else
                    Img_dst->rgbpix[i * Img_src->Width + j] = 0;
            }
        }
    }

    DestroyImage(&GrayImage);
    return *Img_dst;
}

/*
    Chech Color values - Used in HSL - RGB conversion
*/
float CheckColorValue(float TempColor, float Temporary_1, float Temporary_2)
{
    float NewColor;

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

/*
    C O N V E R T -  RGB to HSV
*/
void ConvertImage_RGB_to_HSL(struct Image *Img_src, struct Image *Img_dst)
{
    FILE *fdebug = NULL;
    float R_scaled;
    float G_scaled;
    float B_scaled;
    float C_max;
    float C_min;
    float Delta;
    float Hue;

    float del_R, del_B, del_G;
    float Saturation, Luma = 0;

    int i, j;

    if ((Img_src->Num_channels != 3) || (Img_dst->Num_channels != 3))
    {
        #ifdef DEBUG_FILE
                fdebug = fopen(DEBUG_FILE, "wt");
                fprintf(fdebug, "The input images are not with 3 channels\n");
                fclose(fdebug);
        #endif // DEBUG_FILE
        return;
    }

    if (Img_src->Width * Img_src->Height != Img_dst->Width * Img_dst->Height)
    {
        SetDestination(Img_src, Img_dst);
    }

    if (Img_src->ColorSpace != 2)
    {
        #ifdef DEBUG_FILE
                fdebug = fopen(DEBUG_FILE, "wt");
                fprintf(fdebug, "The input image is not in RGB format\n");
                fclose(fdebug);
        #endif // DEBUG_FILE
        return;
    }

    Img_dst->ColorSpace = 5;


    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            R_scaled = Img_src->rgbpix[3 * (i * Img_src->Width + j) + 0] / (float)255;
            G_scaled = Img_src->rgbpix[3 * (i * Img_src->Width + j) + 1] / (float)255;
            B_scaled = Img_src->rgbpix[3 * (i * Img_src->Width + j) + 2] / (float)255;
            C_max = MAX(R_scaled, MAX(G_scaled, B_scaled));
            C_min = MIN(R_scaled, MIN(G_scaled, B_scaled));
            Delta = C_max - C_min;

            // HUE
            if (C_max == R_scaled)
            {
                Hue = RoundValue_toX_SignificantBits((60 * (fmod(((G_scaled - B_scaled) / Delta), 6))), 2);
                if (Hue < 0) Hue = 360 + Hue;
                if (Hue > 360) Hue = fmod(Hue, 360);
            }
            else if (C_max == G_scaled)
            {
                Hue = RoundValue_toX_SignificantBits((60 * (((B_scaled - R_scaled) / Delta) + 2)),2);
                if (Hue < 0) Hue = 360 + Hue;
                if (Hue > 360) Hue = fmod(Hue, 360);
            }
            else if (C_max == B_scaled)
            {
                Hue = RoundValue_toX_SignificantBits((60 * (((R_scaled - G_scaled) / Delta) + 4)),2);
                if (Hue < 0) Hue = 360 + Hue;
                if (Hue > 360) Hue = fmod(Hue, 360);
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

            Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 0] = RoundValue_toX_SignificantBits((Hue / (float)360) * 100, 2);
            Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 1] = RoundValue_toX_SignificantBits(Saturation * 100, 2);
            Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 2] = Luma * 100;//round(Luma * 100);

        }
    }
}

/*
    C O N V E R T -  HSL to RGB
*/
void ConvertImage_HSL_to_RGB(struct Image *Img_src, struct Image *Img_dst)
{
    FILE *fdebug = NULL;
    float R_temp;
    float G_temp;
    float B_temp;
    float C, X, m;
    float Hue;
    float Temporary_1, Temporary_2;
    float Saturation, Luma = 0;

    int i, j;

    if ((Img_src->Num_channels != 3) || (Img_dst->Num_channels != 3))
    {
        #ifdef DEBUG_FILE
                fdebug = fopen(DEBUG_FILE, "wt");
                fprintf(fdebug, "The input images are not with 3 channels\n");
                fclose(fdebug);
        #endif // DEBUG_FILE
        return;
    }

    if (Img_src->Width * Img_src->Height != Img_dst->Width * Img_dst->Height)
    {
        SetDestination(Img_src, Img_dst);
    }

    if (Img_src->ColorSpace != 5)
    {
        #ifdef DEBUG_FILE
                fdebug = fopen(DEBUG_FILE, "wt");
                fprintf(fdebug, "The input image is not in HSL format\n");
                fclose(fdebug);
        #endif // DEBUG_FILE
        return;
    }

    Img_dst->ColorSpace = 2;


    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {

            Hue = Img_src->rgbpix[3 * (i * Img_src->Width + j) + 0] / (float)100;

            Saturation = Img_src->rgbpix[3 * (i * Img_src->Width + j) + 1] / (float)100;

            Luma = Img_src->rgbpix[3 * (i * Img_src->Width + j) + 2] / (float)100;

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

            Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 0] = RoundValue_toX_SignificantBits(R_temp,2);
            Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 1] = RoundValue_toX_SignificantBits(G_temp,2);
            Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 2] = RoundValue_toX_SignificantBits(B_temp,2);

        }
    }
}


/*
    C O N V E R T  - RGB to XYZ
*/
void Convert_RGB_to_XYZ(struct Image *Img_src, struct Image *Img_dst)
{
    int i, j;
    float X, Y, Z;
    float var_R, var_B, var_G;

    for (i = 0; i < Img_src->Height; i++)
    {
        for (j = 0; j < Img_src->Width; j++)
        {
            var_R = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 0];
            var_G = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 1];
            var_B = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 2];

            //if (i == 638 && j == 2372)
            //	_getch();
            var_R /= 255;
            var_G /= 255;
            var_B /= 255;

            if (var_R > 0.04045)
                var_R = pow(((var_R + 0.055) / 1.055), 2.4);
            else
                var_R = var_R / 12.92;
            if (var_G > 0.04045)
                var_G = pow(((var_G + 0.055) / 1.055), 2.4);
            else
                var_G = var_G / 12.92;
            if (var_B > 0.04045)
                var_B = pow(((var_B + 0.055) / 1.055), 2.4);
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

            X = RoundValue_toX_SignificantBits(X, 0);
            Y = RoundValue_toX_SignificantBits(Y, 0);
            Z = RoundValue_toX_SignificantBits(Z, 0);

            Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 0] = X;
            Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 1] = Y;
            Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 2] = Z;

        }
    }
}

/*
C O N V E R T  - XYZ to RGB
*/
void Convert_XYZ_to_RGB(struct Image *Img_src, struct Image *Img_dst)
{
    int i, j;
    float var_X, var_Y, var_Z;
    float R, B, G;

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_src->Width; j++)
        {
            var_X = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 0];// +Img_src->rgbpix[6 * (i * Img_dst->Width + j) + 3] / 100.0;
            var_Y = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 1];// +Img_src->rgbpix[6 * (i * Img_dst->Width + j) + 4] / 100.0;
            var_Z = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 2];// +Img_src->rgbpix[6 * (i * Img_dst->Width + j) + 5] / 100.0;

            var_X /= 100.0;
            var_Y /= 100.0;
            var_Z /= 100.0;

            R = var_X *  3.2406 + var_Y * -1.5372 + var_Z * -0.4986;
            G = var_X * -0.9689 + var_Y *  1.8758 + var_Z *  0.0415;
            B = var_X *  0.0557 + var_Y * -0.2040 + var_Z *  1.0570;

            //R = var_X *  2.3707 + var_Y * -0.9001 + var_Z * -0.4706;
            //G = var_X * -0.5139 + var_Y *  1.4253 + var_Z *  0.0886;
            //B = var_X *  0.0053 + var_Y * -0.0147 + var_Z *  1.0094;

            //if (i == 638 && j == 2372)
            //	printf("ds");
            if (R < 0)
                R = 0;
            if (G < 0)
                G = 0;
            if (B < 0)
                B = 0;

            if (R > 0.0031308)
                R = 1.055 * pow((double)R, (1 / 2.4)) - 0.055;
            else
                R = 12.92 * R;
            if (G > 0.0031308)
                G = 1.055 * pow((double)G, (1 / 2.4)) - 0.055;
            else
                G = 12.92 * G;
            if (B > 0.0031308)
                B = 1.055 * pow((double)B , (1 / 2.4)) - 0.055;
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

            R = RoundValue_toX_SignificantBits(R, 0);
            G = RoundValue_toX_SignificantBits(G, 0);
            B = RoundValue_toX_SignificantBits(B, 0);

            Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 0] = R;
            Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 1] = G;
            Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 2] = B;
        }
    }
}

/*
    C O N V E R T - RGB to L*ab
*/
void ConvertImage_RGB_to_LAB(struct Image *Img_src, struct Image *Img_dst, struct WhitePoint WhitePoint_XYZ)
{
    FILE *fdebug = NULL;
    struct Image Img_XYZ = CreateNewImage(&Img_XYZ,Img_src->Width, Img_src->Height, 3, 2, 8);

    int i, j;
    float L, a, b;
    float X, Y, Z;
    float F_x, F_y, F_z;
    float R_t, G_t, B_t;
    float RatioY, RatioX, RatioZ;
    float e = 0.008856;
    float k = 903.3;
    float dult;
    if ((Img_src->Num_channels != 3) || (Img_dst->Num_channels != 3))
    {
        #ifdef DEBUG_FILE
                fdebug = fopen(DEBUG_FILE, "wt");
                fprintf(fdebug, "The input images are not with 3 channels\n");
                fclose(fdebug);
        #endif // DEBUG_FILE
        return;
    }

    if (Img_src->Width * Img_src->Height != Img_dst->Width * Img_dst->Height)
    {
        SetDestination(Img_src, Img_dst);
    }
    Img_dst->rgbpix = (unsigned char *)realloc(Img_dst->rgbpix, Img_dst->Height * Img_dst->Width * Img_dst->Num_channels* sizeof(unsigned char));

    if (Img_src->ColorSpace != 2)
    {
        #ifdef DEBUG_FILE
                fdebug = fopen(DEBUG_FILE, "wt");
                fprintf(fdebug, "The input image is not in RGB format\n");
                fclose(fdebug);
        #endif // DEBUG_FILE
        return;
    }

    Img_dst->ColorSpace = 4;

    // 1st step: Convert to XYZ color space
    Convert_RGB_to_XYZ(Img_src, &Img_XYZ);

    for (i = 0; i < Img_src->Height; i++)
    {
        for (j = 0; j < Img_src->Width; j++)
        {
            X = Img_XYZ.rgbpix[3 * (i * Img_dst->Width + j) + 0];// +Img_XYZ.rgbpix[6 * (i * Img_dst->Width + j) + 3] / 100.0;
            Y = Img_XYZ.rgbpix[3 * (i * Img_dst->Width + j) + 1];// +Img_XYZ.rgbpix[6 * (i * Img_dst->Width + j) + 4] / 100.0;
            Z = Img_XYZ.rgbpix[3 * (i * Img_dst->Width + j) + 2];// +Img_XYZ.rgbpix[6 * (i * Img_dst->Width + j) + 5] / 100.0;

            X /= 100.0;
            Y /= 100.0;
            Z /= 100.0;

            RatioX = X / 1;// (100 * WhitePoint_XYZ.X);
            RatioY = Y / 1;// (100 * WhitePoint_XYZ.Y);
            RatioZ = Z / 1;// (100 * WhitePoint_XYZ.Z);

            RatioX = RoundValue_toX_SignificantBits(RatioX, 4);
            RatioY = RoundValue_toX_SignificantBits(RatioY, 4);
            RatioZ = RoundValue_toX_SignificantBits(RatioZ, 4);

            if (RatioX > e)
            {
                F_x = pow((double)RatioX, pow((double)3, -1));
            }
            else
            {
                F_x = (k * RatioX + 16) / 116;
            }

            if (RatioY > e)
            {
                F_y = pow((double)RatioY, pow((double)3, -1));
            }
            else
            {
                F_y = (k * RatioY + 16) / 116;

            }

            if (RatioZ > e)
            {
                F_z = pow((double)RatioZ, pow((double)3, -1));
            }
            else
            {
                F_z = (k * RatioZ + 16) / 116;
            }


            //L = 116 * F_y - 16;
            R_t = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 0] / 255.0;
            G_t = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 1] / 255.0;
            B_t = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 2] / 255.0;

            F_x = RoundValue_toX_SignificantBits(F_x, 3);
            F_y = RoundValue_toX_SignificantBits(F_y, 3);
            F_z = RoundValue_toX_SignificantBits(F_z, 3);

            dult = MIN(R_t, MIN(G_t, B_t)) + MAX(R_t, MAX(G_t, B_t));
            L = ((float)dult / 2) * 100;

            a = 500 * (F_x - F_y);
            b = 200 * (F_y - F_z);

            L = RoundValue_toX_SignificantBits(L, 0);
            a = RoundValue_toX_SignificantBits(a, 0);
            b = RoundValue_toX_SignificantBits(b, 0);
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

            Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 0] = L;
            if (L - Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 0] > 0.5) Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 0] += 1;
            Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 1] = a;
            if (a - Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 1] > 0.5) Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 1] += 1;
            Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 2] = b;
            if (b - Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 2] > 0.5) Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 2] += 1;

        }
    }

    DestroyImage(&Img_XYZ);
}

/*
    C O N V E R T - L*ab to RGB
*/
void ConvertImage_LAB_to_RGB(struct Image *Img_src, struct Image *Img_dst, struct WhitePoint WhitePoint_XYZ)
{
    FILE *fdebug = NULL;
    struct Image Img_XYZ = CreateNewImage(&Img_XYZ, Img_src->Width, Img_src->Height, 3, 2, 8);

    int i, j;
    float L, a, b;
    float X, Y, Z;
    float P;
    float Number;
    float RatioY, RatioX, RatioZ;
    if ((Img_src->Num_channels != 3) || (Img_dst->Num_channels != 3))
    {
        #ifdef DEBUG_FILE
                fdebug = fopen(DEBUG_FILE, "wt");
                fprintf(fdebug, "The input images are not with 3 channels\n");
                fclose(fdebug);
        #endif // DEBUG_FILE
        return;
    }

    if (Img_src->Width * Img_src->Height != Img_dst->Width * Img_dst->Height)
    {
        SetDestination(Img_src, Img_dst);
    }

    if (Img_src->ColorSpace != 4)
    {
        #ifdef DEBUG_FILE
                fdebug = fopen(DEBUG_FILE, "wt");
                fprintf(fdebug, "The input image is not in L*ab format\n");
                fclose(fdebug);
        #endif // DEBUG_FILE
        return;
    }

    Img_dst->ColorSpace = 2;

    // 1st step: Convert to XYZ color space

    for (i = 0; i < Img_dst->Height; i++)
    {
        for (j = 0; j < Img_dst->Width; j++)
        {
            L = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 0];
            a = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 1];
            b = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 2];

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

            X = RoundValue_toX_SignificantBits(X, 0);
            Y = RoundValue_toX_SignificantBits(Y, 0);
            Z = RoundValue_toX_SignificantBits(Z, 0);
            Img_XYZ.rgbpix[3 * (i * Img_dst->Width + j) + 0] = X;
            Img_XYZ.rgbpix[3 * (i * Img_dst->Width + j) + 1] = Y;
            Img_XYZ.rgbpix[3 * (i * Img_dst->Width + j) + 2] = Z;

        }
    }
    Convert_XYZ_to_RGB(&Img_XYZ, Img_dst);

    DestroyImage(&Img_XYZ);
}



float xyz_to_lab(float c)
{
    return c > 216.0 / 24389.0 ? pow((double)c, 1.0 / 3.0) : c * (841.0 / 108.0) + (4.0 / 29.0);
}

void WhiteBalanceGREENY(struct Image *src, struct Image *dst, struct WhitePoint WhitePoint_XYZ)
{
    float LuminanceAverage = 0;
    struct ColorPoint_UV UV;
    struct WhitePoint WhitePointXYZ_new;
    struct ColorPoint_RGB RGB;
    struct ColorPoint_XYZ XYZ;
    struct ColorPoint_XYZ XYZ_D;
    float RatioX, RatioY, RatioZ;
    float e = 0.008856;
    float u,v;
    float k = 903.3;
    float F_x, F_z, F_y;

    //XYZ
    //float Matrix_M_a[9] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
    //float matrix_M_min1[9] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
    //BRADFORD
    //float Matrix_M_a[9] = { 0.8951000, 0.2664000, -0.1614000, -0.7502000, 1.7135000, 0.0367000, 0.0389000, -0.0685000, 1.0296000 };
    //float matrix_M_min1[9] = { 0.9869929, -0.1470543, 0.1599627, 0.4323053, 0.5183603, 0.0492912, -0.0085287, 0.0400428, 0.9684867 };
    //CAT97s
    //float Matrix_M_a[9] = { 0.08562, 0.3372, -0.1934, -0.836, 1.8327, 0.0033, 0.0357, -0.0469, 1.0112 };
    //float matrix_M_min1[9] = { 0.9869929, -0.1470543, 0.1599627, 0.4323053, 0.5183603, 0.0492912, -0.0085287, 0.0400428, 0.9684867 };
    //CAT02s
    float Matrix_M_a[9] = { 0.7328, 0.4296, -0.1624, -0.7036, 1.6975, 0.0061, 0.0030, 0.0136, 0.9834 };
    float matrix_M_min1[9] = { 0.9869929, -0.1470543, 0.1599627, 0.4323053, 0.5183603, 0.0492912, -0.0085287, 0.0400428, 0.9684867 };
    //VON Kries
    //float Matrix_M_a[9] = { 0.40024, 0.7076, -0.08081, -0.2263, 1.16532, 0.0457, 0, 0, 0.91822 };
    //float matrix_M_min1[9] = { 1.8599364, -1.1293816, 0.2198974, 0.3611914, 0.6388125, -0.0000064, 0, 0, 1.08906 };

    float S_params[3];
    float D_params[3];
    float S_D_ParamsMatrix[9];
    float MatrixMultiplication_1[9];
    float matrix_M_final[9];
    float maxv;
    float P, Number;
    int i, j, z;
    long double R_Global = 0;
    long double G_Global = 0;
    long double B_Global = 0;
    float MAX_VALUE = 0;
    int Razlika_R, Razlika_G, Razlika_B;
    int tmpKelvin = 0;

    float R, G, B, dult;
    float X, Y, Z, L, a, b, bbb, intensity;



    // FULL

    R_Global = 0;
    G_Global = 0;
    B_Global = 0;

    for (i = 0; i < src->Height; i++)
    {
        for (j = 0; j < src->Width; j++)
        {
            R = (float)src->rgbpix[(i*dst->Width + j) * 3 + 0];
            G = (float)src->rgbpix[(i*dst->Width + j) * 3 + 1];
            B = (float)src->rgbpix[(i*dst->Width + j) * 3 + 2];

            /* if there is a perfect white pixel in the image */
            //if (R > 252 && G > 252 && B > 252)
            //	return;

            R_Global += R;
            G_Global += G;
            B_Global += B;
        }
    }

    R_Global /= (float)(src->Height * src->Width);
    G_Global /= (float)(src->Height * src->Width);
    B_Global /= (float)(src->Height * src->Width);

    LuminanceAverage = R_Global + G_Global + B_Global;
    R = R_Global;
    G = G_Global;
    B = B_Global;
    MAX_VALUE = MAX(R, MAX(G, B));

    if (MAX_VALUE != G)
    {
        //WhitebalanceCorrectionBLUEorRED(src, dst, WhitePoint_XYZ);
        //return;
    }

    Razlika_B = 255 - MAX_VALUE;
//	if (R == MAX_VALUE)
//	{
    //R += Razlika_B;
    //G += Razlika_B;
    //B += Razlika_B;
/*	}
    else if (G == MAX_VALUE)
    {
        R += Razlika;
        B += Razlika;
    }
    else if (B == MAX_VALUE)
    {
        G += Razlika;
        R += Razlika;
    }
*/
    RGB.R = RoundValue_toX_SignificantBits(R, 0);
    RGB.G = RoundValue_toX_SignificantBits(G, 0);
    RGB.B = RoundValue_toX_SignificantBits(B, 0);

    XYZ = POINT_Convert_RGB_to_XYZ(&RGB);
    UV = POINT_Convert_XYZ_to_UV(&XYZ);

    WhitePointXYZ_new.u = UV.u;
    WhitePointXYZ_new.v = UV.v;

    WhitePointXYZ_new.X = XYZ.X;
    WhitePointXYZ_new.Y = XYZ.Y;
    WhitePointXYZ_new.Z = XYZ.Z;

    ColorTemperature(&WhitePointXYZ_new, 0);// EXP_HIGH_T);

    //printf("Zone_ALL\nu: %f v: %f, K:%d\n\n", UV.u, UV.v, WhitePointXYZ_new.Temperature);


    tmpKelvin = WhitePoint_XYZ.Temperature / 100;

    if (tmpKelvin <= 66){

        R = 255;

        G = tmpKelvin;
        G = 99.4708025861 * log(G) - 161.1195681661;


        if (tmpKelvin <= 19)
        {
            B = 0;
        }
        else {

            B = tmpKelvin - 10;
            B = 138.5177312231 * log(B) - 305.0447927307;

        }

    }
    else {

        R = tmpKelvin - 60;
        R = 329.698727446 * pow((double)R, -0.1332047592);

        G = tmpKelvin - 60;
        G = 288.1221695283 * pow((double)G, -0.0755148492);

        B = 255;

    }

    tmpKelvin = WhitePointXYZ_new.Temperature / 100;

    if (tmpKelvin <= 66){

        Razlika_R = 255;

        Razlika_G = tmpKelvin;
        Razlika_G = 99.4708025861 * log((double)Razlika_G) - 161.1195681661;


        if (tmpKelvin <= 19)
        {
            Razlika_B = 0;
        }
        else {

            Razlika_B = tmpKelvin - 10;
            Razlika_B = 138.5177312231 * log((double)Razlika_B) - 305.0447927307;

        }

    }
    else {

        Razlika_R = tmpKelvin - 60;
        Razlika_R = 329.698727446 * pow(Razlika_R, -0.1332047592);

        Razlika_G = tmpKelvin - 60;
        Razlika_G = 288.1221695283 * pow(Razlika_G, -0.0755148492);

        Razlika_B = 255;
    }

    //Razlika_R = R - Razlika_R;
    //Razlika_G = G - Razlika_G;
    //Razlika_B = B - Razlika_B;
    /*Luminance between 0 and 1*/
    LuminanceAverage /=(3.0 * 255);
    /*change Matrix_M_a to match the luminance*/
    for(i = 0; i < 9; i++)
    {
        //if(i == 2)Matrix_M_a[i] *= (0 *LuminanceAverage); // cherveno
        //if(i == 8)Matrix_M_a[i] *= (0 *LuminanceAverage); // Zeleno
        //else
            Matrix_M_a[i] *= (1.4 * LuminanceAverage);
            //matrix_M_min1[i] *= (0.4 * LuminanceAverage);
        //Matrix_M_a[i] += LuminanceAverage - 0.8; // Epic fail
        //Matrix_M_a[i] += LuminanceAverage - 0.9;
        //if (i == 1)  Matrix_M_a[i] *= (0.1 * LuminanceAverage);
        //else Matrix_M_a[i] *= (0.7 * LuminanceAverage);
    }

    WhitePointXYZ_new.X /= 100.0;
    WhitePointXYZ_new.Y /= 100.0;
    WhitePointXYZ_new.Z /= 100.0;

    //#pragma omp parallel for private(j, col, R, G, B, intensity,L,a,bbb,X,Y,Z)
    for (i = 0; i<src->Height; i++)
    {
        for (j = 0; j<src->Width; j++)
        {
            maxv = 255;

            R = (float)src->rgbpix[(i*dst->Width + j) * 3 + 0];
            G = (float)src->rgbpix[(i*dst->Width + j) * 3 + 1];
            B = (float)src->rgbpix[(i*dst->Width + j) * 3 + 2];

            RGB.R = R;
            RGB.G = G;
            RGB.B = B;
            // Convert RGB point to XYZ
            XYZ = POINT_Convert_RGB_to_XYZ(&RGB);
            XYZ.X /= 100.0;
            XYZ.Y /= 100.0;
            XYZ.Z /= 100.0;

            /*
            | X_d |           | X_s |
            | Y_d |   = |M| * | Y_s |
            | Z_s |           | Z_s |
            http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
            */
            S_params[0] = WhitePointXYZ_new.X * Matrix_M_a[0] + WhitePointXYZ_new.Y * Matrix_M_a[1] + WhitePointXYZ_new.Z * Matrix_M_a[2];
            S_params[1] = WhitePointXYZ_new.X * Matrix_M_a[3] + WhitePointXYZ_new.Y * Matrix_M_a[4] + WhitePointXYZ_new.Z * Matrix_M_a[5];
            S_params[2] = WhitePointXYZ_new.X * Matrix_M_a[6] + WhitePointXYZ_new.Y * Matrix_M_a[7] + WhitePointXYZ_new.Z * Matrix_M_a[8];

            D_params[0] = WhitePoint_XYZ.X * Matrix_M_a[0] + WhitePoint_XYZ.Y * Matrix_M_a[1] + WhitePoint_XYZ.Z * Matrix_M_a[2];
            D_params[1] = WhitePoint_XYZ.X * Matrix_M_a[3] + WhitePoint_XYZ.Y * Matrix_M_a[4] + WhitePoint_XYZ.Z * Matrix_M_a[5];
            D_params[2] = WhitePoint_XYZ.X * Matrix_M_a[6] + WhitePoint_XYZ.Y * Matrix_M_a[7] + WhitePoint_XYZ.Z * Matrix_M_a[8];

            /* Compute M_min1 matrix * S/D */
            S_D_ParamsMatrix[0] = D_params[0] / S_params[0];
            S_D_ParamsMatrix[1] = 0;
            S_D_ParamsMatrix[2] = 0;
            S_D_ParamsMatrix[3] = 0;
            S_D_ParamsMatrix[4] = D_params[1] / S_params[1];
            S_D_ParamsMatrix[5] = 0;
            S_D_ParamsMatrix[6] = 0;
            S_D_ParamsMatrix[7] = 0;
            S_D_ParamsMatrix[8] = D_params[2] / S_params[2];

            MatrixMultiplication_1[0] = matrix_M_min1[0] * S_D_ParamsMatrix[0];
            MatrixMultiplication_1[1] = matrix_M_min1[1] * S_D_ParamsMatrix[4];
            MatrixMultiplication_1[2] = matrix_M_min1[2] * S_D_ParamsMatrix[8];
            MatrixMultiplication_1[3] = matrix_M_min1[3] * S_D_ParamsMatrix[0];
            MatrixMultiplication_1[4] = matrix_M_min1[4] * S_D_ParamsMatrix[4];
            MatrixMultiplication_1[5] = matrix_M_min1[5] * S_D_ParamsMatrix[8];
            MatrixMultiplication_1[6] = matrix_M_min1[6] * S_D_ParamsMatrix[0];
            MatrixMultiplication_1[7] = matrix_M_min1[7] * S_D_ParamsMatrix[4];
            MatrixMultiplication_1[8] = matrix_M_min1[8] * S_D_ParamsMatrix[8];

            /* Compute MatrixMultiplication_1 * matrix_M */

            matrix_M_final[0] = MatrixMultiplication_1[0] * Matrix_M_a[0] + MatrixMultiplication_1[1] * Matrix_M_a[3] + MatrixMultiplication_1[2] * Matrix_M_a[6];
            matrix_M_final[1] = MatrixMultiplication_1[0] * Matrix_M_a[1] + MatrixMultiplication_1[1] * Matrix_M_a[4] + MatrixMultiplication_1[2] * Matrix_M_a[7];
            matrix_M_final[2] = MatrixMultiplication_1[0] * Matrix_M_a[2] + MatrixMultiplication_1[1] * Matrix_M_a[5] + MatrixMultiplication_1[2] * Matrix_M_a[8];
            matrix_M_final[3] = MatrixMultiplication_1[3] * Matrix_M_a[0] + MatrixMultiplication_1[4] * Matrix_M_a[3] + MatrixMultiplication_1[5] * Matrix_M_a[6];
            matrix_M_final[4] = MatrixMultiplication_1[3] * Matrix_M_a[1] + MatrixMultiplication_1[4] * Matrix_M_a[4] + MatrixMultiplication_1[5] * Matrix_M_a[7];
            matrix_M_final[5] = MatrixMultiplication_1[3] * Matrix_M_a[2] + MatrixMultiplication_1[4] * Matrix_M_a[5] + MatrixMultiplication_1[5] * Matrix_M_a[8];
            matrix_M_final[6] = MatrixMultiplication_1[6] * Matrix_M_a[0] + MatrixMultiplication_1[7] * Matrix_M_a[3] + MatrixMultiplication_1[8] * Matrix_M_a[6];
            matrix_M_final[7] = MatrixMultiplication_1[6] * Matrix_M_a[1] + MatrixMultiplication_1[7] * Matrix_M_a[4] + MatrixMultiplication_1[8] * Matrix_M_a[7];
            matrix_M_final[8] = MatrixMultiplication_1[6] * Matrix_M_a[2] + MatrixMultiplication_1[7] * Matrix_M_a[5] + MatrixMultiplication_1[8] * Matrix_M_a[8];

            XYZ_D.X = matrix_M_final[0] * XYZ.X + matrix_M_final[1] * XYZ.Y + matrix_M_final[2] * XYZ.Z;
            XYZ_D.Y = matrix_M_final[3] * XYZ.X + matrix_M_final[4] * XYZ.Y + matrix_M_final[5] * XYZ.Z;
            XYZ_D.Z = matrix_M_final[6] * XYZ.X + matrix_M_final[7] * XYZ.Y + matrix_M_final[8] * XYZ.Z;
            //RGB: 150,55,7
            //XYZ: 0.14, 0.09, 0.01
            RGB = POINT_Convert_XYZ_to_RGB(&XYZ_D);

            dst->rgbpix[(i*dst->Width + j) * 3 + 0] = RGB.R;
            dst->rgbpix[(i*dst->Width + j) * 3 + 1] = RGB.G;
            dst->rgbpix[(i*dst->Width + j) * 3 + 2] = RGB.B;
        }
    }
    //WhitebalanceCorrectionBLUEorRED(src, dst, WhitePoint_XYZ);
}

/*
    P O I N T  - convert RGB to XYZ
*/
struct ColorPoint_XYZ POINT_Convert_RGB_to_XYZ(struct ColorPoint_RGB *RGB_Point)
{
    float R, G, B;
    struct ColorPoint_XYZ XYZ;
    R = RGB_Point->R;
    G = RGB_Point->G;
    B = RGB_Point->B;

    R /= 255;
    G /= 255;
    B /= 255;

    if (R > 0.04045)
        R = pow(((R + 0.055) / 1.055), 2.4);
    else
        R = R / 12.92;
    if (G > 0.04045)
        G = pow(((G + 0.055) / 1.055), 2.4);
    else
        G = G / 12.92;
    if (B > 0.04045)
        B = pow(((B + 0.055) / 1.055), 2.4);
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

/*
    P O I N T  - convert XYZ to RGB
*/
struct ColorPoint_RGB POINT_Convert_XYZ_to_RGB(struct  ColorPoint_XYZ *XYZ)
{
    float R, G, B;
    struct ColorPoint_RGB RGB;
    float X, Y, Z;

    X = XYZ->X;
    Y = XYZ->Y;
    Z = XYZ->Z;

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
        R = 1.055 * pow((double)R, (1 / 2.4)) - 0.055;
    else
        R = 12.92 * R;
    if (G > 0.0031308)
        G = 1.055 * pow((double)G, (1 / 2.4)) - 0.055;
    else
        G = 12.92 * G;
    if (B > 0.0031308)
        B = 1.055 * pow((double)B , (1 / 2.4)) - 0.055;
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

    RGB.R = RoundValue_toX_SignificantBits(R, 0);
    RGB.G = RoundValue_toX_SignificantBits(G, 0);
    RGB.B = RoundValue_toX_SignificantBits(B, 0);

    return RGB;
}

/*
    P O I N T  - convert  XYZ to UV  coordinates
*/
struct ColorPoint_UV POINT_Convert_XYZ_to_UV(struct ColorPoint_XYZ *XYZ)
{
    struct ColorPoint_UV UV;
    UV.u = 4 * XYZ->X / (float)(XYZ->X + 15 * XYZ->Y + 3 * XYZ->Z);
    UV.v = 6 * XYZ->Y / (float)(XYZ->X + 15 * XYZ->Y + 3 * XYZ->Z);

    return UV;
}

/*
    P O I N T  - convert  UV to XYZ  coordinates
*/
struct point_xy POINT_Convert_UV_to_XY(struct ColorPoint_UV *UV)
{
    struct point_xy XY;

    XY.X = 3 * UV->u / (2 * UV->u - 8 * UV->v + 4);
    XY.Y = 2 * UV->v / (2 * UV->u - 8 * UV->v + 4);

    return XY;
}

/*
    C O L O R - Temperature
*/
void ColorTemperature(struct WhitePoint *WhitePoint_lab, int AlgoType )
{
    float X_e = 0.3366;
    float Y_e = 0.1735;
    float A_0 = -949.86315;
    float A_1 = 6253.803;
    float t_1 = 0.92159;
    float A_2 = 28.706;
    float t_2 = 0.20039;
    float A_3 = 0.00004;
    float t_3 = 0.07125;
    //
    float n;
    float CCT;
    struct point_xy XY;
    struct ColorPoint_UV UV;
    if (AlgoType == 0)
    {
        // Calculate color temperature and XYZ coordinates from UV coordinates
        if (WhitePoint_lab->u != 0)
        {
            UV.u = WhitePoint_lab->u;
            UV.v = WhitePoint_lab->v;

            //Caclulate XY from UV
            XY = POINT_Convert_UV_to_XY(&UV);
        }
        else
        {
            //Calculate XY from XYZ;
            XY.X = WhitePoint_lab->X / (WhitePoint_lab->X + WhitePoint_lab->Y + WhitePoint_lab->Z);
            XY.Y = WhitePoint_lab->Y / (WhitePoint_lab->X + WhitePoint_lab->Y + WhitePoint_lab->Z);
        }

        n = (XY.X - 0.332) / (0.1858 - XY.Y);
        CCT = 449 * pow(n, 3) + 3525 * pow(n, 2) + 6823.3 * n + 5520.33;
    }
    else
    {
        //EXP_HIGH_T
        XY.X = WhitePoint_lab->X / (WhitePoint_lab->X + WhitePoint_lab->Y + WhitePoint_lab->Z);
        XY.Y = WhitePoint_lab->Y / (WhitePoint_lab->X + WhitePoint_lab->Y + WhitePoint_lab->Z);
        n = (XY.X - X_e) / (XY.Y - Y_e);
        CCT = A_0 + A_1 * exp(-n / t_1) + A_2 * exp(-n / t_2) + A_3*exp(-n/t_3);
    }

    //Differences in values for both algorithms
    // 6347 - pic1     4287 - pic 2; //Algo_type = 0
    // 6344 - pic1     4293 - pic 2; // Algo_type = 1;
    WhitePoint_lab->Temperature = CCT;
}

//
/*

//R_Global += R;
//G_Global += G;
//B_Global += B;

//dult = MIN(R, MIN(G, B)) + MAX(R, MAX(G, B));
//intensity = (float)dult / 2.0;

R /= (float)maxv;
G /= (float)maxv;
B /= (float)maxv;

//CONVERT RGB - XYZ
if (R > 0.04045)
R = pow(((R + 0.055) / 1.055), 2.4);
else
R = R / 12.92;
if (G > 0.04045)
G = pow(((G + 0.055) / 1.055), 2.4);
else
G = G / 12.92;
if (B > 0.04045)
B = pow(((B + 0.055) / 1.055), 2.4);
else
B = B / 12.92;


X = R * 0.4124 + G * 0.3576 + B * 0.1805;
Y = R * 0.2126 + G * 0.7152 + B * 0.0722;
Z = R * 0.0193 + G * 0.1192 + B * 0.9505;


if (X < 0) X = 0;
if (Y < 0) Y = 0;
if (Z < 0) Z = 0;

// XYZ - Lab
RatioX = X / (WhitePointXYZ_new.X / 100.0);
RatioY = Y / (WhitePointXYZ_new.Y / 100.0);
RatioZ = Z / (WhitePointXYZ_new.Z / 100.0);

//RatioX /= 100;
//RatioY /= 100;
//RatioZ /= 100;
if (RatioX > e)
{
    F_x = pow(RatioX, pow(3, -1));
}
else
{
    F_x = (k * RatioX + 16) / 116;
}

if (RatioY > e)
{
    F_y = pow(RatioY, pow(3, -1));
}
else
{
    F_y = (k * RatioY + 16) / 116;
}

if (RatioZ > e)
{
    F_z = pow(RatioZ, pow(3, -1));
}
else
{
    F_z = (k * RatioZ + 16) / 116;
}

if (RatioY > 0.008856)
{
    L = 116 * pow(RatioY, (0.333)) - 16;
}
else
L = 903.3 *  RatioY;

a = 500 * (F_x - F_y);

b = 200 * (F_y - F_z);

// Lab to XYZ
P = (L + 16) / 116.0;

Number = P;
if (Number > 6 / 29.0)
{
    Number = pow(Number, 3);
}
else
{
    Number = 3 * pow(6 / 29.0, 2) * (Number - 4 / 29.0);
}

Y = WhitePoint_XYZ.Y *
Number;

Number = P + a / 500.0;
if (Number > 6 / 29.0)
{
    Number = pow(Number, 3);
}
else
{
    Number = 3 * pow(6 / 29.0, 2) * (Number - 4 / 29.0);
}
X = WhitePoint_XYZ.X *
Number;

Number = P - b / 200;
if (Number > 6 / 29.0)
{
    Number = pow(Number, 3);
}
else
{
    Number = 3 * pow(6 / 29.0, 2) * (Number - 4 / 29.0);
}
Z = WhitePoint_XYZ.Z *
Number;

if (Z > 0.35585 && WhitePoint_XYZ.Temperature < 3700)
    Z = 0.3585;

if (Z > 0.82521 && WhitePoint_XYZ.Temperature < 5400)
    Z = 0.82521;
// XYZ to RGB

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
R = 1.055 * pow(R, (1 / 2.4)) - 0.055;
else
R = 12.92 * R;
if (G > 0.0031308)
G = 1.055 * pow(G, (1 / 2.4)) - 0.055;
else
G = 12.92 * G;
if (B > 0.0031308)
B = 1.055 * pow(B, (1 / 2.4)) - 0.055;
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

*/

float pow_func(float Number, float Stepen, int precision)
{
    int i = 0;
    int StepenInteger = 0;
    float NewNumber = 1;
    int flag_Reverse = 0;
    float Chislitel = 1, Dopylnenie = 0;

    if (Stepen > 0 && Stepen < 1)
    {
        Stepen = 1 / Stepen;
        flag_Reverse = 1;
    }

    StepenInteger = Stepen; // ako sme imali za stepen 2.3 - zapisvame samo 2

    for (i = 0; i < StepenInteger; i++)
    {
        NewNumber *= Number;
    }

    for (i = 0; i < (Stepen - StepenInteger) * precision; i++)
    {
        Chislitel *= Number;
    }
    Dopylnenie = Chislitel / (precision * precision);

    NewNumber += Dopylnenie;

    if (flag_Reverse == 1) NewNumber = 1 / NewNumber;

    return NewNumber;
}

/*
    W H I T E   B A L A N C E  algorithm using RGB and Temperature of src and dest
*/
void WhitebalanceCorrectionBLUEorRED(struct Image *src, struct Image *dst, struct WhitePoint WhitePoint_lab)
{
    float LuminanceAverage = 0;
    struct ColorPoint_UV UV;
    struct WhitePoint WhitePointXYZ_new;
    struct ColorPoint_RGB RGB;
    struct ColorPoint_XYZ XYZ;
    struct ColorPoint_XYZ XYZ_D;

    float maxv;
    float P, Number;
    int i, j, z;
    long double R_Global = 0;
    long double G_Global = 0;
    long double B_Global = 0;
    float MAX_VALUE = 0;
    int Razlika_R, Razlika_G, Razlika_B;
    int tmpKelvin = 0;

    float R, G, B, dult;
    float X, Y, Z, L, a, b, bbb, intensity;

    R_Global = 0;
    G_Global = 0;
    B_Global = 0;

    for (i = 0; i < src->Height; i++)
    {
        for (j = 0; j < src->Width; j++)
        {
            R = (float)src->rgbpix[(i*dst->Width + j) * 3 + 0];
            G = (float)src->rgbpix[(i*dst->Width + j) * 3 + 1];
            B = (float)src->rgbpix[(i*dst->Width + j) * 3 + 2];

            R_Global += R;
            G_Global += G;
            B_Global += B;
        }
    }

    R_Global /= (float)(src->Height * src->Width);
    G_Global /= (float)(src->Height * src->Width);
    B_Global /= (float)(src->Height * src->Width);

    if (MAX(R_Global, MAX(G_Global, B_Global)) == G_Global)
    {
    //	memcpy(src->rgbpix, dst->rgbpix, 3 * src->Height * src->Width * sizeof(unsigned char));
    //	return;
    }

    RGB.R = RoundValue_toX_SignificantBits(R_Global, 0);
    RGB.G = RoundValue_toX_SignificantBits(G_Global, 0);
    RGB.B = RoundValue_toX_SignificantBits(B_Global, 0);

    XYZ = POINT_Convert_RGB_to_XYZ(&RGB);
    UV = POINT_Convert_XYZ_to_UV(&XYZ);

    WhitePointXYZ_new.u = UV.u;
    WhitePointXYZ_new.v = UV.v;

    WhitePointXYZ_new.X = XYZ.X;
    WhitePointXYZ_new.Y = XYZ.Y;
    WhitePointXYZ_new.Z = XYZ.Z;

    ColorTemperature(&WhitePointXYZ_new, 0);// EXP_HIGH_T);

    tmpKelvin = WhitePointXYZ_new.Temperature / 100;

    if (tmpKelvin <= 66){

        Razlika_R = 255;

        Razlika_G = tmpKelvin;
        Razlika_G = 99.4708025861 * log((double)Razlika_G) - 161.1195681661;


        if (tmpKelvin <= 19)
        {
            Razlika_B = 0;
        }
        else {

            Razlika_B = tmpKelvin - 10;
            Razlika_B = 138.5177312231 * log((double)Razlika_B) - 305.0447927307;

        }

    }
    else
    {

        Razlika_R = tmpKelvin - 60;
        Razlika_R = 329.698727446 * pow(Razlika_R, -0.1332047592);

        Razlika_G = tmpKelvin - 60;
        Razlika_G = 288.1221695283 * pow(Razlika_G, -0.0755148492);

        Razlika_B = 255;

    }
    //printf("Zone_ALL\nu: %f v: %f, K:%d\n\n", UV.u, UV.v, WhitePointXYZ_new.Temperature);


    for (i = 0; i < src->Height; i++)
    {
        for (j = 0; j < src->Width; j++)
        {
            maxv = 255;

            R = (float)src->rgbpix[(i*dst->Width + j) * 3 + 0];
            G = (float)src->rgbpix[(i*dst->Width + j) * 3 + 1];
            B = (float)src->rgbpix[(i*dst->Width + j) * 3 + 2];

            RGB.R = RoundValue_toX_SignificantBits(R * 255 / (float)Razlika_R, 0);
            RGB.G = RoundValue_toX_SignificantBits(G * 255 / (float)Razlika_G, 0);
            RGB.B = RoundValue_toX_SignificantBits(B * 255 / (float)Razlika_B, 0);

            if (RGB.R > 255) RGB.R = 255;
            if (RGB.G > 255) RGB.G = 255;
            if (RGB.B > 255) RGB.B = 255;
            if (RGB.R < 0) RGB.R = 0;
            if (RGB.G < 0) RGB.G = 0;
            if (RGB.B < 0) RGB.B = 0;

            dst->rgbpix[(i*dst->Width + j) * 3 + 0] = RGB.R;
            dst->rgbpix[(i*dst->Width + j) * 3 + 1] = RGB.G;
            dst->rgbpix[(i*dst->Width + j) * 3 + 2] = RGB.B;
        }
    }
}

/*
    Create   H I S T O G R A M
*/
void HistogramForImage(struct Histogram *hist, struct Image *Img_src, short NumberOfLayers)
{
    //FILE * fp;
    int i, j, z;
    long int maxValue = 0;

    struct Image grayscaledImage = CreateNewImage(&grayscaledImage, Img_src->Width, Img_src->Height, 1, 1, 8);
    hist->Bins = pow(2, Img_src->imageDepth);

    hist->NumberOfLayers = NumberOfLayers;

    if (NumberOfLayers == 1)
    {
        /*if the image depth is 8bit -> we will have a histogram for 256 values*/
        hist->values = (long int *)calloc(hist->Bins, sizeof(long int));

        /* check if the input image is RGB or grayscaled. If it is RGB, convert to grayscaled */
        if (Img_src->Num_channels == 1)
        {
            memcpy(grayscaledImage.rgbpix, Img_src->rgbpix, Img_src->Width* Img_src->Height * sizeof(unsigned char));
        }
        else
        {
            ConvertToGrayscale_1Channel(Img_src, &grayscaledImage);
        }

        /* Work with grayscaledImage */
        for (i = 0; i < Img_src->Height; i++)
        {
            for (j = 0; j < Img_src->Width; j++)
            {
                hist->values[grayscaledImage.rgbpix[i * Img_src->Width + j]]++;
                if (hist->values[grayscaledImage.rgbpix[i * Img_src->Width + j]] > maxValue)
                    maxValue = hist->values[grayscaledImage.rgbpix[i * Img_src->Width + j]];
            }
        }
    }
    else
    {
        //if (Img_src->Num_channels != 3) return -1;

        /*if the image depth is 8bit -> we will have a histogram for 256 * 3 values*/
        hist->values = (long int *)calloc(3 *hist->Bins, sizeof(long int));

        /* Work with colored imput image */
        for (i = 0; i < Img_src->Height; i++)
        {
            for (j = 0; j < Img_src->Width; j++)
            {
                for (z = 0; z < 3; z++)
                {
                    hist->values[3 *(Img_src->rgbpix[3* (i * Img_src->Width + j) +z]) + z]++;
                    if (hist->values[3 * (Img_src->rgbpix[3 * (i * Img_src->Width + j) + z]) + z] > maxValue)
                        maxValue = hist->values[3 * (Img_src->rgbpix[3 * (i * Img_src->Width + j) + z]) + z];
                }
            }
        }
    }
    /*
    fopen_s(&fp, "blqk.txt", "wt");
    for (i = 0; i < 256; i++)
    {
        fprintf(fp, "%ld: %ld\n", i, (hist->values[3 * i + 0]));
    }
    fprintf(fp, "\n\n");
    for (i = 0; i < 256; i++)
    {
        fprintf(fp, "%ld: %ld\n", i, (hist->values[3 * i + 1]));
    }

    fprintf(fp, "\n\n");
    for (i = 0; i < 256; i++)
    {
        fprintf(fp, "%ld: %ld\n", i, (hist->values[3 * i + 2]));
    }

    fclose(fp);
    */
    hist->MaxValue = maxValue;
}

/*
    Convert  H I S TO G R A M   to  I M A G E
*/
void ConvertHistToImage(struct Histogram *hist, struct Image *Img_src)
{
    int i, j, k, z;
    int write = 1;
    float ScaleNumber = 0;
    long int MaxValue = hist->MaxValue;
    int Char_array[81];
    int NumberofCalcs = 0;
    int average;

    /* Set size for the Hist Image */
    if (hist->Bins == 256)
    {
        hist->Size_x = 2 * hist->Bins + 30;
        if (MaxValue > 600) // we have to scale if it is too big
        {
            ScaleNumber = MaxValue / 600;
            hist->Size_y = (MaxValue / ScaleNumber + 40);
        }
        else {
            hist->Size_y = MaxValue + 40;
            ScaleNumber = ((MaxValue + 40) / 600.0);
        }
    }

    Img_src->Num_channels = hist->NumberOfLayers;
    if (hist->NumberOfLayers == 3) Img_src->ColorSpace = 2;
    else Img_src->ColorSpace = 1;

    Img_src->rgbpix = (unsigned char *)realloc(Img_src->rgbpix, hist->NumberOfLayers * hist->Size_y * hist->Size_x * sizeof(unsigned char));
    Img_src->Width = hist->Size_x;
    Img_src->Height = hist->Size_y;

    for (j = 0; j < hist->Size_x; j++)
    {
        for (i = hist->Size_y - 1;  i >= 0; i--)
        {
            for (k = 0; k < hist->NumberOfLayers; k++)
            {
                //if (Img_src->rgbpix[hist->NumberOfLayers * (i * hist->Size_x + j) + k] != 255)
                Img_src->rgbpix[hist->NumberOfLayers * (i * hist->Size_x + j) + k] = 0;

                if (i == 0 || j == 0 || j == hist->Size_x - 1 || i == hist->Size_y - 1)
                {
                    Img_src->rgbpix[hist->NumberOfLayers * (i * hist->Size_x + j) + k] = 255;
                    continue;
                }

                if (j % 2 == 0)
                {
                    if ((j <= 2 * hist->Bins + 20) && j > 20) // ako e chetno
                    {
                        if (i > (hist->Size_y - 20 - (hist->values[hist->NumberOfLayers * ((j - 20) / 2) + k] / ScaleNumber)) && (i < hist->Size_y - 20))
                        {
                            Img_src->rgbpix[hist->NumberOfLayers * (i * hist->Size_x + j) + k] = 255;
                        }
                    }
                }
                else
                {
                    if ((j < 2 * hist->Bins + 20) && j > 20)
                    {
                        average = ((hist->values[hist->NumberOfLayers *((j - 19) / 2) + k] / ScaleNumber) + (hist->values[hist->NumberOfLayers * ((j - 21) / 2) + k] / ScaleNumber)) / 2;
                        if (i >(hist->Size_y - 20 - (average)) && (i < hist->Size_y - 20))
                        {
                            Img_src->rgbpix[hist->NumberOfLayers * (i * hist->Size_x + j) + k] = 255;
                        }
                    }
                    //Img_src->rgbpix[hist->NumberOfLayers * (i * hist->Size_x + j) + k] = 0;
                }
            }
        }
    }


    for (k = MaxValue; k > 0; k--)
    {
        for (i = 0; i < 81; i++)
        {
            Char_array[i] = 0;
        }
        NumberofCalcs++;
        write = k / 10;
        write = k - (write * 10);
        k =k /10;
        if (write == 0)
        {
            Char_array[4] = 255;
            Char_array[5] = 255;
            Char_array[12] = 255;
            Char_array[15] = 255;
            Char_array[20] = 255;
            Char_array[25] = 255;
            Char_array[28] = 255;
            Char_array[35] = 255;
            Char_array[37] = 255;
            Char_array[44] = 255;
            Char_array[46] = 255;
            Char_array[53] = 255;
            Char_array[56] = 255;
            Char_array[61] = 255;
            Char_array[66] = 255;
            Char_array[69] = 255;
            Char_array[76] = 255;
            Char_array[77] = 255;
        }
        else if(write == 1)
        {
            Char_array[20] = 255;
            Char_array[12] = 255;
            Char_array[4] = 255;
            Char_array[5] = 255;
            Char_array[13] = 255;
            Char_array[14] = 255;
            Char_array[22] = 255;
            Char_array[23] = 255;
            Char_array[31] = 255;
            Char_array[32] = 255;
            Char_array[40] = 255;
            Char_array[41] = 255;
            Char_array[49] = 255;
            Char_array[50] = 255;
            Char_array[58] = 255;
            Char_array[59] = 255;
            Char_array[67] = 255;
            Char_array[68] = 255;
            Char_array[69] = 255;
            Char_array[66] = 255;
        }
        else if (write == 2)
        {
            Char_array[10] = 255;
            Char_array[2] = 255;
            Char_array[3] = 255;
            Char_array[4] = 255;
            Char_array[5] = 255;
            Char_array[6] = 255;
            Char_array[15] = 255;
            Char_array[24] = 255;
            Char_array[32] = 255;
            Char_array[40] = 255;
            Char_array[48] = 255;
            Char_array[56] = 255;
            Char_array[64] = 255;
            Char_array[65] = 255;
            Char_array[66] = 255;
            Char_array[67] = 255;
            Char_array[68] = 255;
            Char_array[69] = 255;
        }
        else if (write == 3)
        {
            Char_array[11] = 255;
            Char_array[3]  = 255;
            Char_array[4]  = 255;
            Char_array[14] = 255;
            Char_array[24] = 255;
            Char_array[32] = 255;
            Char_array[40] = 255;
            Char_array[50] = 255;
            Char_array[60] = 255;
            Char_array[68] = 255;
            Char_array[76] = 255;
            Char_array[75] = 255;
            Char_array[65] = 255;
        }
        else if (write == 4)
        {
            Char_array[5] = 255;
            Char_array[13] = 255;
            Char_array[21] = 255;
            Char_array[29] = 255;
            Char_array[37] = 255;
            Char_array[38] = 255;
            Char_array[39] = 255;
            Char_array[40] = 255;
            Char_array[41] = 255;
            Char_array[42] = 255;
            Char_array[43] = 255;
            Char_array[34] = 255;
            Char_array[25] = 255;
            Char_array[16] = 255;
            Char_array[52] = 255;
            Char_array[61] = 255;
            Char_array[70] = 255;
            Char_array[79] = 255;
        }
        else if (write == 5)
        {
            Char_array[10] = 255;
            Char_array[2] = 255;
            Char_array[3] = 255;
            Char_array[4] = 255;
            Char_array[5] = 255;
            Char_array[6] = 255;
            Char_array[19] = 255;
            Char_array[28] = 255;
            Char_array[37] = 255;
            Char_array[38] = 255;
            Char_array[39] = 255;
            Char_array[40] = 255;
            Char_array[41] = 255;
            Char_array[42] = 255;
            Char_array[43] = 255;
            Char_array[52] = 255;
            Char_array[61] = 255;
            Char_array[70] = 255;
            Char_array[69] = 255;
            Char_array[68] = 255;
            Char_array[67] = 255;
            Char_array[66] = 255;
            Char_array[65] = 255;
            Char_array[64] = 255;
        }
        else if (write == 6)
        {
            Char_array[4] = 255;
            Char_array[5] = 255;
            Char_array[12] = 255;
            Char_array[15] = 255;
            Char_array[20] = 255;
            Char_array[28] = 255;
            Char_array[37] = 255;
            Char_array[46] = 255;
            Char_array[56] = 255;
            Char_array[66] = 255;
            Char_array[76] = 255;
            Char_array[77] = 255;
            Char_array[69] = 255;
            Char_array[61] = 255;
            Char_array[51] = 255;
            Char_array[41] = 255;
            Char_array[40] = 255;
            Char_array[48] = 255;
        }
        else if (write == 7)
        {
            Char_array[2] = 255;
            Char_array[3] = 255;
            Char_array[4] = 255;
            Char_array[5] = 255;
            Char_array[6] = 255;
            Char_array[7] = 255;
            Char_array[16] = 255;
            Char_array[24] = 255;
            Char_array[33] = 255;
            Char_array[41] = 255;
            Char_array[50] = 255;
            Char_array[58] = 255;
            Char_array[67] = 255;
            Char_array[75] = 255;
        }
        else if (write == 8)
        {
            Char_array[3] = 255;
            Char_array[4] = 255;
            Char_array[11] = 255;
            Char_array[14] = 255;
            Char_array[19] = 255;
            Char_array[24] = 255;
            Char_array[29] = 255;
            Char_array[32] = 255;
            Char_array[39] = 255;
            Char_array[40] = 255;
            Char_array[47] = 255;
            Char_array[50] = 255;
            Char_array[55] = 255;
            Char_array[60] = 255;
            Char_array[65] = 255;
            Char_array[68] = 255;
            Char_array[75] = 255;
            Char_array[76] = 255;
        }
        else if (write == 9)
        {
            Char_array[3] = 255;
            Char_array[4] = 255;
            Char_array[11] = 255;
            Char_array[14] = 255;
            Char_array[19] = 255;
            Char_array[24] = 255;
            Char_array[29] = 255;
            Char_array[32] = 255;
            Char_array[39] = 255;
            Char_array[40] = 255;
            Char_array[33] = 255;
            Char_array[42] = 255;
            Char_array[51] = 255;
            Char_array[60] = 255;
            Char_array[69] = 255;
            Char_array[77] = 255;
            Char_array[76] = 255;
            Char_array[75] = 255;
        }
        for (j = 0; j < 9; j++)
        {
            for (i = 0; i < 9; i++)
            {
                for (z = 0; z < hist->NumberOfLayers; z++)
                {
                    Img_src->rgbpix[hist->NumberOfLayers * ((15 + i) * hist->Size_x + (hist->Size_x / 2) - (10 * NumberofCalcs) + j) + z] = Char_array[i * 9 + j];
                }
            }
        }
    }

    /* write 0*/
    for (i = 0; i < 81; i++)
    {
        Char_array[i] = 0;
    }
    Char_array[4] = 255;
    Char_array[5] = 255;
    Char_array[12] = 255;
    Char_array[15] = 255;
    Char_array[20] = 255;
    Char_array[25] = 255;
    Char_array[28] = 255;
    Char_array[35] = 255;
    Char_array[37] = 255;
    Char_array[44] = 255;
    Char_array[46] = 255;
    Char_array[53] = 255;
    Char_array[56] = 255;
    Char_array[61] = 255;
    Char_array[66] = 255;
    Char_array[69] = 255;
    Char_array[76] = 255;
    Char_array[77] = 255;

    for (j = 0; j < 9; j++)
    {
        for (i = 0; i < 9; i++)
        {
            for (k = 0; k < hist->NumberOfLayers; k++)
            {
                Img_src->rgbpix[hist->NumberOfLayers * ((hist->Size_y - 15 + i) * hist->Size_x + 10 + j) + k] = Char_array[i * 9 + j];
            }
        }
    }

    /* write 2*/
    for (i = 0; i < 81; i++)
    {
        Char_array[i] = 0;
    }
    Char_array[10] = 255;
    Char_array[2] = 255;
    Char_array[3] = 255;
    Char_array[4] = 255;
    Char_array[5] = 255;
    Char_array[6] = 255;
    Char_array[15] = 255;
    Char_array[24] = 255;
    Char_array[32] = 255;
    Char_array[40] = 255;
    Char_array[48] = 255;
    Char_array[56] = 255;
    Char_array[64] = 255;
    Char_array[65] = 255;
    Char_array[66] = 255;
    Char_array[67] = 255;
    Char_array[68] = 255;
    Char_array[69] = 255;

    for (j = 0; j < 9; j++)
    {
        for (i = 0; i < 9; i++)
        {
            for (k = 0; k < hist->NumberOfLayers; k++)
            {
                Img_src->rgbpix[hist->NumberOfLayers * ((hist->Size_y - 15 + i) * hist->Size_x + hist->Size_x - 40 + j) + k] = Char_array[i * 9 + j];
            }
        }
    }

    /* write 5*/
    for (i = 0; i < 81; i++)
    {
        Char_array[i] = 0;
    }
    Char_array[10] = 255;
    Char_array[2] = 255;
    Char_array[3] = 255;
    Char_array[4] = 255;
    Char_array[5] = 255;
    Char_array[6] = 255;
    Char_array[19] = 255;
    Char_array[28] = 255;
    Char_array[37] = 255;
    Char_array[38] = 255;
    Char_array[39] = 255;
    Char_array[40] = 255;
    Char_array[41] = 255;
    Char_array[42] = 255;
    Char_array[43] = 255;
    Char_array[52] = 255;
    Char_array[61] = 255;
    Char_array[70] = 255;
    Char_array[69] = 255;
    Char_array[68] = 255;
    Char_array[67] = 255;
    Char_array[66] = 255;
    Char_array[65] = 255;
    Char_array[64] = 255;

    for (j = 0; j < 9; j++)
    {
        for (i = 0; i < 9; i++)
        {
            for (k = 0; k < hist->NumberOfLayers; k++)
            {
                Img_src->rgbpix[hist->NumberOfLayers * ((hist->Size_y - 15 + i) * hist->Size_x + hist->Size_x - 30 + j) + k] = Char_array[i * 9 + j];
                Img_src->rgbpix[hist->NumberOfLayers * ((hist->Size_y - 15 + i) * hist->Size_x + hist->Size_x - 20 + j) + k] = Char_array[i * 9 + j];
            }
        }
    }

}




int dft(long int length, int length2, long double real_sample[], long double imag_sample[], long double temp_real[], long double temp_imag[])
{
    long int i, j, k, l;
    long double arg;
    long double CurrentValCalcs;
    long double cosarg, sinarg;
    double radius;
    double Pitagor;
    //long double *temp_real = NULL, *temp_imag = NULL;

    //temp_real = calloc(length * length2, sizeof(long double));
    //temp_imag = calloc(length * length2, sizeof(long double));
    //if (temp_real == NULL || temp_imag == NULL)
    //{
    //	return(FALSE);
    //}

    radius = length2 / 2 - (0.35 * length2 / 2);

    for (k = 0; k < length; k += 1)
    {
        for (l = 0; l < length2; l += 1)
        {

            Pitagor = sqrt(pow((length2 / 2 - l), 2) + pow((length / 2 - k), 2));
            if (Pitagor > (MIN(length, length2) - 3) )
            {
                temp_real[k * length2 + l] = 0;
                temp_imag[k * length2 + l] = 0;
                continue;
            }

            temp_real[k * length2 + l] = 0;
            temp_imag[k * length2 + l] = 0;
            for (i = 0; i < length; i += 1)
            {
                for (j = 0; j < length2; j += 1)
                {
                    arg = -1.0 * 2.0 * 3.141592654 * ((long double)(i * k) / (long double)length + (long double)(j * l) / (long double)length2);

                    cosarg = cos(arg);
                    sinarg = sin(arg);
                    temp_real[k * length2 + l] += (real_sample[i * length2 + j] * cosarg - imag_sample[i * length2 + j] * sinarg);
                    temp_imag[k * length2 + l] += (real_sample[i * length2 + j] * sinarg + imag_sample[i * length2 + j] * cosarg);
                }
            }
        }
    }


    for (i = 0; i<length * length2; i += 1)
    {
        real_sample[i] = temp_real[i];
        imag_sample[i] = temp_imag[i];
    }

    //free(temp_real);
    //free(temp_imag);
    return(TRUE);
}



//Inverse Discrete Fourier Transform


int inverse_dft(long int length, int length2, long double real_sample[], long double imag_sample[], long double temp_real[], long double temp_imag[])
{
    long int i, j, k, l;
    long double arg;
    long double cosarg, sinarg;
    //long double *temp_real = NULL, *temp_imag = NULL;

    //temp_real = calloc(length * length2, sizeof(long double));
    //temp_imag = calloc(length * length2, sizeof(long double));
    //if (temp_real == NULL || temp_imag == NULL)
    //{
    //	return(FALSE);
    //}

    for (k = 0; k < length; k += 1)
    {
        for (l = 0; l < length2; l += 1)
        {
            temp_real[k * length2 + l] = 0;
            temp_imag[k * length2 + l] = 0;
            for (i = 0; i < length; i += 1)
            {
                //arg = 2.0 * 3.141592654 * (long double)i / (long double)length;
                for (j = 0; j < length2; j += 1)
                {
                    arg = 2.0 * 3.141592654 * ((long double)(i * k) / (long double)length + (long double)(j * l) / (long double)length2);

                    cosarg = cos(arg);
                    sinarg = sin(arg);
                    temp_real[k * length2 + l] += (real_sample[i * length2 + j] * cosarg - imag_sample[i * length2 + j] * sinarg);
                    temp_imag[k * length2 + l] += (real_sample[i * length2 + j] * sinarg + imag_sample[i * length2 + j] * cosarg);
                }
            }
        }
    }


    for (i = 0; i<length * length2; i += 1)
    {
        real_sample[i] = temp_real[i] / (long double)(length * length2);
        imag_sample[i] = temp_imag[i] / (long double)(length * length2);
    }

    //free(temp_real);
    //free(temp_imag);
    return(TRUE);
}


void SpatialToFrequencyDomain(struct Image *img_src, struct Image *img_dst)
{
    // perform DFT
    // The number of frequencies corresponds to the number of pixels in the spatial domain image, i.e. the image in the spatial and Fourier domain are of the same size.
    // Formula is published here: http://homepages.inf.ed.ac.uk/rbf/HIPR2/fourier.htm

    long double* dr = NULL;
    long double* di = NULL;
    long double* fr = NULL;
    long double* fi = NULL;

    int i, j, k, l, z;
    float MATH_PI = 3.14;
    long double CurrentValCalcs = 0;
    long double CurrentValCalcs2 = 0;
    //_Complex double ui = 0;
    int Min = 655035, Max = 0;
    int Width = img_src->Width;
    int Height = img_src->Height;
    long double *Arr_calc = NULL;
    long double *Arr_calc2 = NULL;
    long double *Arr_calc3 = NULL;
    long double *Arr_calc4 = NULL;
    int broi = 0;
    //struct Image Mapping = CreateNewImage(&Mapping, img_src->Width, img_src->Height, 1, 1, 8);
    struct Image grayscaledImage = CreateNewImage(&grayscaledImage, img_src->Width, img_src->Height, 1, 1, 8);

    Arr_calc = (long double *)malloc(img_src->Width * img_src->Height * sizeof(long double));
    Arr_calc2 = (long double *)malloc(img_src->Width * img_src->Height * sizeof(long double));
    Arr_calc3 = (long double *)malloc(img_src->Width * img_src->Height * sizeof(long double));
    Arr_calc4 = (long double *)malloc(img_src->Width * img_src->Height * sizeof(long double));

    if (img_src->Num_channels != 1) ConvertToGrayscale_1Channel(img_src, &grayscaledImage);
    else memcpy(grayscaledImage.rgbpix, img_src->rgbpix, img_src->Width * img_src->Height * sizeof(unsigned char));

    if (grayscaledImage.Width * grayscaledImage.Height != img_dst->Width * img_dst->Height)
    {
        SetDestination(&grayscaledImage, img_dst);
    }

    dr = (long double *)malloc(grayscaledImage.Width * grayscaledImage.Height * sizeof(long double));
    di = (long double *)malloc(grayscaledImage.Width * grayscaledImage.Height * sizeof(long double));
    fr = (long double *)malloc(grayscaledImage.Width * grayscaledImage.Height * sizeof(long double));
    fi = (long double *)malloc(grayscaledImage.Width * grayscaledImage.Height * sizeof(long double));

    for (i = 0; i < grayscaledImage.Height * grayscaledImage.Width; i++)
    {
        di[i] = 0;
        dr[i] = grayscaledImage.rgbpix[i];
    }

    dft(grayscaledImage.Height , grayscaledImage.Width, dr, di, fr, fi);
    inverse_dft(grayscaledImage.Height , grayscaledImage.Width, dr, di, fr, fi);

    for (i = 0; i < grayscaledImage.Height * grayscaledImage.Width; i++)
    {
        if (dr[i] < 0) dr[i] = 0;
        if (dr[i] > 255) dr[i] = 255;
        img_dst->rgbpix[i] = RoundValue_toX_SignificantBits(dr[i], 0);
    }

    free(fr);
    free(fi);
    free(dr);
    free(di);
}

