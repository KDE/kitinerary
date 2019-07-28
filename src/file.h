/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef KITINERARY_FILE_H
#define KITINERARY_FILE_H

#include "kitinerary_export.h"

#include <QVector>

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
 *  - PkPass files. Their idenfifier is determined by their pass type idenfifier and their serial number.
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
    File(File&&);
    ~File();
    File& operator=(const File&) = delete;
    File& operator=(File&&);

    /** Sets the file name. Needs to be done before calling open(). */
    void setFileName(const QString &fileName);

    enum OpenMode { Read, Write };
    /** Open the file for reading or writing. A filename needs to be set before calling this.
     *  All read/write operations require the file to be open as a precondition.
     */
    bool open(OpenMode mode) const;
    /** Error message in case opening the file failed. */
    QString errorString() const;
    /** Save and close the file. Automatically called from the dtor. */
    void close();

    /** Lists the identifiers of all reservations in this file. */
    QVector<QString> reservations() const;
    /** Loads the reservation with the given identifier. */
    QVariant reservation(const QString &resId) const;
    /** Add a reservation to this file. A new unique idenfifier will be generated for the reservation. */
    void addReservation(const QVariant &res);
    /** Add a reservation to this file. The given idenfifier will be used. */
    void addReservation(const QString &id, const QVariant &res);

    /** Returns the pass identifier used in here for @p pass. */
    static QString passId(const KPkPass::Pass *pass);
    static QString passId(const QString &passTypeIdenfier, const QString &serialNumber);

    /** Lists all pkpass files in this file. */
    QVector<QString> passes() const;
    /** Pass data for the given pass id. */
    QByteArray passData(const QString &passId) const;
    /** Add a pkpass file to this file. */
    void addPass(KPkPass::Pass *pass, const QByteArray &rawData);
    /** Add a pkpass file with the given pass id. */
    void addPass(const QString &passId, const QByteArray &rawData);

    /** Makes sure the resulting file name is something that can safely be used without messing up the
     *  file system or archive structure.
     */
    static QString normalizeDocumentFileName(const QString &name);

    /** Lists all document identifiers. */
    QVector<QString> documents() const;
    /** Loads the document meta data of document @p id. */
    QVariant documentInfo(const QString &id) const;
    /** Loads the content of document @p id. */
    QByteArray documentData(const QString &id) const;
    /** Adds a document and associated meta data to the file. */
    void addDocument(const QString &id, const QVariant &docInfo, const QByteArray &docData);

    /** List custom data in the given namespace. */
    QVector<QString> listCustomData(const QString &scope) const;
    /** Returns the custom data in the given namespace and with the given id. */
    QByteArray customData(const QString &scope, const QString &id) const;
    /** Adds a custom data element with identifier @p id in to namespace @p scope. */
    void addCustomData(const QString &scope, const QString &id, const QByteArray &data);

private:
    std::unique_ptr<FilePrivate> d;
};

}

#endif // KITINERARY_FILE_H
