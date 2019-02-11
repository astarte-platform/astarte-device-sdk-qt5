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

#ifndef HYPERDRIVECONFIG_H
#define HYPERDRIVECONFIG_H

namespace Hyperdrive {

namespace StaticConfig {

Q_DECL_CONSTEXPR const char *transportAstarteDataDir() { return "/usr/share/hyperdrive/transport-astarte"; }
// TODO: generate these from CMake
constexpr int hyperdriveMajorVersion() { return 0; }
constexpr int hyperdriveMinorVersion() { return 91; }
constexpr int hyperdriveReleaseVersion() { return 141; }

}

}

#endif
