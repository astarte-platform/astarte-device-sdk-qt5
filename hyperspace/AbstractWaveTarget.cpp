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

#include "AbstractWaveTarget_p.h"

#include <QtCore/QDebug>
#include <QtCore/QSharedData>
#include <QtCore/QTimer>

#include <astartetransport.h>

#include <cachemessage.h>

#include <HyperspaceCore/Fluctuation>
#include <HyperspaceCore/Rebound>
#include <HyperspaceCore/Wave>

namespace Hyperspace
{

AbstractWaveTarget::AbstractWaveTarget(const QByteArray &interface, Hyperdrive::AstarteTransport *astarteTransport, QObject *parent)
    : QObject(parent)
    , d_w_ptr(new AbstractWaveTargetPrivate)
{
    Q_D(AbstractWaveTarget);
    d->interface = interface;
    d->astarteTransport = astarteTransport;

    connect(d->astarteTransport, &Hyperdrive::AstarteTransport::waveReceived, this, &AbstractWaveTarget::handleReceivedWave);
}

AbstractWaveTarget::~AbstractWaveTarget()
{
    Q_D(AbstractWaveTarget);
    delete d_w_ptr;
}

QByteArray AbstractWaveTarget::interface() const
{
    Q_D(const AbstractWaveTarget);

    return d->interface;
}

bool AbstractWaveTarget::isReady() const
{
    return true;
}

void AbstractWaveTarget::handleReceivedWave(const QByteArray &interface, const Wave &wave)
{
    Q_D(AbstractWaveTarget);
    if (interface != d->interface) {
        return;
    }

    QTimer::singleShot(0, this, [this, wave] {
        waveFunction(wave);
    });
}

void AbstractWaveTarget::sendRebound(const Rebound &rebound)
{
    Q_UNUSED(rebound)
}

void AbstractWaveTarget::sendFluctuation(const QByteArray &targetPath, const Fluctuation &payload)
{
    Hyperdrive::CacheMessage c;
    c.setTarget(QByteArray("/%1%2").replace("%1", interface()).replace("%2", targetPath));
    c.setPayload(payload.payload());
    c.setAttributes(payload.attributes());
    Hyperdrive::Interface::Type interfaceType =
        static_cast<Hyperdrive::Interface::Type>(payload.attributes().value("interfaceType").toInt());
    c.setInterfaceType(interfaceType);
    QTimer::singleShot(0, this, [this, c] {
        Q_D(AbstractWaveTarget);
        d->astarteTransport->cacheMessage(c);
    });
}

}

#include "moc_AbstractWaveTarget.cpp"
