cmake_minimum_required(VERSION 3.20)

project(qsvn VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt5 COMPONENTS Test)

add_executable(qsvn
    src/main.cpp
    src/ApplicationSettings.h
    src/ApplicationSettings.cpp
    src/CommandLine.h
    src/CommandLine.cpp
    src/ExternalToolCommand.h
    src/ExternalToolCommand.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/SvnClient.h
    src/SvnClient.cpp
    src/CheckoutDialog.h
    src/CheckoutDialog.cpp
    src/BranchTagDialog.h
    src/BranchTagDialog.cpp
    src/CommitDialog.h
    src/CommitDialog.cpp
    src/ConflictsDialog.h
    src/ConflictsDialog.cpp
    src/ExportDialog.h
    src/ExportDialog.cpp
    src/ImportDialog.h
    src/ImportDialog.cpp
    src/LogDialog.h
    src/LogDialog.cpp
    src/MergeDialog.h
    src/MergeDialog.cpp
    src/PropertiesDialog.h
    src/PropertiesDialog.cpp
    src/RepositoryBrowserDialog.h
    src/RepositoryBrowserDialog.cpp
    src/RelocateDialog.h
    src/RelocateDialog.cpp
    src/SettingsDialog.h
    src/SettingsDialog.cpp
    src/SwitchDialog.h
    src/SwitchDialog.cpp
)

target_link_libraries(qsvn PRIVATE Qt5::Widgets)

include(CTest)

if(BUILD_TESTING AND TARGET Qt5::Test)
    add_executable(svnclient_tests
        tests/svnclient_tests.cpp
        src/ApplicationSettings.h
        src/ApplicationSettings.cpp
        src/CommandLine.h
        src/CommandLine.cpp
        src/ExternalToolCommand.h
        src/ExternalToolCommand.cpp
        src/SvnClient.h
        src/SvnClient.cpp
    )

    target_include_directories(svnclient_tests PRIVATE src)
    target_link_libraries(svnclient_tests PRIVATE Qt5::Test)

    add_test(NAME svnclient_tests COMMAND svnclient_tests)
endif()

install(TARGETS qsvn RUNTIME DESTINATION bin)
install(FILES packaging/kde/qsvn-dolphin.desktop DESTINATION share/kio/servicemenus)
install(FILES packaging/thunar/uca.xml DESTINATION share/qsvn/thunar)
install(PROGRAMS
    "packaging/nautilus/qsvn/Open in qsvn"
    "packaging/nautilus/qsvn/Settings"
    "packaging/nautilus/qsvn/Checkout"
    "packaging/nautilus/qsvn/Create Repository Here"
    "packaging/nautilus/qsvn/Repo Browser"
    "packaging/nautilus/qsvn/Show Status"
    "packaging/nautilus/qsvn/Check Repository"
    "packaging/nautilus/qsvn/Export"
    "packaging/nautilus/qsvn/Import"
    "packaging/nautilus/qsvn/Branch Tag"
    "packaging/nautilus/qsvn/Switch"
    "packaging/nautilus/qsvn/Relocate"
    "packaging/nautilus/qsvn/Merge"
    "packaging/nautilus/qsvn/Update"
    "packaging/nautilus/qsvn/Update to Revision"
    "packaging/nautilus/qsvn/Commit"
    "packaging/nautilus/qsvn/Add"
    "packaging/nautilus/qsvn/Ignore"
    "packaging/nautilus/qsvn/Copy"
    "packaging/nautilus/qsvn/Delete"
    "packaging/nautilus/qsvn/Rename"
    "packaging/nautilus/qsvn/Revert"
    "packaging/nautilus/qsvn/Set Changelist"
    "packaging/nautilus/qsvn/Remove Changelist"
    "packaging/nautilus/qsvn/Resolve"
    "packaging/nautilus/qsvn/Lock"
    "packaging/nautilus/qsvn/Unlock"
    "packaging/nautilus/qsvn/Cleanup"
    "packaging/nautilus/qsvn/Conflicts"
    "packaging/nautilus/qsvn/Edit Conflict"
    "packaging/nautilus/qsvn/Diff"
    "packaging/nautilus/qsvn/Create Patch"
    "packaging/nautilus/qsvn/Apply Patch"
    "packaging/nautilus/qsvn/Log"
    "packaging/nautilus/qsvn/Blame"
    "packaging/nautilus/qsvn/Properties"
    DESTINATION share/qsvn/nautilus-scripts/qsvn
)
