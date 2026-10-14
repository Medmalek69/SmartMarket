#ifndef MAIN_H
#define MAIN_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QVector>
#include <QString>

struct Client {
    QString id;
    QString nom;
    QString adresse;
    QString gouvernorat;
    QString telephone;
    QString type;
    double solde;
    int retardJours;
    QString statutLivraison;
};

class GaugeWidget;
class StatsChartsWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void switchTab(int index);
    void onRechercherChanged(const QString &text);
    void onTrierChanged(int index);
    void onAjouterClient();
    void onVoirClient(int row);
    void onModifierClient(int row);
    void onSupprimerClient(int row);
    void onExporterPDF();
    void onRelancerClient(int row);
    void onRelancerTous();
    void onVoirHistoriqueComplet();
    void onOptimiserTournee();

private:
    void initData();
    void setupUI();
    QWidget* createSidebar();
    QWidget* createTopHeader();
    QWidget* createTabBar();

    QWidget* createPageListe();
    QWidget* createPageGestion();
    QWidget* createPageStatistiques();
    QWidget* createPageCarte();

    void refreshTableListe();
    void refreshTableGestion();
    void addHistorique(const QString &eventText);

    QVector<Client> clients;
    QStringList historiqueLogs;

    QStackedWidget *stackedWidget;
    QVector<QPushButton*> tabButtons;
    QLineEdit *searchEdit;
    QComboBox *sortCombo;

    QTableWidget *tableListe;
    QTableWidget *tableGestion;
    QListWidget *listHistoriqueRelances;
    QListWidget *listHistoriqueDetaille;
    QListWidget *listTournees;
    QLabel *lblSoldeGlobal;

    GaugeWidget *gaugeWidgetRef;
    StatsChartsWidget *statsWidgetRef;
};

#endif // MAIN_H