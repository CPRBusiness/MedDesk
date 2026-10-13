#include <QTest>

#include "patientmanager.h"

#include <stdexcept>
#include <string>

class PatientManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void invalidAddDoesNotConsumeId()
    {
        PatientManager manager;

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            manager.addPatient("")
            );

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            manager.addPatient("   ")
            );

        const Patient patient = manager.addPatient("Ana");

        QCOMPARE(patient.getId(), 1);
    }

    void deletedIdsAreNotReused()
    {
        PatientManager manager;

        const Patient first = manager.addPatient("Ana");
        const Patient second = manager.addPatient("Andrei");
        const Patient third = manager.addPatient("Maria");

        QVERIFY(manager.removePatient(second.getId()));
        QVERIFY(!manager.findPatient(second.getId()).has_value());

        const Patient fourth = manager.addPatient("Pavel");

        QCOMPARE(fourth.getId(), third.getId() + 1);

        QVERIFY(manager.findPatient(first.getId()).has_value());
        QVERIFY(manager.findPatient(third.getId()).has_value());
    }

    void invalidRenameKeepsOriginalName()
    {
        PatientManager manager;
        const Patient patient = manager.addPatient("Ana");

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            manager.renamePatient(patient.getId(), "   ")
            );

        const auto found = manager.findPatient(patient.getId());

        QVERIFY(found.has_value());
        QVERIFY(found->getName() == "Ana");
    }

    void missingIdDoesNotChangeOtherPatients()
    {
        PatientManager manager;
        const Patient patient = manager.addPatient("Ana");

        QVERIFY(!manager.findPatient(999).has_value());
        QVERIFY(!manager.removePatient(999));
        QVERIFY(!manager.renamePatient(999, "Alt nume"));

        const auto found = manager.findPatient(patient.getId());

        QVERIFY(found.has_value());
        QVERIFY(found->getName() == "Ana");
    }

    void returnedPatientIsAnIndependentCopy()
    {
        PatientManager manager;
        const Patient patient = manager.addPatient("Ana");

        const auto snapshot =
            manager.findPatient(patient.getId());

        QVERIFY(snapshot.has_value());

        for (int i = 0; i < 256; ++i) {
            manager.addPatient(
                "Pacient " + std::to_string(i)
                );
        }

        QVERIFY(
            manager.renamePatient(patient.getId(), "Ana Popescu")
            );

        const auto updated =
            manager.findPatient(patient.getId());

        QVERIFY(updated.has_value());
        QVERIFY(updated->getName() == "Ana Popescu");
        QVERIFY(snapshot->getName() == "Ana");

        QVERIFY(manager.removePatient(patient.getId()));
        QVERIFY(!manager.findPatient(patient.getId()).has_value());

        QVERIFY(snapshot->getName() == "Ana");
    }
};

QTEST_APPLESS_MAIN(PatientManagerTest)

#include "patientmanager_test.moc"