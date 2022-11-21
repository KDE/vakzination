# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>

add_library(vakzinationstatic STATIC)
target_sources(vakzinationstatic PRIVATE certificatesmodel.cpp)
target_link_libraries(vakzinationstatic PUBLIC Qt::Core Qt::Gui KF5::ConfigCore KHealthCertificate PRIVATE KF5::I18n KPim::Itinerary)

add_executable(vakzination main.cpp resources.qrc)
target_include_directories(vakzination PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(vakzination
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::QuickControls2
    Qt::Svg
    KF5::I18n
    KF5::ConfigCore
    KF5::CoreAddons
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
else()
    target_link_libraries(vakzination Qt::Widgets)
endif()

install(TARGETS vakzination ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
