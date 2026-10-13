#ifndef PATIENT_H
#define PATIENT_H

#include <string>

class Patient
{
public:
    Patient(int id, const std::string& name);

    int getId() const;
    const std::string& getName() const;
    void setName(const std::string& name);

private:
    int id_;
    std::string name_;
};

#endif // PATIENT_H
