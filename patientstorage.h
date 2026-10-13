#ifndef PATIENTSTORAGE_H
#define PATIENTSTORAGE_H

#include "patientmanager.h"

#include <QString>

class PatientStorage
{
public:
    PatientStorage();

    static void save(const QString& filePath, const PatientSnapshot& snapshot);
};

#endif // PATIENTSTORAGE_H
