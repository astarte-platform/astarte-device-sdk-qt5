/*
 * Copyright (C) 2017 Ispirata Srl
 *
 * This file is part of Astarte.
 * Astarte is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Astarte is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Astarte.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HEMERA_HEMERALITERALS_H
#define HEMERA_HEMERALITERALS_H

#include <QtCore/QLatin1String>

namespace Hemera {

namespace Literals {

static constexpr QLatin1String literal(const char *l) { return QLatin1String(l); }

/// Errors
namespace Errors {

constexpr const char *badRequest() { return "com.ispirata.Hemera.Error.BadRequest"; }
constexpr const char *canceled() { return "com.ispirata.Hemera.Error.Canceled"; }
constexpr const char *errorHandlingError() { return "com.ispirata.Hemera.ErrorHandlingError"; }
constexpr const char *failedRequest() { return "com.ispirata.Hemera.Error.FailedRequest"; }
constexpr const char *notAllowed() { return "com.ispirata.Hemera.Error.NotAllowed"; }
constexpr const char *notFound() { return "com.ispirata.Hemera.Error.NotFound"; }
constexpr const char *parseError() { return "com.ispirata.Hemera.Error.ParseError"; }
constexpr const char *timeout() { return "com.ispirata.Hemera.Error.Timeout"; }
constexpr const char *unhandledRequest() { return "com.ispirata.Hemera.Error.UnhandledRequest"; }

}

}

}

#endif
