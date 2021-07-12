#! /usr/bin/env bash
# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
$XGETTEXT `find -name \*.cpp -o -name \*.qml -o -name \*.js` -o $podir/vakzination.pot
