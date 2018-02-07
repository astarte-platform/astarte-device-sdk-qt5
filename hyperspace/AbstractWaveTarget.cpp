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
