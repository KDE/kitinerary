/*
   SPDX-FileCopyrightText: 2022 Luca Weiss <luca@z3ntu.xyz>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(pdf) {
    let reservations = [];
    for (var i = 0; i < pdf.pageCount; ++i) {
        var page = pdf.pages[i];
        var nextBarcode = null;
        var images = page.images;
        for (var j = 0; j < images.length && !nextBarcode; ++j) {
            nextBarcode = Barcode.decodeQR(images[j]);
            if (nextBarcode)
                reservations.push(decodeBarcode(page.text, nextBarcode));
        }
    }
    return reservations
}

function decodeBarcode(text, barcode) {
    var res = JsonLd.newBusReservation();

    // Time and date is only in the PDF text
    const times = text.match(/(\d{2}\.\d{2}\.\d{4} \d{2}:\d{2})/g);
    if (times.length !== 2) {
        console.log("Failed to extract departure/arrival time from text: " + times);
        return null;
    }

    res.reservationFor.departureTime = JsonLd.toDateTime(times[0], "dd.MM.yyyy hh:mm", "en");
    res.reservationFor.arrivalTime = JsonLd.toDateTime(times[1], "dd.MM.yyyy hh:mm", "en");

    // The rest of the info can be found in the QR code, separated by |
    const parts = barcode.split("|");
    if (parts.length != 17) {
        console.log("Failed to extract info from barcode, got " + parts.length + " parts.");
        return null;
    }

    res.reservationId = parts[0];
    res.reservationFor.busNumber = parts[2];

    const departureStop = stops[parts[3]];
    res.reservationFor.departureBusStop.identifier = parts[3];
    res.reservationFor.departureBusStop.name = departureStop.name;
    res.reservationFor.departureBusStop.geo.latitude = departureStop.coords[0];
    res.reservationFor.departureBusStop.geo.longitude = departureStop.coords[1];

    const arrivalStop = stops[parts[4]];
    res.reservationFor.arrivalBusStop.identifier = parts[4];
    res.reservationFor.arrivalBusStop.name = arrivalStop.name;
    res.reservationFor.arrivalBusStop.geo.latitude = arrivalStop.coords[0];
    res.reservationFor.arrivalBusStop.geo.longitude = arrivalStop.coords[1];

    res.reservedTicket.ticketedSeat.seatNumber = parts[8];

    res.totalPrice = parts[10];
    res.underName.familyName = parts[14];
    res.underName.givenName = parts[15];
    res.underName.birthDate = parts[16];

    res.reservedTicket.ticketToken = "qrCode:" + barcode;

    return res;
}

// Converted from GTFS data: https://gist.github.com/z3ntu/605c459f1897f0e6ce31325df7bac3d7
// Last updated: 2024-01-15
var stops = {
    "AAM": {name: "Amarante", coords: [41.26606, -8.07221]},
    "ABD": {name: "Breda", coords: [51.59649, 4.7783]},
    "ABR": {name: "Braga", coords: [41.55613, -8.42517]},
    "ABS": {name: "Valencia - Bus Station", coords: [39.48053, -0.3874]},
    "ABT": {name: "Alicante", coords: [38.3375, -0.49149]},
    "ABU": {name: "Burgos", coords: [42.33825, -3.70123]},
    "ABW": {name: "Benidorm - Bus Station", coords: [38.55028, -0.1253]},
    "ABY": {name: "Barcelona - Airport Prat T1", coords: [41.28768, 2.0729]},
    "ABZ": {name: "Castellon", coords: [39.98893, -0.05142]},
    "ACD": {name: "Lisbon - Airport", coords: [38.76861, -9.12833]},
    "ACO": {name: "Coimbra", coords: [40.21628, -8.4368]},
    "ACW": {name: "Madrid - Barajas Airport", coords: [40.49233, -3.59385]},
    "ACX": {name: "Zaragoza", coords: [41.6595, -0.91134]},
    "ADC": {name: "Voiron - Champfeuillet", coords: [45.34994, 5.56775]},
    "ADU": {name: "Düsseldorf", coords: [51.22288, 6.79555]},
    "AFR": {name: "Frankfurt - Main bus station", coords: [50.10448, 8.66254]},
    "AFT": {name: "Fatima", coords: [39.63137, -8.68028]},
    "AGN": {name: "Agen", coords: [44.16675, 0.60577]},
    "AKA": {name: "Karlsruhe", coords: [48.99157, 8.40034]},
    "ALC": {name: "A Coruña", coords: [43.35325, -8.4053]},
    "ALE": {name: "Lisbon - Oriente", coords: [38.76784, -9.09936]},
    "AMS": {name: "Amsterdam - Schiphol Airport", coords: [52.30855, 4.76144]},
    "ANB": {name: "Antibes", coords: [43.59961, 7.08636]},
    "ANG": {name: "Angoulême", coords: [45.65343, 0.16559]},
    "AOS": {name: "Aosta", coords: [45.73524, 7.32441]},
    "AOV": {name: "Oviedo", coords: [43.36917, -5.85088]},
    "APL": {name: "Ponte de Lima", coords: [41.75994, -8.57991]},
    "AQU": {name: "L'Aquila", coords: [42.36044, 13.37731]},
    "ARK": {name: "Milan Bergamo - Airport", coords: [45.66575, 9.69725]},
    "ARN": {name: "Nantes - Airport", coords: [47.15788, -1.60068]},
    "ASA": {name: "Santiago de Compostela", coords: [42.86991, -8.54411]},
    "AST": {name: "Santander", coords: [43.45951, -3.80968]},
    "ATT": {name: "Stuttgart - Airport", coords: [48.6918, 9.19669]},
    "AUF": {name: "Auxerre", coords: [47.79662, 3.58448]},
    "AVA": {name: "Valladolid", coords: [41.64114, -4.73208]},
    "AVG": {name: "Vigo", coords: [42.23462, -8.7137]},
    "AVR": {name: "Vila Real", coords: [41.29974, -7.74987]},
    "BDL": {name: "Bandol", coords: [43.13934, 5.76632]},
    "BEP": {name: "Bellegarde - Vouvray", coords: [46.11532, 5.79464]},
    "BGM": {name: "Bergamo", coords: [45.69161, 9.6762]},
    "BIJ": {name: "Biarritz - Airport", coords: [43.47201, -1.53241]},
    "BLB": {name: "Bilbao", coords: [43.26131, -2.94995]},
    "BLF": {name: "Belfort", coords: [47.62856, 6.8569]},
    "BOL": {name: "Bologna", coords: [44.50393, 11.34688]},
    "BRC": {name: "Briançon", coords: [44.8893, 6.6329]},
    "BRG": {name: "Bruges", coords: [51.19547, 3.21647]},
    "BSA": {name: "Brescia", coords: [45.53297, 10.2151]},
    "CAB": {name: "Cabourg", coords: [49.29431, -0.10655]},
    "CAG": {name: "Calais - Port", coords: [50.97315, 1.87803]},
    "CAR": {name: "Carcassonne", coords: [43.21254, 2.34552]},
    "CAT": {name: "Castets", coords: [43.87712, -1.14344]},
    "CBH": {name: "Capbreton - Hossegor", coords: [43.64113, -1.42571]},
    "CDG": {name: "Paris - Roissy Charles De Gaulle Airport", coords: [49.01089, 2.55893]},
    "CHP": {name: "Champigny-sur-Marne", coords: [48.81269, 2.53248]},
    "CLS": {name: "Cluses", coords: [46.06148, 6.58234]},
    "CLU": {name: "Cluses - La Maladière", coords: [46.0493, 6.5951]},
    "CMX": {name: "Chamonix Sud - Bus station", coords: [45.9173, 6.8668]},
    "COL": {name: "Colmar", coords: [48.07375, 7.34779]},
    "CRM": {name: "Courmayeur", coords: [45.79133, 6.97046]},
    "CRO": {name: "Crolles - Le Rafour", coords: [45.26961, 5.89323]},
    "CRX": {name: "Châteauroux", coords: [46.84914, 1.70904]},
    "CSM": {name: "Cavalaire-sur-Mer", coords: [43.1723, 6.52948]},
    "CVM": {name: "Charleville-Mézières", coords: [49.76881, 4.72488]},
    "DLP": {name: "Marne-la-Vallée - Chessy", coords: [48.86573, 2.78304]},
    "DNR": {name: "Saint-Malo", coords: [48.65058, -2.02168]},
    "DOL": {name: "Deauville", coords: [49.35911, 0.08406]},
    "EAP": {name: "Basel Mulhouse Freiburg - Airport", coords: [47.60074, 7.53178]},
    "EAS": {name: "San Sebastián", coords: [43.31713, -1.97738]},
    "EBU": {name: "Saint-Etienne", coords: [45.44241, 4.40297]},
    "EPL": {name: "Epinal", coords: [48.17775, 6.44205]},
    "ESS": {name: "Essen", coords: [51.45025, 7.01491]},
    "FEC": {name: "Fécamp", coords: [49.75923, 0.37519]},
    "FFA": {name: "Frankfurt - Airport", coords: [50.05276, 8.57753]},
    "FRZ": {name: "Florence", coords: [43.7552, 11.17221]},
    "GAP": {name: "Gap", coords: [44.56341, 6.08519]},
    "GFR": {name: "Mont-Saint-Michel", coords: [48.61209, -1.50918]},
    "GIA": {name: "Girona", coords: [41.97896, 2.81731]},
    "GOA": {name: "Genoa - Via Fanti d'Italia", coords: [44.41635, 8.91905]},
    "GVA": {name: "Geneva - Airport", coords: [46.23012, 6.10928]},
    "HEI": {name: "Heidelberg", coords: [49.40612, 8.67312]},
    "HYP": {name: "Hyères - Place Louis Versin", coords: [43.11702, 6.13415]},
    "LBY": {name: "La Baule", coords: [47.28838, -2.39165]},
    "LDE": {name: "Tarbes", coords: [43.22334, 0.04721]},
    "LHY": {name: "The Hague", coords: [52.07986, 4.32571]},
    "LIG": {name: "Liège", coords: [50.62407, 5.56857]},
    "LJU": {name: "Ljubljana", coords: [46.05774, 14.50965]},
    "LON": {name: "Lorient Lanester", coords: [47.77787, -3.34215]},
    "LSN": {name: "Lausanne", coords: [46.53611, 6.62374]},
    "LSO": {name: "Les Sables-d'Olonne", coords: [46.5009, -1.78186]},
    "LUX": {name: "Luxembourg", coords: [49.59998, 6.10525]},
    "LVD": {name: "Le Lavandou", coords: [43.13799, 6.36434]},
    "LYS": {name: "Lyon - Saint-Exupéry Airport", coords: [45.71909, 5.07953]},
    "MAA": {name: "Marseille Provence Airport", coords: [43.44357, 5.21993]},
    "MCA": {name: "Murcia", coords: [37.98508, -1.13947]},
    "MDS": {name: "Madrid - South Station", coords: [40.39426, -3.67772]},
    "MGE": {name: "Montdauphin - Guillestre", coords: [44.67468, 6.6159]},
    "MHM": {name: "Mannheim", coords: [49.47827, 8.47282]},
    "MIR": {name: "Mirandela", coords: [41.48126, -7.18084]},
    "MSY": {name: "Paris South - Massy-Palaiseau", coords: [48.72575, 2.25682]},
    "MUZ": {name: "Munich - Bus station", coords: [48.14245, 11.55006]},
    "NAQ": {name: "Nancy - Centre", coords: [48.69481, 6.19049]},
    "NCE": {name: "Nice - Airport T1", coords: [43.66492, 7.20984]},
    "NCY": {name: "Annecy", coords: [45.90167, 6.12129]},
    "NRB": {name: "Narbonne - Croix Sud", coords: [43.16525, 2.9884]},
    "NUR": {name: "Nuremberg", coords: [49.4478, 11.08598]},
    "ORY": {name: "Paris - Orly Airport", coords: [48.73063, 2.36406]},
    "PAT": {name: "Porto - Airport", coords: [41.23542, -8.66963]},
    "PDG": {name: "Port Grimaud", coords: [43.27537, 6.57892]},
    "PDL": {name: "Paris - Pont de Levallois", coords: [48.89774, 2.28125]},
    "PET": {name: "Pornichet", coords: [47.26083, -2.33536]},
    "PGF": {name: "Perpignan", coords: [42.69493, 2.87924]},
    "PGX": {name: "Périgueux", coords: [45.14311, 0.69851]},
    "PIL": {name: "Pilsen", coords: [49.74633, 13.36297]},
    "POR": {name: "Porto - TIC Campanhã", coords: [41.15186, -8.58244]},
    "PRF": {name: "Prague", coords: [50.08936, 14.44061]},
    "PUF": {name: "Pau - Technopôle Héliparc", coords: [43.319, -0.36215]},
    "QJZ": {name: "Nantes", coords: [47.24878, -1.52089]},
    "QKU": {name: "Cologne - Airport", coords: [50.88144, 7.11708]},
    "QRH": {name: "Rotterdam", coords: [51.92353, 4.46653]},
    "QUE": {name: "Quimper Est (Le Rouillen)", coords: [47.99919, -4.05204]},
    "QXB": {name: "Aix-en-Provence", coords: [43.511, 5.44747]},
    "QXG": {name: "Angers", coords: [47.46476, -0.55773]},
    "RCF": {name: "Rochefort", coords: [45.96508, -0.95944]},
    "ROM": {name: "Rome - Tiburtina", coords: [41.90952, 12.52827]},
    "ROZ": {name: "Rouen - Zénith Parc Expo", coords: [49.39207, 1.05879]},
    "RSY": {name: "La Roche-sur-Yon", coords: [46.67312, -1.43525]},
    "RYN": {name: "Royan", coords: [45.6261, -1.01637]},
    "SAR": {name: "Saarbrücken", coords: [49.24178, 7.00013]},
    "SCQ": {name: "Santiago de Compostela Airport", coords: [42.89281, -8.42171]},
    "SDN": {name: "Sedan", coords: [49.69523, 4.93071]},
    "SEL": {name: "Basel", coords: [47.5461, 7.58904]},
    "SLG": {name: "Sallanches", coords: [45.9355, 6.63627]},
    "SMX": {name: "Sainte-Maxime", coords: [43.31521, 6.63322]},
    "SRO": {name: "Sanremo", coords: [43.81423, 7.7718]},
    "STE": {name: "Saintes", coords: [45.75557, -0.65213]},
    "STG": {name: "Saint-Gaudens", coords: [43.10508, 0.72897]},
    "TER": {name: "Teramo - Piazza San Francesco", coords: [42.66047, 13.70695]},
    "TGR": {name: "Torino - Bus station", coords: [45.07017, 7.65781]},
    "TLN": {name: "Toulon", coords: [43.12779, 5.93075]},
    "TPG": {name: "Teramo - Piazza Garibaldi", coords: [42.66158, 13.69835]},
    "TRC": {name: "Tourcoing", coords: [50.72139, 3.16305]},
    "TRR": {name: "Tarragona", coords: [41.11824, 1.24422]},
    "TST": {name: "Trieste", coords: [45.65703, 13.77093]},
    "TVC": {name: "Tignes - Val Claret", coords: [45.45696, 6.90176]},
    "ULM": {name: "Ulm", coords: [48.42586, 10.01041]},
    "URT": {name: "Utrecht", coords: [52.0901, 5.10517]},
    "VAA": {name: "Valença", coords: [42.0248, -8.6398]},
    "VNM": {name: "Venice - Mestre", coords: [45.48241, 12.23359]},
    "VNT": {name: "Venice - Tronchetto", coords: [45.44148, 12.30497]},
    "VRN": {name: "Verona", coords: [45.43146, 10.98106]},
    "VTR": {name: "Vitoria", coords: [42.85911, -2.68433]},
    "XAI": {name: "Aime - La Plagne", coords: [45.55448, 6.64747]},
    "XAM": {name: "Amsterdam City Center - Sloterdijk", coords: [52.38972, 4.83844]},
    "XAN": {name: "Antwerp", coords: [51.21925, 4.41715]},
    "XAS": {name: "Amiens", coords: [49.89709, 2.31149]},
    "XAV": {name: "Avranches", coords: [48.69015, -1.36967]},
    "XAX": {name: "Parc Astérix", coords: [49.13689, 2.5704]},
    "XBA": {name: "Bayonne", coords: [43.4976, -1.47902]},
    "XBB": {name: "Bourg-en-Bresse", coords: [46.19934, 5.21535]},
    "XBC": {name: "Barcelona Nord - Bus Station", coords: [41.39496, 2.18327]},
    "XBE": {name: "Brive-la-Gaillarde", coords: [45.14591, 1.48273]},
    "XBL": {name: "Blois", coords: [47.61411, 1.32389]},
    "XBM": {name: "Bourg-Saint-Maurice", coords: [45.61957, 6.77172]},
    "XBN": {name: "Besançon", coords: [47.2215, 5.97861]},
    "XBP": {name: "Bordeaux Pessac", coords: [44.80436, -0.63259]},
    "XBS": {name: "Bourges", coords: [47.06382, 2.36815]},
    "XBT": {name: "Brest - Bus station", coords: [48.38749, -4.48214]},
    "XCA": {name: "Caen", coords: [49.17643, -0.34764]},
    "XCD": {name: "Chalon-sur-Saône", coords: [46.78205, 4.84426]},
    "XCF": {name: "Clermont-Ferrand", coords: [45.77078, 3.0823]},
    "XCN": {name: "Cannes", coords: [43.57089, 7.01458]},
    "XCX": {name: "Carhaix-Plouguer - festival", coords: [48.2783, -3.56231]},
    "XCY": {name: "Chambéry", coords: [45.56995, 5.91727]},
    "XDB": {name: "Lille", coords: [50.63863, 3.07649]},
    "XDE": {name: "Dunkirk", coords: [51.03171, 2.36846]},
    "XDI": {name: "Dijon", coords: [47.32405, 5.02779]},
    "XDP": {name: "Dieppe", coords: [49.9221, 1.08067]},
    "XDX": {name: "Dax", coords: [43.72026, -1.04965]},
    "XER": {name: "Strasbourg", coords: [48.57424, 7.75426]},
    "XET": {name: "Étretat", coords: [49.70505, 0.20853]},
    "XFJ": {name: "Fréjus - Saint-Raphaël", coords: [43.43558, 6.73739]},
    "XFY": {name: "Saint-Gervais-les-Bains - Le Fayet", coords: [45.90612, 6.70098]},
    "XGB": {name: "Grenoble - Oxford", coords: [45.20456, 5.70107]},
    "XGC": {name: "Geneva - Bus station", coords: [46.20838, 6.14674]},
    "XGE": {name: "Grenoble - Bus Station", coords: [45.19283, 5.71429]},
    "XGP": {name: "Guingamp", coords: [48.55584, -3.14254]},
    "XHD": {name: "Hendaye", coords: [43.3511, -1.78328]},
    "XHF": {name: "Honfleur", coords: [49.41921, 0.23715]},
    "XIZ": {name: "Reims", coords: [49.21475, 3.99456]},
    "XLH": {name: "Le Havre", coords: [49.4921, 0.12534]},
    "XLM": {name: "Milan", coords: [45.48976, 9.12759]},
    "XLR": {name: "La Rochelle", coords: [46.15304, -1.14148]},
    "XLS": {name: "Limoges", coords: [45.83615, 1.26848]},
    "XLT": {name: "Lorient", coords: [47.7554, -3.36413]},
    "XLU": {name: "Lourdes", coords: [43.10018, -0.04155]},
    "XLV": {name: "Laval", coords: [48.07594, -0.76056]},
    "XLZ": {name: "Lannemezan", coords: [43.11434, 0.38781]},
    "XMF": {name: "Montbéliard", coords: [47.50894, 6.80195]},
    "XMK": {name: "Montélimar", coords: [44.55966, 4.74526]},
    "XMO": {name: "Moutiers", coords: [45.4868, 6.5294]},
    "XMS": {name: "Le Mans", coords: [48.01741, 0.14943]},
    "XMT": {name: "Montpellier", coords: [43.58423, 3.86]},
    "XMW": {name: "Montauban Sud", coords: [43.98177, 1.33166]},
    "XMX": {name: "Morlaix", coords: [48.57752, -3.83294]},
    "XMZ": {name: "Metz", coords: [49.11063, 6.18331]},
    "XNS": {name: "Nîmes", coords: [43.8173, 4.36177]},
    "XNT": {name: "Niort", coords: [46.30755, -0.48562]},
    "XOP": {name: "Poitiers", coords: [46.58444, 0.33602]},
    "XOS": {name: "Orléans", coords: [47.89613, 1.85401]},
    "XPB": {name: "Paris - Bercy Seine", coords: [48.83568, 2.38016]},
    "XPD": {name: "Paris - La Défense (Terminal Jules Verne)", coords: [48.89132, 2.24233]},
    "XQR": {name: "Quimper", coords: [47.99446, -4.09355]},
    "XRF": {name: "Marseille - St-Charles Bus station", coords: [43.30417, 5.37986]},
    "XRN": {name: "Rouen - Rive gauche", coords: [49.43409, 1.09247]},
    "XSB": {name: "Saint-Brieuc", coords: [48.50682, -2.76644]},
    "XSD": {name: "Paris Nord - Saint-Denis Université", coords: [48.94645, 2.36457]},
    "XSJ": {name: "Saint-Jean-de-Luz", coords: [43.38616, -1.66076]},
    "XSN": {name: "Saint-Nazaire", coords: [47.28553, -2.21268]},
    "XTO": {name: "Tours", coords: [47.38349, 0.70217]},
    "XTS": {name: "Toulouse", coords: [43.61329, 1.45222]},
    "XVG": {name: "Valence - Centre", coords: [44.92684, 4.89264]},
    "XVS": {name: "Vannes", coords: [47.66492, -2.75143]},
    "XYL": {name: "Lyon - Perrache Bus Station", coords: [45.74971, 4.82678]},
    "XZN": {name: "Avignon - Le Pontet", coords: [43.96062, 4.85593]},
    "XZR": {name: "Béziers", coords: [43.33657, 3.22058]},
    "ZAG": {name: "Zagreb", coords: [45.80341, 15.99227]},
    "ZDH": {name: "Mulhouse", coords: [47.74218, 7.34187]},
    "ZEP": {name: "London - Victoria Coach Station", coords: [51.49251, -0.14834]},
    "ZFJ": {name: "Rennes", coords: [48.1041, -1.6699]},
    "ZFQ": {name: "Bordeaux Saint-Jean - Terres de Borde", coords: [44.82303, -0.55452]},
    "ZRC": {name: "Zürich", coords: [47.38119, 8.53747]},
    "ZYR": {name: "Brussels City Center - Midi Train station", coords: [50.83496, 4.33306]},
};
