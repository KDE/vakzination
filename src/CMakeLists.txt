# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>

if (TARGET KPim::Itinerary)
    set(HAVE_KITINERARY TRUE)
endif()
configure_file(config-vakzination.h.in ${CMAKE_CURRENT_BINARY_DIR}/config-vakzination.h)

add_library(vakzinationstatic STATIC)
target_sources(vakzinationstatic PRIVATE certificatesmodel.cpp)
target_link_libraries(vakzinationstatic PUBLIC Qt::Core PRIVATE KF5::ConfigCore KF5::I18n)

if (TARGET KPim::Itinerary)
    target_link_libraries(vakzinationstatic PRIVATE KPim::Itinerary)
endif()

add_executable(vakzination main.cpp resources.qrc)
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
    vakzinationstatic
)

if (ANDROID)
    kirigami_package_breeze_icons(ICONS
        folder-open
        edit-paste
        dialog-ok
        question
        dialog-error-symbolic
    )
endif()

install(TARGETS vakzination ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
