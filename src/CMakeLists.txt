# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>

add_definitions(-DTRANSLATION_DOMAIN=\"vakzination\")

add_library(vakzinationstatic STATIC)
target_sources(vakzinationstatic PRIVATE certificatesmodel.cpp)
target_link_libraries(vakzinationstatic PUBLIC Qt::Core Qt::Gui KF6::ConfigCore KHealthCertificate PRIVATE KF6::I18n KPim6::Itinerary)

add_executable(vakzination main.cpp resources.qrc)
target_include_directories(vakzination PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(vakzination
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::QuickControls2
    Qt::Svg
    KF6::I18n
    KF6::ConfigCore
    KF6::CoreAddons
    vakzinationstatic
)

if (ANDROID)
    kirigami_package_breeze_icons(ICONS
        dialog-error-symbolic
        dialog-ok
        edit-paste
        flashlight-off
        flashlight-on
        folder-open
        question
        view-barcode-qr
        view-list-details
    )
    ecm_add_android_apk(vakzination ANDROID_DIR ${CMAKE_SOURCE_DIR}/android)
else()
    target_link_libraries(vakzination Qt::Widgets)
endif()

install(TARGETS vakzination ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
