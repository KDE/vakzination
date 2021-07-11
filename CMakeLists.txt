# SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(vakzination)

include(FeatureSummary)

set(QT5_MIN_VERSION 5.15)
set(KF5_MIN_VERSION 5.83)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMPoQmTools)
include(KDEClangFormat)
include(KDEGitCommitHooks)

find_package(Qt5 ${QT5_MIN_VERSION} REQUIRED COMPONENTS Core Gui Qml QuickControls2 Svg)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS Kirigami2 I18n)

find_package(KHealthCertificate REQUIRED)

add_subdirectory(src)

install(FILES org.kde.vakzination.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.vakzination.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

file(GLOB_RECURSE CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
