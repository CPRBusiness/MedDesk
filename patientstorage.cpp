#include "patientstorage.h"

#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

#include <stdexcept>

PatientStorage::PatientStorage() {}

void PatientStorage::save(
    const QString& filePath,
    const PatientSnapshot& snapshot)
{
    QJsonArray patientsArray;

    for(const Patient& patient : snapshot.patients){
        QJsonObject patientObject;
        patientObject["id"] = patient.getId();
        patientObject["name"] = QString::fromStdString(patient.getName());

        patientsArray.append(patientObject);
    }

    QJsonObject root;
    root["formatVersion"] = 1;
    root["nextPatientId"] = snapshot.nextPatientId;
    root["patients"] = patientsArray;

    const QJsonDocument document(root);
    const QByteArray data = document.toJson(QJsonDocument::Indented);

    QSaveFile file(filePath);

    if(!file.open(QIODevice::WriteOnly)){
        throw std::runtime_error(
            std::string("Fisierul nu poate fi deschis: ")
            + file.errorString().toStdString());
    }

    if(file.write(data) != data.size()){
        throw std::runtime_error(
            std::string("Scrierea fisierului a esuat: ")
            + file.errorString().toStdString());
    }

    if(!file.commit()){
        throw std::runtime_error(
            std::string("Salvarea fisierului a esuat: ")
            + file.errorString().toStdString());
    }
}