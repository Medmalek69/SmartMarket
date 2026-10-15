#include "loginfondwidget.h"

#include <QPainter>

LoginFondWidget::LoginFondWidget(QWidget *parent)
    : QWidget(parent),
      m_fondOriginal(QStringLiteral(":/login/images_login/login_fond.png"))
{
    setAutoFillBackground(false);
}

void LoginFondWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_fondRedimensionne = QPixmap(); // recalculée au prochain paintEvent
}

void LoginFondWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    // Image introuvable : fond bleu clair de secours
    if (m_fondOriginal.isNull()) {
        painter.fillRect(rect(), QColor(0xd6, 0xe6, 0xf2));
        return;
    }

    const qreal ratio = devicePixelRatioF();
    const QSize cible = size() * ratio;
    if (m_fondRedimensionne.isNull() || m_fondRedimensionne.size() != cible) {
        // Agrandir l'image pour COUVRIR toute la zone sans la déformer
        m_fondRedimensionne = m_fondOriginal.scaled(cible, Qt::KeepAspectRatioByExpanding,
                                                    Qt::SmoothTransformation);
        m_fondRedimensionne.setDevicePixelRatio(ratio);
    }

    // Centrer l'image (ce qui dépasse est simplement rogné)
    const QSizeF tailleLogique = QSizeF(m_fondRedimensionne.size()) / ratio;
    painter.drawPixmap(QPointF((width() - tailleLogique.width()) / 2.0,
                               (height() - tailleLogique.height()) / 2.0),
                       m_fondRedimensionne);
}
