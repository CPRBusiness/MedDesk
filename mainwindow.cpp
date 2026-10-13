#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <algorithm>
#include <stdexcept>
#include <QMessageBox>
#include <QString>
#include <QListWidgetItem>
#include <QVariant>
#include <QInputDialog>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::filterPatients(const QString& text){
    const QString query = text.trimmed();

    for(int i = 0; i < ui->patientList->count(); i++){
        auto* item = ui->patientList->item(i);

        const bool matches = item->text().contains(query, Qt::CaseInsensitive);

        item->setHidden(!matches);
    }

    auto*current = ui->patientList->currentItem();

    if(current && current->isHidden()){
        ui->patientList->clearSelection();
        ui->patientList->setCurrentRow(-1);
    }
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
        const Patient patient = patientManager_.addPatient(name.toStdString());

        const QString label = QString("%1 - %2")
                                  .arg(patient.getId())
                                  .arg(QString::fromStdString((patient.getName())));

        auto* item = new QListWidgetItem(label);
        item->setData(Qt::UserRole, patient.getId());

        ui->patientList->addItem(item);

        ui->patientNameEdit->clear();
        ui->patientNameEdit->setFocus();

        filterPatients(ui->searchPatientEdit->text());
    }
    catch (const std::invalid_argument& error){
        QMessageBox::warning(
            this,
            "Date incomplete",
            QString::fromUtf8(error.what())
        );
    }

}


void MainWindow::on_deletePatientButton_clicked()
{
    auto* item = ui->patientList->currentItem();

    if(!item){
        QMessageBox::warning(
            this,
            "Selectie necesara",
            "Selecteaza un pacient"
        );
        return;
    }

    const int id = item->data(Qt::UserRole).toInt();

    if(!patientManager_.removePatient(id)){
        QMessageBox::warning(
            this,
            "Eroare",
            "Pacientul selectat nu a fost gasit."
        );
        return;
    }

    const int row = ui->patientList->row(item);
    delete ui->patientList->takeItem(row);
}


void MainWindow::on_editPatientButton_clicked()
{
    auto* item = ui->patientList->currentItem();

    if(!item){
        QMessageBox::warning(
            this,
            "Selectie necesara",
            "Selecteaza un pacient"
        );
        return;
    }

    const int id = item->data(Qt::UserRole).toInt();

    const auto patient = patientManager_.findPatient(id);

    if(!patient){
        QMessageBox::warning(
            this,
            "Selectie necesara",
            "Selecteaza un pacient."
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
        if(!patientManager_.renamePatient(id, name.toStdString())){
            QMessageBox::warning(
                this,
                "Eroare",
                "Pacientul selectat nu a fost gasit."
                );
            return;
        }
        item->setText(
            QString("%1 - %2")
                .arg(id)
                .arg(name)
            );

        filterPatients(ui->searchPatientEdit->text());
    }
    catch (const std::invalid_argument& error){
        QMessageBox::warning(
            this,
            "Date incomplete",
            QString::fromUtf8(error.what())
        );
    }
}


void MainWindow::on_searchPatientEdit_textChanged(const QString& text)
{
    filterPatients(text);
}

