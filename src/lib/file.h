/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include <QList>

#include <memory>

namespace KPkPass {
class Pass;
}

class QIODevice;
class QString;
class QVariant;

namespace KItinerary {

class FilePrivate;

/** A file containing a bundle of reservations and associated documents.
 *  This is used to export or transfer a set of reservation-related documents
 *  while keeping the associations between them.
 *
 *  A KItinerary::File can contain the following elements:
 *  - JSON-LD reservation objects (see KItinerary::Reservation). Each reservation has a UUID.
 *  - PkPass files. Their identifier is determined by their pass type identifier and their serial number.
 *  - JSON-LD document objects (see KItinerary::CreativeWork) and their associated file content. Each document has a UUID.
 *  - Application-specific data in custom namespaces.
 */
class KITINERARY_EXPORT File
{
public:
    explicit File();
    /** Create a File instance for the file named @p fileName. */
    explicit File(const QString &fileName);
    /** Create a File instance for the given i/o device. */
    explicit File(QIODevice *device);
    File(const File&) = delete;
    File(File&&) noexcept;
    ~File();
    File& operator=(const File&) = delete;
    File& operator=(File&&) noexcept;

    /** Sets the file name. Needs to be done before calling open(). */
    void setFileName(const QString &fileName);

    enum OpenMode { Read, Write };
    /** Open the file for reading or writing. A filename needs to be set before calling this.
     *  All read/write operations require the file to be open as a precondition.
     */
    [[nodiscard]] bool open(OpenMode mode) const;
    /** Error message in case opening the file failed. */
    [[nodiscard]] QString errorString() const;
    /** Save and close the file. Automatically called from the dtor. */
    void close();

    /** Lists the identifiers of all reservations in this file. */
    [[nodiscard]] QList<QString> reservations() const;
    /** Loads the reservation with the given identifier. */
    [[nodiscard]] QVariant reservation(const QString &resId) const;
    /** Add a reservation to this file. A new unique identifier will be generated for the reservation. */
    void addReservation(const QVariant &res);
    /** Add a reservation to this file. The given identifier will be used. */
    void addReservation(const QString &id, const QVariant &res);

    /** Returns the pass identifier used in here for @p pass. */
    [[nodiscard]] static QString passId(const KPkPass::Pass *pass);
    [[nodiscard]] static QString passId(const QString &passTypeIdenfier, const QString &serialNumber);
    /** Decodes an identifier returned by passId() again. */
    struct PkPassIdentifier {
        QString passTypeIdenfier;
        QString serialNumber;
    };
    [[nodiscard]] static PkPassIdentifier decodePassId(QStringView);

    /** Lists all pkpass files in this file. */
    [[nodiscard]] QList<QString> passes() const;
    /** Pass data for the given pass id. */
    [[nodiscard]] QByteArray passData(const QString &passId) const;
    /** Add a pkpass file to this file. */
    void addPass(KPkPass::Pass *pass, const QByteArray &rawData);
    /** Add a pkpass file with the given pass id. */
    void addPass(const QString &passId, const QByteArray &rawData);

    /** Makes sure the resulting file name is something that can safely be used without messing up the
     *  file system or archive structure.
     */
    [[nodiscard]] static QString normalizeDocumentFileName(const QString &name);

    /** Lists all document identifiers. */
    [[nodiscard]] QList<QString> documents() const;
    /** Loads the document meta data of document @p id. */
    [[nodiscard]] QVariant documentInfo(const QString &id) const;
    /** Loads the content of document @p id. */
    [[nodiscard]] QByteArray documentData(const QString &id) const;
    /** Adds a document and associated meta data to the file. */
    void addDocument(const QString &id, const QVariant &docInfo, const QByteArray &docData);

    /** List custom data in the given namespace. */
    [[nodiscard]] QList<QString> listCustomData(QStringView scope) const;
    /** Returns @c true if custom data with the given id exists in @p scope. */
    [[nodiscard]] bool hasCustomData(QStringView scope, const QString &id) const;
    /** Returns the custom data in the given namespace and with the given id. */
    [[nodiscard]] QByteArray customData(QStringView scope, const QString &id) const;
    /** Adds a custom data element with identifier @p id in to namespace @p scope. */
    void addCustomData(QStringView scope, const QString &id, const QByteArray &data);

private:
    std::unique_ptr<FilePrivate> d;
};

}

