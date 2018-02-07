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

#ifndef HYPERDRIVE_MQTTCLIENTWRAPPER_P_H
#define HYPERDRIVE_MQTTCLIENTWRAPPER_P_H

#include "hyperdrivemqttclientwrapper.h"

#include <HemeraCore/Operation>

#include <QtCore/QDateTime>
#include <QtCore/QTimer>

#include <hemeraasyncinitobject_p.h>

#include <mosquittopp.h>

namespace Hyperdrive {

class HyperdriveMosquittoClient;

class MQTTClientWrapperPrivate : public Hemera::AsyncInitObjectPrivate
{
public:
    MQTTClientWrapperPrivate(MQTTClientWrapper* q) : Hemera::AsyncInitObjectPrivate(q)
                                                   , mosquitto(nullptr)
                                                   , status(MQTTClientWrapper::DisconnectedStatus)
                                                   , lwtRetained(false)
                                                   , keepAlive(5 * 60)
                                                   , cleanSession(false)
                                                   , sessionPresent(false)
                                                   , publishQoS(1)
                                                   , subscribeQoS(1) {}

    Q_DECLARE_PUBLIC(MQTTClientWrapper)

    HyperdriveMosquittoClient *mosquitto;

    MQTTClientWrapper::Status status;
    QByteArray hardwareId;
    QByteArray customerId;

    QByteArray lwtTopic;
    QByteArray lwtMessage;
    MQTTClientWrapper::MQTTQoS lwtQos;
    bool lwtRetained;

    quint64 keepAlive;
    bool cleanSession;
    bool sessionPresent;
    bool ignoreSslErrors;
    int publishQoS;
    int subscribeQoS;
    QUrl serverUrl;
    QDateTime clientCertificateExpiry;
    QTimer *connackTimer;

    // SSL
    QString pathToCA;
    QString pathToPKey;
    QString pathToCertificate;

    void setStatus(MQTTClientWrapper::Status s);

    // MQTT CALLBACKS
    void on_connect(int rc);
    void on_disconnect(int rc);
    void on_publish(int mid);
    void on_message(const struct mosquitto_message *message);
    void on_subscribe(int mid, int qos_count, const int *granted_qos);
    void on_unsubscribe(int mid);
    void on_log(int level, const char *str);
    void on_error();
};

class HyperdriveMosquittoClient : public mosqpp::mosquittopp
{
public:
    explicit HyperdriveMosquittoClient(MQTTClientWrapperPrivate *d, const char *id, bool clean_session)
        : mosquittopp(id, clean_session)
        , d(d) {}
    virtual ~HyperdriveMosquittoClient() {}

    // MQTT CALLBACKS. Just redirect to our private class
    inline virtual void on_connect(int rc) override { d->on_connect(rc); }
    inline virtual void on_disconnect(int rc) override { d->on_disconnect(rc); }
    inline virtual void on_publish(int mid) override { d->on_publish(mid); }
    inline virtual void on_message(const struct mosquitto_message *message) override { d->on_message(message); }
    inline virtual void on_subscribe(int mid, int qos_count, const int *granted_qos) override { d->on_subscribe(mid, qos_count, granted_qos); }
    inline virtual void on_unsubscribe(int mid) override { d->on_unsubscribe(mid); }
    inline virtual void on_log(int level, const char *str) override { d->on_log(level, str); }
    inline virtual void on_error() override { d->on_error(); }

private:
    MQTTClientWrapperPrivate *d;
};

}

#endif // HYPERDRIVE_MQTTCLIENTWRAPPER_P_H
