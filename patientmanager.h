#ifndef PATIENTMANAGER_H
#define PATIENTMANAGER_H

#include "patient.h"

#include <optional>
#include <string>
#include <vector>

class PatientManager
{
public:
    PatientManager();

    Patient addPatient(const std::string& name);
    std::optional<Patient> findPatient(int id) const;
    bool renamePatient(int id, const std::string& name);
    bool removePatient(int id);

private:
    static void validateName(const std::string& name);

    std::vector<Patient> patients_;
    int nextPatientId_ = 1;
};

#endif // PATIENTMANAGER_H
