/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2021 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "testcppchecklibrarydata.h"

const QString TestCppcheckLibraryData::TempCfgFile = "./tmp.cfg";

void TestCppcheckLibraryData::init()
{
    result.clear();
    libraryData.clear();
    fileLibraryData.clear();
}

void TestCppcheckLibraryData::xmlReaderError()
{
    loadCfgFile(":/files/xml_reader_error.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), false);
    qDebug() << result;
}

void TestCppcheckLibraryData::unhandledElement()
{
    loadCfgFile(":/files/unhandled_element.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), false);
    qDebug() << result;

    loadCfgFile(":/files/platform_type_unhandled_element.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), false);
    qDebug() << result;

    loadCfgFile(":/files/memory_resource_unhandled_element.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), false);
    qDebug() << result;
}

void TestCppcheckLibraryData::mandatoryAttributeMissing()
{
    loadCfgFile(":/files/mandatory_attribute_missing.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), false);
    qDebug() << result;
}

void TestCppcheckLibraryData::podtypeValid()
{
    // Load library data from file
    loadCfgFile(":/files/podtype_valid.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), true);

    // Swap libray data read from file to other object
    libraryData.swap(fileLibraryData);

    // Do size and content checks against swapped data.
    QCOMPARE(libraryData.podtypes.size(), 2);

    QCOMPARE(libraryData.podtypes[0].name, "bool");
    QCOMPARE(libraryData.podtypes[0].stdtype.isEmpty(), true);
    QCOMPARE(libraryData.podtypes[0].sign.isEmpty(), true);
    QCOMPARE(libraryData.podtypes[0].size.isEmpty(), true);

    QCOMPARE(libraryData.podtypes[1].name, "ulong");    
    QCOMPARE(libraryData.podtypes[1].stdtype, "uint32_t");    
    QCOMPARE(libraryData.podtypes[1].sign, "u");
    QCOMPARE(libraryData.podtypes[1].size, "4");

    // Save library data to file
    saveCfgFile(TempCfgFile, libraryData);

    fileLibraryData.clear();
    QCOMPARE(fileLibraryData.podtypes.size(), 0);

    // Reload library data from file
    loadCfgFile(TempCfgFile, fileLibraryData, result, true);
    QCOMPARE(result.isNull(), true);

    // Verify no data got lost or modified
    QCOMPARE(libraryData.podtypes.size(), fileLibraryData.podtypes.size());
    QCOMPARE(libraryData.podtypes.size(), 2);
    for (int i=0; i < libraryData.podtypes.size(); i++) {
        QCOMPARE(libraryData.podtypes[i].name, fileLibraryData.podtypes[i].name);
        QCOMPARE(libraryData.podtypes[i].stdtype, fileLibraryData.podtypes[i].stdtype);
        QCOMPARE(libraryData.podtypes[i].sign, fileLibraryData.podtypes[i].sign);
        QCOMPARE(libraryData.podtypes[i].size, fileLibraryData.podtypes[i].size);
    }
}

void TestCppcheckLibraryData::typechecksValid()
{
    // Load library data from file
    loadCfgFile(":/files/typechecks_valid.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), true);

    // Swap libray data read from file to other object
    libraryData.swap(fileLibraryData);

    // Do size and content checks against swapped data.
    QCOMPARE(libraryData.typeChecks.size(), 3);

    CppcheckLibraryData::TypeChecks check = libraryData.typeChecks[0];
    QCOMPARE(check.size(), 2);
    QCOMPARE(check[0].first, "suppress");
    QCOMPARE(check[0].second, "std::insert_iterator");
    QCOMPARE(check[1].first, "check");
    QCOMPARE(check[1].second, "std::pair");

    check = libraryData.typeChecks[1];
    QCOMPARE(check.isEmpty(), true);

    check = libraryData.typeChecks[2];
    QCOMPARE(check.size(), 1);
    QCOMPARE(check[0].first, "check");
    QCOMPARE(check[0].second, "std::tuple");

    // Save library data to file
    saveCfgFile(TempCfgFile, libraryData);

    fileLibraryData.clear();
    QCOMPARE(fileLibraryData.typeChecks.size(), 0);

    // Reload library data from file
    loadCfgFile(TempCfgFile, fileLibraryData, result, true);
    QCOMPARE(result.isNull(), true);

    // Verify no data got lost or modified
    QCOMPARE(libraryData.typeChecks.size(), fileLibraryData.typeChecks.size());
    QCOMPARE(libraryData.typeChecks.size(), 3);
    for (int idx=0; idx < libraryData.typeChecks.size(); idx++) {
        CppcheckLibraryData::TypeChecks lhs = libraryData.typeChecks[idx];
        CppcheckLibraryData::TypeChecks rhs = fileLibraryData.typeChecks[idx];
        QCOMPARE(lhs.size(), rhs.size());
        QCOMPARE(lhs, rhs);
    }
}

void TestCppcheckLibraryData::smartPointerValid()
{
    // Load library data from file
    loadCfgFile(":/files/smartptr_valid.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), true);

    // Swap libray data read from file to other object
    libraryData.swap(fileLibraryData);

    // Do size and content checks against swapped data.
    QCOMPARE(libraryData.smartPointers.size(), 3);

    QCOMPARE(libraryData.smartPointers[0], "wxObjectDataPtr");
    QCOMPARE(libraryData.smartPointers[1], "wxScopedArray");
    QCOMPARE(libraryData.smartPointers[2], "wxScopedPtr");

    // Save library data to file
    saveCfgFile(TempCfgFile, libraryData);

    fileLibraryData.clear();
    QCOMPARE(fileLibraryData.smartPointers.size(), 0);

    // Reload library data from file
    loadCfgFile(TempCfgFile, fileLibraryData, result, true);
    QCOMPARE(result.isNull(), true);

    // Verify no data got lost or modified
    QCOMPARE(libraryData.smartPointers.size(), fileLibraryData.smartPointers.size());
    QCOMPARE(libraryData.smartPointers.size(), 3);
    QCOMPARE(libraryData.smartPointers, fileLibraryData.smartPointers);
}

void TestCppcheckLibraryData::platformTypeValid()
{
    // Load library data from file
    loadCfgFile(":/files/platform_type_valid.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), true);

    // Swap libray data read from file to other object
    libraryData.swap(fileLibraryData);

    // Do size and content checks against swapped data.
    QCOMPARE(libraryData.platformTypes.size(), 3);

    QCOMPARE(libraryData.platformTypes[0].name, "platform");
    QCOMPARE(libraryData.platformTypes[0].value, "with attribute and empty");
    QCOMPARE(libraryData.platformTypes[0].types.size(), 0);
    QCOMPARE(libraryData.platformTypes[0].platforms.size(), 2);
    QCOMPARE(libraryData.platformTypes[0].platforms[0], "win64");
    QCOMPARE(libraryData.platformTypes[0].platforms[1].isEmpty(), true);

    QCOMPARE(libraryData.platformTypes[1].name, "types");
    QCOMPARE(libraryData.platformTypes[1].value, "all");
    QCOMPARE(libraryData.platformTypes[1].types.size(), 5);
    QCOMPARE(libraryData.platformTypes[1].types,
            QStringList({"unsigned", "long", "pointer", "const_ptr", "ptr_ptr"}));
    QCOMPARE(libraryData.platformTypes[1].platforms.isEmpty(), true);

    QCOMPARE(libraryData.platformTypes[2].name, "types and platform");
    QCOMPARE(libraryData.platformTypes[2].value.isEmpty(), true);
    QCOMPARE(libraryData.platformTypes[2].types.size(), 2);
    QCOMPARE(libraryData.platformTypes[2].types, QStringList({"pointer", "ptr_ptr"}));
    QCOMPARE(libraryData.platformTypes[2].platforms.size(), 1);
    QCOMPARE(libraryData.platformTypes[2].platforms[0], "win32");

    // Save library data to file
    saveCfgFile(TempCfgFile, libraryData);

    fileLibraryData.clear();
    QCOMPARE(fileLibraryData.platformTypes.size(), 0);

    // Reload library data from file
    loadCfgFile(TempCfgFile, fileLibraryData, result, true);
    QCOMPARE(result.isNull(), true);

    // Verify no data got lost or modified
    QCOMPARE(libraryData.platformTypes.size(), fileLibraryData.platformTypes.size());
    QCOMPARE(libraryData.platformTypes.size(), 3);
    for (int idx=0; idx < libraryData.platformTypes.size(); idx++) {
        CppcheckLibraryData::PlatformType lhs = libraryData.platformTypes[idx];
        CppcheckLibraryData::PlatformType rhs = fileLibraryData.platformTypes[idx];
        QCOMPARE(lhs.name, rhs.name);
        QCOMPARE(lhs.value, rhs.value);
        QCOMPARE(lhs.types.size(), rhs.types.size());
        QCOMPARE(lhs.types, rhs.types);
        QCOMPARE(lhs.platforms.size(), rhs.platforms.size());
        QCOMPARE(lhs.platforms, rhs.platforms);
    }
}

void TestCppcheckLibraryData::memoryResourceValid()
{
    // Load library data from file
    loadCfgFile(":/files/memory_resource_valid.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), true);

    // Swap libray data read from file to other object
    libraryData.swap(fileLibraryData);

    // Do size and content checks against swapped data.
    QCOMPARE(libraryData.memoryresource.size(), 2);
    QCOMPARE(libraryData.memoryresource[0].type, "memory");
    QCOMPARE(libraryData.memoryresource[0].alloc.size(), 4);
    QCOMPARE(libraryData.memoryresource[0].dealloc.size(), 1);
    QCOMPARE(libraryData.memoryresource[0].use.size(), 0);

    QCOMPARE(libraryData.memoryresource[0].alloc[0].name, "malloc");
    QCOMPARE(libraryData.memoryresource[0].alloc[0].bufferSize, "malloc");
    QCOMPARE(libraryData.memoryresource[0].alloc[0].isRealloc, false);
    QCOMPARE(libraryData.memoryresource[0].alloc[0].init, false);
    QCOMPARE(libraryData.memoryresource[0].alloc[0].arg, -1);
    QCOMPARE(libraryData.memoryresource[0].alloc[0].reallocArg, -1);

    QCOMPARE(libraryData.memoryresource[0].alloc[1].name, "calloc");
    QCOMPARE(libraryData.memoryresource[0].alloc[1].bufferSize, "calloc");
    QCOMPARE(libraryData.memoryresource[0].alloc[1].isRealloc, false);
    QCOMPARE(libraryData.memoryresource[0].alloc[1].init, true);
    QCOMPARE(libraryData.memoryresource[0].alloc[1].arg, -1);
    QCOMPARE(libraryData.memoryresource[0].alloc[1].reallocArg, -1);

    QCOMPARE(libraryData.memoryresource[0].alloc[2].name, "realloc");
    QCOMPARE(libraryData.memoryresource[0].alloc[2].bufferSize, "malloc:2");
    QCOMPARE(libraryData.memoryresource[0].alloc[2].isRealloc, true);
    QCOMPARE(libraryData.memoryresource[0].alloc[2].init, false);
    QCOMPARE(libraryData.memoryresource[0].alloc[2].arg, -1);
    QCOMPARE(libraryData.memoryresource[0].alloc[2].reallocArg, -1);

    QCOMPARE(libraryData.memoryresource[0].alloc[3].name, "UuidToString");
    QCOMPARE(libraryData.memoryresource[0].alloc[3].bufferSize.isEmpty(), true);
    QCOMPARE(libraryData.memoryresource[0].alloc[3].isRealloc, false);
    QCOMPARE(libraryData.memoryresource[0].alloc[3].init, false);
    QCOMPARE(libraryData.memoryresource[0].alloc[3].arg, 2);
    QCOMPARE(libraryData.memoryresource[0].alloc[3].reallocArg, -1);

    QCOMPARE(libraryData.memoryresource[0].dealloc[0].name, "HeapFree");
    QCOMPARE(libraryData.memoryresource[0].dealloc[0].arg, 3);

    QCOMPARE(libraryData.memoryresource[1].type, "resource");
    QCOMPARE(libraryData.memoryresource[1].alloc.size(), 1);
    QCOMPARE(libraryData.memoryresource[1].dealloc.size(), 1);
    QCOMPARE(libraryData.memoryresource[1].use.size(), 0);

    QCOMPARE(libraryData.memoryresource[1].alloc[0].name, "_wfopen_s");
    QCOMPARE(libraryData.memoryresource[1].alloc[0].bufferSize.isEmpty(), true);
    QCOMPARE(libraryData.memoryresource[1].alloc[0].isRealloc, false);
    QCOMPARE(libraryData.memoryresource[1].alloc[0].init, true);
    QCOMPARE(libraryData.memoryresource[1].alloc[0].arg, 1);
    QCOMPARE(libraryData.memoryresource[1].alloc[0].reallocArg, -1);

    QCOMPARE(libraryData.memoryresource[1].dealloc[0].name, "fclose");
    QCOMPARE(libraryData.memoryresource[1].dealloc[0].arg, -1);

    // Save library data to file
    saveCfgFile(TempCfgFile, libraryData);

    fileLibraryData.clear();
    QCOMPARE(fileLibraryData.memoryresource.size(), 0);

    // Reload library data from file
    loadCfgFile(TempCfgFile, fileLibraryData, result, true);
    QCOMPARE(result.isNull(), true);

    // Verify no data got lost or modified
    QCOMPARE(libraryData.memoryresource.size(), fileLibraryData.memoryresource.size());
    QCOMPARE(libraryData.memoryresource.size(), 2);

    for (int idx=0; idx < libraryData.memoryresource.size(); idx++) {
        CppcheckLibraryData::MemoryResource lhs = libraryData.memoryresource[idx];
        CppcheckLibraryData::MemoryResource rhs = fileLibraryData.memoryresource[idx];

        QCOMPARE(lhs.type, rhs.type);
        QCOMPARE(lhs.alloc.size(), rhs.alloc.size());
        QCOMPARE(lhs.dealloc.size(), rhs.dealloc.size());
        QCOMPARE(lhs.use, rhs.use);

        for (int num=0; num < lhs.alloc.size(); num++) {
            QCOMPARE(lhs.alloc[num].name, rhs.alloc[num].name);
            QCOMPARE(lhs.alloc[num].bufferSize, rhs.alloc[num].bufferSize);
            QCOMPARE(lhs.alloc[num].isRealloc, rhs.alloc[num].isRealloc);
            QCOMPARE(lhs.alloc[num].init, rhs.alloc[num].init);
            QCOMPARE(lhs.alloc[num].arg, rhs.alloc[num].arg);
            QCOMPARE(lhs.alloc[num].reallocArg, rhs.alloc[num].reallocArg);
        }
        for (int num=0; num < lhs.dealloc.size(); num++) {
            QCOMPARE(lhs.dealloc[num].name, rhs.dealloc[num].name);
            QCOMPARE(lhs.dealloc[num].arg, rhs.dealloc[num].arg);
        }
    }
}

void TestCppcheckLibraryData::loadCfgFile(QString filename, CppcheckLibraryData &data, QString &result, bool removeFile)
{
    QFile file(filename);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    result = data.open(file);
    file.close();
    if (removeFile) {
        file.remove();
    }
}

void TestCppcheckLibraryData::saveCfgFile(QString filename, CppcheckLibraryData &data)
{
    QFile file(filename);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream textStream(&file);
    textStream << data.toString() << '\n';
    file.close();
}

QTEST_MAIN(TestCppcheckLibraryData)
