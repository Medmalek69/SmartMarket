#include <QApplication>

#include "login.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Projet LOCAL du module Login : on teste la fenêtre seule.
    // (Le style est dans login.ui, propriété styleSheet de containerLogin.)
    Login fenetre;
    fenetre.show();

    return app.exec();
}
