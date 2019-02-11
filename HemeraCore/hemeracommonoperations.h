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

#ifndef HEMERA_COMMONOPERATIONS_H
#define HEMERA_COMMONOPERATIONS_H

#include <HemeraCore/Global>
#include <HemeraCore/Operation>

class QJsonDocument;
class QProcess;

namespace Hemera {

class HEMERA_QT5_SDK_EXPORT SequentialOperation : public Hemera::Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(SequentialOperation)

    Q_PRIVATE_SLOT(d, void finishedOp(Hemera::Operation *))

public:
    enum SequenceOption {
        NoSequenceOptions = 0,
        ContinueOnFailure = 1
    };
    Q_DECLARE_FLAGS(SequenceOptions, SequenceOption)
    Q_FLAGS(SequenceOptions)
    Q_ENUM(SequenceOption)

    explicit SequentialOperation(const QList<Hemera::Operation*> &sequence, QObject *parent = nullptr);
    explicit SequentialOperation(const QList<Hemera::Operation*> &sequence, const QList<Hemera::Operation*> &revertSequence, QObject *parent = nullptr);
    explicit SequentialOperation(const QList<Hemera::Operation*> &sequence, const QList<Hemera::Operation*> &revertSequence, SequenceOptions options, ExecutionOptions execOptions = NoOptions, QObject *parent = nullptr);
    virtual ~SequentialOperation();

    bool isRunningCleanup() const;
    bool hasCleanupSucceeded() const;

Q_SIGNALS:
    void stepFinished(Hemera::Operation *op);
    void stepFailed(Hemera::Operation *op);
    void cleanupStarted();
    void cleanupSucceeded();
    void cleanupFailed(Hemera::Operation *failedStep);

protected Q_SLOTS:
    virtual void startImpl() Q_DECL_OVERRIDE Q_DECL_FINAL;

private:
    class Private;
    Private * const d;
};

class HEMERA_QT5_SDK_EXPORT CompositeOperation : public Hemera::Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(CompositeOperation)

public:
    enum CompositeOption {
        NoCompositeOptions = 0,
        DontAbortOnFailure = 1
    };
    Q_DECLARE_FLAGS(CompositeOptions, CompositeOption)
    Q_FLAGS(CompositeOptions)
    Q_ENUM(CompositeOption)

    explicit CompositeOperation(const QList<Hemera::Operation*> &operations, QObject *parent = nullptr);
    explicit CompositeOperation(const QList<Hemera::Operation*> &operations, CompositeOptions options,
                                ExecutionOptions execOptions = NoOptions, QObject *parent = nullptr);
    virtual ~CompositeOperation();

protected Q_SLOTS:
    virtual void startImpl() Q_DECL_OVERRIDE Q_DECL_FINAL;

private:
    class Private;
    Private * const d;
};

class HEMERA_QT5_SDK_EXPORT FailureOperation : public Hemera::Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(FailureOperation)

public:
    explicit FailureOperation(const QString &errorName, const QString &errorMessage, QObject *parent = nullptr);
    virtual ~FailureOperation();

protected Q_SLOTS:
    virtual void startImpl() Q_DECL_OVERRIDE Q_DECL_FINAL;

private:
    class Private;
    Private * const d;
};

class HEMERA_QT5_SDK_EXPORT SuccessOperation : public Hemera::Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(SuccessOperation)

public:
    explicit SuccessOperation(QObject *parent = nullptr);
    virtual ~SuccessOperation();

protected Q_SLOTS:
    virtual void startImpl() Q_DECL_OVERRIDE Q_DECL_FINAL;

private:
    class Private;
    Private * const d;
};

class HEMERA_QT5_SDK_EXPORT ObjectOperation : public Hemera::Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(ObjectOperation)

    Q_PROPERTY(QObject * result READ result)

public:
    virtual ~ObjectOperation();
    virtual QObject *result() const = 0;

protected:
    explicit ObjectOperation(QObject *parent = nullptr);

private:
    class Private;
    Private * const d;
};


class HEMERA_QT5_SDK_EXPORT VariantOperation : public Hemera::Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(VariantOperation)

    Q_PROPERTY(QVariant result READ result)

public:
    virtual ~VariantOperation();
    virtual QVariant result() const = 0;

protected:
    explicit VariantOperation(QObject *parent = nullptr);

private:
    class Private;
    Private * const d;
};

class HEMERA_QT5_SDK_EXPORT VariantMapOperation : public Hemera::Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(VariantMapOperation)

    Q_PROPERTY(QVariantMap result READ result)

public:
    virtual ~VariantMapOperation();
    virtual QVariantMap result() const = 0;

protected:
    explicit VariantMapOperation(QObject *parent = nullptr);

private:
    class Private;
    Private * const d;
};

class HEMERA_QT5_SDK_EXPORT BoolOperation : public Hemera::Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(BoolOperation)

    Q_PROPERTY(bool result READ result)

public:
    virtual ~BoolOperation();
    virtual bool result() const = 0;

protected:
    BoolOperation(QObject *parent = nullptr);

private:
    class Private;
    Private * const d;
};

class HEMERA_QT5_SDK_EXPORT UIntOperation : public Hemera::Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(UIntOperation)

    Q_PROPERTY(uint result READ result)

public:
    virtual ~UIntOperation();
    virtual uint result() const = 0;

protected:
    UIntOperation(QObject *parent = nullptr);

private:
    class Private;
    Private * const d;
};

class HEMERA_QT5_SDK_EXPORT ByteArrayOperation : public Hemera::Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(ByteArrayOperation)

    Q_PROPERTY(QByteArray result READ result)

public:
    virtual ~ByteArrayOperation();
    virtual QByteArray result() const = 0;

protected:
    ByteArrayOperation(QObject *parent = nullptr);

private:
    class Private;
    Private * const d;
};

class HEMERA_QT5_SDK_EXPORT SSLCertificateOperation : public Hemera::Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(SSLCertificateOperation)

    Q_PROPERTY(QByteArray privateKey READ privateKey)
    Q_PROPERTY(QByteArray certificate READ certificate)

public:
    virtual ~SSLCertificateOperation();
    virtual QByteArray privateKey() const = 0;
    virtual QByteArray certificate() const = 0;

protected:
    SSLCertificateOperation(QObject *parent = nullptr);

private:
    class Private;
    Private * const d;
};

class HEMERA_QT5_SDK_EXPORT StringOperation : public Hemera::Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(StringOperation)

    Q_PROPERTY(QString result READ result)

public:
    virtual ~StringOperation();
    virtual QString result() const = 0;

protected:
    StringOperation(QObject *parent = nullptr);

private:
    class Private;
    Private * const d;
};

class HEMERA_QT5_SDK_EXPORT StringListOperation : public Hemera::Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(StringListOperation)

    Q_PROPERTY(QStringList result READ result)

public:
    virtual ~StringListOperation();
    virtual QStringList result() const = 0;

protected:
    StringListOperation(QObject *parent = nullptr);

private:
    class Private;
    Private * const d;
};

class HEMERA_QT5_SDK_EXPORT JsonOperation : public Hemera::Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(JsonOperation)

    Q_PROPERTY(QJsonDocument result READ result)

public:
    virtual ~JsonOperation();
    virtual QJsonDocument result() const = 0;

protected:
    JsonOperation(QObject *parent = nullptr);

private:
    class Private;
    Private * const d;
};

class HEMERA_QT5_SDK_EXPORT UrlOperation : public Hemera::Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(UrlOperation)

    Q_PROPERTY(QUrl result READ result)

public:
    virtual ~UrlOperation();
    virtual QUrl result() const = 0;

protected:
    UrlOperation(QObject *parent = nullptr);

private:
    class Private;
    Private * const d;
};

}

#endif // HEMERA_COMMONOPERATIONS_H
