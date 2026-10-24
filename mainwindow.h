#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "patientmanager.h"
#include <QMainWindow>
#include <QString>

#include<optional>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class PatientTableModel;
class QSortFilterProxyModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_aboutButton_clicked();

    void on_addPatientButton_clicked();

    void on_deletePatientButton_clicked();

    void on_editPatientButton_clicked();

    void on_searchPatientEdit_textChanged(const QString &text);

    void on_savePatientsButton_clicked();

    void on_loadPatientsButton_clicked();

private:
    Ui::MainWindow *ui;
    PatientManager patientManager_;
    PatientTableModel* patientTableModel_ = nullptr;
    QSortFilterProxyModel* patientProxyModel_ = nullptr;


    void refreshPatientsList();

    std::optional<int> selectedPatientId() const;
};
#endif // MAINWINDOW_H
