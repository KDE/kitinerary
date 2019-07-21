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

class QString;
class QVariant;

namespace KItinerary {

class FilePrivate;

/** A file containing a bundle of reservations and associated documents.
 *  This is used to export or transfer a set of reservation-related documents
 *  while keeping the associations between them.
 */
class KITINERARY_EXPORT File
{
public:
    explicit File();
    /** Create a File instance for the file named @p fileName. */
    explicit File(const QString &fileName);
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
    /** Save and close the file. Automatically called from the dtor. */
    void close();

    /** Lists the identifiers of all reservations in this file. */
    QVector<QString> reservations() const;
    /** Loads the reservation with the given identifier. */
    QVariant reservation(const QString &resId) const;
    /** Add a reservation to this file. */
    void addReservation(const QVariant &res);

    /** Returns the pass identifier used in here for @p pass. */
    static QString passId(const KPkPass::Pass *pass);
    static QString passId(const QString &passTypeIdenfier, const QString &serialNumber);

    /** Lists all pkpass files in this file. */
    QVector<QString> passes() const;
    /** Pass data for the given pass id. */
    QByteArray passData(const QString &passId) const;
    /** Add a pkpass file to this file. */
    void addPass(KPkPass::Pass *pass, const QByteArray &rawData);

    // TODO documents

private:
    std::unique_ptr<FilePrivate> d;
};

}

#endif // KITINERARY_FILE_H
