#ifndef PATIENTTABLEMODEL_H
#define PATIENTTABLEMODEL_H

#include "patient.h"

#include <QAbstractTableModel>
#include <QVariant>

#include <vector>

class PatientTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit PatientTableModel(QObject* parent = nullptr);

    int rowCount(
        const QModelIndex& parent = QModelIndex()) const override;

    int columnCount(
        const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(
        const QModelIndex& index,
        int role = Qt::DisplayRole) const override;

    QVariant headerData(
        int section,
        Qt::Orientation orientation,
        int role = Qt::DisplayRole) const override;

    void setPatients(std::vector<Patient> patients);

private:
    std::vector<Patient> patients_;
};

#endif // PATIENTTABLEMODEL_H