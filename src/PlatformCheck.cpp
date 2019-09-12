// Copyright 2005-2019 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifdef MUMBLE
	#include "mumble_pch.hpp"
#else
	#include "murmur_pch.h"
#endif

#include <QtCore/QLibrary>

#include "PlatformCheck.h"

bool PlatformCheck::IsWine() {
#ifdef Q_OS_WIN
	// Detect if we're running under Wine.
	// For more info, see https://wiki.winehq.org/Developer_FAQ#How_can_I_detect_Wine.3F
	if (QLibrary::resolve(QLatin1String("ntdll.dll"), "wine_get_version") != NULL) {
		return true;
	}
#endif
	return false;
}
