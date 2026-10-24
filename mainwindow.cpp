#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "patientstorage.h"
#include "patienttablemodel.h"

#include <utility>
#include <QMessageBox>
#include <QString>
#include <QVariant>
#include <QInputDialog>
#include <QLineEdit>
#include <QFileDialog>
#include <QAbstractItemView>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QSortFilterProxyModel>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    patientTableModel_ = new PatientTableModel(this);

    patientProxyModel_ = new QSortFilterProxyModel(this);
    patientProxyModel_->setSourceModel(patientTableModel_);
    patientProxyModel_->setFilterCaseSensitivity(Qt::CaseInsensitive);
    patientProxyModel_->setSortCaseSensitivity(Qt::CaseInsensitive);
    patientProxyModel_->setFilterKeyColumn(-1);

    ui->patientsTable->setModel(patientProxyModel_);

    ui->patientsTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->patientsTable->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->patientsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->patientsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->patientsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    ui->patientsTable->setSortingEnabled(true);
    ui->patientsTable->sortByColumn(0, Qt::AscendingOrder);

    refreshPatientsList();

    setUnsavedChanges(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refreshPatientsList(){
    patientTableModel_->setPatients(patientManager_.snapshot().patients);
    patientProxyModel_->setFilterFixedString(ui->searchPatientEdit->text().trimmed());
}

void MainWindow::on_aboutButton_clicked()
{
    QMessageBox::information(
        this,
        "MedDesk",
        "Aplicatie pentru gestionarea pacientilor si programarilor.");
}


void MainWindow::on_addPatientButton_clicked()
{
    const QString name = ui->patientNameEdit->text().trimmed();

    try{
        patientManager_.addPatient(name.toStdString());
        setUnsavedChanges(true);

        refreshPatientsList();

        ui->patientNameEdit->clear();
        ui->patientNameEdit->setFocus();
    }
    catch (const std::exception& error){
        QMessageBox::warning(
            this,
            "Eroare la adaugare",
            QString::fromUtf8(error.what())
        );
    }

}


void MainWindow::on_deletePatientButton_clicked()
{
    const auto id = selectedPatientId();

    if(!id.has_value()){
        QMessageBox::warning(
            this,
            "Selectie necesara",
            "Selecteaza un pacient"
        );
        return;
    }

    if(!patientManager_.removePatient(*id)){
        QMessageBox::warning(
            this,
            "Pacient inexistent",
            "Pacientul selectat nu a fost gasit."
        );
        return;
    }

    setUnsavedChanges(true);

    refreshPatientsList();
}


void MainWindow::on_editPatientButton_clicked()
{
    const auto id = selectedPatientId();

    if(!id.has_value()){
        QMessageBox::warning(
            this,
            "Selectie necesara",
            "Selecteaza un pacient"
        );
        return;
    }

    const auto patient = patientManager_.findPatient(*id);

    if(!patient.has_value()){
        QMessageBox::warning(
            this,
            "Pacient inexistent",
            "Pacientul selectat nu mai exista."
        );
        return;
    }

    bool accepted = false;

    const QString name = QInputDialog::getText(
        this,
        "Editeaza pacient",
        "Numele pacientului:",
        QLineEdit::Normal,
        QString::fromStdString(patient->getName()),
        &accepted
    ).trimmed();

    if(!accepted){
        return;
    }

    try{
        if(!patientManager_.renamePatient(*id, name.toStdString())){
            QMessageBox::warning(
                this,
                "Eroare",
                "Pacientul selectat nu a fost gasit."
                );
            return;
        }

        if(name.toStdString() != patient->getName()){
            setUnsavedChanges(true);
        }

        refreshPatientsList();
    }
    catch (const std::exception& error){
        QMessageBox::warning(
            this,
            "Date incomplete",
            QString::fromUtf8(error.what())
        );
    }
}


void MainWindow::on_searchPatientEdit_textChanged(const QString& text)
{
    patientProxyModel_->setFilterFixedString(text.trimmed());
}


void MainWindow::on_savePatientsButton_clicked()
{
    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "Salveaza pacientii",
        "patients.json",
        "Fisiere JSON (*.json)");

    if(filePath.isEmpty()){
        return;
    }
    try{
        const PatientSnapshot snapshot = patientManager_.snapshot();

        PatientStorage::save(filePath, snapshot);
        setUnsavedChanges(false);

        QMessageBox::information(
            this,
            "Salvare reusita",
            "Pacientii au fost salvati.");
    }
    catch (const std::exception& error){
        QMessageBox::critical(
            this,
            "Eroare la salvare",
            QString::fromUtf8(error.what()));
    }
}


void MainWindow::on_loadPatientsButton_clicked()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        "Incarca pacientii",
        "",
        "Fisiere JSON (*.json)");

    if(filePath.isEmpty()){
        return;
    }

    if (!confirmDiscardChanges())
    {
        return;
    }

    try{
        PatientSnapshot snapshot = PatientStorage::load(filePath);

        patientManager_.restore(std::move(snapshot));
        setUnsavedChanges(false);
        refreshPatientsList();

        QMessageBox::information(
            this,
            "Incarcare reusita",
            "Pacientii au fost incarcati.");
    }
    catch (const std::exception& error){
        QMessageBox::critical(
            this,
            "Eroare la incarcare",
            QString::fromUtf8(error.what()));
    }
}

std::optional<int> MainWindow::selectedPatientId() const{
    const QModelIndexList selectedRows = ui->patientsTable->selectionModel()->selectedRows();

    if(selectedRows.isEmpty()){
        return std::nullopt;
    }

    const QModelIndex index = selectedRows.first();

    bool valid = false;
    const int id = index.data(Qt::UserRole).toInt(&valid);

    if(!valid){
        return std::nullopt;
    }

    return id;
}

void MainWindow::setUnsavedChanges(bool modified)
{
    hasUnsavedChanges_ = modified;

    const QString marker = modified ? " *" : "";

    setWindowTitle(
        QString("MedDesk - Gestionare pacienti%1").arg(marker));
}

bool MainWindow::confirmDiscardChanges()
{
    if (!hasUnsavedChanges_)
    {
        return true;
    }

    const auto answer = QMessageBox::warning(
        this,
        "Modificari nesalvate",
        "Exista modificari nesalvate. "
        "Continui fara sa le salvezi?",
        QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Cancel);

    return answer == QMessageBox::Discard;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (confirmDiscardChanges())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}