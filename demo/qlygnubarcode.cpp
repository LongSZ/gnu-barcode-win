#include "qlygnubarcode.h"
#include <QDebug>
#include <string.h>

QlyGNUBarcode::QlyGNUBarcode()
    :bc(nullptr),
      m_barcode_type(BARCODE_128),
      m_NO_ASCII(false),
      m_margin(0),
      m_fgColor(Qt::black),
      m_bgColor(Qt::white) {

}

void QlyGNUBarcode::setMargin(int whitespace) {
    m_margin = whitespace;
}

QColor QlyGNUBarcode::foregroundColor() const {
    return m_fgColor;
}

void QlyGNUBarcode::setForegroundColor(const QColor & fgColor) {
    m_fgColor = fgColor;
}

QColor QlyGNUBarcode::backgroundColor() const {
    return  m_bgColor;
}

void QlyGNUBarcode::setBackgroundColor(const QColor & bgColor) {
    m_bgColor = bgColor;
}

QSize QlyGNUBarcode::minSize() {
    if( bc ) return QSize();
    return QSize(bc->width, bc->height);
}

bool QlyGNUBarcode::encodeText(const QString &text) {
    if( bc ) Barcode_Delete(bc);
    bc = Barcode_Create((char *) text.toLocal8Bit().data());
    //bc = Barcode_Create((char *)text.toStdString().c_str());
    //Barcode_Position(bc, 0, 0, 0, 0, 1.0);
    bc->margin = m_margin;
    int flags = m_barcode_type;
    if(m_NO_ASCII) {
        flags = m_barcode_type | BARCODE_NO_ASCII;
    }
    else {
        flags = m_barcode_type;
    }
    bc->flags = flags;

    Barcode_Encode(bc, flags);
    updateSizeInfo();
    return (bc);
}

bool QlyGNUBarcode::updateSizeInfo() {
    if( !bc ) return false;

    if ( !bc->partial || !bc->textinfo ) {
        bc->error = EINVAL;
        return false;
    }
    /* First, calculate barlen */
    int barlen = bc->partial[0] - '0';
    for (char * ptr = bc->partial + 1; *ptr; ptr++) {
        if ( isdigit(*ptr) ) {
            barlen += (*ptr - '0');
        }
        else if ( islower(*ptr) ) {
            barlen += (*ptr - 'a' + 1);
        }
    }

    m_minWidth = barlen; // 这个宽度是计算出的最小宽度
    m_minHeight = 80; // 默认的高度

    qDebug() << "m_minWidth = " << m_minWidth;
    qDebug() << "m_minHeight = " << m_minHeight;

    /* The scale factor depends on bar length */
    if ( (fabs(bc->scalef) < 1e-6) ) {
        if ( !bc->width ) {
            bc->width = barlen; /* default */
        }
        bc->scalef = (double) bc->width / (double)barlen;

        //qDebug() << "fabs(bc->scalef) < 1e-6 , bc->width = " << bc->width << "bc->scalef = " << bc->scalef;
    }

    /* The width defaults to "just enough" */
    if ( !bc->width ) {
        bc->width = barlen * bc->scalef + 1;
    }

    /* But it can be too small, in this case enlarge and center the area */
    if (bc->width < barlen * bc->scalef) {
        int wid = barlen * bc->scalef + 1;
        bc->xoff -= (wid - bc->width) / 2 ;
        bc->width = wid;
        /* Can't extend too far on the left */
        if (bc->xoff < 0) {
            bc->width += -bc->xoff;
            bc->xoff = 0;
        }
    }

    /* The height defaults to 80 points (rescaled) */
    if ( !bc->height ) {
        bc->height = 80 * bc->scalef;
    }

    /* If too small (5 + text), reduce the scale factor and center */
    int i = 5 + 10 * (( bc->flags & BARCODE_NO_ASCII) == 0 );
    if (bc->height < i * bc->scalef ) {
        double scaleg = ((double) bc->height) / i;
        int wid = bc->width * scaleg / bc->scalef;
        bc->xoff += ( bc->width - wid ) / 2;
        bc->width = wid;
        bc->scalef = scaleg;
    }
    m_margin = bc->margin;

    m_global_width  = bc->xoff + bc->width  + 2 * bc->margin;
    m_global_height = bc->yoff + bc->height + 2 * bc->margin;

    return true;
}

bool QlyGNUBarcode::render(QPainter &painter, QRect rect)
{
    int xoffset = bc->xoff;
    int yoffset = bc->yoff;
    int width = bc->width;
    int height = bc->height;
    double scalef = bc->scalef;

    bc->xoff = rect.left();
    bc->xoff = rect.top();
    bc->width = rect.width() - 2 * m_margin;
    bc->height = rect.height() - 2 * m_margin;
    bc->scalef = 0.0;

    updateSizeInfo();
    bool ret = render(painter);

    bc->xoff = xoffset;
    bc->yoff = yoffset;
    bc->width = width;
    bc->height = height;
    bc->scalef = scalef;

    return ret;
}

bool QlyGNUBarcode::drawBar(QPainter &painter)
{
    int mode = '-';
    int i; /* text below bars */
    char * ptr;
    double xpos = bc->margin + (bc->partial[0] - '0') * bc->scalef;
    for (ptr = bc->partial + 1, i = 1; *ptr; ptr++, i++) {
        /* special cases: '+' and '-' */
        if (*ptr == '+' || *ptr == '-') {
            mode = *ptr; /* don't count it */
            i++;
            continue;
        }
        /* j is the width of this bar/space */
        int j;
        if (isdigit (*ptr))   j = *ptr - '0';
        else                  j = *ptr - 'a' + 1;

        double x0, y0, yr;
        if (i % 2) /* bar */ {

            //qDebug() << "bc->xoff = " << bc->xoff << ", xpos = " << xpos;
            x0 = bc->xoff + xpos;// + (j * scalef) / 2;
            y0 = bc->yoff + bc->margin;
            yr = bc->height;
            if ( !(bc->flags & BARCODE_NO_ASCII) ) { /* leave space for text */
                if (mode == '-') {
                    /* text below bars: 10 points or five points */
                    //y0 += (isdigit(*ptr) ? 10 : 5) * scalef;
                    yr -= (isdigit(*ptr) ? 10 : 5) * bc->scalef;
                }
                else { /* '+' */
                    /* text above bars: 10 or 0 from bottom, and 10 from top */
                    y0 += (isdigit(*ptr) ? 10 : 0) * bc->scalef;
                    yr -= (isdigit(*ptr) ? 20 : 10) * bc->scalef;
                }
            }
            painter.fillRect(QRect(x0, y0, (j * bc->scalef), yr ), m_fgColor);
            //qDebug() << "fillRect: " <<  QRect(x0, y0, (j * m_scalef), yr );
        }
        xpos += j * bc->scalef;
    }
    return true;
}

bool QlyGNUBarcode::drawText(QPainter &painter)
{

    int mode = '-'; /* reinstantiate default */
    if (!(bc->flags & BARCODE_NO_ASCII)) {
        painter.save();
        painter.setPen(m_fgColor);

        for (char * ptr = bc->textinfo; ptr; ptr = strchr(ptr, ' ')) {
            //qDebug() << "*";
            while (*ptr == ' ') ptr++;

            if (!*ptr) break;
            if (*ptr == '+' || *ptr == '-') {
                mode = *ptr; continue;
            }
            double f1, f2;
            char c;
            if (sscanf(ptr, "%lf:%lf:%c", &f1, &f2, &c) != 3) {
                //fprintf(stderr, "barcode: impossible data: %s\n", ptr);
                continue;
            }
            painter.setFont(QFont("Helvetica", (int)(0.8 * f2 * bc->scalef)));
            int x_pos = bc->xoff + f1 * bc->scalef + bc->margin;

            int y_pos = 0;
            if(mode == '-') {
                y_pos = (double)bc->yoff + bc->margin + bc->height ;//- 8 * bc->scalef;
            }
            else {
                y_pos =  (double)bc->yoff + bc->margin;
            }
            painter.drawText(QPoint(x_pos, y_pos ), QString(QChar(c)));
        }
        painter.restore();
    }
    return true;
}
QImage QlyGNUBarcode::paintImage(QSize size, QImage::Format format)
{
    if(!bc) return QImage();

    int w = size.width();
    double h = size.height();
    return paintImage(w / bc->width, h, format);
}

QImage QlyGNUBarcode::paintImage(double width_scale, int height, QImage::Format format)
{
    if(!bc) return QImage();

    int bcWidth = bc->width; // 保存现场
    int bcHeight = bc->height;
    float bcScalef = bc->scalef;

    bc->width = bc->width * width_scale;
    bc->scalef = width_scale;
    bc->height = height;
    qDebug() << "bc->margin = " << bc->margin;

    int w = bc->width + 2 * bc->margin;
    int h = bc->height + 2 * bc->margin;

    qDebug() << "in QtGnuBarcode::toImage(), w = " << w << ", h = " << h;
    QImage img(w, h, format);
    QPainter painter(&img);
    img.fill(m_bgColor);
    painter.setBrush(m_fgColor);
    render(painter);

    bc->width = bcWidth; // 恢复原状
    bc->height = bcHeight;
    bc->scalef = bcScalef;
    return img;
}

bool QlyGNUBarcode::render(QPainter &painter)
{
    if( !bc ) return false;

    int w = bc->width + 2 * bc->margin;
    int h = bc->height + 2 * bc->margin;
    painter.fillRect(QRect(0, 0, w, h), m_bgColor);

    if ( !bc->partial || !bc->textinfo ) {
        bc->error = EINVAL;
        return false;
    }
    drawBar(painter);

    qDebug() << "bc->flags & BARCODE_NO_ASCII = " <<( bc->flags & BARCODE_NO_ASCII);
    if (! (bc->flags & BARCODE_NO_ASCII) ) {
        drawText(painter);
    }
    return true;
}
