# SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

find_package(Qt${QT_MAJOR_VERSION}Test REQUIRED)

ecm_add_test(certificatesmodeltest.cpp TEST_NAME certificatesmodeltest LINK_LIBRARIES vakzinationstatic Qt::Test KF${KF_MAJOR_VERSION}::I18n KPim${KF_MAJOR_VERSION}::Itinerary)
