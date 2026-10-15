#include "login.h"
#include "ui_login.h"

// ===================== MODULE LOGIN : #include à copier dans hydrosync.cpp =====================
#include <QAction>
#include <QGraphicsDropShadowEffect>
#include <QIcon>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QStackedWidget>
// =================== FIN MODULE LOGIN : #include ===================

Login::Login(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Login)
{
    ui->setupUi(this);
    initialiserModuleLogin();   // ← cette ligne aussi dans le constructeur de HydroSync
}

Login::~Login()
{
    delete ui;
}

// ============================================================================================
//  MODULE LOGIN : tout ce qui suit est à copier dans hydrosync.cpp lors de l'intégration.
//  Dans le bloc copié, remplacer « Login:: » par « HydroSync:: » (Ctrl+H dans Qt Creator).
// ============================================================================================

namespace {

// Indices des pages du stackedWidget interne SWLogin
constexpr int IndexPageConnexionLogin  = 0;   // PLoginConnexion
constexpr int IndexPageInscriptionLogin = 1;  // PLoginInscription

// Index du module Employé dans le stackedWidget principal SWHydroSync (contrat de l'équipe)
constexpr int IndexPageEmployeHydroSync = 1;  // PEmpl

// Ressources du module (login.qrc, préfixe /login)
const QString CheminLogoLogin        = QStringLiteral(":/login/images_login/login_logo_hydrosync.png");
const QString CheminIconeEmailLogin  = QStringLiteral(":/login/images_login/login_icone_email.png");
const QString CheminIconeCadenasLogin = QStringLiteral(":/login/images_login/login_icone_cadenas.png");
const QString CheminIconeOeilLogin   = QStringLiteral(":/login/images_login/login_icone_oeil.png");
const QString CheminIconeFaceIdLogin = QStringLiteral(":/login/images_login/login_icone_face_id.png");

} // namespace

// ---------- Initialisation (ce que Qt Designer ne sait pas faire) ----------
void Login::initialiserModuleLogin()
{
    // Ombre légère sous la carte (effet verre)
    auto *ombre = new QGraphicsDropShadowEffect(ui->carteLogin);
    ombre->setBlurRadius(40);
    ombre->setOffset(0, 8);
    ombre->setColor(QColor(0, 0, 0, 70));
    ui->carteLogin->setGraphicsEffect(ombre);

    // Logo de l'en-tête
    ui->lblLogoLogin->setPixmap(QPixmap(CheminLogoLogin)
                                    .scaled(52, 52, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // Icônes à gauche des champs (+ petit œil pour les mots de passe)
    configurerChampLogin(ui->txtIdConnexionLogin,           CheminIconeEmailLogin,   false);
    configurerChampLogin(ui->txtMdpConnexionLogin,          CheminIconeCadenasLogin, true);
    configurerChampLogin(ui->txtIdInscriptionLogin,         CheminIconeEmailLogin,   false);
    configurerChampLogin(ui->txtMdpInscriptionLogin,        CheminIconeCadenasLogin, true);
    configurerChampLogin(ui->txtConfirmationInscriptionLogin, CheminIconeCadenasLogin, true);

    // Boutons Face ID
    const QList<QPushButton *> boutonsFaceId = { ui->btnFaceIdConnexionLogin,
                                                 ui->btnFaceIdInscriptionLogin };
    for (QPushButton *bouton : boutonsFaceId) {
        bouton->setIcon(QIcon(CheminIconeFaceIdLogin));
        bouton->setIconSize(QSize(28, 28));
    }

    // Le module démarre sur la page de connexion
    ui->SWLogin->setCurrentIndex(IndexPageConnexionLogin);
}

void Login::configurerChampLogin(QLineEdit *champ, const QString &cheminIcone, bool motDePasse)
{
    // Icône à GAUCHE du champ
    champ->addAction(QIcon(cheminIcone), QLineEdit::LeadingPosition);

    if (motDePasse) {
        // Icône œil à DROITE : une action "cochable" (allumée / éteinte)
        QAction *oeil = champ->addAction(QIcon(CheminIconeOeilLogin), QLineEdit::TrailingPosition);
        oeil->setCheckable(true);
        connect(oeil, &QAction::toggled, this, [champ](bool visible) {
            champ->setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
        });
    }
}

// ---------- Navigation interne : Connexion <-> Inscription ----------
void Login::on_btnVersInscriptionLogin_clicked()
{
    ui->SWLogin->setCurrentIndex(IndexPageInscriptionLogin);
}

void Login::on_btnVersConnexionLogin_clicked()
{
    ui->SWLogin->setCurrentIndex(IndexPageConnexionLogin);
}

// ---------- Validation Connexion ----------
void Login::on_btnConnexionLogin_clicked()
{
    const QString id = ui->txtIdConnexionLogin->text().trimmed();
    const QString motDePasse = ui->txtMdpConnexionLogin->text();

    if (id.isEmpty()) {
        QMessageBox::warning(this, "Connexion", "Veuillez saisir votre ID ou votre email.");
        ui->txtIdConnexionLogin->setFocus();
        return;
    }
    if (motDePasse.isEmpty()) {
        QMessageBox::warning(this, "Connexion", "Veuillez saisir votre mot de passe.");
        ui->txtMdpConnexionLogin->setFocus();
        return;
    }

    // TEMPORAIRE : plus tard, vérification dans la base de données

    ui->txtMdpConnexionLogin->clear();

    // Navigation après connexion : page Employé du stackedWidget principal SWHydroSync.
    // - Projet intégré : SWHydroSync existe, on change de page.
    // - Projet local du module : SWHydroSync n'existe pas, on affiche seulement un message.
    if (auto *principal = findChild<QStackedWidget *>("SWHydroSync")) {
        principal->setCurrentIndex(IndexPageEmployeHydroSync);
    } else {
        QMessageBox::information(this, "Connexion",
                                 "Connexion réussie !\n"
                                 "(Après l'intégration, vous serez redirigé vers le module Employé.)");
    }
}

// ---------- Validation Inscription ----------
void Login::on_btnInscriptionLogin_clicked()
{
    const QString id = ui->txtIdInscriptionLogin->text().trimmed();
    const QString motDePasse = ui->txtMdpInscriptionLogin->text();
    const QString confirmation = ui->txtConfirmationInscriptionLogin->text();

    if (id.isEmpty()) {
        QMessageBox::warning(this, "Inscription", "Veuillez saisir votre ID ou votre email.");
        ui->txtIdInscriptionLogin->setFocus();
        return;
    }
    if (motDePasse.isEmpty()) {
        QMessageBox::warning(this, "Inscription", "Veuillez saisir un mot de passe.");
        ui->txtMdpInscriptionLogin->setFocus();
        return;
    }
    if (confirmation.isEmpty()) {
        QMessageBox::warning(this, "Inscription", "Veuillez vérifier votre mot de passe.");
        ui->txtConfirmationInscriptionLogin->setFocus();
        return;
    }
    if (motDePasse != confirmation) {
        QMessageBox::warning(this, "Inscription", "Les mots de passe ne correspondent pas.");
        ui->txtConfirmationInscriptionLogin->setFocus();
        return;
    }

    // TEMPORAIRE : plus tard, enregistrement dans la base de données
    QMessageBox::information(this, "Inscription", "Compte créé avec succès !");
}

// ---------- Autres boutons ----------
void Login::on_btnMdpOublieLogin_clicked()
{
    QMessageBox::information(this, "Mot de passe oublié",
                             "Fonctionnalité de récupération du mot de passe.");
}

void Login::on_btnFaceIdConnexionLogin_clicked()
{
    afficherFaceIdIndisponibleLogin();
}

void Login::on_btnFaceIdInscriptionLogin_clicked()
{
    afficherFaceIdIndisponibleLogin();
}

void Login::afficherFaceIdIndisponibleLogin()
{
    // Pas de vraie reconnaissance faciale pour l'instant
    QMessageBox::information(this, "Face ID",
                             "Authentification Face ID non disponible pour le moment.");
}

// =================== FIN MODULE LOGIN ===================
