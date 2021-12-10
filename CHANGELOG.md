# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [1.0.1] - Unreleased
### Added
- Add public functions for connecting (connectToAstarte) and disconnecting (disconnectFromAstarte).
- Add support for all array types by adding QList<T> overloads to sendData

### Changed
- Make the SDK build with Qt versions earlier than 5.6. http redirects will not be supported on
  those versions.
- Rename `agentKey` to `pairingJwt` in the configuration, deprecating the old name.
- Deprecate `endpoint` in the configuration, introduce `pairingUrl` and `realm` in the configuration
  to replace it.

### Fixed
- Fixed `sendUnset` bug that prevented it from working.
- Emit `connectionStatusChanged` only after performing the internal connection status handling (e.g.
  publishing the introspection).

## [1.0.0] - 2021-06-28

## [1.0.0-rc.0] - 2021-05-10
### Changed
- Minor optimizations to MQTT subscribes.

## [1.0.0-beta.2] - 2021-03-25

## [1.0.0-beta.1] - 2021-02-16
### Added
- Expose connection status in the SDK.
- Expose a way to set the MQTT keepalive.
- Make the HTTP client follow redirects.

## [0.11.4] - 2021-01-26
### Fixed
- Use the correct `binaryblob` type instead of `binary`.

## [0.11.3] - 2020-09-24

## [0.11.2] - 2020-08-04

## [0.11.1] - 2020-05-18

## [0.11.0] - 2020-04-13

## [0.11.0-rc.1] - 2020-03-26

## [0.11.0-rc.0] - 2020-02-27

## [0.10.1] - 2019-10-02

## [0.10.0] - 2019-04-17

## [0.10.0-rc.1] - 2019-04-10
### Fixed
- Endpoint pattern regexp now allows underscores.

## [0.10.0-rc.0] - 2019-04-03
### Added
- Allow sending data from interfaces with object aggregation

## [0.10.0-beta.3] - 2018-12-19

## [0.10.0-beta.2] - 2018-10-19

## [0.10.0-beta.1] - 2018-08-27
### Added
- First Astarte release.
