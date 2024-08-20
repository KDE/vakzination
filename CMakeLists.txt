# SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

# KDE Applications version, managed by release script.
set(RELEASE_SERVICE_VERSION_MAJOR "23")
set(RELEASE_SERVICE_VERSION_MINOR "11")
set(RELEASE_SERVICE_VERSION_MICRO "70")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(vakzination VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION 6.6)
set(KF_MIN_VERSION 6.3.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMPoQmTools)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMAddTests)
include(ECMAddAndroidApk)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX VAKZINATION
    VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/version.h
)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Gui Qml QuickControls2 Svg)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Kirigami2 I18n Config Prison CoreAddons)

find_package(KHealthCertificate REQUIRED)
find_package(KPim6Itinerary 6.1.0 REQUIRED)

if(ANDROID)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/android/version.gradle.in ${CMAKE_BINARY_DIR}/version.gradle)
else()
    find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Widgets)
endif()

add_definitions(-DQT_NO_KEYWORDS)

ki18n_install(po)

add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()

install(FILES org.kde.vakzination.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.vakzination.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

file(GLOB_RECURSE CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
