/*
 * This file is part of Astarte.
 *
 * Copyright 2017 Ispirata Srl
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
