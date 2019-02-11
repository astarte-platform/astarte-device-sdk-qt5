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

#ifndef HYPERSPACE_ABSTRACTWAVETARGET_H
#define HYPERSPACE_ABSTRACTWAVETARGET_H

#include <HyperspaceCore/Global>

#include <HyperspaceCore/Fluctuation>
#include <HyperspaceCore/Rebound>

namespace Hyperdrive {
class AstarteTransport;
}

/**
 * @defgroup HyperspaceCore Hyperspace Core
 *
 * Hyperspace Core contains all the basic types and mechanism to enable Hyperspace usage, both as a server and as a client,
 * for Hemera applications.
 *
 * It is contained in the Hyperspace:: namespace.
 */

namespace Hyperspace {

class AbstractWaveTargetPrivate;

class HYPERSPACE_QT5_EXPORT AbstractWaveTarget : public QObject
{
    Q_OBJECT

    Q_DECLARE_PRIVATE_D(d_w_ptr, AbstractWaveTarget)
    Q_DISABLE_COPY(AbstractWaveTarget)

public:
    AbstractWaveTarget(const QByteArray &interface, Hyperdrive::AstarteTransport *astarteTransport, QObject *parent = nullptr);
    virtual ~AbstractWaveTarget();

    QByteArray interface() const;

    bool isReady() const;

Q_SIGNALS:
    void ready();

protected Q_SLOTS:
    /**
     * @brief Send a rebound for a received wave
     *
     * Call this method upon processing a wave. The rebound must have the same ID of its corresponding Wave.
     */
    void sendRebound(const Hyperspace::Rebound &rebound);

    /**
     * @brief Send a fluctuation for this target
     *
     * Call this method whenever the internal representation (e.g.: GET) of the target changes.
     */
    void sendFluctuation(const QByteArray &targetPath, const Fluctuation &payload);

    void handleReceivedWave(const QByteArray &interface, const Wave &wave);


protected:
    AbstractWaveTargetPrivate * const d_w_ptr;

    virtual void waveFunction(const Wave &wave) = 0;
};

}

#endif // HYPERSPACE_ABSTRACTWAVE_H
