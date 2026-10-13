#include "patient.h"

Patient::Patient(int id, const std::string& name)
    :id_(id), name_(name)
{
}

int Patient::getId() const{
    return id_;
}

const std::string& Patient::getName() const{
    return name_;
}

void Patient::setName(const std::string& name){
    name_ = name;
}