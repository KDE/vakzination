# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>

add_executable(vakzination main.cpp certificatesmodel.cpp resources.qrc)
target_link_libraries(vakzination
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    Qt5::QuickControls2
    Qt5::Svg
    KF5::I18n
    KF5::ConfigCore
    KHealthCertificate
)
install(TARGETS vakzination ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
