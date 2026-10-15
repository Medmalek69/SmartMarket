# HydroSync_Login : module Login (connexion + inscription)

Projet **local** du module Login de HydroSync (C++ / Qt Widgets, qmake).
Il suit le *Guide d'intégration* : un seul `.ui` par projet, des noms d'objets significatifs,
et un `QStackedWidget` dont chaque page est un module.

## Contenu du dossier

| Fichier / dossier | Rôle | À l'intégration |
|---|---|---|
| `HydroSync_Login.pro` | Projet qmake (ouvrir celui-ci dans Qt Creator) | Reprendre le bloc « Module Login » dans `HydroSync.pro` |
| `login.ui` | **L'interface** : `containerLogin` contient tout le module (carte, `SWLogin`, 2 pages, style) | Copier-coller `containerLogin` dans la page `PLogin` |
| `login.h` / `login.cpp` | Fenêtre du projet local + code du module (slots, initialisation) | Copier les blocs « MODULE LOGIN » dans `hydrosync.h` / `.cpp` |
| `loginfondwidget.h` / `.cpp` | Dessine l'image de fond (widget promu de `containerLogin`) | Copier les 2 fichiers + les ajouter au `.pro` |
| `login.qrc` | Ressources du module (préfixe `/login`) | Copier + ajouter au `.pro` |
| `images_login/` | Images, toutes préfixées `login_` | Copier le dossier |
| `main.cpp` | Lance la fenêtre en local | **Ne pas copier** |

## Anciens noms → nouveaux noms

| Avant | Maintenant |
|---|---|
| `HydroSync/` (dossier) et `HydroSync.pro` | `HydroSync_Login/` et `HydroSync_Login.pro` (le nom `HydroSync.pro` est réservé au projet d'intégration) |
| `mainwindow.h/.cpp` (classe `MainWindow`, interface écrite en code) | `login.h/.cpp/.ui` (classe `Login`, interface dans `login.ui`) |
| `backgroundwidget`, `paintEvent` de la fenêtre | `loginfondwidget.h/.cpp` (classe `LoginFondWidget`) |
| `resources.qrc`, `resources/images/` | `login.qrc` (préfixe `/login`), `images_login/` |
| `background.png`, `hydrosync_logo.png` | `login_fond.png`, `login_logo_hydrosync.png` |
| `email.png`, `lock.png`, `eye.png`, `face_id.png` | `login_icone_email.png`, `login_icone_cadenas.png`, `login_icone_oeil.png`, `login_icone_face_id.png` |
| Style dans `main.cpp` | Style dans `login.ui` (propriété `styleSheet` de `containerLogin`) : il suit le copier-coller |

## Noms des widgets (tous terminent par `Login`)

Structure : `containerLogin` ➜ `carteLogin` ➜ `SWLogin` (stackedWidget interne)

| Index de `SWLogin` | Page | Widgets |
|---|---|---|
| 0 | `PLoginConnexion` | `txtIdConnexionLogin`, `txtMdpConnexionLogin`, `btnMdpOublieLogin`, `btnConnexionLogin`, `btnFaceIdConnexionLogin`, `btnVersInscriptionLogin` |
| 1 | `PLoginInscription` | `txtIdInscriptionLogin`, `txtMdpInscriptionLogin`, `txtConfirmationInscriptionLogin`, `btnInscriptionLogin`, `btnFaceIdInscriptionLogin`, `btnVersConnexionLogin` |

Les slots suivent la convention du guide `on_<nomObjet>_clicked()` (connexion automatique de Qt) :
si vous renommez un widget, renommez aussi son slot.

## Contrat de navigation (à respecter par toute l'équipe)

Stacked widget principal du projet d'intégration : **`SWHydroSync`**

| Index | Page | Contenu |
|---|---|---|
| 0 | `PLogin` | Module Login (ce projet) |
| 1 | `PEmpl` | Module Employé |
| 2 | `PMaint` | Module Équipements & Maintenance |
| 3 | `PStock` | Module Stock |
| 4 | `PClient` | Module Clients |
| 5 | `PParam` | Paramètre |

- **Connexion réussie** ➜ le code fait `SWHydroSync->setCurrentIndex(1)` (page Employé). Il cherche `SWHydroSync` par son nom : dans ce projet local il n'existe pas, donc seul un message s'affiche ; dans le projet intégré, la navigation marche **sans rien modifier**.
- **Déconnexion** (bouton du menu de chaque module) ➜ `ui->SWHydroSync->setCurrentIndex(0);` (retour à `PLogin`).

## Intégration pas à pas

1. **Projet HydroSync** (créé une seule fois par le responsable d'intégration) : `hydrosync.ui` avec `SWHydroSync` et les 6 pages ci-dessus, dans cet ordre.
2. **Fichiers** : copier dans le projet HydroSync `loginfondwidget.h`, `loginfondwidget.cpp`, `login.qrc` et le dossier `images_login/`. Dans `HydroSync.pro`, ajouter :
   ```
   SOURCES   += loginfondwidget.cpp
   HEADERS   += loginfondwidget.h
   RESOURCES += login.qrc
   ```
3. **Interface** : dans `login.ui`, sélectionner **`containerLogin`** ➜ `Ctrl+C`. Dans `hydrosync.ui`, sélectionner la page **`PLogin`** ➜ `Ctrl+V`. Puis clic droit sur `PLogin` ➜ *Disposition* ➜ *Mettre en forme dans une grille* (et marges à 0) pour que le module remplisse la page.
   Vérifier dans l'inspecteur d'objets qu'**aucun nom ne se termine par `_2`**, et que `containerLogin` est bien de classe `LoginFondWidget` (sinon : clic droit ➜ *Promouvoir en…* ➜ classe `LoginFondWidget`, fichier d'en-tête `loginfondwidget.h`).
4. **Code** :
   - `login.cpp` : copier les `#include` du bloc « MODULE LOGIN » en haut de `hydrosync.cpp`.
   - `login.h` : copier le bloc entre « MODULE LOGIN : déclarations » dans `hydrosync.h` (sections `private slots` et `private`).
   - `login.cpp` : copier tout ce qui suit le grand commentaire « MODULE LOGIN » (constantes, `initialiserModuleLogin`, slots…) à la fin de `hydrosync.cpp`, puis remplacer `Login::` par `HydroSync::` (`Ctrl+H`).
   - Dans le **constructeur de `HydroSync`**, juste après `ui->setupUi(this);` ajouter : `initialiserModuleLogin();`
5. **Tester** : lancer, vérifier que le login s'affiche en premier, que Connexion ↔ Inscription marche, et que « Log in » (ID + mot de passe remplis) ouvre la page Employé.
6. **Git** : `git pull` avant de commencer, intégrer **un membre à la fois**, puis `git add .`, `git commit -m "Intégration module Login"`, `git push`.

## Compiler ce projet seul

1. Qt 6.x avec le kit MinGW 64-bit, dans un chemin sans accent.
2. Qt Creator ➜ *Ouvrir un fichier ou un projet* ➜ `HydroSync_Login.pro`.
3. `Ctrl+B` pour compiler, `Ctrl+R` pour lancer.

## Limites actuelles

- Aucune base de données : la connexion accepte tout ID et tout mot de passe non vides, l'inscription n'enregistre rien.
- Face ID et « Mot de passe oublié » affichent seulement un message.
