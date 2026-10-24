#ifndef PATIENTSTORAGE_H
#define PATIENTSTORAGE_H

#include "patientmanager.h"

#include <QString>

class PatientStorage
{
public:
    PatientStorage();

    static void save(const QString& filePath, const PatientSnapshot& snapshot);
    static PatientSnapshot load(const QString& filePath);
};

#endif // PATIENTSTORAGE_H
