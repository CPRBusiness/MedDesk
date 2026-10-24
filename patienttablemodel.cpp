#include "patienttablemodel.h"

#include <QString>

#include <limits>
#include <stdexcept>

PatientTableModel::PatientTableModel(QObject* parent) : QAbstractTableModel(parent)
{

}

int PatientTableModel::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid()){
        return 0;
    }

    return static_cast<int>(patients_.size());
}

int PatientTableModel::columnCount(const QModelIndex& parent) const{
    if(parent.isValid()){
        return 0;
    }

    return 2;
}

QVariant PatientTableModel::data(const QModelIndex& index, int role) const{
    if(!index.isValid()
        || index.model() != this
        || index.row() < 0
        || index.row() >= rowCount()
        || index.column() < 0
        || index.column() >= columnCount())
    {
        return {};
    }

    const Patient& patient = patients_[static_cast<std::size_t>(index.row())];

    if(role == Qt::UserRole){
        return patient.getId();
    }

    if(role != Qt::DisplayRole){
        return {};
    }

    switch (index.column()){
    case 0:
        return patient.getId();

    case 1:
        return QString::fromStdString(patient.getName());

    default:
        return {};
    }
}

QVariant PatientTableModel::headerData(int section, Qt::Orientation orientation, int role) const{
    if(role != Qt::DisplayRole){
        return {};
    }

    if(orientation == Qt::Horizontal){
        switch(section){
        case 0:
            return QStringLiteral("ID");

        case 1:
            return QStringLiteral("Nume");

        default:
            return {};
        }
    }

    if((section >= 0) && (section < rowCount())){
        return section + 1;
    }

    return {};
}

void PatientTableModel::setPatients(std::vector<Patient> patients){
    if(patients.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())){
        throw std::length_error("Prea multi pacienti pentru modelul tabelului.");
    }

    beginResetModel();
    patients_.swap(patients);
    endResetModel();
}