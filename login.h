#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>

class QLineEdit;

QT_BEGIN_NAMESPACE
namespace Ui {
class Login;
}
QT_END_NAMESPACE

// Fenêtre du projet LOCAL du module Login (page de connexion + page d'inscription).
// À l'intégration, cette classe n'est pas reprise : on copie le contenu de containerLogin
// dans la page PLogin de hydrosync.ui, puis les slots / fonctions marqués
// « MODULE LOGIN » (voir README.md).
class Login : public QMainWindow
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();

// ===================== MODULE LOGIN : déclarations à copier dans hydrosync.h =====================
private slots:
    // --- Navigation interne du module (SWLogin : 0 = connexion, 1 = inscription) ---
    void on_btnVersInscriptionLogin_clicked();
    void on_btnVersConnexionLogin_clicked();

    // --- Boutons de la page de connexion ---
    void on_btnConnexionLogin_clicked();
    void on_btnMdpOublieLogin_clicked();
    void on_btnFaceIdConnexionLogin_clicked();

    // --- Boutons de la page d'inscription ---
    void on_btnInscriptionLogin_clicked();
    void on_btnFaceIdInscriptionLogin_clicked();

private:
    void initialiserModuleLogin();   // à appeler dans le constructeur, juste après setupUi()
    void configurerChampLogin(QLineEdit *champ, const QString &cheminIcone, bool motDePasse);
    void afficherFaceIdIndisponibleLogin();
// =================== FIN MODULE LOGIN : déclarations à copier dans hydrosync.h ===================

private:
    Ui::Login *ui;
};

#endif // LOGIN_H
