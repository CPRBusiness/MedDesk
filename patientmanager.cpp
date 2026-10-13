#include "patientmanager.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <unordered_set>

PatientManager::PatientManager() {}

void PatientManager::validateName(const std::string& name){
    if(name.find_first_not_of(" \t\r\n\f\v") == std::string::npos){
        throw std::invalid_argument("Numele pacientului este obligatoriu.");
    }
}

Patient PatientManager::addPatient(const std::string& name){
    validateName(name);

    if (nextPatientId_ == std::numeric_limits<int>::max())
    {
        throw std::overflow_error(
            "Nu mai sunt disponibile ID-uri pentru pacienti.");
    }

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

void PatientManager::restore(PatientSnapshot snapshot){
    if(snapshot.nextPatientId < 1){
        throw std::invalid_argument("Urmatorul ID trebuie sa fie pozitiv.");
    }

    std::unordered_set<int> ids;

    for(const Patient& patient : snapshot.patients){
        validateName(patient.getName());

        if(patient.getId() < 1){
            throw std::invalid_argument("ID-ul pacientului trebuie sa fie pozitiv");
        }

        if(!ids.insert(patient.getId()).second){
            throw std::invalid_argument(
                "Fisierul contine ID-uri duplicate.");
        }

        if(patient.getId() >= snapshot.nextPatientId){
            throw std::invalid_argument(
                "Urmatorul ID trebuie sa fie mai mare decat toate ID-urile existente");
        }
    }

    patients_.swap(snapshot.patients);
    nextPatientId_ = snapshot.nextPatientId;
}