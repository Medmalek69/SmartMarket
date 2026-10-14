#include "main.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QPdfWriter>
#include <QTextDocument>
#include <QPainter>
#include <QPainterPath>
#include <QDateTime>
#include <algorithm>

// ============================================================================
// WIDGETS GRAPHIQUES (Jauge, Stats Dynamiques, Map Tunisie)
// ============================================================================

class GaugeWidget : public QWidget {
    QVector<Client> *clientsRef;
public:
    GaugeWidget(QVector<Client> *clients, QWidget *parent = nullptr)
        : QWidget(parent), clientsRef(clients) {
        setMinimumHeight(190);
    }
    void refreshGauge() { update(); }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        double totalImpaye = 0;
        int nbAlertes = 0;
        if (clientsRef) {
            for(const auto &c : *clientsRef) {
                if(c.solde < 0) totalImpaye += c.solde;
                if(c.retardJours > 0) nbAlertes++;
            }
        }

        int w = width();
        int r = qMin(w - 40, height() * 2 - 60);
        QRectF arcRect((w - r) / 2.0, 20, r, r);

        QPen pen;
        pen.setWidth(18);
        pen.setCapStyle(Qt::FlatCap);

        pen.setColor(QColor(230, 57, 70));
        p.setPen(pen);
        p.drawArc(arcRect, 120 * 16, 60 * 16);

        pen.setColor(QColor(255, 183, 3));
        p.setPen(pen);
        p.drawArc(arcRect, 60 * 16, 60 * 16);

        pen.setColor(QColor(42, 157, 143));
        p.setPen(pen);
        p.drawArc(arcRect, 0 * 16, 60 * 16);

        p.setPen(QColor(34, 34, 34));
        p.setFont(QFont("Segoe UI", 10, QFont::Medium));
        p.drawText(QRect(0, 70, w, 25), Qt::AlignCenter, "Solde Total Impayé");

        p.setFont(QFont("Segoe UI", 18, QFont::Bold));
        p.drawText(QRect(0, 95, w, 35), Qt::AlignCenter, QString::number(totalImpaye, 'f', 0) + " TND");

        QRect badgeRect((w - 170) / 2, 140, 170, 28);
        p.setBrush(QColor(217, 4, 41));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(badgeRect, 6, 6);

        p.setPen(Qt::white);
        p.setFont(QFont("Segoe UI", 9, QFont::Bold));
        p.drawText(badgeRect, Qt::AlignCenter, QString::number(nbAlertes) + " Alertes de Retard !");
    }
};

class StatsChartsWidget : public QWidget {
    QVector<Client> *clientsRef;
public:
    StatsChartsWidget(QVector<Client> *clients, QWidget *parent = nullptr)
        : QWidget(parent), clientsRef(clients) {
        setMinimumSize(380, 360);
    }
    void refreshStats() { update(); }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        int ent = 0, rev = 0, part = 0;
        if (clientsRef) {
            for(const auto &c : *clientsRef) {
                if(c.type == "Entreprise") ent++;
                else if(c.type == "Revendeur") rev++;
                else part++;
            }
        }
        int total = qMax(1, ent + rev + part);

        QRectF donutRect(40, 15, 140, 140);
        int thickness = 28;

        auto drawSlice = [&](int startAngle, int spanAngle, QColor color) {
            QPen pen(color, thickness, Qt::SolidLine, Qt::FlatCap);
            p.setPen(pen);
            p.drawArc(donutRect.adjusted(thickness/2, thickness/2, -thickness/2, -thickness/2),
                      startAngle * 16, spanAngle * 16);
        };

        int angleEnt = int((double(ent) / total) * 360);
        int angleRev = int((double(rev) / total) * 360);
        int anglePart = 360 - angleEnt - angleRev;

        int currentAngle = 90;
        drawSlice(currentAngle, -angleEnt, QColor(92, 118, 141));
        currentAngle -= angleEnt;
        drawSlice(currentAngle, -angleRev, QColor(233, 169, 59));
        currentAngle -= angleRev;
        drawSlice(currentAngle, -anglePart, QColor(23, 162, 147));

        auto drawLegend = [&](int y, QColor c, QString label, int val) {
            p.setBrush(c);
            p.setPen(Qt::NoPen);
            p.drawRoundedRect(210, y, 16, 16, 3, 3);
            p.setPen(QColor(34, 34, 34));
            p.setFont(QFont("Segoe UI", 10));
            p.drawText(235, y + 13, QString("%1 (%2)").arg(label).arg(val));
        };
        drawLegend(45, QColor(92, 118, 141), "Entreprise", ent);
        drawLegend(75, QColor(233, 169, 59), "Revendeur", rev);
        drawLegend(105, QColor(23, 162, 147), "Particulier", part);

        p.setPen(QColor(34, 34, 34));
        p.setFont(QFont("Segoe UI", 13, QFont::Bold));
        p.drawText(20, 195, "Évolution des Commandes");

        int gx = 50, gy = 220, gw = width() - 80, gh = 110;
        p.setPen(QPen(QColor(204, 204, 204), 1));
        for (int i = 0; i <= 4; ++i) {
            int y = gy + gh - (i * gh / 4);
            p.drawLine(gx, y, gx + gw, y);
            p.setPen(QColor(34, 34, 34));
            p.setFont(QFont("Segoe UI", 8));
            p.drawText(gx - 30, y + 4, QString::number(i * 10));
            p.setPen(QPen(QColor(204, 204, 204), 1));
        }

        QStringList mois = {"Jan", "Fév", "Mar", "Avr", "Mai", "Juin"};
        QVector<int> valeurs = {12, 18, 25, 20, 28, int(clientsRef ? clientsRef->size() * 5 + 10 : 30)};
        QVector<QPointF> pts;

        for (int i = 0; i < mois.size(); ++i) {
            double x = gx + (i * gw / double(mois.size() - 1));
            double y = gy + gh - (valeurs[i] / 40.0) * gh;
            pts.append(QPointF(x, y));

            p.setPen(QColor(34, 34, 34));
            p.setFont(QFont("Segoe UI", 9));
            p.drawText(int(x) - 12, gy + gh + 20, mois[i]);
        }

        p.setPen(QPen(QColor(74, 137, 220), 3));
        for (int i = 0; i < pts.size() - 1; ++i) {
            p.drawLine(pts[i], pts[i+1]);
        }
        p.setBrush(QColor(43, 88, 118));
        for (const auto &pt : pts) {
            p.drawEllipse(pt, 4, 4);
        }
    }
};

class TunisiaMapWidget : public QWidget {
public:
    TunisiaMapWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setMinimumSize(380, 340);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        p.setBrush(QColor(203, 220, 240));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(rect(), 10, 10);

        QPainterPath land;
        land.moveTo(170, 35);
        land.lineTo(210, 40);
        land.lineTo(245, 65);
        land.lineTo(235, 100);
        land.lineTo(240, 145);
        land.lineTo(220, 190);
        land.lineTo(190, 240);
        land.lineTo(165, 290);
        land.lineTo(120, 260);
        land.lineTo(135, 180);
        land.lineTo(110, 120);
        land.lineTo(140, 50);
        land.closeSubpath();

        p.setBrush(QColor(244, 243, 239));
        p.setPen(QPen(QColor(160, 160, 160), 1.5));
        p.drawPath(land);

        auto drawPin = [&](int x, int y, QColor color, QString city) {
            p.setBrush(color);
            p.setPen(QPen(Qt::white, 1.5));
            p.drawEllipse(QPoint(x, y), 7, 7);
            p.setBrush(Qt::white);
            p.drawEllipse(QPoint(x, y), 2, 2);

            p.setPen(QColor(20, 20, 20));
            p.setFont(QFont("Segoe UI", 8, QFont::Bold));
            p.drawText(x + 10, y + 4, city);
        };

        drawPin(175, 48, QColor(217, 83, 79), "Bizerte");
        drawPin(198, 72, QColor(42, 157, 143), "Tunis");
        drawPin(182, 88, QColor(244, 162, 97), "Ariana");
        drawPin(230, 80, QColor(43, 123, 186), "Hammamet");
        drawPin(225, 135, QColor(29, 78, 137), "Sousse");
        drawPin(160, 155, QColor(233, 196, 106), "Kairouan");
        drawPin(210, 185, QColor(56, 176, 0), "Sfax");

        QRect bubble(130, 210, 190, 45);
        p.setBrush(Qt::white);
        p.setPen(QPen(QColor(204, 204, 204), 1));
        p.drawRoundedRect(bubble, 6, 6);

        p.setPen(QColor(20, 20, 20));
        p.setFont(QFont("Segoe UI", 8, QFont::Bold));
        p.drawText(bubble.adjusted(8, 6, -5, -5), "✓ 001 Mohamed Ben Salah\n   - Tunis (Livré)");
    }
};

// ============================================================================
// MAINWINDOW METHODS
// ============================================================================

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), gaugeWidgetRef(nullptr), statsWidgetRef(nullptr) {
    setWindowTitle("HydroSync - Gestion des Clients");
    resize(1220, 740);

    initData();
    setupUI();
    switchTab(0);
}

MainWindow::~MainWindow() {}

void MainWindow::initData() {
    clients = {
        {"001", "Mohamed Ben Salah", "14 Ave. Habib Bourguiba, Tunis", "Tunis", "71 234 567", "Particulier", -2450, 15, "Livré"},
        {"002", "Fatma Trabelsi", "3 Rue de Marseille, Sousse", "Sousse", "55 876 543", "Entreprise", -5000, 30, "En route"},
        {"003", "Bilel Mejri", "Rt. de Teniour Km 5, Sfax", "Sfax", "98 345 678", "Entreprise", -1200, 7, "Livré"},
        {"004", "Amira Khemiri", "20 Cité El Ghazala, Ariana", "Ariana", "20 456 789", "Particulier", 4800, 0, "En cours"},
        {"005", "Kais Saidi", "Medina de Kairouan", "Kairouan", "73 567 890", "Revendeur", -850, 5, "En attente"},
        {"006", "Yasmine Bouzid", "8 Blvd. de l'Environnement", "Hammamet", "22 678 901", "Entreprise", 5500, 0, "Livré"},
        {"007", "Mourad Belkhodja", "14 Rue Mohamed V., Bizerte", "Bizerte", "50 789 012", "Entreprise", 2400, 0, "En attente"}
    };

    historiqueLogs = {
        "📝 001 Mohamed Ben Salah — Nouvelle commande passée (1 900 TND) - Il y a 1h",
        "💶 002 Fatma Trabelsi — Paiement reçu (2 500 TND) - Il y a 3h",
        "📅 004 Amira Khemiri — Contrat mis à jour - Hier à 16h00",
        "🚚 003 Bilel Mejri — Livraison effectuée à Sfax - Hier à 11h30",
        "👤 006 Yasmine Bouzid — Nouveau client ajouté - 12 Oct"
    };
}

void MainWindow::setupUI() {
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(createSidebar());

    // Utilisation de border-image pour le background back.jpg (comme le code de ton ami)
    QWidget *rightContainer = new QWidget();
    rightContainer->setObjectName("rightContainer");
    rightContainer->setAttribute(Qt::WA_StyledBackground, true);
    rightContainer->setStyleSheet(
        "#rightContainer {"
        "  border-image: url(:/back.jpg) 0 0 0 0 stretch stretch;"
        "}"
        );

    QVBoxLayout *rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    rightLayout->addWidget(createTopHeader());
    rightLayout->addWidget(createTabBar());

    stackedWidget = new QStackedWidget();
    stackedWidget->setStyleSheet("background: transparent;");
    stackedWidget->addWidget(createPageListe());
    stackedWidget->addWidget(createPageGestion());
    stackedWidget->addWidget(createPageStatistiques());
    stackedWidget->addWidget(createPageCarte());

    rightLayout->addWidget(stackedWidget, 1);
    mainLayout->addWidget(rightContainer, 1);
}

QWidget* MainWindow::createSidebar() {
    QWidget *sidebar = new QWidget();
    sidebar->setFixedWidth(220);
    sidebar->setStyleSheet("background-color: #ffffff; border-right: 1px solid #dcdcdc;");

    QVBoxLayout *layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(0, 15, 0, 20);
    layout->setSpacing(8);

    QLabel *logo = new QLabel("⚙️ HydroSync");
    logo->setStyleSheet("font-size: 22px; font-weight: bold; color: #196496; padding: 15px 20px;");
    layout->addWidget(logo);

    QStringList menus = {"👥  Employé", "🔧  Équipements &\n      Maintenance", "📦  Stock", "🤝  Clients", "⚙️  Parametre"};
    for (int i = 0; i < menus.size(); ++i) {
        QPushButton *btn = new QPushButton(menus[i]);
        btn->setCursor(Qt::PointingHandCursor);
        if (i == 3) {
            btn->setStyleSheet(
                "QPushButton { background-color: #d0e1f0; color: #111; font-size: 15px; font-weight: bold;"
                "border: none; text-align: left; padding: 14px 22px; }"
                );
        } else {
            btn->setStyleSheet(
                "QPushButton { background-color: transparent; color: #333; font-size: 15px;"
                "border: none; text-align: left; padding: 14px 22px; }"
                "QPushButton:hover { background-color: #f0f4f8; }"
                );
        }
        layout->addWidget(btn);
    }

    layout->addStretch();

    QPushButton *btnLogout = new QPushButton("↪ déconnexion");
    btnLogout->setCursor(Qt::PointingHandCursor);
    btnLogout->setStyleSheet(
        "QPushButton { background-color: #246893; color: white; font-size: 14px; font-weight: bold;"
        "border-radius: 10px; padding: 10px; margin: 0 20px; }"
        "QPushButton:hover { background-color: #1b4f72; }"
        );
    connect(btnLogout, &QPushButton::clicked, this, &QWidget::close);
    layout->addWidget(btnLogout);

    return sidebar;
}

QWidget* MainWindow::createTopHeader() {
    QWidget *header = new QWidget();
    header->setFixedHeight(70);
    header->setStyleSheet("background-color: #2c73a3;");

    QHBoxLayout *layout = new QHBoxLayout(header);
    layout->setContentsMargins(30, 10, 25, 10);

    layout->addStretch(1);

    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("🔍 Rechercher un client (par Nom, Tél, ID)...");
    searchEdit->setFixedWidth(380);
    searchEdit->setStyleSheet(
        "QLineEdit { background-color: #dce4ec; border: none; border-radius: 14px;"
        "padding: 8px 15px; font-size: 14px; color: #222; }"
        );
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::onRechercherChanged);
    layout->addWidget(searchEdit);

    layout->addStretch(1);

    QLabel *userLbl = new QLabel("🔔   👤  H.Robert\n         (Technicienne de maintenance)");
    userLbl->setStyleSheet("color: white; font-size: 12px; font-weight: bold;");
    layout->addWidget(userLbl);

    return header;
}

QWidget* MainWindow::createTabBar() {
    QWidget *bar = new QWidget();
    bar->setFixedHeight(85);
    bar->setStyleSheet("background-color: rgba(238, 243, 247, 230); border-bottom: 1px solid #c5d1db;");

    QHBoxLayout *layout = new QHBoxLayout(bar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QStringList titles = {
        "☰\nListe",
        "⚙️\nGestion",
        "📊\nStatistiques",
        "🗺️\nCarte des Livraisons"
    };

    for (int i = 0; i < titles.size(); ++i) {
        QPushButton *btn = new QPushButton(titles[i]);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        connect(btn, &QPushButton::clicked, this, [this, i]() { switchTab(i); });
        tabButtons.append(btn);
        layout->addWidget(btn);
    }

    return bar;
}

void MainWindow::switchTab(int index) {
    stackedWidget->setCurrentIndex(index);

    for (int i = 0; i < tabButtons.size(); ++i) {
        if (i == index) {
            tabButtons[i]->setStyleSheet(
                "QPushButton { background-color: #365873; color: white; font-size: 16px; font-weight: bold;"
                "border: none; border-bottom: 4px solid #1c92d2; }"
                );
        } else {
            tabButtons[i]->setStyleSheet(
                "QPushButton { background-color: transparent; color: #222; font-size: 16px;"
                "border: none; }"
                "QPushButton:hover { background-color: rgba(221, 231, 240, 150); }"
                );
        }
    }
}

QWidget* MainWindow::createPageListe() {
    QWidget *page = new QWidget();
    page->setStyleSheet("background: transparent;");
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(25, 20, 25, 25);
    layout->setSpacing(15);

    QHBoxLayout *toolLayout = new QHBoxLayout();

    sortCombo = new QComboBox();
    sortCombo->addItems({"Trier par...", "Nom (A - Z)", "ID client", "Type (Entreprise/Particulier)", "Région / Gouvernorat", "Solde (Chiffre d'Affaires)"});
    sortCombo->setFixedWidth(230);
    sortCombo->setStyleSheet(
        "QComboBox { background-color: #e8eef3; border: 1px solid #b8c4ce; border-radius: 6px; padding: 8px 12px; font-size: 13px; color: #222; }"
        );
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onTrierChanged);
    toolLayout->addWidget(sortCombo);

    toolLayout->addStretch();

    QPushButton *btnAdd = new QPushButton("+ Ajouter Nouveau Client");
    btnAdd->setCursor(Qt::PointingHandCursor);
    btnAdd->setStyleSheet(
        "QPushButton { background-color: #31536b; color: white; font-weight: bold; font-size: 13px;"
        "padding: 10px 18px; border-radius: 6px; }"
        "QPushButton:hover { background-color: #243e50; }"
        );
    connect(btnAdd, &QPushButton::clicked, this, &MainWindow::onAjouterClient);
    toolLayout->addWidget(btnAdd);

    QPushButton *btnPdf = new QPushButton("📄 Exporter (PDF)");
    btnPdf->setCursor(Qt::PointingHandCursor);
    btnPdf->setStyleSheet(
        "QPushButton { background-color: #31536b; color: white; font-weight: bold; font-size: 13px;"
        "padding: 10px 18px; border-radius: 6px; }"
        "QPushButton:hover { background-color: #243e50; }"
        );
    connect(btnPdf, &QPushButton::clicked, this, &MainWindow::onExporterPDF);
    toolLayout->addWidget(btnPdf);

    layout->addLayout(toolLayout);

    tableListe = new QTableWidget();
    tableListe->setColumnCount(7);
    tableListe->setHorizontalHeaderLabels({"ID client", "Nom", "Adresse de livraison", "Téléphone", "Type", "Solde (TND)", "Actions"});
    tableListe->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableListe->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tableListe->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Fixed);
    tableListe->setColumnWidth(6, 140);
    tableListe->verticalHeader()->setVisible(false);
    tableListe->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableListe->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableListe->setStyleSheet(
        "QTableWidget { background-color: #ffffff; color: #222222; border-radius: 8px; font-size: 13px; gridline-color: #d0d7de; }"
        "QHeaderView::section { background-color: #d8dee4; color: #111; font-weight: bold; padding: 8px; border: 1px solid #c5cdd6; }"
        );

    layout->addWidget(tableListe);
    refreshTableListe();

    return page;
}

void MainWindow::refreshTableListe() {
    QString filter = searchEdit ? searchEdit->text().trimmed().toLower() : "";
    tableListe->setRowCount(0);

    for (int i = 0; i < clients.size(); ++i) {
        const Client &c = clients[i];
        if (!filter.isEmpty()) {
            if (!c.nom.toLower().contains(filter) &&
                !c.telephone.toLower().contains(filter) &&
                !c.id.toLower().contains(filter)) {
                continue;
            }
        }

        int row = tableListe->rowCount();
        tableListe->insertRow(row);
        tableListe->setRowHeight(row, 42);

        QTableWidgetItem *item0 = new QTableWidgetItem(c.id);
        QTableWidgetItem *item1 = new QTableWidgetItem(c.nom);
        QTableWidgetItem *item2 = new QTableWidgetItem(c.adresse);
        QTableWidgetItem *item3 = new QTableWidgetItem(c.telephone);
        QTableWidgetItem *item4 = new QTableWidgetItem(c.type);
        QTableWidgetItem *item5 = new QTableWidgetItem(QString::number(qAbs(c.solde), 'f', 0) + " TND");

        item0->setForeground(QColor(20, 20, 20));
        item1->setForeground(QColor(20, 20, 20));
        item2->setForeground(QColor(20, 20, 20));
        item3->setForeground(QColor(20, 20, 20));
        item4->setForeground(QColor(20, 20, 20));
        item5->setForeground(c.solde < 0 ? QColor(183, 28, 28) : QColor(27, 94, 32));

        tableListe->setItem(row, 0, item0);
        tableListe->setItem(row, 1, item1);
        tableListe->setItem(row, 2, item2);
        tableListe->setItem(row, 3, item3);
        tableListe->setItem(row, 4, item4);
        tableListe->setItem(row, 5, item5);

        QWidget *actionWidget = new QWidget();
        QHBoxLayout *actLayout = new QHBoxLayout(actionWidget);
        actLayout->setContentsMargins(4, 2, 4, 2);
        actLayout->setSpacing(4);

        QPushButton *btnView = new QPushButton("👁️");
        QPushButton *btnEdit = new QPushButton("✏️");
        QPushButton *btnDel  = new QPushButton("🗑️");

        QString btnStyle = "QPushButton { background: transparent; border: none; font-size: 15px; padding: 2px; }"
                           "QPushButton:hover { background-color: #d0e1f0; border-radius: 4px; }";
        btnView->setStyleSheet(btnStyle);
        btnEdit->setStyleSheet(btnStyle);
        btnDel->setStyleSheet(btnStyle);

        connect(btnView, &QPushButton::clicked, this, [this, i]() { onVoirClient(i); });
        connect(btnEdit, &QPushButton::clicked, this, [this, i]() { onModifierClient(i); });
        connect(btnDel,  &QPushButton::clicked, this, [this, i]() { onSupprimerClient(i); });

        actLayout->addWidget(btnView);
        actLayout->addWidget(btnEdit);
        actLayout->addWidget(btnDel);

        tableListe->setCellWidget(row, 6, actionWidget);
    }
}

QWidget* MainWindow::createPageGestion() {
    QWidget *page = new QWidget();
    page->setStyleSheet("background: transparent;");
    QHBoxLayout *layout = new QHBoxLayout(page);
    layout->setContentsMargins(25, 20, 25, 25);
    layout->setSpacing(20);

    QString cardStyle = "QWidget#card { background-color: #ffffff; border-radius: 10px; }";

    QWidget *leftCard = new QWidget();
    leftCard->setObjectName("card");
    leftCard->setStyleSheet(cardStyle);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftCard);
    leftLayout->setContentsMargins(20, 20, 20, 20);

    QLabel *titleLeft = new QLabel("Gestion des Soldes et Relances\n<span style='font-size:13px; font-weight:normal; color:#555;'>Suivi des Soldes Clients</span>");
    titleLeft->setStyleSheet("font-size: 18px; font-weight: bold; color: #111;");
    leftLayout->addWidget(titleLeft);

    tableGestion = new QTableWidget();
    tableGestion->setColumnCount(6);
    tableGestion->setHorizontalHeaderLabels({"ID", "Nom", "Téléphone", "Solde Actuel (TND)", "Statut", "Action"});
    tableGestion->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableGestion->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tableGestion->verticalHeader()->setVisible(false);
    tableGestion->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableGestion->setStyleSheet("QTableWidget { background: #ffffff; color: #222; border: none; font-size: 13px; }");
    leftLayout->addWidget(tableGestion);

    QWidget *rightCard = new QWidget();
    rightCard->setObjectName("card");
    rightCard->setStyleSheet(cardStyle);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightCard);
    rightLayout->setContentsMargins(20, 20, 20, 20);

    QLabel *titleRight = new QLabel("Relances Automatiques &\nIndicateur Global");
    titleRight->setStyleSheet("font-size: 18px; font-weight: bold; color: #111;");
    rightLayout->addWidget(titleRight);

    gaugeWidgetRef = new GaugeWidget(&clients);
    rightLayout->addWidget(gaugeWidgetRef);

    QLabel *lblHist = new QLabel("Historique de Relances");
    lblHist->setStyleSheet("font-size: 15px; font-weight: bold; color: #111;");
    rightLayout->addWidget(lblHist);

    listHistoriqueRelances = new QListWidget();
    listHistoriqueRelances->setStyleSheet("QListWidget { background: #ffffff; color: #222; border: none; font-size: 13px; }");
    listHistoriqueRelances->addItem("001  Mohamed Ben Salah   -2 450 TND   Retard (15j)");
    listHistoriqueRelances->addItem("003  Bilel Mejri                   -1 200 TND   Retard (7j)");
    listHistoriqueRelances->addItem("002  Fatma Trabelsi            -5 000 TND   Retard (30j)");
    rightLayout->addWidget(listHistoriqueRelances);

    QPushButton *btnRelanceAuto = new QPushButton("Envoyer Relances Automatiques 📩");
    btnRelanceAuto->setCursor(Qt::PointingHandCursor);
    btnRelanceAuto->setStyleSheet(
        "QPushButton { background-color: #31536b; color: white; font-size: 14px; font-weight: bold;"
        "padding: 10px; border-radius: 6px; }"
        "QPushButton:hover { background-color: #223b4c; }"
        );
    connect(btnRelanceAuto, &QPushButton::clicked, this, &MainWindow::onRelancerTous);
    rightLayout->addWidget(btnRelanceAuto);

    lblSoldeGlobal = new QLabel("Dernière relance envoyée : Hier à 14h30");
    lblSoldeGlobal->setAlignment(Qt::AlignCenter);
    lblSoldeGlobal->setStyleSheet("color: #444; font-size: 12px;");
    rightLayout->addWidget(lblSoldeGlobal);

    layout->addWidget(leftCard, 6);
    layout->addWidget(rightCard, 5);

    refreshTableGestion();
    return page;
}

void MainWindow::refreshTableGestion() {
    if (!tableGestion) return;
    tableGestion->setRowCount(0);

    for (int i = 0; i < clients.size(); ++i) {
        const Client &c = clients[i];
        int row = tableGestion->rowCount();
        tableGestion->insertRow(row);
        tableGestion->setRowHeight(row, 48);

        QTableWidgetItem *item0 = new QTableWidgetItem(c.id);
        QTableWidgetItem *item1 = new QTableWidgetItem(c.nom);
        QTableWidgetItem *item2 = new QTableWidgetItem(c.telephone);

        QString soldeStr = (c.solde >= 0 ? "+" : "") + QString::number(c.solde, 'f', 0) + " TND";
        QTableWidgetItem *item3 = new QTableWidgetItem(soldeStr);
        item3->setForeground(c.solde < 0 ? QColor(183, 28, 28) : QColor(27, 94, 32));

        QString statutStr = (c.retardJours > 0) ? QString("Retard (%1j)").arg(c.retardJours) : "Payé";
        QTableWidgetItem *item4 = new QTableWidgetItem(statutStr);
        item4->setForeground(c.retardJours > 0 ? QColor(183, 28, 28) : QColor(27, 94, 32));

        item0->setForeground(QColor(20, 20, 20));
        item1->setForeground(QColor(20, 20, 20));
        item2->setForeground(QColor(20, 20, 20));

        tableGestion->setItem(row, 0, item0);
        tableGestion->setItem(row, 1, item1);
        tableGestion->setItem(row, 2, item2);
        tableGestion->setItem(row, 3, item3);
        tableGestion->setItem(row, 4, item4);

        if (c.retardJours > 0) {
            QPushButton *btnRel = new QPushButton("Relancer");
            btnRel->setCursor(Qt::PointingHandCursor);
            btnRel->setStyleSheet(
                "QPushButton { background-color: #365873; color: white; font-weight: bold; border-radius: 5px; padding: 5px 10px; }"
                "QPushButton:hover { background-color: #1f3547; }"
                );
            connect(btnRel, &QPushButton::clicked, this, [this, i]() { onRelancerClient(i); });
            tableGestion->setCellWidget(row, 5, btnRel);
        } else {
            QLabel *lblOk = new QLabel("OK");
            lblOk->setAlignment(Qt::AlignCenter);
            lblOk->setStyleSheet("color: #2e7d32; font-weight: bold;");
            tableGestion->setCellWidget(row, 5, lblOk);
        }
    }

    if (gaugeWidgetRef) {
        gaugeWidgetRef->refreshGauge();
    }
}

QWidget* MainWindow::createPageStatistiques() {
    QWidget *page = new QWidget();
    page->setStyleSheet("background: transparent;");
    QHBoxLayout *layout = new QHBoxLayout(page);
    layout->setContentsMargins(25, 20, 25, 25);
    layout->setSpacing(20);

    QString cardStyle = "QWidget#card { background-color: #ffffff; border-radius: 10px; }";

    QWidget *leftCard = new QWidget();
    leftCard->setObjectName("card");
    leftCard->setStyleSheet(cardStyle);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftCard);
    leftLayout->setContentsMargins(20, 20, 20, 20);

    QLabel *titleLeft = new QLabel("Statistiques Clients : Répartition\n& Évolution");
    titleLeft->setStyleSheet("font-size: 18px; font-weight: bold; color: #111;");
    leftLayout->addWidget(titleLeft);

    statsWidgetRef = new StatsChartsWidget(&clients);
    leftLayout->addWidget(statsWidgetRef, 1);

    QWidget *rightCard = new QWidget();
    rightCard->setObjectName("card");
    rightCard->setStyleSheet(cardStyle);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightCard);
    rightLayout->setContentsMargins(20, 20, 20, 20);

    QLabel *titleRight = new QLabel("Historique Détaillé des Clients");
    titleRight->setStyleSheet("font-size: 18px; font-weight: bold; color: #111;");
    rightLayout->addWidget(titleRight);

    listHistoriqueDetaille = new QListWidget();
    listHistoriqueDetaille->setStyleSheet(
        "QListWidget { background: #ffffff; color: #222; border: none; font-size: 13px; }"
        "QListWidget::item { padding: 10px 4px; border-bottom: 1px solid #e0e0e0; }"
        );
    for (const QString &log : historiqueLogs) {
        listHistoriqueDetaille->addItem(log);
    }
    rightLayout->addWidget(listHistoriqueDetaille, 1);

    QPushButton *btnFullHist = new QPushButton("Voir Historique Complet");
    btnFullHist->setCursor(Qt::PointingHandCursor);
    btnFullHist->setStyleSheet(
        "QPushButton { background-color: #31536b; color: white; font-size: 14px; font-weight: bold;"
        "padding: 10px; border-radius: 6px; }"
        "QPushButton:hover { background-color: #223b4c; }"
        );
    connect(btnFullHist, &QPushButton::clicked, this, &MainWindow::onVoirHistoriqueComplet);
    rightLayout->addWidget(btnFullHist);

    layout->addWidget(leftCard, 1);
    layout->addWidget(rightCard, 1);

    return page;
}

QWidget* MainWindow::createPageCarte() {
    QWidget *page = new QWidget();
    page->setStyleSheet("background: transparent;");
    QHBoxLayout *layout = new QHBoxLayout(page);
    layout->setContentsMargins(25, 20, 25, 25);
    layout->setSpacing(20);

    QString cardStyle = "QWidget#card { background-color: #ffffff; border-radius: 10px; }";

    QWidget *leftCard = new QWidget();
    leftCard->setObjectName("card");
    leftCard->setStyleSheet(cardStyle);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftCard);
    leftLayout->setContentsMargins(20, 20, 20, 20);

    QLabel *titleLeft = new QLabel("Géolocalisation des Points de\nLivraison");
    titleLeft->setStyleSheet("font-size: 18px; font-weight: bold; color: #111;");
    leftLayout->addWidget(titleLeft);

    TunisiaMapWidget *mapWidget = new TunisiaMapWidget();
    leftLayout->addWidget(mapWidget, 1);

    QWidget *rightCard = new QWidget();
    rightCard->setObjectName("card");
    rightCard->setStyleSheet(cardStyle);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightCard);
    rightLayout->setContentsMargins(20, 20, 20, 20);

    QLabel *titleRight = new QLabel("Suivi des Tournées & Livraisons");
    titleRight->setStyleSheet("font-size: 18px; font-weight: bold; color: #111;");
    rightLayout->addWidget(titleRight);

    listTournees = new QListWidget();
    listTournees->setStyleSheet(
        "QListWidget { background: #ffffff; color: #222; border: none; font-size: 13px; font-weight: 500; }"
        "QListWidget::item { padding: 10px 6px; border-bottom: 1px solid #e0e0e0; }"
        );
    listTournees->addItem("✅  001 Mohamed Ben Salah - Tunis - Livré");
    listTournees->addItem("🚚  002 Fatma Trabelsi - Sousse - En route");
    listTournees->addItem("✅  003 Bilel Mejri - Sfax - Livré");
    listTournees->addItem("🧭  004 Amira Khemiri - Ariana - En cours");
    listTournees->addItem("⏸️  005 Kais Saidi - Kairouan - En attente");
    rightLayout->addWidget(listTournees, 1);

    QPushButton *btnOptimiser = new QPushButton("Optimiser la Tournée");
    btnOptimiser->setCursor(Qt::PointingHandCursor);
    btnOptimiser->setStyleSheet(
        "QPushButton { background-color: #31536b; color: white; font-size: 14px; font-weight: bold;"
        "padding: 10px; border-radius: 6px; }"
        "QPushButton:hover { background-color: #223b4c; }"
        );
    connect(btnOptimiser, &QPushButton::clicked, this, &MainWindow::onOptimiserTournee);
    rightLayout->addWidget(btnOptimiser);

    layout->addWidget(leftCard, 1);
    layout->addWidget(rightCard, 1);

    return page;
}

void MainWindow::onRechercherChanged(const QString &) {
    refreshTableListe();
}

void MainWindow::onTrierChanged(int index) {
    if (index == 1) { // Nom (A - Z)
        std::sort(clients.begin(), clients.end(), [](const Client &a, const Client &b) {
            return a.nom < b.nom;
        });
    } else if (index == 2) { // ID client
        std::sort(clients.begin(), clients.end(), [](const Client &a, const Client &b) {
            return a.id < b.id;
        });
    } else if (index == 3) { // Type
        std::sort(clients.begin(), clients.end(), [](const Client &a, const Client &b) {
            return a.type < b.type;
        });
    } else if (index == 4) { // Région / Gouvernorat
        std::sort(clients.begin(), clients.end(), [](const Client &a, const Client &b) {
            return a.gouvernorat < b.gouvernorat;
        });
    } else if (index == 5) { // Solde
        std::sort(clients.begin(), clients.end(), [](const Client &a, const Client &b) {
            return qAbs(a.solde) > qAbs(b.solde);
        });
    }
    refreshTableListe();
    refreshTableGestion();
}

void MainWindow::onAjouterClient() {
    QDialog dlg(this);
    dlg.setWindowTitle("Ajouter Nouveau Client");
    dlg.setFixedWidth(360);

    QFormLayout *form = new QFormLayout(&dlg);
    QLineEdit *editNom = new QLineEdit();
    QLineEdit *editAdr = new QLineEdit();
    QLineEdit *editGouv = new QLineEdit("Tunis");
    QLineEdit *editTel = new QLineEdit();
    QComboBox *comboType = new QComboBox();
    comboType->addItems({"Particulier", "Entreprise", "Revendeur"});
    QLineEdit *editSolde = new QLineEdit("-1500");

    form->addRow("Nom complet :", editNom);
    form->addRow("Adresse de livraison :", editAdr);
    form->addRow("Gouvernorat :", editGouv);
    form->addRow("Téléphone :", editTel);
    form->addRow("Type :", comboType);
    form->addRow("Solde (TND) [négatif = dû] :", editSolde);

    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    form->addRow(box);

    if (dlg.exec() == QDialog::Accepted && !editNom->text().trimmed().isEmpty()) {
        QString newId = QString("%1").arg(clients.size() + 1, 3, 10, QChar('0'));
        Client c = {
            newId,
            editNom->text().trimmed(),
            editAdr->text().trimmed(),
            editGouv->text().trimmed(),
            editTel->text().trimmed(),
            comboType->currentText(),
            editSolde->text().toDouble(),
            0,
            "En attente"
        };
        clients.append(c);
        refreshTableListe();
        refreshTableGestion();
        if (statsWidgetRef) statsWidgetRef->refreshStats();
        addHistorique(QString("👤 %1 %2 — Nouveau client ajouté - Maintenant").arg(c.id, c.nom));
    }
}

void MainWindow::onVoirClient(int row) {
    if (row < 0 || row >= clients.size()) return;
    const Client &c = clients[row];
    QMessageBox::information(this, "Détails du Client",
                             QString("<b>ID :</b> %1<br>"
                                     "<b>Nom :</b> %2<br>"
                                     "<b>Adresse :</b> %3 (%4)<br>"
                                     "<b>Téléphone :</b> %5<br>"
                                     "<b>Type :</b> %6<br>"
                                     "<b>Solde :</b> %7 TND<br>"
                                     "<b>Statut Livraison :</b> %8")
                                 .arg(c.id, c.nom, c.adresse, c.gouvernorat, c.telephone, c.type)
                                 .arg(c.solde).arg(c.statutLivraison));
}

void MainWindow::onModifierClient(int row) {
    if (row < 0 || row >= clients.size()) return;
    Client &c = clients[row];

    QDialog dlg(this);
    dlg.setWindowTitle("Modifier Client - " + c.nom);
    dlg.setFixedWidth(360);

    QFormLayout *form = new QFormLayout(&dlg);
    QLineEdit *editNom = new QLineEdit(c.nom);
    QLineEdit *editAdr = new QLineEdit(c.adresse);
    QLineEdit *editTel = new QLineEdit(c.telephone);
    QComboBox *comboType = new QComboBox();
    comboType->addItems({"Particulier", "Entreprise", "Revendeur"});
    comboType->setCurrentText(c.type);
    QLineEdit *editSolde = new QLineEdit(QString::number(c.solde));

    form->addRow("Nom complet :", editNom);
    form->addRow("Adresse :", editAdr);
    form->addRow("Téléphone :", editTel);
    form->addRow("Type :", comboType);
    form->addRow("Solde (TND) [ex: 0 pour payé, négatif si dû] :", editSolde);

    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    form->addRow(box);

    if (dlg.exec() == QDialog::Accepted) {
        c.nom = editNom->text().trimmed();
        c.adresse = editAdr->text().trimmed();
        c.telephone = editTel->text().trimmed();
        c.type = comboType->currentText();
        c.solde = editSolde->text().toDouble();
        if (c.solde >= 0) {
            c.retardJours = 0;
        }

        refreshTableListe();
        refreshTableGestion();
        if (statsWidgetRef) statsWidgetRef->refreshStats();
        addHistorique(QString("✏️ %1 %2 — Fiche client modifiée (Solde: %3 TND)").arg(c.id, c.nom).arg(c.solde));
    }
}

void MainWindow::onSupprimerClient(int row) {
    if (row < 0 || row >= clients.size()) return;
    QString nom = clients[row].nom;
    if (QMessageBox::question(this, "Confirmation de suppression",
                              "Voulez-vous vraiment supprimer le client " + nom + " ?") == QMessageBox::Yes) {
        addHistorique(QString("🗑️ %1 %2 — Client supprimé").arg(clients[row].id, nom));
        clients.removeAt(row);
        refreshTableListe();
        refreshTableGestion();
        if (statsWidgetRef) statsWidgetRef->refreshStats();
    }
}

void MainWindow::onExporterPDF() {
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter la liste en PDF", "Repertoire_Clients_HydroSync.pdf", "Fichiers PDF (*.pdf)");
    if (fileName.isEmpty()) return;

    QPdfWriter pdfWriter(fileName);
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));
    pdfWriter.setResolution(300);

    QString html = "<h2 style='color:#2c73a3;'>HydroSync - Répertoire des Clients</h2>";
    html += "<table border='1' cellspacing='0' cellpadding='6' width='100%'>";
    html += "<tr style='background-color:#d8dee4;'><th>ID</th><th>Nom</th><th>Adresse</th><th>Téléphone</th><th>Type</th><th>Solde (TND)</th></tr>";

    for (const Client &c : clients) {
        html += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6 TND</td></tr>")
        .arg(c.id, c.nom, c.adresse, c.telephone, c.type)
            .arg(QString::number(c.solde, 'f', 0));
    }
    html += "</table>";

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&pdfWriter);

    QMessageBox::information(this, "Export PDF réussi", "Le répertoire des clients a été exporté avec succès dans :\n" + fileName);
}

void MainWindow::onRelancerClient(int row) {
    if (row < 0 || row >= clients.size()) return;
    const Client &c = clients[row];
    QString heure = QTime::currentTime().toString("HH:mm");
    listHistoriqueRelances->insertItem(0, QString("📩 %1 %2 — Relance envoyée à %3").arg(c.id, c.nom, heure));
    lblSoldeGlobal->setText("Dernière relance envoyée : Aujourd'hui à " + heure);
    addHistorique(QString("🔔 %1 %2 — Relance d'impayé envoyée (%3 TND)").arg(c.id, c.nom).arg(c.solde));
    QMessageBox::information(this, "Relance envoyée", "Un SMS et un Email de relance ont été envoyés à " + c.nom + " (" + c.telephone + ").");
}

void MainWindow::onRelancerTous() {
    int count = 0;
    QString heure = QTime::currentTime().toString("HH:mm");
    for (const Client &c : clients) {
        if (c.retardJours > 0) {
            count++;
            listHistoriqueRelances->insertItem(0, QString("📩 %1 %2 — Relance auto envoyée (%3)").arg(c.id, c.nom, heure));
        }
    }
    lblSoldeGlobal->setText("Dernière relance globale : Aujourd'hui à " + heure);
    QMessageBox::information(this, "Relances Automatiques",
                             QString("%1 relances automatiques ont été envoyées avec succès aux clients en retard.").arg(count));
}

void MainWindow::onVoirHistoriqueComplet() {
    QMessageBox::information(this, "Historique Complet des Clients", historiqueLogs.join("\n\n"));
}

void MainWindow::onOptimiserTournee() {
    listTournees->clear();
    listTournees->addItem("🚀 1. 007 Mourad Belkhodja - Bizerte (Départ 08h00)");
    listTournees->addItem("🚀 2. 004 Amira Khemiri - Ariana (Passage 09h15)");
    listTournees->addItem("✅ 3. 001 Mohamed Ben Salah - Tunis (Livré)");
    listTournees->addItem("✅ 4. 006 Yasmine Bouzid - Hammamet (Livré)");
    listTournees->addItem("🚚 5. 002 Fatma Trabelsi - Sousse (Arrivée 11h30)");
    listTournees->addItem("🧭 6. 005 Kais Saidi - Kairouan (Arrivée 13h00)");
    listTournees->addItem("✅ 7. 003 Bilel Mejri - Sfax (Livré)");

    QMessageBox::information(this, "Optimisation de la Tournée",
                             "L'itinéraire de livraison a été optimisé du Nord au Sud (Bizerte ➔ Ariana ➔ Tunis ➔ Hammamet ➔ Sousse ➔ Kairouan ➔ Sfax) pour réduire le temps de trajet de 28%.");
}

void MainWindow::addHistorique(const QString &eventText) {
    historiqueLogs.prepend(eventText);
    if (listHistoriqueDetaille) {
        listHistoriqueDetaille->insertItem(0, eventText);
    }
}