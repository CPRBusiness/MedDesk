#include <QTest>

#include "patientmanager.h"
#include "patientstorage.h"
#include "patienttablemodel.h"

#include <QByteArray>
#include <QFile>
#include <QTemporaryDir>
#include <QSignalSpy>

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
    void saveAndLoadPreservesPatientsAndNextId()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString path = directory.filePath("patients.json");

        PatientManager original;
        const Patient first = original.addPatient("Ana");
        const Patient second = original.addPatient("Mihai");
        const Patient third = original.addPatient("Ioana");

        QVERIFY(original.renamePatient(first.getId(), "Ana Popescu"));
        QVERIFY(original.removePatient(third.getId()));

        PatientStorage::save(path, original.snapshot());

        PatientManager restored;
        restored.restore(PatientStorage::load(path));

        const auto restoredFirst = restored.findPatient(first.getId());
        const auto restoredSecond = restored.findPatient(second.getId());

        QVERIFY(restoredFirst.has_value());
        QVERIFY(restoredSecond.has_value());

        QCOMPARE(restoredFirst->getName(), std::string("Ana Popescu"));
        QCOMPARE(restoredSecond->getName(), std::string("Mihai"));

        QVERIFY(!restored.findPatient(third.getId()).has_value());
        QCOMPARE(restored.snapshot().patients.size(), std::size_t{2});

        const Patient next = restored.addPatient("Elena");
        QCOMPARE(next.getId(), 4);
    }
    void emptyListCanBeSavedAndLoaded()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString path = directory.filePath("empty.json");

        PatientManager empty;
        PatientStorage::save(path, empty.snapshot());

        PatientManager restored;
        restored.addPatient("Pacient existent");

        restored.restore(PatientStorage::load(path));

        QVERIFY(restored.snapshot().patients.empty());

        const Patient first = restored.addPatient("Ana");
        QCOMPARE(first.getId(), 1);
    }
    void malformedJsonIsRejected()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString path = directory.filePath("invalid.json");
        const QByteArray data("{ acesta nu este JSON valid");

        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(data), data.size());
        file.close();

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            PatientStorage::load(path));
    }
    void fractionalPatientIdIsRejected()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString path = directory.filePath("fractional.json");

        const QByteArray data = R"json(
        {
            "formatVersion": 1,
            "nextPatientId": 2,
            "patients": [
                {
                    "id": 1.5,
                    "name": "Ana"
                }
            ]
        }
    )json";

        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(data), data.size());
        file.close();

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            PatientStorage::load(path));
    }
    void duplicateIdsKeepExistingPatientsUnchanged()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString path = directory.filePath("duplicates.json");

        const QByteArray data = R"json(
        {
            "formatVersion": 1,
            "nextPatientId": 8,
            "patients": [
                {
                    "id": 7,
                    "name": "Ana"
                },
                {
                    "id": 7,
                    "name": "Mihai"
                }
            ]
        }
    )json";

        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(data), data.size());
        file.close();

        PatientManager manager;
        const Patient existing = manager.addPatient("Pacient existent");

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            manager.restore(PatientStorage::load(path)));

        const auto preserved = manager.findPatient(existing.getId());

        QVERIFY(preserved.has_value());
        QCOMPARE(preserved->getName(), std::string("Pacient existent"));
        QCOMPARE(manager.snapshot().patients.size(), std::size_t{1});
        QVERIFY(!manager.findPatient(7).has_value());

        const Patient next = manager.addPatient("Elena");
        QCOMPARE(next.getId(), 2);
    }
    void savingToMissingDirectoryIsRejected()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString path =
            directory.filePath("missing/patients.json");

        PatientManager manager;
        manager.addPatient("Ana");

        QVERIFY_THROWS_EXCEPTION(
            std::runtime_error,
            PatientStorage::save(path, manager.snapshot()));
    }

    void emptyTableModelHasTwoColumns(){
        PatientTableModel model;

        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(model.columnCount(), 2);

        QCOMPARE(model.headerData(0, Qt::Horizontal).toString(), QString("ID"));

        QCOMPARE(model.headerData(1, Qt::Horizontal).toString(), QString("Nume"));

        QVERIFY(!model.data(QModelIndex()).isValid());
        QVERIFY(!model.index(0,0).isValid());
    }

    void tableModelExposesPatientData()
    {
        PatientTableModel model;

        model.setPatients({
            Patient(7, "Ana"),
            Patient(12, "Mihai")
        });

        QCOMPARE(model.rowCount(), 2);

        const QModelIndex idIndex = model.index(0, 0);
        const QModelIndex nameIndex = model.index(0, 1);

        QCOMPARE(model.data(idIndex).toInt(), 7);
        QCOMPARE(model.data(nameIndex).toString(), QString("Ana"));

        QCOMPARE(model.data(nameIndex, Qt::UserRole).toInt(), 7);

        QCOMPARE(
            model.data(model.index(1, 1)).toString(),
            QString("Mihai"));

        QCOMPARE(model.rowCount(idIndex), 0);
        QCOMPARE(model.columnCount(idIndex), 0);

        QVERIFY(!model.data(nameIndex, Qt::ToolTipRole).isValid());
    }

    void replacingTableDataEmitsReset()
    {
        PatientTableModel model;
        model.setPatients({Patient(1, "Ana")});

        QSignalSpy resetSpy(
            &model,
            &QAbstractItemModel::modelReset);

        QVERIFY(resetSpy.isValid());

        model.setPatients({
            Patient(8, "Elena"),
            Patient(9, "Ioana")
        });

        QCOMPARE(resetSpy.count(), 1);
        QCOMPARE(model.rowCount(), 2);

        QCOMPARE(
            model.data(model.index(0, 0)).toInt(),
            8);

        QCOMPARE(
            model.data(model.index(0, 1)).toString(),
            QString("Elena"));

        model.setPatients({});

        QCOMPARE(resetSpy.count(), 2);
        QCOMPARE(model.rowCount(), 0);
    }
};

QTEST_APPLESS_MAIN(PatientManagerTest)

#include "patientmanager_test.moc"