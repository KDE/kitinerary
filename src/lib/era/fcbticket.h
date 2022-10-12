/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_FCBTICKET_H
#define KITINERARY_FCBTICKET_H

#include <KItinerary/Uic9183Block>

#include <QList>

namespace KItinerary {

class UPERDecoder;

/** ERA Flexible Content Barcode (FCB)
 *  @see ERA TAP TSI TD B.12 §11 and the corresponding ASN.1 definition
 */
namespace Fcb {

class IssuingData {
    Q_GADGET
    Q_PROPERTY(int securityProviderNum MEMBER securityProviderNum)
    Q_PROPERTY(QByteArray securityProviderIA5 MEMBER securityProviderIA5)
    Q_PROPERTY(int issuerNum MEMBER issuerNum)
    Q_PROPERTY(QByteArray issuerIA5 MEMBER issuerIA5)
    Q_PROPERTY(int issuingYear MEMBER issuingYear)
    Q_PROPERTY(int issuingDay MEMBER issuingDay)
    Q_PROPERTY(int issuingTime MEMBER issuingTime)
    Q_PROPERTY(QString issuerName MEMBER issuerName)
    Q_PROPERTY(bool specimen MEMBER specimen)
    Q_PROPERTY(bool securePaperTicket MEMBER securePaperTicket)
    Q_PROPERTY(bool activated MEMBER activated)
    Q_PROPERTY(QByteArray currency MEMBER currency)
    Q_PROPERTY(int currencyFract MEMBER currencyFract)
    Q_PROPERTY(QByteArray issuerPNR MEMBER issuerPNR)
    // TODO extension data
    Q_PROPERTY(int issuedOnTrainNum MEMBER issuedOnTrainNum)
    Q_PROPERTY(QByteArray issuedOnTrainIA5 MEMBER issuedOnTrainIA5)
    Q_PROPERTY(int issuedOnLine MEMBER issuedOnLine)
    // TODO pointOfSale
public:
    void decode(UPERDecoder &decoder);

    int securityProviderNum; // INTEGER (1..32000) OPTIONAL
    QByteArray securityProviderIA5; // IA5String OPTIONAL
    int issuerNum; // INTEGER (1..32000) OPTIONAL
    QByteArray issuerIA5; // IA5String OPTIONAL
    int issuingYear; // INTEGER (2016..2269)
    int issuingDay; // INTEGER (1..366)
    int issuingTime; // INTEGER (0..1440) OPTIONAL
    QString issuerName; // UTF8String OPTIONAL
    bool specimen;
    bool securePaperTicket;
    bool activated;
    QByteArray currency = QByteArray("EUR"); // IA5String (SIZE(3)) DEFAULT "EUR"
    int currencyFract = 2; // INTEGER (1..3) DEFAULT 2
    QByteArray issuerPNR; //IA5String OPTIONAL
    // TODO extension ExtensionData OPTIONAL,
    int issuedOnTrainNum; // OPTIONAL
    QByteArray issuedOnTrainIA5; // OPTIONAL
    int issuedOnLine; // OPTIONAL
    // TODO pointOfSale GeoCoordinateType OPTIONAL
};

class TravelerType {
    Q_GADGET
    Q_PROPERTY(QString firstName MEMBER firstName)
    Q_PROPERTY(QString secondName MEMBER secondName)
    Q_PROPERTY(QString lastName MEMBER lastName)
    Q_PROPERTY(QByteArray idCard MEMBER idCard)
    Q_PROPERTY(QByteArray passportId MEMBER passportId)
    Q_PROPERTY(QByteArray title MEMBER title)
    // TODO gender
    Q_PROPERTY(QByteArray customerIdIA5 MEMBER customerIdIA5)
    Q_PROPERTY(int customerIdNum MEMBER customerIdNum)
    Q_PROPERTY(int yearOfBirth MEMBER yearOfBirth)
    Q_PROPERTY(int dayOfBirth MEMBER dayOfBirth)
    Q_PROPERTY(bool ticketHolder MEMBER ticketHolder)
    // TODO passengerType
    // TODO passengerWithReducedMobility
    Q_PROPERTY(int countryOfResidence MEMBER countryOfResidence)
    Q_PROPERTY(int countryOfPassport MEMBER countryOfPassport)
    Q_PROPERTY(int countryOfIdCard MEMBER countryOfIdCard)
    // TODO status
public:
    void decode(UPERDecoder &decoder);

    QString firstName; // UTF8String OPTIONAL
    QString secondName; // UTF8String OPTIONAL
    QString lastName; // UTF8String OPTIONAL
    QByteArray idCard; // IA5String OPTIONAL
    QByteArray passportId; // IA5String OPTIONAL
    QByteArray title; // IA5String (SIZE(1..3)) OPTIONAL
    // TODO gender GenderType OPTIONAL
    QByteArray customerIdIA5; // IA5String OPTIONAL
    int customerIdNum; // INTEGER OPTIONAL
    int yearOfBirth; // INTEGER (1901..2155) OPTIONAL
    int dayOfBirth; // INTEGER (0..370) OPTIONAL
    bool ticketHolder;
    // TODO passengerType PassengerType OPTIONAL
    // TODO passengerWithReducedMobility BOOLEAN OPTIONAL
    int countryOfResidence; // INTEGER (1..999) OPTIONAL
    int countryOfPassport; // INTEGER (1..999) OPTIONAL
    int countryOfIdCard; // INTEGER (1..999) OPTIONAL
    // TODO status SEQUENCE OF CustomerStatusType OPTIONAL
};

class TravelerData {
    Q_GADGET
    Q_PROPERTY(QList<KItinerary::Fcb::TravelerType> traveler MEMBER traveler CONSTANT)
    Q_PROPERTY(QByteArray preferredLanguage MEMBER preferredLanguage)
    Q_PROPERTY(QString groupName MEMBER groupName)
public:
    void decode(UPERDecoder &decoder);

    QList<TravelerType> traveler; // OPTIONAL
    QByteArray preferredLanguage; // IA5String (SIZE(2)) OPTIONAL
    QString groupName; // UTF8String OPTIONAL
};

class KITINERARY_EXPORT UicRailTicketData {
    Q_GADGET
    Q_PROPERTY(KItinerary::Fcb::IssuingData issuingDetail MEMBER issuingDetail CONSTANT)
    Q_PROPERTY(KItinerary::Fcb::TravelerData travelerDetail MEMBER travelerDetail CONSTANT)
public:
    UicRailTicketData();
    UicRailTicketData(const Uic9183Block &block);

    void decode(UPERDecoder &decoder);

    IssuingData issuingDetail;
    TravelerData travelerDetail; // optional
    // TODO
    // transportDocument SEQUENCE OF DocumentData OPTIONAL
    // controlDetail ControlData OPTIONAL
    // extension SEQUENCE OF ExtensionData OPTIONAL

    static constexpr const char RecordId[] = "U_FLEX";

private:
    Uic9183Block m_block;
};

}
}

Q_DECLARE_METATYPE(KItinerary::Fcb::IssuingData)
Q_DECLARE_METATYPE(KItinerary::Fcb::TravelerType)
Q_DECLARE_METATYPE(KItinerary::Fcb::TravelerData)
Q_DECLARE_METATYPE(KItinerary::Fcb::UicRailTicketData)

#endif // KITINERARY_FCBTICKET_H
