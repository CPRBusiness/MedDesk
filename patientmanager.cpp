#include "patientmanager.h"

#include <algorithm>
#include <stdexcept>

PatientManager::PatientManager() {}

void PatientManager::validateName(const std::string& name){
    if(name.find_first_not_of(" \t\r\n\f\v") == std::string::npos){
        throw std::invalid_argument("Numele pacientului este obligatoriu.");
    }
}

Patient PatientManager::addPatient(const std::string& name){
    validateName(name);

    Patient patient(nextPatientId_, name);
    patients_.push_back(patient);
    ++nextPatientId_;

    return patient;
}

std::optional<Patient> PatientManager::findPatient(int id) const{
    const auto it = std::find_if(
        patients_.begin(),
        patients_.end(),
        [id](const Patient& patient){
            return patient.getId() == id;
        }
    );

    if(it == patients_.end()){
        return std::nullopt;
    }

    return *it;
}

bool PatientManager::renamePatient(int id, const std::string& name){
    validateName(name);

    const auto it = std::find_if(
        patients_.begin(),
        patients_.end(),
        [id](const Patient& patient){
            return patient.getId() == id;
        }
    );

    if(it == patients_.end()){
        return false;
    }

    it->setName(name);
    return true;
}

bool PatientManager::removePatient(int id)
{
    const auto it = std::find_if(
        patients_.begin(),
        patients_.end(),
        [id](const Patient& patient) {
            return patient.getId() == id;
        }
        );

    if (it == patients_.end()) {
        return false;
    }

    patients_.erase(it);
    return true;
}

PatientSnapshot PatientManager:: snapshot() const{
    return {patients_, nextPatientId_};
}