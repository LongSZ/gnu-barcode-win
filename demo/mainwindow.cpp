#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qlygnubarcode.h"

#include <QLabel>
#include <QPrintDialog>
#include <QPrinter>
#include <QPainter>

#include <iostream>
using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    QlyGNUBarcode bc;

    //简单使用, 下面带数字
#if 0
    bc.setBarcodeType(BARCODE_I25 | BARCODE_NO_CHECKSUM);
    bc.setNO_ASCII(false);
    bc.setMargin(5);
    bc.encodeText("1923456789");
    //bc.encodeText("798955561");   //为什么前面会自动添加个0
#endif

    //简单使用, 下面不带数字
#if 0
    bc.setBarcodeType(BARCODE_I25 | BARCODE_NO_CHECKSUM);
    bc.setNO_ASCII(true);
    bc.setMargin(5);
    bc.encodeText("798955561");
#endif

    /*
     * UPC 要求字符串长度为 6、7、8（UPC-E）或者 11、12 (UPC-A)。并且字符只能是数字。
     **/
#if 0
    bc.setBarcodeType(BARCODE_UPC);
    //bc.encodeText("1234567");
    bc.encodeText("12345678901");
#endif

    /*
     * EAN 和 UPC 要求差不多，只支持数字。要求数字是 7位或者12位。通常用于商品的编码。
     **/
#if 0
    bc.setBarcodeType(BARCODE_EAN);
    //bc.encodeText("1234567");     //为什么前面会自动添加个0
    //bc.encodeText("123456789012");
#endif

    /*
     * code 128 可以编码任意可以打印出来的 ASCII 字符。
     **/
#if 1
    bc.setBarcodeType(BARCODE_128);
    //bc.encodeText("1234567890ABCDEFGH");
    bc.encodeText("798955561");

    //bc.setBarcodeType(BARCODE_128B);
    //bc.encodeText("3200930012ABC");
#endif

    /*
     * CODE 39 这种编码支持数字和大写字母，还支持”+“、”-“、”*“ 等少数几个特殊符号。但是这个编码比较浪费空间。比如下面这个码，本身没有几个字符，却要占这么长的地方
     **/

#if 0
    bc.setBarcodeType(BARCODE_39EXT);
    //bc.encodeText("1234567890ABCDEFGH");
    bc.encodeText("7989555612");

    //bc.setBarcodeType(BARCODE_128B);
    //bc.encodeText("3200930012ABC");
#endif

    /*
     * 交叉25码。只支持数字。带校验的时候只支持奇数位数，如果输入字符是偶数，则编码是前面会加个 0。 如果不想要这个 0 可以把校验去掉。比如下面的例子：
     **/
#if 0
    bc.setBarcodeType(BARCODE_I25);
    //bc.encodeText("123456");
    //bc.setBarcodeType(BARCODE_I25 | BARCODE_NO_CHECKSUM);
    bc.encodeText("123");
#endif

    /*
     * Plessey 支持 0123456789ABCDE 。这15个字符。输出结果中的字母永远是大写。
     **/
#if 0 // best
    bc.setBarcodeType(BARCODE_PLS);
    //bc.encodeText("12345ABCDE");
    bc.encodeText("798955561");
#endif

    /*
     * MSI 只支持数字。位数不限。
     **/
#if 0
    bc.setBarcodeType(BARCODE_MSI);
    bc.setMargin(10);
    //bc.encodeText("1234567890");
    bc.encodeText("798955561");
#endif

    /*
     * CODE 93 支持10个数字，26个英文字母，空格等共 48 个字符
     **/
#if 0
    bc.setBarcodeType(BARCODE_93);
    //bc.encodeText("1234567890ABCXYZ+-*");
    bc.encodeText("798955561");
#endif

    QImage *img = new QImage(bc.paintImage(2, 80));
    QPixmap *qp = new QPixmap();
    qp->convertFromImage(*img);
    ui->label->setPixmap(*qp);


    //下面主要是测试打印
    QPixmap pm = ui->widget->grab();
    QImage img_to_print = pm.toImage();
    QPrinter printer;                                   //新建一个QPrinter对象
    QPainter painter(&printer);                         //指定绘图设备为一个QPrinter对象
    QRect rect = painter.viewport();                    //获得QPainter对象的视图矩形区域
    QSize size = img_to_print.size();                   //获得图像的大小

    /* 按照图形的比例大小重新设置视图矩形区域 */
    size.scale(rect.size() * 0.8, Qt::KeepAspectRatio);
    painter.setWindow(img_to_print.rect());             //设置QPainter窗口大小为图像的大小
    painter.setViewport(rect.x() + (rect.width() - size.width()) / 2,
                        rect.y() + (rect.height() - size.height()) / 2,
                        size.width(),
                        size.height());
    painter.drawImage(0, 0, img_to_print);				//打印图像
}
