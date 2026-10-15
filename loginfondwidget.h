#ifndef LOGINFONDWIDGET_H
#define LOGINFONDWIDGET_H

#include <QPixmap>
#include <QWidget>

// Widget "promu" dans login.ui (containerLogin) : dessine l'image de fond du module Login
// en mode « cover » (remplit toute la zone sans déformer l'image, l'excédent est rogné).
//
// À l'intégration : copier loginfondwidget.h / loginfondwidget.cpp dans le projet HydroSync
// et les ajouter au HydroSync.pro (voir README.md, étape 3).
class LoginFondWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginFondWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QPixmap m_fondOriginal;
    QPixmap m_fondRedimensionne;
};

#endif // LOGINFONDWIDGET_H
