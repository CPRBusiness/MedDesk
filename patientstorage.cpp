#include "patientstorage.h"

#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QFile>
#include <QJsonParseError>
#include <QJsonValue>

#include <cmath>
#include <limits>

#include <stdexcept>

namespace
{
int readInteger(const QJsonValue& value)
{
    if (!value.isDouble())
    {
        throw std::invalid_argument(
            "Un camp numeric lipseste sau are tip incorect.");
    }

    const double number = value.toDouble();

    if (!std::isfinite(number)
        || std::trunc(number) != number
        || number < std::numeric_limits<int>::min()
        || number > std::numeric_limits<int>::max())
    {
        throw std::invalid_argument(
            "Valoarea trebuie sa fie un numar intreg "
            "in intervalul tipului int.");
    }

    return static_cast<int>(number);
}
}

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

PatientSnapshot PatientStorage::load(const QString& filePath){
    QFile file(filePath);

    if(!file.open(QIODevice::ReadOnly)){
        throw std::runtime_error(
            std::string("Fisierul nu poate fi deschis: ")
            + file.errorString().toStdString());
    }

    const QByteArray data = file.readAll();

    if(file.error() != QFileDevice::NoError){
        throw std::runtime_error(
            std::string("Citirea fisierului a esuat: ")
            + file.errorString().toStdString());
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(data, &parseError);

    if(parseError.error != QJsonParseError::NoError){
        throw std::invalid_argument(
            std::string("JSON invalid: ")
            + parseError.errorString().toStdString());
    }

    if(!document.isObject()){
        throw std::invalid_argument(
            "Radacina JSON trebuie sa fie un obiect.");
    }

    const QJsonObject root = document.object();

    if (readInteger(root.value("formatVersion")) != 1)
    {
        throw std::invalid_argument(
            "Versiunea formatului nu este suportata.");
    }

    PatientSnapshot snapshot;
    snapshot.nextPatientId =
        readInteger(root.value("nextPatientId"));

    const QJsonValue patientsValue = root.value("patients");

    if (!patientsValue.isArray())
    {
        throw std::invalid_argument(
            "Campul patients trebuie sa fie o lista.");
    }

    const QJsonArray patientsArray = patientsValue.toArray();

    for (const QJsonValue& value : patientsArray)
    {
        if (!value.isObject())
        {
            throw std::invalid_argument(
                "Fiecare pacient trebuie sa fie un obiect.");
        }

        const QJsonObject patientObject = value.toObject();

        const int id =
            readInteger(patientObject.value("id"));

        const QJsonValue nameValue =
            patientObject.value("name");

        if (!nameValue.isString())
        {
            throw std::invalid_argument(
                "Numele pacientului trebuie sa fie text.");
        }

        snapshot.patients.emplace_back(
            id,
            nameValue.toString().toStdString());
    }

    return snapshot;
}