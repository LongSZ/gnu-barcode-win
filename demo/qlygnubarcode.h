#ifndef QTGNUBARCODE_H
#define QTGNUBARCODE_H

#include <QImage>
#include <QPainter>

#include "barcode.h"

class QlyGNUBarcode {
public:
    QlyGNUBarcode();
    ~QlyGNUBarcode() {if( bc ) Barcode_Delete(bc);}

    //对文字进行编码。
    bool encodeText(const QString & text);

    //设置条码的类型。条形码类型很多，如果不设置则选择能编码数据的第一种条码类型,aram barcode_type BARCODE_EAN 等，具体参照 barcode.h
    void setBarcodeType(int barcode_type) {m_barcode_type = barcode_type;}

    //setMargin 条码四周留白区域的大小。
    void setMargin(int whitespace);

    //设置是否在条码上/下面显示对应文字
    void setNO_ASCII(bool on) {m_NO_ASCII = on;}

    QImage paintImage(double width_scale = 2, int height = 60, QImage::Format format = QImage::Format_RGB32);
    QImage paintImage(QSize size, QImage::Format format = QImage::Format_RGB32);
    bool render(QPainter &painter);
    bool render(QPainter &painter, QRect rect);

    //返回条码的尺寸，不包括四周的 margin。
    QSize minSize();

    //返回条码的尺寸，包括四周的 margin。
    QSize size() {return QSize(m_global_width, m_global_height);}

    //返回和设置条码的前景色。
    QColor foregroundColor() const;
    void setForegroundColor(const QColor & fgColor);

    //返回和设置条码的背景色。
    QColor backgroundColor() const;
    void setBackgroundColor(const QColor & bgColor);

private:
    bool updateSizeInfo();
    bool drawBar(QPainter &painter);
    bool drawText(QPainter &painter);

    Barcode_Item *bc;

    int m_barcode_type;
    bool m_NO_ASCII;

    int m_minWidth;
    int m_minHeight;
    int m_margin;
    int m_global_width;     // 整体的宽度，包含 margin
    int m_global_height;    // 整体的高度，包含 margin

    QString m_text;
    QColor m_fgColor;
    QColor m_bgColor;
};

#endif // QTGNUBARCODE_H
