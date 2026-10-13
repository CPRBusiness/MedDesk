#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "patientmanager.h"
#include <QMainWindow>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

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

private:
    Ui::MainWindow *ui;
    PatientManager patientManager_;

    void filterPatients(const QString& text);
};
#endif // MAINWINDOW_H
